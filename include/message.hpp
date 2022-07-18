/**************************************************************
 * @file    message.hpp
 * @brief   Thread message library.
 * @author  Alzn
 * @date    2022-02-25
 **************************************************************/
#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace Message {

template <typename T>
class Queue {
private:
  std::queue<T> mStdQueue;
  std::mutex mLock;
  std::condition_variable mCondition;

public:
  /**
   * @brief   Push one element into queue
   * @author  Alzn
   * @date    2022-02-25
   * @param   element element to be pushed.
   */
  void push(const T &element) {
    std::unique_lock<std::mutex> lock(mLock);
    mStdQueue.push(element);
    lock.unlock();
    mCondition.notify_one();
  }

  /**
   * @brief   Get the first element in the queue and pop it.
   * @author  Alzn
   * @date    2022-02-25
   * @param   element Reference of the element need pop to.
   * @return  bool, true if succeed, false if failed.
   */
  bool popTo(T &element) {
    std::lock_guard<std::mutex> lock(mLock);
    if (mStdQueue.empty()) {
      return false;
    }
    element = mStdQueue.front();
    mStdQueue.pop();
    return true;
  }

  /**
   * @brief   Return true if queue is empty.
   * @author  Alzn
   * @date    2022-02-25
   */
  bool isEmpty(void) {
    std::unique_lock<std::mutex> lock(mLock);
    return mStdQueue.empty();
  }

  /**
   * @brief   Wait for any elements to be pushed into the queue.
   * @author  Alzn
   * @date    2022-02-25
   */
  void wait(void) {
    std::unique_lock<std::mutex> lock(mLock);
    mCondition.wait(lock, [this]() { return !mStdQueue.empty(); });
  }
};

class MessageBase {
public:
  virtual ~MessageBase() = default;
  virtual void managerHandler(void) = 0;
  virtual void handler(void) = 0;
  virtual void init(void) = 0;
  virtual void wait(void) = 0;
};

class Sync : public MessageBase {
private:
  std::mutex mLock;
  std::condition_variable mCondition;
  volatile bool mIsHandled;

  void managerHandler(void);
  void init(void);
  void wait(void);
public:
  virtual ~Sync() = default;
};

class Async : public MessageBase {
private:
  void managerHandler(void);
  void init(void);
  void wait(void);
public:
  virtual ~Async() = default;
};

class Manager {
private:
  const Manager &operator=(const Manager &);

  Queue<std::shared_ptr<MessageBase>> mQueue;

public:
  void send(const std::shared_ptr<MessageBase> &messagePtr);
  void waitAndHandleOne(void);
};

} // namespace message

/* END OF FILE */