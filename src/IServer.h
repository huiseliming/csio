#pragma once
#include <memory>
#include <vector>
#include <unordered_map>

class Connection;
class IMessageHandler;

class IServer
{
public:
    IServer() = default;
    virtual ~IServer() = default;

    virtual bool Start() = 0;



    virtual void WaitThreadJoin() = 0;

    virtual void MessageClient(std::shared_ptr<Connection> pClient, std::vector<uint8_t>&& inMessageData) = 0;

    virtual void MessageAllClients(std::vector<uint8_t>&& inMessageData, std::shared_ptr<Connection> pIgnoreClient = nullptr) = 0;


    // Callback on client connect , return false refuse connection
    virtual bool OnClientConnect(std::shared_ptr<Connection> pConnection) { return true; }
    // Callback on client disconnect
    virtual void OnClientDisconnect(std::shared_ptr<Connection> pConnection) {}
    // Callback on client message reach
    virtual void OnMessage(std::shared_ptr<Connection> pConnection, std::vector<uint8_t> inMessageData) {}
};


class TServer : public IServer
{
public:

private:
    std::unordered_map<uint32_t, IMessageHandler> m_MessageHandlerMap;

};