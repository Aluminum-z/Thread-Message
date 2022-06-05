/**************************************************************
 * @file    message.cpp
 * @brief   Thread message library.
 * @author  Alzn
 * @date    2022-02-28
 *************************************************************/

#include "message.hpp"

namespace Message {
/**
 * @brief   Sync message handler for manager.
 * @author  Alzn
 * @date    2022-02-28
 */
void Sync::managerHandler(void) {
  std::unique_lock<std::mutex> lock(mLock);
  handler();
  mIsHandled = true;
  lock.unlock();
  mCondition.notify_one();
}

/**
 * @brief   Wait for the sync message to be processed.
 * @author  Alzn
 * @date    2022-02-28
 */
void Sync::wait(void) {
  std::unique_lock<std::mutex> lock(mLock);
  mCondition.wait(lock, [this]() { return mIsHandled; });
}

/**
 * @brief   Initialize befor send message.
 * @author  Alzn
 * @date    2022-02-28
 */
void Sync::init(void) {
  std::lock_guard<std::mutex> lock(mLock);
  mIsHandled = false;
}

/**
 * @brief   Async message handler for manager.
 * @author  Alzn
 * @date    2022-02-28
 */
void Async::managerHandler(void) {
  handler();
}

/**
 * @brief   Async message do not require waiting.
 * @author  Alzn
 * @date    2022-02-28
 */
void Async::wait(void) {
}

/**
 * @brief   Async message do not require initialization.
 * @author  Alzn
 * @date    2022-02-28
 */
void Async::init(void) {
}

/**
 * @brief   Send message to the thread that waiting the message.
 * @author  Alzn
 * @date    2022-02-28
 * @param   messagePtr A shared_ptr pointer to message data.
 */
void Manager::send(const std::shared_ptr<MessageBase> &messagePtr) {
  if (!messagePtr) {
    return;
  }
  messagePtr->init();
  mQueue.push(messagePtr);
  messagePtr->wait();
}

/**
 * @brief   Waiting for a message and handle it.
 * @author  Alzn
 * @date    2022-02-28
 */
void Manager::waitAndHandleOne(void) {
  mQueue.wait();
  std::shared_ptr<MessageBase> messagePtr;
  mQueue.popTo(messagePtr);
  if (messagePtr) {
    messagePtr->managerHandler();
  }
}

} // namespace Message

/* END OF FILE */