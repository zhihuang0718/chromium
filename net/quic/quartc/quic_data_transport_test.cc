// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/quartc/quic_data_transport.h"

#include <queue>

#include "net/quic/test_tools/mock_clock.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace net {

namespace {

class MockQuartcClock : public QuartcClockInterface {
 public:
  explicit MockQuartcClock(MockClock* clock) : clock_(clock) {}

  int64_t NowMicroseconds() override {
    return clock_->WallNow().ToUNIXMicroseconds();
  }

 private:
  MockClock* clock_;
};

// Single-threaded scheduled task runner based on a MockClock.
//
// Simulates asynchronous execution on a single thread by holding scheduled
// tasks until Run() is called. Performs no synchronization, assumes that
// Schedule() and Run() are called on the same thread.
class FakeTaskRunner : public QuartcTaskRunnerInterface {
 public:
  explicit FakeTaskRunner(MockClock* clock)
      : tasks_([this](const TaskType& l, const TaskType& r) {
          // Items at a later time should run after items at an earlier time.
          // Priority queue comparisons should return true if l appears after r.
          return l->time() > r->time();
        }),
        clock_(clock) {}

  ~FakeTaskRunner() override {}

  // Runs all tasks scheduled in the next total_ms milliseconds.  Advances the
  // clock by total_ms.  Runs tasks in time order.  Executes tasks scheduled at
  // the same in an arbitrary order.
  void Run(uint32_t total_ms) {
    for (uint32_t i = 0; i < total_ms; ++i) {
      while (!tasks_.empty() && tasks_.top()->time() <= clock_->Now()) {
        tasks_.top()->Run();
        tasks_.pop();
      }
      clock_->AdvanceTime(QuicTime::Delta::FromMilliseconds(1));
    }
  }

 private:
  class InnerTask {
   public:
    InnerTask(std::function<void()> task, QuicTime time)
        : task_(std::move(task)), time_(time) {}

    void Cancel() { cancelled_ = true; }

    void Run() {
      if (!cancelled_) {
        task_();
      }
    }

    QuicTime time() const { return time_; }

   private:
    bool cancelled_ = false;
    std::function<void()> task_;
    QuicTime time_;
  };

 public:
  // Hook for cancelling a scheduled task.
  class ScheduledTask : public QuartcTaskRunnerInterface::ScheduledTask {
   public:
    explicit ScheduledTask(std::shared_ptr<InnerTask> inner)
        : inner_(std::move(inner)) {}

    // Cancel if the caller deletes the ScheduledTask.  This behavior is
    // consistent with the actual task runner Quartc uses.
    ~ScheduledTask() override { Cancel(); }

    // ScheduledTask implementation.
    void Cancel() override { inner_->Cancel(); }

   private:
    std::shared_ptr<InnerTask> inner_;
  };

  // See QuartcTaskRunnerInterface.
  std::unique_ptr<QuartcTaskRunnerInterface::ScheduledTask> Schedule(
      Task* task,
      uint64_t delay_ms) override {
    auto inner = std::shared_ptr<InnerTask>(new InnerTask(
        [task] { task->Run(); },
        clock_->Now() + QuicTime::Delta::FromMilliseconds(delay_ms)));
    tasks_.push(inner);
    return std::unique_ptr<QuartcTaskRunnerInterface::ScheduledTask>(
        new ScheduledTask(inner));
  }

  // Schedules a function to run immediately.
  void Schedule(std::function<void()> task) {
    tasks_.push(std::shared_ptr<InnerTask>(
        new InnerTask(std::move(task), clock_->Now())));
  }

 private:
  // InnerTasks are shared by the queue and ScheduledTask (which hooks into it
  // to implement Cancel()).
  using TaskType = std::shared_ptr<InnerTask>;
  std::priority_queue<TaskType,
                      std::vector<TaskType>,
                      std::function<bool(const TaskType&, const TaskType&)>>
      tasks_;
  MockClock* clock_;
};

class FakeTransportObserver {
 public:
  virtual ~FakeTransportObserver() {}

  // Called when the other peer is trying to send message.
  virtual void OnTransportReadPacket(const std::string& data) = 0;
};

// Used by the QuartcPacketWriter.
class FakeTransport : public QuartcSessionInterface::PacketTransport {
 public:
  explicit FakeTransport(FakeTaskRunner* task_runner, MockClock* clock)
      : task_runner_(task_runner), clock_(clock) {}

  void SetDestination(FakeTransport* dest) {
    if (!dest_) {
      dest_ = dest;
      dest_->SetDestination(this);
    }
  }

  FakeTransportObserver* observer() { return observer_; }

  void SetObserver(FakeTransportObserver* observer) { observer_ = observer; }

  void SetAsync(bool async) { async_ = async; }

  bool CanWrite() override { return true; }

  int Write(const char* data, size_t len) override {
    // If the destination is not set.
    if (!dest_) {
      return -1;
    }
    // Advance the time 10us to ensure the RTT is never 0ms.
    clock_->AdvanceTime(QuicTime::Delta::FromMicroseconds(10));
    if (async_ && task_runner_) {
      std::string packet(data, len);
      task_runner_->Schedule([this, packet] { send(packet); });
    } else {
      send(std::string(data, len));
    }
    return static_cast<int>(len);
  }

  void send(const std::string& data) {
    DCHECK(dest_);
    DCHECK(dest_->observer());
    dest_->observer()->OnTransportReadPacket(data);
  }

 private:
  // The writing destination of this channel.
  FakeTransport* dest_ = nullptr;
  // The observer of this channel. Called when the received the data.
  FakeTransportObserver* observer_ = nullptr;
  // If async, will send packets by running asynchronous tasks.
  bool async_ = false;
  // Used to send data asynchronously.
  FakeTaskRunner* task_runner_;
  // The test clock.  Used to ensure the RTT is not 0.
  MockClock* clock_;
};

class QuicDataTransportDelegate : public QuicDataTransport::Delegate,
                                  public QuartcStreamInterface::Delegate {
 public:
  ~QuicDataTransportDelegate() override {}

  void OnIncomingStream(QuartcStreamInterface* stream) override {
    stream->SetDelegate(this);
    incoming_streams_.push_back(std::move(stream));
  }

  void OnReceived(QuartcStreamInterface* stream,
                  const char* data,
                  size_t size) override {
    last_received_data_ = std::string(data, size);
  }

  void OnClose(QuartcStreamInterface* stream) override {}

  void OnCanWrite(QuartcStreamInterface* stream) override {}

  std::string data() { return last_received_data_; }

  size_t num_incoming_streams() { return incoming_streams_.size(); }

 private:
  std::vector<QuartcStreamInterface*> incoming_streams_;
  std::string last_received_data_;
};

class QuicDataTransportForTest : public QuicDataTransport,
                                 public FakeTransportObserver {
 public:
  explicit QuicDataTransportForTest(
      bool is_server,
      QuartcSessionInterface::PacketTransport* packet_transport)
      : QuicDataTransport(is_server, packet_transport) {
    static_cast<FakeTransport*>(packet_transport)->SetObserver(this);
  }

  // FakeTransportObserver override.
  void OnTransportReadPacket(const std::string& data) override {
    quartc_session()->OnTransportReceived(data.c_str(), data.length());
  }
};

class QuicDataTransportTest : public ::testing::Test {
 public:
  QuicDataTransportTest() {
    clock_.AdvanceTime(QuicTime::Delta::FromMilliseconds(1000));
    client_packet_transport_.reset(new FakeTransport(&task_runner_, &clock_));
    server_packet_transport_.reset(new FakeTransport(&task_runner_, &clock_));

    client_packet_transport_->SetAsync(true);
    client_packet_transport_->SetDestination(server_packet_transport_.get());

    client_.reset(
        new QuicDataTransportForTest(false, client_packet_transport_.get()));
    server_.reset(
        new QuicDataTransportForTest(true, server_packet_transport_.get()));

    client_->SetDelegate(&client_data_transport_delegate_);
    server_->SetDelegate(&server_data_transport_delegate_);
  }

  ~QuicDataTransportTest() override {
    client_packet_transport_.reset();
    server_packet_transport_.reset();
    client_.reset();
    server_.reset();
  }

  void TestCryptoHandshake() {
    client_->Connect();
    RunTasks();
  }

  void TestSendAndRecvMessage() {
    auto* client_stream = client_->CreateQuicDataStream();
    EXPECT_NE(client_stream, nullptr);
    const char kTestMessage1[] = "Hello Server";
    client_stream->Write(kTestMessage1, strlen(kTestMessage1),
                         QuartcStreamInterface::WriteParameters());
    RunTasks();
    EXPECT_EQ(1u, server_data_transport_delegate_.num_incoming_streams());
    EXPECT_EQ(kTestMessage1, server_data_transport_delegate_.data());

    auto* server_stream = server_->CreateQuicDataStream();
    EXPECT_NE(server_stream, nullptr);
    const char kTestMessage2[] = "Hello Client";
    server_stream->Write(kTestMessage2, strlen(kTestMessage2),
                         QuartcStreamInterface::WriteParameters());
    RunTasks();
    EXPECT_EQ(1u, client_data_transport_delegate_.num_incoming_streams());
    EXPECT_EQ(kTestMessage2, client_data_transport_delegate_.data());
  }

  void RunTasks() { task_runner_.Run(200); }

 private:
  MockClock clock_;
  MockQuartcClock quartc_clock_{&clock_};
  FakeTaskRunner task_runner_{&clock_};

  std::unique_ptr<FakeTransport> client_packet_transport_;
  std::unique_ptr<FakeTransport> server_packet_transport_;
  std::unique_ptr<QuicDataTransportForTest> client_;
  std::unique_ptr<QuicDataTransportForTest> server_;
  QuicDataTransportDelegate client_data_transport_delegate_;
  QuicDataTransportDelegate server_data_transport_delegate_;
};

TEST_F(QuicDataTransportTest, SendRecvMessageWithFakePacketTransport) {
  TestCryptoHandshake();
  TestSendAndRecvMessage();
}

}  // namespace

}  // namespace net
