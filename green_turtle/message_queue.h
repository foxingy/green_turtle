#ifndef __MESSAGE_QUEUE__
#define __MESSAGE_QUEUE__
#include <assert.h>
#include <cstddef>
#include <atomic>
#include <noncopyable.h>

namespace green_turtle{

//support POD data only
//1:1 MessageQueue
//N:1 equals N*(1:1)
template<class T>
class MessageQueue : NonCopyable
{
 public:
  typedef T value_type;
 public:
  MessageQueue(size_t size = 128*1024)
    : read_idx_(0)
    , write_idx_(0)
    , size_(size)
    , array_(new T[size]())
  {
    assert(size >= 2);
    assert(array_);
  }

  ~MessageQueue()
  {
    delete[] array_;
  }

  bool Push(const value_type& v)
  {
    size_t const current = write_idx_.load(std::memory_order_relaxed);
    size_t next = current + 1;
    if(next == size_)
    {
      next = 0;
    }

    if (next != read_idx_.load(std::memory_order_acquire))
    {
      array_[current] = v;
      write_idx_.store(next, std::memory_order_release);
      return true;
    }

    return false;
  }

  bool Pop(value_type& v)
  {
    size_t const current = read_idx_.load(std::memory_order_relaxed);
    if (current == write_idx_.load(std::memory_order_acquire))
    {
      return false;
    }

    size_t next = current + 1;
    if(next == size_)
    {
      next = 0;
    }
    v = array_[current];
    read_idx_.store(next, std::memory_order_release);
    return true;
  }
private:
  std::atomic<size_t> read_idx_;
  std::atomic<size_t> write_idx_;
  const size_t        size_;
  value_type          *array_;
};

}
#endif