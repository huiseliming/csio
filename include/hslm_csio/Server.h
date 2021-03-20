#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <asio.hpp>
#include <string_view>
#include "Connection.h"
#include "MessageHandler.h"

class NetworkHelper
{
public:
    static uint64_t EndpointToUint64Id(asio::ip::tcp::endpoint Endpoint)
    {
        uint64_t Uint64Id = static_cast<uint64_t>(Endpoint.address().to_v4().to_uint()) << 16;
        return Uint64Id + static_cast<uint64_t>(Endpoint.port());
    }

    static asio::ip::tcp::endpoint Uint64IdToEndpoint(uint64_t Uint64Id)
    {
        return asio::ip::tcp::endpoint(asio::ip::address_v4(static_cast<uint32_t>(Uint64Id >> 16)), static_cast<uint16_t>(Uint64Id));
    }
};

class IServer
{
public:
    IServer() = default;
    virtual ~IServer() = default;

    virtual bool Start(uint16_t Port, uint32_t NumThread) = 0;

    //virtual bool IsRunning() = 0;

    virtual void Stop() = 0;

    virtual void MessageClient(SMessage&& Msg, std::string Address, uint16_t Port) = 0;

    virtual void MessageClient(SMessage&& Msg, uint32_t Uint32Address, uint16_t Port) = 0;

    virtual void MessageClient(SMessage&& Msg, std::shared_ptr<CConnection> Client) = 0;

    virtual void MessageAllClients(SMessage&& Msg, std::shared_ptr<CConnection> IgnoreClientPtr) = 0;

    virtual void Update(size_t MaxMessages) = 0;

    // Callback on Client connect , return false refuse CConnection, Called in io_context.run()
    virtual bool OnClientConnect(std::shared_ptr<CConnection> ConnectionPtr) = 0;
    // Callback on Client disconnect, Called in io_context.run()
    virtual void OnClientDisconnect(std::shared_ptr<CConnection> ConnectionPtr) = 0;
    // Callback on Client SMessage reach, Called Update(size_t MaxMessages)
    virtual void OnMessage(std::shared_ptr<CConnection> Client, SMessage&& Msg) = 0;
};

class CServer : public IServer
{
public:
    friend class CConnection;
public:
    CServer();

    virtual ~CServer() override;

    virtual bool Start(uint16_t Port, uint32_t NumThread = 0) override;

    virtual void Stop() override;

    void WaitThreadJoin();

    void WaitForClientConnection();

    void ServerSharedReferenceRemove(std::shared_ptr<CConnection> Client);

    virtual void MessageClient(SMessage&& Msg, std::string Address, uint16_t Port = 0) override;

    virtual void MessageClient(SMessage&& Msg, uint32_t Uint32Address, uint16_t Port = 0) override;

    virtual void MessageClient(SMessage&& Msg, std::shared_ptr<CConnection> Client) override;

    virtual void MessageAllClients(SMessage&& Msg, std::shared_ptr<CConnection> IgnoreClientPtr = nullptr) override;

    virtual void Update(size_t MaxMessages = -1) override;

protected:
    virtual bool OnClientConnect(std::shared_ptr<CConnection> Client) override;

    virtual void OnClientDisconnect(std::shared_ptr<CConnection> Client) override;

    virtual void OnMessage(std::shared_ptr<CConnection> Client, SMessage&& Msg) override;

    uint32_t ConnectionCounter = 0;

    TThreadSafeDeque<SMessageTo> MessageToLocal;
    std::unordered_map<uint32_t, std::vector<std::shared_ptr<CConnection>>> ConnectionMap;

    asio::io_context IoContext;
    asio::io_context::strand ConnectionsStrand;
    std::vector<std::thread> Threads;
    asio::ip::tcp::acceptor Acceptor;

    TTSQueue<std::function<void()>> SyncTaskQueue;

    

public:
    CMessageHandlerManager& GetMessageHandlerManager();
protected:
    CMessageHandlerManager MessageHandlerManager;
};
