/**************************************************************
 * @file    utMessage.cpp
 * @brief   Unit test for message library.
 * @author  Alzn
 * @date    2022-02-28
 *************************************************************/

#include "message.hpp"
#include "gtest/gtest.h"
#include <random>
#include <thread>

namespace {
class TestDataManager {
private:
  TestDataManager(){};
  ~TestDataManager(){};
  TestDataManager(const TestDataManager &);
  const TestDataManager &operator=(const TestDataManager &);

  std::vector<uint32_t> mValueVector;
  size_t mSize;

public:
  static TestDataManager &getInstance() {
    static TestDataManager instance;
    return instance;
  }

  void clear(void) {
    mValueVector.clear();
  }

  void setSize(size_t size) {
    mValueVector.reserve(size);
    mSize = size;
  }

  void addValue(uint32_t id, uint32_t value) {
    if (id < mSize) {
      mValueVector[id] += value;
    }
  }

  void resetValue(void) {
    for (size_t i = 0; i < mSize; i++) {
      mValueVector[i] = 0;
    }
  }

  uint32_t getValue(uint32_t id) {
    if (id < mSize) {
      return mValueVector[id];
    }
    return 0;
  }
};

/* Asynchronized Message Test */
class AsyncMessageTest : public testing::Test {
  void SetUp(void) {
    TestDataManager &dataManager = TestDataManager::getInstance();
    dataManager.clear();
  }

  void TearDown(void) {
    TestDataManager &dataManager = TestDataManager::getInstance();
    dataManager.clear();
  }
};

TEST_F(AsyncMessageTest, baseSendTest) {
  const uint32_t threadNum = 10;
  const uint32_t times = 1000;

  /* Main thread <- Test thread */
  class AsyncMessage : public Message::Async {
  public:
    uint32_t id;
    uint32_t value;
    void handler(void) {
      if (id < threadNum) {
        TestDataManager &dataManager = TestDataManager::getInstance();
        dataManager.addValue(id, value);
      }
    }
  };

  Message::Manager msgManager;

  auto testThreadMain = [&msgManager](uint32_t id) {
    for (uint32_t i = 0; i < times; i++) {
      int delayValue = std::rand() / ((RAND_MAX + 1u) / 100);
      std::chrono::microseconds delayUs(delayValue);
      std::this_thread::sleep_for(delayUs);
      std::shared_ptr<AsyncMessage> msgPtr = std::make_shared<AsyncMessage>();
      msgPtr->id = id;
      msgPtr->value = i;
      msgManager.send(msgPtr);
    }
  };

  TestDataManager &dataManager = TestDataManager::getInstance();
  dataManager.setSize(threadNum);
  dataManager.resetValue();

  try {
    std::thread *pThreadArr[threadNum];
    for (uint32_t i = 0; i < threadNum; i++) {
      pThreadArr[i] = new std::thread(testThreadMain, i);
    }
    for (uint32_t i = 0; i < threadNum; i++) {
      for (uint32_t i = 0; i < times; i++) {
        msgManager.waitAndHandleOne();
      }
    }
    for (uint32_t i = 0; i < threadNum; i++) {
      if (nullptr == pThreadArr[i]) {
        continue;
      }
      pThreadArr[i]->join();
      delete pThreadArr[i];
      pThreadArr[i] = nullptr;
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  uint32_t expectValue = 0;
  for (uint32_t i = 0; i < times; i++) {
    expectValue += i;
  }
  for (uint32_t i = 0; i < threadNum; i++) {
    ASSERT_EQ(dataManager.getValue(i), expectValue) << "ID = " << i;
  }
}

/* Synchronized Message Test */
class SyncMessageTest : public testing::Test {
  void SetUp(void) {
    TestDataManager &dataManager = TestDataManager::getInstance();
    dataManager.clear();
  }

  void TearDown(void) {
    TestDataManager &dataManager = TestDataManager::getInstance();
    dataManager.clear();
  }
};

TEST_F(SyncMessageTest, baseSendTest) {
  const uint32_t threadNum = 10;
  const uint32_t times = 1000;
  const uint32_t waitTimes = times / 2;

  /* Test thread -> Main thread */
  class syncMessage : public Message::Sync {
  public:
    int mSendValue;
    int mReturnValue;
    bool mWait;
    void handler(void) {
      if (mWait) {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
      }
      mReturnValue = mSendValue;
    }
  };

  Message::Manager msgManager;

  auto testThreadMain = [&msgManager](uint32_t id) {
    TestDataManager &dataManager = TestDataManager::getInstance();
    for (uint32_t i = 0; i < times; i++) {
      std::shared_ptr<syncMessage> msgPtr = std::make_shared<syncMessage>();
      msgPtr->mWait = (times > waitTimes);
      msgPtr->mSendValue = i;
      msgPtr->mReturnValue = 0;
      msgManager.send(msgPtr);
      dataManager.addValue(id, msgPtr->mReturnValue);
    }
  };

  TestDataManager &dataManager = TestDataManager::getInstance();
  dataManager.setSize(threadNum);
  dataManager.resetValue();

  try {
    std::thread *pThreadArr[threadNum];
    for (uint32_t i = 0; i < threadNum; i++) {
      pThreadArr[i] = new std::thread(testThreadMain, i);
    }
    for (uint32_t i = 0; i < threadNum; i++) {
      for (uint32_t i = 0; i < times; i++) {
        msgManager.waitAndHandleOne();
      }
    }
    for (uint32_t i = 0; i < threadNum; i++) {
      if (nullptr == pThreadArr[i]) {
        continue;
      }
      pThreadArr[i]->join();
      delete pThreadArr[i];
      pThreadArr[i] = nullptr;
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }

  uint32_t expectValue = 0;
  for (uint32_t i = 0; i < times; i++) {
    expectValue += i;
  }
  for (uint32_t i = 0; i < threadNum; i++) {
    ASSERT_EQ(dataManager.getValue(i), expectValue) << "ID = " << i;
  }
}

} // namespace

/* END OF FILE */