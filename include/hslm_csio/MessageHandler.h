#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include "Message.h"

struct IMessageHandler;

class CMessageHandlerManager
{
public:
    void RegisterMessageHandler(EMessageId MsgId, std::unique_ptr<IMessageHandler>&& MsgHandlerPtr);

    void DeregisterMessageHandler(EMessageId MsgId);

    IMessageHandler* GetMessageHandler(uint32_t MsgId);

private:
    std::vector<std::unique_ptr<IMessageHandler>> MessageHandlers;

};

struct IMessageHandler
{
    IMessageHandler() = default;
    virtual ~IMessageHandler() = default;
    virtual void operator()(std::vector<uint8_t>&& Data) = 0;
};

class CTestMessageHandler : public IMessageHandler
{

    virtual void operator()(std::vector<uint8_t>&& Data)
    {
        std::string str;
        str.append((char*)Data.data(), Data.size());
        std::cout << str;
    }
};
