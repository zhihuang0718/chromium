// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/p2p/ice_transport.h"

#include "base/memory/ptr_util.h"
#include "content/renderer/p2p/ipc_network_manager.h"
#include "content/renderer/p2p/ipc_socket_factory.h"
#include "content/renderer/render_thread_impl.h"

namespace content {

IceTransport::IceTransport(P2PSocketDispatcher* p2p_socket_dispatcher) {
  network_manager_.reset(new IpcNetworkManager(p2p_socket_dispatcher));
  socket_factory_.reset(new IpcPacketSocketFactory(p2p_socket_dispatcher));
  port_allocator_.reset(new cricket::BasicPortAllocator(network_manager_.get(),
                                                        socket_factory_.get()));

  ice_transport_channel_ = base::MakeUnique<cricket::P2PTransportChannel>(
      "ice", 1, port_allocator_.get());

  ice_transport_channel_->SignalReadPacket.connect(this,
                                                   &IceTransport::OnReadPacket);
  ice_transport_channel_->SignalCandidateGathered.connect(
      this, &IceTransport::OnCandidateGathered);
}

IceTransport::~IceTransport(){};

// PacketTransport overrides.
bool IceTransport::CanWrite() {
  DCHECK(ice_transport_channel_);
  return ice_transport_channel_->writable();
}

int IceTransport::Write(const char* buffer, size_t buf_len) {
  DCHECK(ice_transport_channel_);
  return ice_transport_channel_->SendPacket(buffer, buf_len,
                                            rtc::PacketOptions(), 0);
}

void IceTransport::SetIceRole(cricket::IceRole role) {
  DCHECK(ice_transport_channel_);
  ice_transport_channel_->SetIceRole(role);
}

void IceTransport::OnReadPacket(rtc::PacketTransportInternal* transport,
                                const char* data,
                                size_t data_size,
                                const rtc::PacketTime& packet_time,
                                int flags) {
  DCHECK(delegate_);
  delegate_->OnReadPacket(data, data_size);
}

void IceTransport::OnCandidateGathered(
    cricket::IceTransportInternal* ice_transport,
    const cricket::Candidate& candidate) {
  LOG(INFO) << "Candidate gathered:" << candidate.ToString();
}
}  // namespace content
