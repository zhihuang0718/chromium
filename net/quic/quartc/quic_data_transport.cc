// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/quartc/quic_data_transport.h"

#include "base/logging.h"

namespace net {

QuicDataTransport::QuicDataTransport(
    bool is_server,
    QuartcSessionInterface::PacketTransport* packet_transport,
    blink::WebQuicTransportDelegate* delegate)
    : is_server_(is_server),
      delegate_(delegate) {
  DCHECK(delegate);

  QuartcFactoryConfig factory_config;
  // Create a QuartcFactory with nullptr task_runner_ and nullptr clock_. We can
  // try to hack the QuartcFactory so that it uses the Chromium builtin
  // QuicAlarmFactory and QuicConnectionHelper.
  quartc_factory_ = CreateQuartcFactory(factory_config);

  DCHECK(quartc_factory_);
  QuartcFactoryInterface::QuartcSessionConfig session_config;
  session_config.is_server = is_server;
  if (!is_server) {
    session_config.unique_remote_server_id = "unique_remote_server_id";
  }
  session_config.packet_transport = packet_transport;

  quartc_session_ = quartc_factory_->CreateQuartcSession(session_config);
  quartc_session_->SetDelegate(this);
  if (is_server) {
    LOG(INFO) << "Server StartCryptoHandshake...";
    quartc_session_->StartCryptoHandshake();
  }
}

QuicDataTransport::~QuicDataTransport() {}

void QuicDataTransport::Connect() {
  if (closed_) {
    LOG(ERROR)
        << "Failed to initiate the crypto handshake. The connection is closed.";
    return;
  }
  if (is_server_) {
    LOG(WARNING) << "Server doesn't need to initiate the cryto handshake.";
    return;
  }

  DCHECK(quartc_session_);
  LOG(INFO) << "Client StartCryptoHandshake...";
  quartc_session_->StartCryptoHandshake();
}

class QuicStreamAdapter : public blink::WebQuicStream,
                          public QuartcStreamInterface::Delegate {
 public:
  QuicStreamAdapter(QuartcStreamInterface* quartc_stream) :
    quartc_stream_(quartc_stream) {
    quartc_stream_->SetDelegate(this);
  }

  ~QuicStreamAdapter() override {
    if (quartc_stream_) {
      quartc_stream_->Close();
    } else {
      LOG(INFO) << "In ~QuicStreamAdapter but stream already closed.";
    }
  }

  void Write(const char* data, size_t length) override {
    if (quartc_stream_) {
      quartc_stream_->Write(data, length, QuartcStreamInterface::WriteParameters());
    } else {
      LOG(WARNING) << "Can't write, stream already closed.";
    }
  }

  void SetDelegate(blink::WebQuicStreamDelegate* delegate) override {
    web_quic_stream_delegate_ = delegate;
  }

 private:
  void OnReceived(QuartcStreamInterface* stream,
                          const char* data,
                          size_t size) override {
    if (web_quic_stream_delegate_) {
      web_quic_stream_delegate_->OnRead(data, size);
    } else {
      LOG(WARNING) << "Received data before WebQuicStreamDelegate hooked up.";
    }
  }
  void OnClose(QuartcStreamInterface* stream) override {
    LOG(INFO) << "Got close event from QuartcStreamInterface";
    quartc_stream_ = nullptr;
  }
  void OnCanWrite(QuartcStreamInterface* stream) override {}

  QuartcStreamInterface* quartc_stream_;
  blink::WebQuicStreamDelegate* web_quic_stream_delegate_ = nullptr;
};

blink::WebQuicStream* QuicDataTransport::CreateStream() {
  QuartcStreamInterface* stream = CreateQuicDataStream();
  if (!stream) {
    return nullptr;
  }
  return new QuicStreamAdapter(stream);
}

// Delegate overrides.
void QuicDataTransport::OnCryptoHandshakeComplete() {
  LOG(INFO) << "Crypto handshake is completed.";
  secure_ = true;
}

void QuicDataTransport::OnIncomingStream(QuartcStreamInterface* stream) {
  DCHECK(delegate_);
  delegate_->OnIncomingStream(new QuicStreamAdapter(stream));
}

void QuicDataTransport::OnConnectionClosed(int error_code, bool from_remote) {
  closed_ = true;
}

QuartcStreamInterface* QuicDataTransport::CreateQuicDataStream() {
  if (!secure_) {
    LOG(ERROR) << "Cannot create a stream before the crypto ";
    return nullptr;
  }
  DCHECK(quartc_session_);
  return quartc_session_->CreateOutgoingStream(
      QuartcSessionInterface::OutgoingStreamParameters());
}

}  // namespace net
