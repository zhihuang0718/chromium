// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_WEB_ICE_TRANSPORT_IMPL_H_
#define CONTENT_RENDERER_WEB_ICE_TRANSPORT_IMPL_H_

#include "content/renderer/p2p/socket_dispatcher.h"
#include "content/renderer/p2p/socket_client_delegate.h"
#include "net/base/ip_endpoint.h"
#include "net/quic/quartc/quartc_session_interface.h"
#include "third_party/WebKit/public/platform/WebIceTransport.h"

namespace content {

class WebIceTransportImpl : public blink::WebIceTransport,
                            public net::QuartcSessionInterface::PacketTransport,
                            public P2PSocketClientDelegate {
 public:
  // TODO: Not sure if we need the dispatcher here.
  WebIceTransportImpl(P2PSocketDispatcher* dispatcher);
  ~WebIceTransportImpl() override;

  // Set pointer to Quartc session that will receive callbacks for packets received.
  void set_quartc_session(net::QuartcSessionInterface* session);

  // TODO: IDL methods (e.g. setCandidate, startGathering?)

  // net::QuartcSessionInterface::PacketTransport implementation.
  bool CanWrite() override;
  int Write(const char* buffer, size_t buf_len) override;

  // P2PSocketClientDelegate overrides.
  void OnOpen(const net::IPEndPoint& local_address,
              const net::IPEndPoint& remote_address) override {};
  void OnIncomingTcpConnection(const net::IPEndPoint& address,
                               P2PSocketClient* client) override {};
  void OnSendComplete(const P2PSendPacketMetrics& send_metrics) override {};
  void OnError() override {};
  void OnDataReceived(const net::IPEndPoint& address,
                      const std::vector<char>& data,
                      const base::TimeTicks& timestamp) override;

 private:
  net::QuartcSessionInterface* quartc_session_;
};

}

#endif  // CONTENT_RENDERER_WEB_ICE_TRANSPORT_IMPL_H_
