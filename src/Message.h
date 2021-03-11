#pragma once
#include <vector>

struct SMessageHeader
{
    uint32_t MessageId;
    uint32_t DataSize;
};

struct SMessage
{
    SMessage();


    SMessage(const SMessage& Message);
    SMessage(SMessage&& Message);
    SMessage& operator=(const SMessage& Message);
    SMessage& operator=(SMessage&& Message);

    void Reset();
    void UpdateDataSize();

    SMessageHeader Header;
    std::vector<uint8_t> Data;
};
