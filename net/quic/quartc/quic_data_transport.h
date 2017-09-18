// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_QUARTC_QUIC_DATA_STREAM
#define NET_QUIC_QUARTC_QUIC_DATA_STREAM

#include "net/quic/platform/api/quic_export.h"
#include "net/quic/quartc/quartc_factory_interface.h"
#include "net/quic/quartc/quartc_session_interface.h"
#include "net/quic/quartc/quartc_stream_interface.h"
#include "third_party/WebKit/public/platform/WebQuicStream.h"
#include "third_party/WebKit/public/platform/WebQuicTransport.h"
#include "third_party/WebKit/public/platform/WebQuicTransportDelegate.h"

namespace net {

class QUIC_EXPORT_PRIVATE QuicDataTransport
    : public QuartcSessionInterface::Delegate,
      public blink::WebQuicTransport {
 public:
  // QuicDataTransport doesn't own |packet_transport|;
  QuicDataTransport(bool is_server,
                    QuartcSessionInterface::PacketTransport* packet_transport,
                    blink::WebQuicTransportDelegate* delegate);
  ~QuicDataTransport() override;

  void Connect() override;
  // Ultimately just calls CreateQuicDataStream, but returns a wrapper object
  // that implements the Blink interface.
  blink::WebQuicStream* CreateStream() override;

  QuartcSessionInterface* quartc_session() { return quartc_session_.get(); }

 private:
  // Delegate overrides.
  void OnCryptoHandshakeComplete() override;

  void OnIncomingStream(QuartcStreamInterface* stream) override;

  void OnConnectionClosed(int error_code, bool from_remote) override;

  QuartcStreamInterface* CreateQuicDataStream();

  bool secure_ = false;
  bool closed_ = false;
  bool is_server_ = false;
  std::unique_ptr<QuartcFactoryInterface> quartc_factory_;
  std::unique_ptr<QuartcSessionInterface> quartc_session_;
  blink::WebQuicTransportDelegate* delegate_ = nullptr;
};

}  // namespace net
#endif  // NET_QUIC_QUARTC_QUIC_DATA_STREAM
