#include "hslm_csio/Client.h"


CClient::CClient()
    :IdleWorkPtr(new asio::io_context::work(IoContext))
{
    Thread = std::move(std::thread([this] { IoContext.run(); }));
}

CClient::~CClient()
{
    IdleWorkPtr.reset();
    Disconnect();
    ConnectionPtr.reset();
    if (Thread.joinable())
        Thread.join();
}

bool CClient::Connect(const std::string Host, const uint16_t Port)
{
    try
    {
        asio::ip::tcp::resolver Resolver(IoContext);
        auto Endpoints = Resolver.resolve(Host, std::to_string(Port));
        ConnectionPtr = std::make_shared<CConnection>(CConnection::EOwner::kClient, IoContext, asio::ip::tcp::socket(IoContext), MessageToLocal);
        ConnectionPtr->ConnectToServer(Endpoints);
    }
    catch (const std::exception& Exception)
    {
        std::cout << "[Client] Exception: " << Exception.what() << std::endl;
        return false;
    }
    return true;
}

void CClient::Disconnect()
{
    if (ConnectionPtr && ConnectionPtr->IsConnected())
        ConnectionPtr->Disconnect();
}

CConnection::EState CClient::GetConnectionState()
{
    return ConnectionPtr->GetState();
}

void CClient::WaitConnected()
{
    while (ConnectionPtr->GetState() == CConnection::EState::Connecting)
    {
        std::this_thread::yield();
    }
}

void CClient::MessageServer(SMessage&& Msg)
{
    if (ConnectionPtr->IsConnected())
        ConnectionPtr->SendMessageToRemote(std::move(Msg));
}

void CClient::Update(size_t MaxMessages)
{
    size_t MessageCount = 0;
    while (MessageCount < MaxMessages && !MessageToLocal.empty())
    {
        auto MsgTo = std::move(MessageToLocal.pop_front());
        IMessageHandler* MessageHandler = MessageHandlerManager.GetMessageHandler(MsgTo.Message.Header.MessageId);
        if (MessageHandler != nullptr) {
            (*MessageHandler)(std::move(MsgTo.Message.Data));
        }
        else {
            OnMessage(MsgTo.RemoteConnection, std::move(MsgTo.Message));
        }
        MessageCount++;
    }
}

void CClient::OnMessage(std::shared_ptr<CConnection> Connection, SMessage&& Msg)
{
    std::cout << "[Client] OnMessage: MessageId(" << Msg.Header.MessageId << ") DataSize(" << Msg.Header.DataSize << ")\n";
}
