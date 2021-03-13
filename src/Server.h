#pragma once
#include "IServer.h"
#include <memory>
#include <vector>

class CServer : public IServer
{
public:
    CServer();

    virtual ~CServer() override;

    virtual bool Start(uint16_t Port, uint32_t NumThread = 0) override;

    virtual void Stop() override;

    void WaitThreadJoin();

    void WaitForClientConnection();

    virtual void MessageClient(SMessage&& Msg, std::string Address, uint16_t Port = 0) override;

    virtual void MessageClient(SMessage&& Msg, std::shared_ptr<CConnection> Client) override;

    virtual void MessageAllClients(SMessage&& Msg, std::shared_ptr<CConnection> IgnoreClientPtr = nullptr) override;

    virtual void Update(size_t MaxMessages = -1) override;

protected:
    virtual bool OnClientConnect(std::shared_ptr<CConnection> Client) override;
    
    virtual void OnClientDisconnect(std::shared_ptr<CConnection> Client) override;

    virtual void OnMessage(std::shared_ptr<CConnection> Client, SMessage&& Msg) override;

    uint32_t ConnectionCounter = 0;

    TThreadSafeDeque<SMessagesTo> MessageToLocal;
    std::unordered_map<uint32_t, std::vector<std::shared_ptr<CConnection>>> ConnectionMap;

    asio::io_context IoContext;
    asio::io_context::strand ConnectionsStrand;
    std::vector<std::thread> Threads;
    asio::ip::tcp::acceptor Acceptor;

    TTSQueue<std::function<void()>> SyncTaskQueue;

public:
    void RegisterMessageHandler(EMessageId MsgId, std::unique_ptr<IMessageHandler>&& MsgHandlerPtr) ;

    void DeregisterMessageHandler(EMessageId MsgId);

    std::vector<std::unique_ptr<IMessageHandler>> MessageHandlers;

};
