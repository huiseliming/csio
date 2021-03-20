#include "hslm_csio/Client.h"



int main()
{
	int MessageCounter = 0;
	CClient	Client;
	Client.Connect("127.0.0.1",4156);
	Client.WaitConnected();
	while (Client.GetConnectionState() == CConnection::EState::Connected)
	{
		Client.Update();
		if (MessageCounter > 100)
		{
			Client.MessageServer(SMessage(EMessageId::kCloseConnection));
			Client.Disconnect();
			break;
		}
		Client.MessageServer(SMessage(EMessageId::kTestMessage, { 'T', 'e', 's', 't' }));
		MessageCounter++;
		std::this_thread::yield();
	}
	if(Client.GetConnectionState() == CConnection::EState::ConnectFailed)
	{
		std::cout << "[Client] ConnectFailed!\n";
	}
	system("pause");
}


