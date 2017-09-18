// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/webicetransportimpl.h"

namespace content {

WebIceTransportImpl::WebIceTransportImpl(P2PSocketDispatcher* dispatcher) {
}

WebIceTransportImpl::~WebIceTransportImpl() {
}

void WebIceTransportImpl::set_quartc_session(net::QuartcSessionInterface* session) {
  quartc_session_ = session;
}

// TODO: IDL methods

bool WebIceTransportImpl::CanWrite() {
  // TODO: some call to some real IceTransport
  return true;
}

int WebIceTransportImpl::Write(const char* buffer, size_t buf_len) {
  if (!CanWrite()) {
    return 0;
  }
  std::vector<char> data(buffer, buffer + buf_len);
  // TODO: some real call to do a write
  return static_cast<int>(buf_len);
}

void WebIceTransportImpl::OnDataReceived(const net::IPEndPoint& address,
                                         const std::vector<char>& data,
                                         const base::TimeTicks& timestamp) {
  if (!quartc_session_) {
    LOG(WARNING) << "Received data before hooked up to QuicTransport!";
    return;
  }
  quartc_session_->OnTransportReceived(data.data(), data.size());
}

}  // namespace content
