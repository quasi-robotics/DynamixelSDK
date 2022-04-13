#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace quasi {

template<typename T>
class MsgQueue
{
public:
  bool push(T const& _data)  {
    {
        std::lock_guard<std::mutex> lock(guard);
        queue.push(_data);
    }
    signal.notify_one();
    return true;
  }

  bool empty() const  {
    std::lock_guard<std::mutex> lock(guard);
    return queue.empty();
  }
  void pop(T& _value) {
    std::unique_lock<std::mutex> lock(guard);
    while (queue.empty()) {
        signal.wait(lock);
    }

    _value = queue.front();
    queue.pop();
  }

  template< class Rep, class Period>
  bool pop(T& _value, const std::chrono::duration<Rep, Period>& rel_time)
  {
    std::unique_lock<std::mutex> lock(guard);
    while (queue.empty()) {
        signal.wait_for(lock, rel_time);
        return false;
    }

    _value = queue.front();
    queue.pop();
    return true;
  }

private:
    std::queue<T> queue;
    mutable std::mutex guard;
    std::condition_variable signal;
};

}