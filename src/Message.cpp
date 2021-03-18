#include "hslm_csio/Message.h"

SMessage::SMessage()
{
	Header.MessageId = 0;
	Header.DataSize = 0;
	Data.resize(0);
}

SMessage::SMessage(EMessageId MessageId)
{
	Header.MessageId = uint32_t(MessageId);
}

SMessage::SMessage(EMessageId MessageId, std::vector<uint8_t>& Data)
{
	Header.MessageId = uint32_t(MessageId);
	this->Data = Data;
}
SMessage::SMessage(EMessageId MessageId, std::vector<uint8_t>&& Data)
{
	Header.MessageId = uint32_t(MessageId);
	this->Data = std::move(Data);
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

void SMessage::GenerateHeaderDataSize()
{
	Header.DataSize = Data.size();
}

void SMessage::ResizeDataSize()
{
	Data.resize(Header.DataSize);
}
