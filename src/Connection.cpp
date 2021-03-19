#include "hslm_csio/Connection.h"
#include <iostream>

CConnection::CConnection(EOwner owner, asio::io_context& IoContext, asio::ip::tcp::socket Socket, TThreadSafeDeque<SMessageTo>& MessageToLocal)
    : IoContext(IoContext)
    , WriteStand(IoContext)
    , Socket(std::move(Socket))
    , MessageToLocal(MessageToLocal)
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
                std::cout << "[Client] Connected!" << std::endl;
                State = EState::Connected;
                SendMessageToRemote(SMessage(EMessageId::kConnectToServer));
                ReadHeader();
            }
            else
            {
                std::cout << "[Client] ConnectToServer Failed!" << std::endl;
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
        RemoteEndpoint = Socket.remote_endpoint();
        Id = Id;
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
    WriteStand.post(
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
    WriteStand.post(
        [this, Self, Msg = std::move(Msg)]{
            MessageToRemote.emplace_back(std::move(Msg));
            if (MessageToRemote.size() <= 1)
            {
                WriteHeader();
            }
        });
}

void CConnection::OnError(std::error_code& ErrorCode)
{
    // Just handle first error code 
    if (!IsConnected())
        return;
    // remote is shutdown if eof 
    if(asio::error::eof == ErrorCode)
    {
        State = EState::Disconnect;
    } else { 
        State = EState::ErrorOccurred;
        GetRemoteEndpoint().address().to_string();
        std::cout << "[Connection] " << HSLM_FUNC_LINE << GetRemoteEndpointString() << " disconnect, ErrorCode: " << ErrorCode.message() << "\n";
    }
    // close socket, return false when IsConnected(asio::ip::tcp::socket::is_open()) check 
    Socket.close();
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
                OnError(ErrorCode);
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
                OnError(ErrorCode);
            }
        });
}

void CConnection::WriteHeader()
{
    auto Self(this->shared_from_this());
    MessageToRemote.front().GenerateHeaderDataSize();
    asio::async_write(Socket, asio::buffer(&MessageToRemote.front().Header, sizeof(SMessageHeader)),
        WriteStand.wrap([this, Self](std::error_code ErrorCode, std::size_t Length)
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
                    OnError(ErrorCode);
                }
            }));
}

void CConnection::WriteBody()
{
    auto Self(this->shared_from_this());
    asio::async_write(Socket, asio::buffer(MessageToRemote.front().Data.data(), MessageToRemote.front().Header.DataSize),
        WriteStand.wrap([this, Self](std::error_code ErrorCode, std::size_t Length)
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
                    OnError(ErrorCode);
                }
            }));
}

void CConnection::AddMessageToQueue()
{
    if (Owner == EOwner::kServer)
        MessageToLocal.emplace_back({ this->shared_from_this(), std::move(MessageTemporaryRead) });
    else
        MessageToLocal.emplace_back({ nullptr, std::move(MessageTemporaryRead) });
}