#include "hslm_csio/Server.h"

CServer::CServer()
	: Acceptor(IoContext)
	, ConnectionsStrand(IoContext)
{

}

CServer::~CServer()
{
	Stop();
}

bool CServer::Start(uint16_t Port, uint32_t NumThread)
{
	try
	{
		auto Endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), Port);
		Acceptor.open(Endpoint.protocol());
		Acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
		Acceptor.bind(Endpoint);
		Acceptor.listen();
		std::cout << "[Server] Start!" << std::endl;
		WaitForClientConnection();
		if (NumThread == 0)
			NumThread = std::thread::hardware_concurrency();
		for (size_t i = 0; i < NumThread; i++)
		{
			Threads.emplace_back(std::move(std::thread([this] { IoContext.run(); })));
		}
	}
	catch (const std::exception& Expection)
	{
		std::cout << "[Server] Exception: " << Expection.what() << std::endl;
		return false;
	}
	return true;
}

void CServer::Stop()
{
	IoContext.stop();
	WaitThreadJoin();
	std::cout << "[Server] Stopped!" << std::endl;
}

void CServer::WaitThreadJoin()
{
	for (auto& Thread : Threads)
	{
		if (Thread.joinable())
			Thread.join();
	}
}

void CServer::WaitForClientConnection()
{
	Acceptor.async_accept(
		[this](std::error_code ErrorCode, asio::ip::tcp::socket Socket)
		{
			if (!ErrorCode)
			{

				std::cout << "[Server] New Connection: " << Socket.remote_endpoint() << std::endl;
				std::shared_ptr<CConnection> NewConnection = std::make_shared<CConnection>(CConnection::EOwner::kServer, IoContext, std::move(Socket), MessageToLocal, this);
				auto RemoteEndpoint = NewConnection->GetRemoteEndpoint();
				if (OnClientConnect(NewConnection))
				{
					ConnectionsStrand.post([this, NewConnection, RemoteEndpoint] {
						auto Uint64Ip = NetworkHelper::EndpointToUint64Id(RemoteEndpoint);
						if (NewConnection->ConnectToClient(Uint64Ip))
						{
							auto Result = ConnectionMap.emplace(RemoteEndpoint.address().to_v4().to_uint(), std::vector<std::shared_ptr<CConnection>>{std::move(NewConnection)});
							if (!Result.second)
							{
								Result.first->second.emplace_back(std::move(NewConnection));
							}
							std::cout << "[Server] " << "[ID:" << Uint64Ip << "] Connection Approved" << std::endl;
						}
						});
				}
				else
				{
					std::cout << "[Server] " << RemoteEndpoint << " Connection Denied" << std::endl;
				}
				WaitForClientConnection();
			}
			else
			{
				std::cout << "[Server] Accept Connection Error: " << ErrorCode.message() << std::endl;
			}
		});
}

void CServer::ServerSharedReferenceRemove(std::shared_ptr<CConnection> Client)
{
	ConnectionsStrand.post(
		[this, Client]() {
			auto Endpoint = Client->GetRemoteEndpoint();
			auto Iterator = ConnectionMap.find(Endpoint.address().to_v4().to_uint());
			assert(Iterator != std::end(ConnectionMap));
			for (auto& Client : Iterator->second)
			{
				if ((Endpoint.port() == Client->GetRemoteEndpoint().port()))
				{
					Client.reset();
					Iterator->second.erase(std::remove(std::begin(Iterator->second), std::end(Iterator->second), nullptr), std::end(Iterator->second));
					break;
				}
			}
		}
	);
}

void CServer::MessageClient(SMessage&& Msg, std::string Address, uint16_t Port)
{
	ConnectionsStrand.post(
		[this, Port, Msg = std::move(Msg), AddressString = std::move(Address)]() mutable {
		asio::error_code ErrorCode;
		auto Address = asio::ip::address::from_string(AddressString, ErrorCode);
		if (ErrorCode)
		{
			std::cout << "[Server] Parse Address Error: " << ErrorCode.message() << std::endl;
			return;
		}
		auto Iterator = ConnectionMap.find(Address.to_v4().to_uint());
		if (Iterator != std::end(ConnectionMap))
		{
			for (auto Client : Iterator->second)
			{
				if ((Port == 0 || Port == Client->GetRemoteEndpoint().port()))
				{
					Client->SendMessageToRemote(std::move(Msg));
				}
			}
		}
	}
	);
}

void CServer::MessageClient(SMessage&& Msg, uint32_t Uint32Address, uint16_t Port)
{
	ConnectionsStrand.post(
		[this, Port, Msg = std::move(Msg), Uint32Address = std::move(Uint32Address)]() mutable {
		auto Iterator = ConnectionMap.find(Uint32Address);
		if (Iterator != std::end(ConnectionMap))
		{
			for (auto Client : Iterator->second)
			{
				if ((Port == 0 || Port == Client->GetRemoteEndpoint().port()))
				{
					Client->SendMessageToRemote(std::move(Msg));
				}
			}
		}
	}
	);
}

void CServer::MessageClient(SMessage&& Msg, std::shared_ptr<CConnection> Client)
{
	ConnectionsStrand.post(
		[this, Client, Msg = std::move(Msg)]() mutable {
		Client->SendMessageToRemote(std::move(Msg));
	}
	);
}

void CServer::MessageAllClients(SMessage&& Msg, std::shared_ptr<CConnection> IgnoreClientPtr)
{
	ConnectionsStrand.post(
		[this, Msg = std::move(Msg), IgnoreClientPtr]()mutable{
		for (auto& Iterator : ConnectionMap)
		{
			for (auto Client : Iterator.second)
			{
				if (Client != IgnoreClientPtr)
				{
					Client->SendMessageToRemote(Msg);
				}
			}
		}
	}
	);
}

void CServer::Update(size_t MaxMessages)
{
	size_t MessageCount = 0;
	while (MessageCount < MaxMessages && !MessageToLocal.empty())
	{
		auto MsgTo = std::move(MessageToLocal.pop_front());
		// If is has forward bit
		if ((uint32_t(EMessageId::kForwardMessageBit) & MsgTo.Message.Header.MessageId) != 0) {
			uint32_t DataSize = MsgTo.Message.Data.size();
			uint8_t* DataPtr = MsgTo.Message.Data.data();
			uint32_t Uint32IP = *(uint32_t*)(DataPtr + DataSize - sizeof(uint32_t));
			uint16_t Port = *(uint16_t*)(DataPtr + DataSize - sizeof(uint32_t) - sizeof(uint16_t));
			MsgTo.Message.Data.resize(MsgTo.Message.Data.size() - sizeof(uint32_t) - sizeof(uint16_t));
			MessageClient(std::move(MsgTo.Message), Uint32IP, Port);
		}
		else
		{
			//Find Handler
			IMessageHandler* MessageHandler = MessageHandlerManager.GetMessageHandler(MsgTo.Message.Header.MessageId);
			if (MessageHandler != nullptr) {
				(*MessageHandler)(std::move(MsgTo.Message.Data));
			}
			else {
				OnMessage(MsgTo.RemoteConnection, std::move(MsgTo.Message));
			}
		}
		MessageCount++;
		while (!SyncTaskQueue.empty())
		{
			SyncTaskQueue.pop_front()();
		}
	}
}

bool CServer::OnClientConnect(std::shared_ptr<CConnection> Client)
{
	ConnectionCounter++;
	std::cout << "[Server] CurrentConnection: " << ConnectionCounter << "\n";
	return true;
}

void CServer::OnClientDisconnect(std::shared_ptr<CConnection> Client)
{
	ConnectionCounter--;
	std::cout << "[Server] CurrentConnection: " << ConnectionCounter << "\n";
}

void CServer::OnMessage(std::shared_ptr<CConnection> Client, SMessage&& Msg)
{
	std::cout << "[Server] OnMessage: MessageId(" << Msg.Header.MessageId << ") DataSize(" << Msg.Header.DataSize << ")\n";
}


CMessageHandlerManager& CServer::GetMessageHandlerManager()
{
	return MessageHandlerManager;
}







