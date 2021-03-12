#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <asio.hpp>
#include <string_view>
#include "Connection.h"

class NetworkHelper
{
public:
    static uint64_t EndpointToUint64Id(asio::ip::tcp::endpoint Endpoint)
    {
        uint64_t Uint64Id = static_cast<uint64_t>(Endpoint.address().to_v4().to_uint()) << 16;
        return Uint64Id + static_cast<uint64_t>(Endpoint.port());
    }

    static asio::ip::tcp::endpoint Uint64IdToEndpoint(uint64_t Uint64Id)
    {
        return asio::ip::tcp::endpoint(asio::ip::address_v4(static_cast<uint32_t>(Uint64Id >> 16)), static_cast<uint16_t>(Uint64Id));
    }
};

class IServer
{
public:
    IServer() = default;
    virtual ~IServer() = default;

    virtual bool Start(uint16_t Port) = 0;

    virtual void Stop() = 0;

    virtual void MessageClient(SMessage&& Msg, std::string Address, uint16_t Port = 0) = 0;

    virtual void MessageClient(SMessage&& Msg, std::shared_ptr<CConnection> Client) = 0;

    virtual void MessageAllClients(SMessage&& Msg, std::shared_ptr<CConnection> IgnoreClientPtr = nullptr) = 0;

    virtual void Update(size_t MaxMessages = -1) = 0;

    // Callback on Client connect , return false refuse CConnection
    virtual bool OnClientConnect(std::shared_ptr<CConnection> ConnectionPtr) = 0;
    // Callback on Client disconnect
    virtual void OnClientDisconnect(std::shared_ptr<CConnection> ConnectionPtr) = 0;
    // Callback on Client SMessage reach
    virtual void OnMessage(std::shared_ptr<CConnection> Client, SMessage&& msg) = 0;
};
