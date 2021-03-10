#pragma once
#include <vector>




struct Message
{
    uint8_t type;
    uint32_t local;
    uint32_t remote;
    uint32_t* payload;

    


    std::vector<uint8_t> data;
};



