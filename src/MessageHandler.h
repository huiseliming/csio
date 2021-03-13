#pragma once
#include <vector>

struct IMessageHandler
{
    IMessageHandler() = default;
    virtual ~IMessageHandler() = default;
    virtual void operator()(std::vector<uint8_t> Data) = 0;
};
