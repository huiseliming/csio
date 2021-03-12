#pragma once
#include <vector>




// Bit32 WhoToWhoBit       1:ServerToClient 0:ClientToServer
// Bit31 ForwardMessageBit 1:ForwardMessage
// Bit30 WithMessageTagBit 1:WithMessageTag
enum class EMessageId : uint32_t
{
    kCloseConnection = 0x0,
    kConnectToServer,
    kConnectToClient,
};



struct SMessageHeader
{
    uint32_t MessageId;
    uint32_t DataSize;
};
//uint64_t MessageTag;
//uint64_t ResponseTag;
struct SMessage
{
    SMessage();
    SMessage(uint32_t MessageId);
    SMessage(EMessageId MessageId);

    SMessage(const SMessage& Message);
    SMessage(SMessage&& Message);
    SMessage& operator=(const SMessage& Message);
    SMessage& operator=(SMessage&& Message);

    void Reset();
    void UpdateDataSize();

    SMessageHeader Header;
    std::vector<uint8_t> Data;
};
