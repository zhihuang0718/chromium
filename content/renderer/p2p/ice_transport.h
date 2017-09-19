// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CONTENT_RENDERER_P2P_ICE_TRANSPORT
#define CONTENT_RENDERER_P2P_ICE_TRANSPORT

#include "content/common/content_export.h"
#include "content/renderer/p2p/socket_dispatcher.h"
#include "net/quic/quartc/quartc_session_interface.h"
#include "third_party/webrtc/p2p/base/p2ptransportchannel.h"
#include "third_party/webrtc/p2p/client/basicportallocator.h"
#include "third_party/webrtc/rtc_base/sigslot.h"

namespace content {

class CONTENT_EXPORT IceTransportDelegate {
 public:
  virtual ~IceTransportDelegate() = 0;
  virtual void OnReadPacket(const char*, size_t) = 0;
  virtual void OnWritable() = 0;
};

// This class wraps an IceTransportChannel for P2P connection.
class CONTENT_EXPORT IceTransport
    : public net::QuartcSessionInterface::PacketTransport,
      public sigslot::has_slots<> {
 public:
  explicit IceTransport(P2PSocketDispatcher* p2p_socket_dispatcher);

  ~IceTransport() override;

  // PcketTransport overrides.
  bool CanWrite() override;

  int Write(const char*, size_t) override;

  void SetIceRole(cricket::IceRole role);

  // Expose the P2PTransportChannel.
  cricket::P2PTransportChannel* ice_transport_channel() {
    return ice_transport_channel_.get();
  }

  // Expose the port allocator so that we can set configuration for it.
  cricket::BasicPortAllocator* port_allocator() {
    return port_allocator_.get();
  }

  void SetDelegate(IceTransportDelegate* delegate) { delegate_ = delegate; }

 private:
  void OnReadPacket(rtc::PacketTransportInternal* transport,
                    const char* data,
                    size_t data_size,
                    const rtc::PacketTime& packet_time,
                    int flags);

  void OnCandidateGathered(cricket::IceTransportInternal* ice_transport,
                           const cricket::Candidate& candidate);

  std::unique_ptr<cricket::P2PTransportChannel> ice_transport_channel_;
  std::unique_ptr<cricket::BasicPortAllocator> port_allocator_;
  std::unique_ptr<rtc::NetworkManager> network_manager_;
  std::unique_ptr<rtc::PacketSocketFactory> socket_factory_;

  IceTransportDelegate* delegate_ = nullptr;
};

}  // namespace content

#endif  // CONTENT_RENDERER_P2P_ICE_TRANSPORT
