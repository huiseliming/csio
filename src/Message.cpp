#include "Message.h"

SMessage::SMessage()
{
	Header.MessageId = 0;
	Header.DataSize = 0;
	Data.resize(0);
}

SMessage::SMessage(uint32_t MessageId)
{
	Header.MessageId = MessageId;
}

SMessage::SMessage(EMessageId MessageId)
{
	Header.MessageId = uint32_t(MessageId);
}

SMessage::SMessage(const SMessage& Message)
{
	Header = Message.Header;
	Data = Message.Data;
}

SMessage::SMessage(SMessage&& Message)
{
	Header = Message.Header;
	Data = std::move(Message.Data);
}

SMessage& SMessage::operator=(const SMessage& Message)
{
	if (std::addressof(Message) != this)
	{
		Header = Message.Header;
		Data = Message.Data;
	}
	return *this;
}

SMessage& SMessage::operator=(SMessage&& Message)
{
	if(std::addressof(Message) != this)
	{
		Header = Message.Header;
		Data = std::move(Message.Data);
	}
	return *this;
}

void SMessage::Reset()
{
	Header.MessageId = 0;
	Header.DataSize = 0;
	Data.resize(0);
}

void SMessage::UpdateDataSize()
{
	Data.resize(Header.DataSize);
}
