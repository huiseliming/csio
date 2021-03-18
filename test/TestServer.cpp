#include "hslm_csio/Server.h"







int main()
{
	CServer Server;
	Server.GetMessageHandlerManager().RegisterMessageHandler(EMessageId::kTestMessage, std::make_unique<CTestMessageHandler>());

	Server.Start(4156);
	while (1)
	{
		Server.Update();
		std::this_thread::yield();
	}
}


