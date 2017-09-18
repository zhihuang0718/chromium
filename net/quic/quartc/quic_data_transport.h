// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_QUARTC_QUIC_DATA_STREAM
#define NET_QUIC_QUARTC_QUIC_DATA_STREAM

#include "net/quic/platform/api/quic_export.h"
#include "net/quic/quartc/quartc_factory_interface.h"
#include "net/quic/quartc/quartc_session_interface.h"
#include "net/quic/quartc/quartc_stream_interface.h"

namespace net {

class QUIC_EXPORT_PRIVATE QuicDataTransport
    : public QuartcSessionInterface::Delegate {
 public:
  // QuicDataTransport doesn't own |packet_transport|;
  QuicDataTransport(bool is_server,
                    QuartcSessionInterface::PacketTransport* packet_transport);
  ~QuicDataTransport() override;

  void Connect();

  QuartcStreamInterface* CreateQuicDataStream();

  class Delegate {
   public:
    virtual ~Delegate() {}
    virtual void OnIncomingStream(QuartcStreamInterface* stream) = 0;
  };

  void SetDelegate(Delegate* delegate) { delegate_ = delegate; }

  QuartcSessionInterface* quartc_session() { return quartc_session_.get(); }

 private:
  // Delegate overrides.
  void OnCryptoHandshakeComplete() override;

  void OnIncomingStream(QuartcStreamInterface* stream) override;

  void OnConnectionClosed(int error_code, bool from_remote) override;

  bool secure_ = false;
  bool closed_ = false;
  bool is_server_ = false;
  std::unique_ptr<QuartcFactoryInterface> quartc_factory_;
  std::unique_ptr<QuartcSessionInterface> quartc_session_;
  Delegate* delegate_ = nullptr;
};

}  // namespace net
#endif  // NET_QUIC_QUARTC_QUIC_DATA_STREAM
