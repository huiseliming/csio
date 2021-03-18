#include "hslm_csio/MessageHandler.h"


void CMessageHandlerManager::RegisterMessageHandler(EMessageId MsgId, std::unique_ptr<IMessageHandler>&& MsgHandlerPtr)
{
    if (MessageHandlers.size() <= uint32_t(MsgId))
    {
        MessageHandlers.resize(uint32_t(MsgId) + 1);
    }
    MessageHandlers[uint32_t(MsgId)] = std::move(MsgHandlerPtr);
}

void CMessageHandlerManager::DeregisterMessageHandler(EMessageId MsgId)
{
    if (MessageHandlers.size() <= uint32_t(MsgId))
    {
        MessageHandlers[uint32_t(MsgId)].reset();
    }
}

IMessageHandler* CMessageHandlerManager::GetMessageHandler(uint32_t MsgId)
{
    if (MsgId < MessageHandlers.size())
    {
        return MessageHandlers[MsgId].get();
    }
    return nullptr;
}