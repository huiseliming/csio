#pragma once
#include <vector>
#include "Message.h"

struct IMessageHandler;

class CMessageHandlerManager
{
public:
    void RegisterMessageHandler(EMessageId MsgId, std::unique_ptr<IMessageHandler>&& MsgHandlerPtr)
    {
        if(MessageHandlers.size() > uint32_t(MsgId))
        {
            MessageHandlers.resize(uint32_t(MsgId) + 1);
        }
        MessageHandlers[uint32_t(MsgId)] = std::move(MsgHandlerPtr);
    }

    void DeregisterMessageHandler(EMessageId MsgId)
    {
        if(MessageHandlers.size() > uint32_t(MsgId))
        {
            MessageHandlers[uint32_t(MsgId)].reset();
        }
    }

    IMessageHandler* GetMessageHandler(uint32_t MsgId)
    {
        if(MsgId < MessageHandlers.size())
        {
            return MessageHandlers[MsgId].get();
        }
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<IMessageHandler>> MessageHandlers;

};

struct IMessageHandler
{
    IMessageHandler() = default;
    virtual ~IMessageHandler() = default;
    virtual void operator()(std::vector<uint8_t> Data) = 0;
};
