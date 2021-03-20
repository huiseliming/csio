#include "hslm_csio/Client.h"



int main()
{
	int MessageCounter = 0;
	std::vector<uint8_t> DataToSend = { 'a', 'b', 'c', 'd' };
	CClient	Client;
	Client.Connect("127.0.0.1",4156);
	Client.WaitConnected();
	while (Client.GetConnectionState() == CConnection::EState::Connected)
	{
		Client.Update();
		if (MessageCounter > 5)
		{
			Client.MessageServer(SMessage(EMessageId::kCloseConnection));
			break;
		}
		Client.MessageServer(SMessage(EMessageId::kTestMessage, DataToSend));
		for (size_t i = 0; i < DataToSend.size(); i++)
		{
			DataToSend[i] += 4;
		}
		MessageCounter++;
		std::this_thread::yield();
	}
	if(Client.GetConnectionState() == CConnection::EState::ConnectFailed)
	{
		std::cout << "[Client] ConnectFailed!\n";
	}
	system("pause");
}


