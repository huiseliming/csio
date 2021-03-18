#pragma once
#include <vector>




// Bit32 ForwardMessageBit 1:ForwardMessage
// Bit31 WhoToWhoBit       1:ServerToClient 0:ClientToServer
// Bit30 WithMessageTagBit 1:WithMessageTag

enum class EMessageId : uint32_t
{
    kCloseConnection = 0x0,
    kConnectToServer,
    kConnectToClient,
    kTestMessage = 4156,
    kWithMessageTagBit = 0x20000000,
    kWhoToWhoBit = 0x40000000,
    kForwardMessageBit = 0x80000000,
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
    SMessage(EMessageId MessageId);
    SMessage(EMessageId MessageId, std::vector<uint8_t>& Data);
    SMessage(EMessageId MessageId, std::vector<uint8_t>&& Data);

    SMessage(const SMessage& Message);
    SMessage(SMessage&& Message);
    SMessage& operator=(const SMessage& Message);
    SMessage& operator=(SMessage&& Message);

    void Reset();
    void GenerateHeaderDataSize();
    void ResizeDataSize();

    SMessageHeader Header;
    std::vector<uint8_t> Data;
};
