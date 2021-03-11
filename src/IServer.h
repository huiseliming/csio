#pragma once
#include <memory>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <asio.hpp>

#include "Connection.h"

class IMessageHandler;

class IServer
{
public:
    IServer() = default;
    virtual ~IServer() = default;

    virtual bool Start(uint16_t port) = 0;

    virtual void Stop() = 0;

    virtual void WaitThreadJoin() = 0;

    virtual void MessageClient(std::shared_ptr<CConnection> pClient, std::vector<uint8_t>&& inMessageData) = 0;

    //virtual void MessageClient(std::shared_ptr<CConnection> pClient, std::vector<uint8_t>& inMessageData) = 0;

    virtual void MessageAllClients(std::vector<uint8_t>&& inMessageData, std::shared_ptr<CConnection> pIgnoreClient = nullptr) = 0;

    //virtual void MessageAllClients(std::vector<uint8_t>& inMessageData, std::shared_ptr<CConnection> pIgnoreClient = nullptr) = 0;

    // Callback on client connect , return false refuse CConnection
    virtual bool OnClientConnect(std::shared_ptr<CConnection> pCConnection) { return true; }
    // Callback on client disconnect
    virtual void OnClientDisconnect(std::shared_ptr<CConnection> pCConnection) {}
    // Callback on client SMessage reach
    virtual void OnMessage(std::shared_ptr<CConnection> pCConnection, std::vector<uint8_t> inMessageData) {}
};

class CServer : public IServer
{
public:
    CServer(uint16_t port)
        : Acceptor(IoContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
        , ConnectionsStrand(IoContext)
    {

    }

    virtual ~CServer()
    {
        Stop();
    }

    bool Start()
    {
        try
        {
            std::cout << "[Server] Start!" << std::endl;
            WaitForClientConnection();
            Thread = std::move(std::thread([this] { IoContext.run(); }));
        }
        catch (const std::exception& e)
        {
            std::cout << "[Server] Exception: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    void Stop()
    {
        IoContext.stop();
        if (Thread.joinable())
            Thread.join();
        std::cout << "[Server] Stopped!" << std::endl;
    }

    void WaitForClientConnection()
    {
        Acceptor.async_accept(
            [this](std::error_code ec, asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    auto remote_endpoint = socket.remote_endpoint();
                    std::cout << "[Server] New Connection: " << remote_endpoint << std::endl;
                    std::shared_ptr<CConnection> newConnection =
                        std::make_shared<CConnection>(CConnection::EOwner::kServer, IoContext, std::move(socket), MessageToLocal);
                    if (OnClientConnect(newConnection))
                    {
                        ConnectionsStrand.post([this, newConnection] {
                            m_connections.push_back(std::move(newConnection));
                            m_connections.back()->ConnectToClient(IdCounter++);
                            std::cout << "[Server] " << "[ID:" << m_connections.back()->GetID() << "] Connection Approved" << std::endl;
                            });
                    }
                    else
                    {
                        std::cout << "[Server] " << remote_endpoint << " Connection Denied" << std::endl;
                    }
                }
                else
                {
                    std::cout << "[Server] New Connectsion Error: " << ec.message() << std::endl;
                }
                WaitForClientConnection();
            });
    }

    void MessageClient(std::string address, SMessage msg)
    {
        ConnectionsStrand.post(
            [this, msg = std::move(msg), address = std::move(address)]() mutable {
            bool bInvalidClientExists = false;
            for (auto& client : m_connections)
            {
                if (client && client->IsConnected())
                {
                    asio::error_code ec;
                    auto internetAddr = asio::ip::address::from_string(address, ec);
                    if (!ec && client->GetRemoteEndpoint().address() == internetAddr) {
                        client->Send(Message(msg));
                    }
                }
                else
                {
                    OnClientDisconnect(client);
                    client.reset();
                    bInvalidClientExists = true;
                }
            }
            if (bInvalidClientExists)
            {
                m_connections.erase(std::remove(std::begin(m_connections), std::end(m_connections), nullptr), std::end(m_connections));
            }
        });
    }

    void MessageClient(std::shared_ptr<CConnection> client, SMessage msg)
    {
        ConnectionsStrand.post(
            [this, client, msg = std::move(msg)]() mutable {
            if (client && client->IsConnected())
            {
                client->Send(Message(msg));
            }
            else
            {
                OnClientDisconnect(client);
                client.reset();
                m_connections.erase(std::remove(std::begin(m_connections), std::end(m_connections), client), std::end(m_connections));
            }
        });
    }

    void MessageAllClients(SMessage msg, std::shared_ptr<CConnection> pIgnoreClient = nullptr)
    {
        ConnectionsStrand.post(
            [this, msg = std::move(msg), pIgnoreClient]()mutable{
            bool bInvalidClientExists = false;
            for (auto& client : m_connections)
            {
                if (client && client->IsConnected())
                {
                    if (client != pIgnoreClient)
                        client->Send(Message(msg));
                }
                else
                {
                    OnClientDisconnect(client);
                    client.reset();
                    bInvalidClientExists = true;
                }
            }
            if (bInvalidClientExists)
            {
                m_connections.erase(std::remove(std::begin(m_connections), std::end(m_connections), nullptr), std::end(m_connections));
            }
        });
    }

    void Update(size_t maxMessages = -1)
    {
        size_t messageCount = 0;
        while (messageCount < maxMessages && !MessageToLocal.empty())
        {
            auto msg = MessageToLocal.pop_front();
            OnMessage(msg.remote, msg.msg);
            messageCount++;
            while (!SyncTaskQueue.empty())
            {
                SyncTaskQueue.front()();
                SyncTaskQueue.pop_front();
            }
        }
    }

protected:
    virtual bool OnClientConnect(std::shared_ptr<CConnection> client)
    {
        return true;
    }

    virtual void OnClientDisconnect(std::shared_ptr<CConnection> client)
    {
    }

    virtual void OnMessage(std::shared_ptr<CConnection> client, SMessage msg)
    {
    }

    TThreadSafeDeque<SMessagesTo>& MessageToLocal;
    std::deque<std::shared_ptr<CConnection>> m_connections;
    asio::io_context IoContext;
    asio::io_context::strand ConnectionsStrand;

    std::thread Thread;

    asio::ip::tcp::acceptor Acceptor;
    uint32_t IdCounter = 1;

    TTSQueue<std::function<void()>> SyncTaskQueue;
};
