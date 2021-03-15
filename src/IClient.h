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

    virtual void Start() = 0;

    virtual void Stop() = 0;

    virtual bool Connect(const std::string Host, const uint16_t Port);

    virtual void Disconnect() = 0;

    virtual void MessageServer(SMessage&& Msg) = 0;

    virtual void OnMessage(std::shared_ptr<CConnection> Connection, SMessage&& Msg) = 0;

    virtual void Update(size_t MaxMessages = -1) = 0;
};


class Client : public IClient
{
    Client()
    :IdleWorkPtr(new asio::io_context::work(IoContext))
    {
        Thread = std::move(std::thread([this] { IoContext.run(); }));  
    }
    virtual ~Client()
    {
        IdleWorkPtr.reset();
        Disconnect();
        ConnectionPtr.reset();
        if(Thread.joinable())
            Thread.join();
    }
    
    virtual bool Connect(const std::string Host, const uint16_t Port) override
    {
        try
        {
            asio::ip::tcp::resolver Resolver(IoContext);
            auto Endpoints = Resolver.resolve(Host,std::to_string(Port));
            ConnectionPtr = std::make_shared<CConnection>(CConnection::EOwner::kClient, IoContext, asio::ip::tcp::socket(IoContext), MessageToLocal);
            ConnectionPtr->ConnectToServer(Endpoints);
        }
        catch (const std::exception& Exception)
        {
            std::cout << "[Client] Exception: " << Exception.what() << std::endl;
            return false;
        }
        return true;
    }

    virtual void Disconnect() override
    {
        if (ConnectionPtr && ConnectionPtr->IsConnected())
            ConnectionPtr->Disconnect();
    }

    virtual void MessageServer(SMessage&& Msg) override
    {
        if (ConnectionPtr->IsConnected()) 
            ConnectionPtr->SendMessageToRemote(std::move(Msg)); 
    }

    virtual void Update(size_t MaxMessages = -1) override
    {
        size_t MessageCount = 0;
        while (MessageCount < MaxMessages && !MessageToLocal.empty())
        {
            auto MsgTo = std::move(MessageToLocal.pop_front());
            IMessageHandler* MessageHandler = MessageHandlerManager.GetMessageHandler(MsgTo.Message.Header.MessageId);
            if(MessageHandler != nullptr){
                (*MessageHandler)(std::move(MsgTo.Message.Data));
            } else {
                OnMessage(MsgTo.RemoteConnection, std::move(MsgTo.Message));
            }
            MessageCount++;
        }
    }

    virtual void OnMessage(std::shared_ptr<CConnection> Connection, SMessage&& Msg) override
    {
        std::cout << "[Client] OnMessage: MessageId(" << Msg.Header.MessageId << ") DataSize(" << Msg.Header.DataSize << ")\n";
    }

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


