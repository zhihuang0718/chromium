// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/quartc/quic_data_transport.h"

#include "base/logging.h"

namespace net {

QuicDataTransport::QuicDataTransport(
    bool is_server,
    QuartcSessionInterface::PacketTransport* packet_transport)
    : is_server_(is_server) {
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

// Delegate overrides.
void QuicDataTransport::OnCryptoHandshakeComplete() {
  LOG(INFO) << "Crypto handshake is completed.";
  secure_ = true;
}

void QuicDataTransport::OnIncomingStream(QuartcStreamInterface* stream) {
  DCHECK(delegate_);
  delegate_->OnIncomingStream(stream);
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
