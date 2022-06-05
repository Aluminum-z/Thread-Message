## 1. 简介
Message库是一个用于线程间通信的简单事件库。

## 2. 功能
Message满足单进程多线程情况下，多个线程之间的事件通信，包含以下几种情况：
- 同步事件队列方式处理
- 异步事件队列方式处理
- 多线程发单线程收

## 3. 结构
- 消息队列：每一个线程用于缓存当前未能及时处理的事件消息的队列缓冲区；
- 消息基类：从该类派生出同步消息基类和异步消息基类，用户消息则再从同步、异步基类中派生；
- 消息管理：对基本的队列进行封装；

## 4. 设计
### 4.1 消息队列(Message::Queue)
消息队列在标准库队列的基础上实现线程安全功能，需要使用模板类的方式设计。接口设计如下：
- push: 将消息数据压入数据缓冲区中
  ```C++
  void push(T element)
  ```
- popTo: 将消息数据从队列中弹出至变量中，成功时返回true，失败时返回false
  ```C++
  bool popTo(T &element)
  ```
- isEmpty: 检查队列中是否为空，为空返回true，非空返回false
  ```C++
  bool isEmpty(void)
  ```
- wait: 阻塞当前线程，直到队列为非空状态
  ```C++
  void wait(void)
  ```

### 4.2 消息基类(Message::MessageBase)
消息基类是消息队列中存储的基本数据结构，同步消息基类和异步消息基类从该类派生，该类包含如下的接口：

\[公有接口\]

- managerHandler: 由消息管理调用的接口，异步消息以及同步消息在此函数内进行特化
  ```C++
  virtual void managerHandler(void) = 0;
  ```

- handler: 实际由用户定义的接口，用户在此函数内对消息进行处理
  ```C++
  virtual void handler(void) = 0;
  ```

- init: 初始化接口，用于初始化发送前需要初始化的内部状态
  ```C++
  virtual void init(void) = 0;
  ```

- wait: 等待消息被处理，只有同步消息才会进行等待，异步消息则直接退出
  ```C++
  virtual void wait(void) = 0;
  ```

### 4.3 同步消息基类(Message::Sync)]
用于声明同步消息的基类，包含如下接口：

\[私有接口\]
- managerHandler: 只能由消息管理器从基类调用该接口，该接口内再调用用户定义的处理函数
  ```C++
  void managerHandler(void);
  ```

- init: 由消息管理器从基类调用该接口，将初始化同步线程用的变量
  ```C++
  void init(void);
  ```

- wait: 由消息管理器从基类调用该接口，将等待消息的处理完成
  ```C++
  void wait(void);
  ```

\[公有接口\]
- handler: 用户定义的处理函数
  ```C++
  void handler(void);
  ```

### 4.4 异步消息基类(Message::Async)
用于声明异步消息的基类，包含如下接口：

\[私有接口\]
- managerHandler: 只能由消息管理器从基类调用该接口，该接口内再调用用户定义的处理函数
  ```C++
  void managerHandler(void);
  ```

- init: 由消息管理器从基类调用该接口，异步消息该接口将不执行任何代码
  ```C++
  void init(void);
  ```

- wait: 由消息管理器从基类调用该接口，异步消息该接口将不执行任何代码
  ```C++
  void wait(void);
  ```

\[公有接口\]
- handler: 用户定义的处理函数
  ```C++
  void handler(void);
  ```
  
### 4.5 消息管理类(Message::Manager)
消息管理类是队列的封装，便于消息的发送和处理，每一个线程有且仅有一个消息管理实例。
接口设计如下：

- send: 发送消息接口，其他线程将调用该接口向目标线程发送消息
  ```C++
  void send(std::shared_ptr<MessageBase> messagePtr);
  ```

- waitAndHandle: 等待并处理消息，接收线程用于循环中处理消息
  ```C++
  void waitAndHandle(void);
  ```

## 5. 使用
主要文件：
- include/message.hpp
- source/message.cpp

使用方法请参考单元测试代码。