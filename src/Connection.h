#pragma once
#include <asio.hpp>
#include <deque>
#include "CSIODefines.h"
#include "Message.h"
#include "ThreadSafeDeque.h"

class CConnection;

struct SMessagesTo
{
    std::shared_ptr<CConnection> RemoteConnection;
    SMessage Message;
};

class CConnection :public std::enable_shared_from_this<CConnection>
{
public:
    enum class EState : uint32_t {
        Undefine,
        Connecting,
        Connected,
        ConnectFailed,
        Disconnect
    }State = EState::Undefine;

    enum class EOwner {
        kUnknow,
        kClient,
        kServer
    }Owner = EOwner::kUnknow;

public:
    CConnection(EOwner owner, asio::io_context& IoContext, asio::ip::tcp::socket Socket, TThreadSafeDeque<SMessagesTo>& MessageToLocal)
        : IoContext(IoContext)
        , WriteStand(IoContext)
        , Socket(std::move(Socket))
        , MessageToLocal(MessageToLocal)
    {
        Owner = owner;
    }

    virtual ~CConnection()
    {

    }

    bool ConnectToServer(const asio::ip::tcp::resolver::results_type& Endpoints)
    {
        State = EState::Connecting;
        assert(Owner == EOwner::kClient && "Only client can connect to server");
        asio::async_connect(Socket, Endpoints,
            [this](std::error_code ErrorCode, asio::ip::tcp::endpoint Endpoint)
            {
                if (!ErrorCode)
                {
                    std::cout << "[Client] Connected!" << std::endl;
                    SendMessageToRemote(SMessage(EMessageId::kConnectToClient));
                    ReadHeader();
                    State = EState::Connected;
                }
                else
                {
                    std::cout << "[Client] ConnectToServer Failed!" << std::endl;
                    State = EState::ConnectFailed;
                }
            });
        return true;
    }

    bool ConnectToClient(uint32_t Id)
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

    void Disconnect()
    {
        if (IsConnected())
        {
            asio::post(IoContext, [this]() {
                Socket.close();
                State = EState::Disconnect;
                });
        }
    }

    bool IsConnected() const { return Socket.is_open(); }

    bool GetID() { return Id; }

    EState GetState() { return State; }

    void SendMessageToRemote(const SMessage& Msg)
    {
        auto Self(this->shared_from_this());
        WriteStand.post(
            [this, Self, Msg = Msg]{
                MessageToRemote.emplace_back(Msg);
                if (MessageToRemote.size() <= 1)
                {
                    WriteHeader();
                }
            });
    }

    void SendMessageToRemote(SMessage&& Msg)
    {
        auto Self(this->shared_from_this());
        WriteStand.post(
            [this, Self, Msg = std::forward<SMessage>(Msg)]{
                MessageToRemote.emplace_back(Msg);
                if (MessageToRemote.size() <= 1)
                {
                    WriteHeader();
                }
            });
    }

    asio::ip::tcp::endpoint GetLocalEndpoint()
    {
        return Socket.local_endpoint();
    }

    asio::ip::tcp::endpoint GetRemoteEndpoint()
    {
        return RemoteEndpoint;
    }

    std::string GetLocalEndpointString()
    {
        return RemoteEndpoint.address().to_v4().to_string() + ":" + std::to_string(RemoteEndpoint.port());
    }

    std::string GetRemoteEndpointString()
    {
        return RemoteEndpoint.address().to_v4().to_string() + ":" + std::to_string(RemoteEndpoint.port());
    }

protected:

    virtual void OnException(std::error_code& ErrorCode)
    {
        GetRemoteEndpoint().address().to_string();
        std::cout << "[Connection] " << CSIO_FUNC_LINE << GetRemoteEndpointString() << " disconnect, ErrorCode: " << ErrorCode.message() <<"\n";
        Socket.close();
        State = EState::Disconnect;
    }

    virtual void ReadHeader()
    {
        auto Self(this->shared_from_this());
        asio::async_read(Socket, asio::buffer(&MessageTemporaryRead.Header, sizeof(SMessageHeader)),
            [this, Self](std::error_code ErrorCode, std::size_t Length)
            {
                if (!ErrorCode)
                {
                    if(MessageTemporaryRead.Header.MessageId == 0)
                    {
                        return;
                    }
                    if (MessageTemporaryRead.Header.DataSize > 0)
                    {
                        MessageTemporaryRead.UpdateDataSize();
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
                    OnException(ErrorCode);
                }
            });
    }

    virtual void ReadBody()
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
                    OnException(ErrorCode);
                }
            });
    }

    virtual void WriteHeader()
    {
        auto Self(this->shared_from_this());
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
                        OnException(ErrorCode);
                    }
                }));
    }

    virtual void WriteBody()
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
                        OnException(ErrorCode);
                    }
                }));
    }

    virtual void AddMessageToQueue()
    {
        if (Owner == EOwner::kServer)
            MessageToLocal.emplace_back({ this->shared_from_this(), std::move(MessageTemporaryRead) });
        else
            MessageToLocal.emplace_back({ nullptr, std::move(MessageTemporaryRead) });
    }

protected:
    asio::io_context& IoContext;
    asio::io_context::strand WriteStand;
    asio::ip::tcp::socket Socket;
    asio::ip::tcp::endpoint RemoteEndpoint;

    uint32_t Id = 0;

    std::deque<SMessage> MessageToRemote;
    TThreadSafeDeque<SMessagesTo>& MessageToLocal;
    SMessage MessageTemporaryRead;


};
