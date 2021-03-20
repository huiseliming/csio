#include "hslm_csio/Server.h"







int main()
{
	CServer Server;
	Server.GetMessageHandlerManager().RegisterMessageHandler(EMessageId::kCloseConnection, std::make_unique<CCloseConnectionHandler>());
	Server.GetMessageHandlerManager().RegisterMessageHandler(EMessageId::kTestMessage, std::make_unique<CTestHandler>());
	Server.Start(4156);
	while (1)
	{
		Server.Update();
		std::this_thread::yield();
	}
}


