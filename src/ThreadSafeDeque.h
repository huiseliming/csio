#pragma once
#include <mutex>
#include <deque>

template<typename T>
class TThreadSafeDeque;

template<typename T>
using TTSQueue = TThreadSafeDeque<T>;

template<typename T>
class TThreadSafeDeque
{
public:
    TThreadSafeDeque() = default;
    TThreadSafeDeque(const TThreadSafeDeque&) = delete;
    TThreadSafeDeque& operator=(const TThreadSafeDeque&) = delete;
    virtual ~TThreadSafeDeque() = default;

    const T& front()
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.front();
    }

    const T& back()
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.back();
    }

    void push_front(const T& item)
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.push_front(item);
    }

    void push_back(const T& item)
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.push_back(item);
    }

    decltype(auto) emplace_front(T&& item)
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.emplace_front(std::forward<T>(item));
    }

    decltype(auto) emplace_back(T&& item)
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.emplace_back(std::forward<T>(item));
    }

    bool empty()
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.empty();
    }

    size_t size()
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.size();
    }

    void clear()
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        return Deque.clear();
    }

    T pop_front()
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        auto front = std::move(Deque.front());
        Deque.pop_front();
        return front;
    }

    T pop_back()
    {
        std::scoped_lock<std::mutex> Lock(Mutex);
        auto back = std::move(Deque.back());
        Deque.pop_back();
        return back;
    }

protected:
    std::deque<T> Deque;
    std::mutex Mutex;
};
