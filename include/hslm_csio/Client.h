#pragma once 
#include <string>
#include <memory>
#include <iostream>
#include "ThreadSafeDeque.h"
#include "Message.h"
#include "Connection.h"
#include "MessageHandler.h"

struct IClient
{
    IClient() = default;
    virtual ~IClient() = default;

    virtual bool Connect(const std::string Host, const uint16_t Port) = 0;

    virtual void Disconnect() = 0;

    virtual void MessageServer(SMessage&& Msg) = 0;

    virtual void OnMessage(std::shared_ptr<CConnection> Connection, SMessage&& Msg) = 0;

    virtual void Update(size_t MaxMessages = -1) = 0;
};


class CClient : public IClient
{
public:
    CClient();

    virtual ~CClient();
    
    virtual bool Connect(const std::string Host, const uint16_t Port) override;

    virtual void Disconnect() override;

    CConnection::EState GetConnectionState();

    void WaitConnected();

    virtual void MessageServer(SMessage&& Msg) override;

    virtual void Update(size_t MaxMessages = -1) override;

    virtual void OnMessage(std::shared_ptr<CConnection> Connection, SMessage&& Msg) override;

protected:
    asio::io_context IoContext;
    std::thread Thread;
    std::shared_ptr<CConnection> ConnectionPtr;

private:
    TThreadSafeDeque<SMessagesTo> MessageToLocal;
    std::unique_ptr<asio::io_service::work> IdleWorkPtr;

public:
    CMessageHandlerManager& GetMessageHandlerManager();

protected: 
    CMessageHandlerManager MessageHandlerManager;

};


