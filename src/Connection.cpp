#include <iostream>
#include "hslm_csio/Connection.h"
#include "hslm_csio/Server.h"

CConnection::CConnection(EOwner owner, asio::io_context& IoContext, asio::ip::tcp::socket Socket, TThreadSafeDeque<SMessageTo>& MessageToLocal, CServer* Server)
    : IoContext(IoContext)
    , WriteStrand(IoContext)
    , Socket(std::move(Socket))
    , MessageToLocal(MessageToLocal)
    , Server(Server)
{
    Owner = owner;
}

CConnection::~CConnection()
{
}

bool CConnection::ConnectToServer(const asio::ip::tcp::resolver::results_type& Endpoints)
{
    State = EState::Connecting;
    assert(Owner == EOwner::kClient && "Only client can connect to server");
    asio::async_connect(Socket, Endpoints,
        [this](std::error_code ErrorCode, asio::ip::tcp::endpoint Endpoint)
        {
            if (!ErrorCode)
            {
                std::cout << "[Client] Connected!\n";
                State = EState::Connected;
                SendMessageToRemote(SMessage(EMessageId::kConnectToServer));
                ReadHeader();
            }
            else
            {
                std::cout << "[Client] ConnectToServer Failed!\n";
                State = EState::ConnectFailed;
            }
        });
    return true;
}

bool CConnection::ConnectToClient(uint32_t Id)
{
    assert(Owner == EOwner::kServer);
    if (Socket.is_open())
    {
        ServerSharedRef.store(true);
        RemoteEndpoint = Socket.remote_endpoint();
        this->Id = Id;
        State = EState::Connected;
        SendMessageToRemote(SMessage(EMessageId::kConnectToClient));
        ReadHeader();
        return true;
    }
    return false;
}

void CConnection::Disconnect()
{
    auto Self(this->shared_from_this());
    asio::post(IoContext, [this, Self]() {
            if (IsConnected())
            {
                Socket.shutdown(asio::ip::tcp::socket::shutdown_both);
                Socket.close();
                State = EState::Disconnect;
            }
        });
}

void CConnection::SendMessageToRemote(const SMessage& Msg)
{
    auto Self(this->shared_from_this());
    WriteStrand.post(
        [this, Self, Msg = Msg] {
            MessageToRemote.emplace_back(std::move(Msg));
            if (MessageToRemote.size() <= 1)
            {
                WriteHeader();
            }
        });
}

void CConnection::SendMessageToRemote(SMessage&& Msg)
{
    auto Self(this->shared_from_this());
    WriteStrand.post(
        [this, Self, Msg = std::move(Msg)]{
            MessageToRemote.emplace_back(std::move(Msg));
            if (MessageToRemote.size() <= 1)
            {
                WriteHeader();
            }
        });
}

void CConnection::OnErrorCode(std::error_code& ErrorCode)
{
    auto Self(this->shared_from_this());
    // remote is shutdown if eof 
    if (asio::error::eof == ErrorCode && State != EState::Disconnect)
    {
        State = EState::Disconnect;
        std::cout << "[Connection] " << HSLM_FUNC_LINE << " <"<< GetRemoteEndpointString() << "> Disconnect\n";
    } else {
        if (State != EState::Disconnect)
        {
            // Even after checking(State != EState::Disconnect), it is still possible that this is printed out due to multithreading problem
            State = EState::ErrorOccurred;
            std::cout << "[Connection] " << HSLM_FUNC_LINE << " <" << GetRemoteEndpointString() << "> Error Occurred, ErrorMessage: " << ErrorCode.message() << "\n";
        }
    }
    if (Owner == EOwner::kServer){
        bool Expected = true;
        if(ServerSharedRef.compare_exchange_strong(Expected, false))
        {
            Server->OnClientDisconnect(shared_from_this());
            Server->ServerSharedReferenceRemove(this->shared_from_this());
            // close socket, return false when IsConnected(asio::ip::tcp::socket::is_open()) check 
            Socket.close();
        }
    }
}

void CConnection::ReadHeader()
{
    auto Self(this->shared_from_this());
    asio::async_read(Socket, asio::buffer(&MessageTemporaryRead.Header, sizeof(SMessageHeader)),
        [this, Self](std::error_code ErrorCode, std::size_t Length)
        {
            if (!ErrorCode)
            {
                if (MessageTemporaryRead.Header.DataSize > 0)
                {
                    MessageTemporaryRead.ResizeDataSize();
                    ReadBody();
                }
                else
                {
                    MessageTemporaryRead.Data.clear();
                    AddMessageToQueue();
                    ReadHeader();
                }
            }
            else
            {
                OnErrorCode(ErrorCode);
            }
        });
}

void CConnection::ReadBody()
{
    auto Self(this->shared_from_this());
    asio::async_read(Socket, asio::buffer(MessageTemporaryRead.Data.data(), MessageTemporaryRead.Header.DataSize),
        [this, Self](std::error_code ErrorCode, std::size_t Length)
        {
            if (!ErrorCode)
            {
                AddMessageToQueue();
                ReadHeader();
            }
            else
            {
                OnErrorCode(ErrorCode);
            }
        });
}

void CConnection::WriteHeader()
{
    auto Self(this->shared_from_this());
    MessageToRemote.front().GenerateHeaderDataSize();
    asio::async_write(Socket, asio::buffer(&MessageToRemote.front().Header, sizeof(SMessageHeader)),
        WriteStrand.wrap([this, Self](std::error_code ErrorCode, std::size_t Length)
            {
                if (!ErrorCode)
                {
                    if (MessageToRemote.front().Header.DataSize > 0)
                    {
                        WriteBody();
                    }
                    else
                    {
                        MessageToRemote.pop_front();
                        if (!MessageToRemote.empty())
                        {
                            WriteHeader();
                        }
                    }
                }
                else
                {
                    OnErrorCode(ErrorCode);
                }
            }));
}

void CConnection::WriteBody()
{
    auto Self(this->shared_from_this());
    asio::async_write(Socket, asio::buffer(MessageToRemote.front().Data.data(), MessageToRemote.front().Header.DataSize),
        WriteStrand.wrap([this, Self](std::error_code ErrorCode, std::size_t Length)
            {
                if (!ErrorCode)
                {
                    MessageToRemote.pop_front();
                    if (!MessageToRemote.empty())
                    {
                        WriteHeader();
                    }
                }
                else
                {
                    OnErrorCode(ErrorCode);
                }
            }));
}

void CConnection::AddMessageToQueue()
{
    //if (Owner == EOwner::kServer)
    MessageToLocal.emplace_back({ this->shared_from_this(), std::move(MessageTemporaryRead) });
    // else
    //     MessageToLocal.emplace_back({ nullptr, std::move(MessageTemporaryRead) });
}