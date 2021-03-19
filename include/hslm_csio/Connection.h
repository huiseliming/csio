#pragma once
#include <asio.hpp>
#include <deque>
#include "HslmDefines.h"
#include "Message.h"
#include "ThreadSafeDeque.h"

class CConnection;

struct SMessageTo
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
        ErrorOccurred,
        Disconnect
    }State = EState::Undefine;

    enum class EOwner {
        kUnknow,
        kClient,
        kServer
    }Owner = EOwner::kUnknow;

public:
    CConnection(EOwner owner, asio::io_context& IoContext, asio::ip::tcp::socket Socket, TThreadSafeDeque<SMessageTo>& MessageToLocal);

    virtual ~CConnection();

    bool ConnectToServer(const asio::ip::tcp::resolver::results_type& Endpoints);

    bool ConnectToClient(uint32_t Id);

    void Disconnect();

    void SendMessageToRemote(const SMessage& Msg);

    void SendMessageToRemote(SMessage&& Msg);

    bool IsConnected() const { return Socket.is_open(); }

    bool GetID() { return Id; }

    EState GetState() { return State; }

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

    virtual void OnError(std::error_code& ErrorCode);

    virtual void ReadHeader();

    virtual void ReadBody();

    virtual void WriteHeader();

    virtual void WriteBody();

    virtual void AddMessageToQueue();

protected:
    asio::io_context& IoContext;
    asio::io_context::strand WriteStand;
    asio::ip::tcp::socket Socket;
    asio::ip::tcp::endpoint RemoteEndpoint;

    uint32_t Id = 0;

    std::deque<SMessage> MessageToRemote;
    TThreadSafeDeque<SMessageTo>& MessageToLocal;
    SMessage MessageTemporaryRead;


};
