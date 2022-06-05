/**************************************************************
 * @file    utQueue.cpp
 * @brief   Unit test for thread save queue.
 * @author  Alzn
 * @date    2022-02-25
 *************************************************************/

#include "message.hpp"
#include "gtest/gtest.h"
#include <thread>

namespace {

TEST(utQueue, basePushPopTest) {
  Message::Queue<int> queue;
  const int testTimes = 100;
  for (int i = 0; i < testTimes; i++) {
    queue.push(i);
  }
  EXPECT_FALSE(queue.isEmpty());
  for (int i = 0; i < testTimes; i++) {
    int value = 0;
    ASSERT_TRUE(queue.popTo(value));
    ASSERT_EQ(value, i);
  }
}

TEST(utQueue, threadSafeTest) {
  typedef struct {
    int id;
    int value;
  } elementStruct;
  Message::Queue<elementStruct> queue;
  const int times = 1024;
  const int threadNum = 32;
  auto threadMain = [&queue](int id) {
    for (int i = 0; i < times; i++) {
      elementStruct element;
      element.id = id;
      element.value = i;
      queue.push(element);
    }
  };

  std::thread *threads[threadNum];

  int expectValue = 0;
  int value[threadNum];

  try {
    for (int i = 0; i < threadNum; i++) {
      threads[i] = new std::thread(threadMain, i);
      value[i] = 0;
    }
    for (int i = 0; i < times; i++) {
      expectValue += i;
    }
    for (int i = 0; i < threadNum * times; i++) {
      queue.wait();
      elementStruct element;
      ASSERT_TRUE(queue.popTo(element));
      if ((0 <= element.id) && (element.id < threadNum)) {
        value[element.id] += element.value;
      }
    }
    for (int i = 0; i < threadNum; i++) {
      threads[i]->join();
      delete threads[i];
      ASSERT_EQ(value[i], expectValue) << "Thread id = " << i;
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
}

} // namespace

/* END OF FILE */