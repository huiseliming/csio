#pragma once



class Message;


class MessageHandler
{
public:
    MessageHandler() = default;
    ~MessageHandler() = default;
    void operator()(Message message)
    {
        return true;
    }
};