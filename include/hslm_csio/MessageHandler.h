#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include "Message.h"
#include "Connection.h"

struct IMessageHandler
{
    IMessageHandler() = default;
    virtual ~IMessageHandler() = default;
    virtual void operator()(std::shared_ptr<CConnection> ConnectionPtr, std::vector<uint8_t>&& Data) = 0;
};

class CMessageHandlerManager
{
public:
    void RegisterMessageHandler(EMessageId MsgId, std::unique_ptr<IMessageHandler>&& MsgHandlerPtr);

    void DeregisterMessageHandler(EMessageId MsgId);

    IMessageHandler* GetMessageHandler(uint32_t MsgId);

private:
    std::vector<std::unique_ptr<IMessageHandler>> MessageHandlers;

};


class CCloseConnectionHandler : public IMessageHandler
{
    virtual void operator()(std::shared_ptr<CConnection> ConnectionPtr, std::vector<uint8_t>&& Data) override
    {
        ConnectionPtr->Disconnect();
    }
};

class CTestHandler : public IMessageHandler
{

    virtual void operator()(std::shared_ptr<CConnection> ConnectionPtr, std::vector<uint8_t>&& Data) override
    {
        std::string str;
        str.append((char*)Data.data(), Data.size());
        std::cout << "[Test] Recv: "<< str << "\n";
    }
};
