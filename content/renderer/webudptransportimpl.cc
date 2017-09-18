// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "base/strings/string_split.h"
#include "content/renderer/webudptransportimpl.h"
#include "net/base/parse_number.h"

namespace content {

WebUdpTransportImpl::WebUdpTransportImpl(P2PSocketDispatcher* dispatcher) :
  network_manager_(new IpcNetworkManager(dispatcher)),
  socket_(new P2PSocketClientImpl(dispatcher)) {
  socket_->Init(P2P_SOCKET_UDP, net::IPEndPoint(net::IPAddress::IPv4AllZeros(), 0), 0, 0, P2PHostAndIPEndPoint(), this);
  network_manager_->StartUpdating();
}

WebUdpTransportImpl::~WebUdpTransportImpl() {
  socket_->Close();
}

blink::WebString WebUdpTransportImpl::Address() const {
  net::IPEndPoint address = local_address_;
  // If the OS only gives us the ANY address, replace it with the default IPv4
  // address as determined by our network manager. This is what webrtc does too
  // to get local ICE candidates.
  if (address.address() == net::IPAddress::IPv4AllZeros()) {
    rtc::IPAddress default_ipv4;
    if (network_manager_->GetDefaultLocalAddress(AF_INET, &default_ipv4)) {
      net::IPAddress net_ip;
      if (net_ip.AssignFromIPLiteral(default_ipv4.ToString())) {
        address = net::IPEndPoint(net_ip, address.port());
      } else {
        LOG(ERROR) << "IP string conversion failed???";
      }
    }
  }
  return blink::WebString::FromUTF8(address.ToString());
}

void WebUdpTransportImpl::SetDestination(const blink::WebString& address_string) {
  std::vector<base::StringPiece> parts = base::SplitStringPiece(
            address_string.Utf8(), ":", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
  if (parts.size() != 2) {
    LOG(ERROR) << "Invalid address format; should be in form \"IP:port\".";
    return;
  }

  net::IPAddress address;
  if (!address.AssignFromIPLiteral(parts[0])) {
    LOG(ERROR) << "Failed to parse IP.";
    return;
  }

  // Parse the port.
  uint32_t port;
  if (!net::ParseUint32(parts[1], &port)) {
    LOG(ERROR) << "Failed to parse port.";
    return;
  }
  remote_address_ = net::IPEndPoint(address, port);
}

void WebUdpTransportImpl::OnOpen(const net::IPEndPoint& local_address,
                    const net::IPEndPoint& remote_address) {
  local_address_ = local_address;
}

}  // namespace content
