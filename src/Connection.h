#pragma once
#include <asio.hpp>
#include <deque>
#include "CSIODefines.h"
#include "Message.h"
#include "ThreadSafeDeque.h"

class CConnection;

struct SMessagesTo
{
    std::shared_ptr<CConnection> remote;
    SMessage msg;
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

    void SendMessageToRemote(const SMessage& msg)
    {
        auto self(this->shared_from_this());
        WriteStand.post(
            [this, self, msg = msg]{
                MessageToRemote.push_back(msg);
                if (MessageToRemote.size() <= 1)
                {
                    WriteHeader();
                }
            });
    }

    void SendMessageToRemote(SMessage&& msg)
    {
        auto self(this->shared_from_this());
        WriteStand.post(
            [this, self, msg = std::forward<SMessage>(msg)]{
                MessageToRemote.push_back(msg);
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

    virtual void OnException()
    {
        GetRemoteEndpoint().address().to_string();
        std::cout << "[Connection] " << CSIO_FUNC_LINE << GetRemoteEndpointString() << " disconnect!\n";
        Socket.close();
        State = EState::Disconnect;
    }

    virtual void ReadHeader()
    {
        auto self(this->shared_from_this());
        asio::async_read(Socket, asio::buffer(&MessageTemporaryRead.Header, sizeof(SMessageHeader)),
            [this, self](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
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
                    OnException();
                }
            });
    }

    virtual void ReadBody()
    {
        auto self(this->shared_from_this());
        asio::async_read(Socket, asio::buffer(MessageTemporaryRead.Data.data(), MessageTemporaryRead.Header.DataSize),
            [this, self](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    AddMessageToQueue();
                    ReadHeader();
                }
                else
                {
                    OnException();
                }
            });
    }

    virtual void WriteHeader()
    {
        auto self(this->shared_from_this());
        asio::async_write(Socket, asio::buffer(&MessageToRemote.front().Header, sizeof(SMessageHeader)),
            WriteStand.wrap([this, self](std::error_code ec, std::size_t length)
                {
                    if (!ec)
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
                        OnException();
                    }
                }));
    }

    virtual void WriteBody()
    {
        auto self(this->shared_from_this());
        asio::async_write(Socket, asio::buffer(MessageToRemote.front().Data.data(), MessageToRemote.front().Header.DataSize),
            WriteStand.wrap([this, self](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        MessageToRemote.pop_front();
                        if (!MessageToRemote.empty())
                        {
                            WriteHeader();
                        }
                    }
                    else
                    {
                        OnException();
                    }
                }));
    }

    virtual void AddMessageToQueue()
    {
        if (Owner == EOwner::kServer)
            MessageToLocal.push_back({ this->shared_from_this(), MessageTemporaryRead });
        else
            MessageToLocal.push_back({ nullptr, MessageTemporaryRead });
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
