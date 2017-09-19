// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/p2p/ice_transport.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "content/renderer/p2p/ipc_network_manager.h"
#include "content/renderer/p2p/ipc_socket_factory.h"
#include "content/renderer/render_thread_impl.h"
#include "crypto/random.h"
#include "jingle/glue/thread_wrapper.h"
#include "third_party/webrtc/pc/webrtcsdp.h"

namespace content {

IceTransport::IceTransport(P2PSocketDispatcher* p2p_socket_dispatcher, blink::WebIceTransportDelegate* delegate) :
 delegate_(delegate) {
  DCHECK(delegate);
  
  // P2PTransportChannel needs the current thread to be wrapped by an
  // rtc::Thread.
  jingle_glue::JingleThreadWrapper::EnsureForCurrentMessageLoop();
  jingle_glue::JingleThreadWrapper::current()->set_send_allowed(true);

  network_manager_.reset(new IpcNetworkManager(p2p_socket_dispatcher));
  socket_factory_.reset(new IpcPacketSocketFactory(p2p_socket_dispatcher));
  port_allocator_.reset(new cricket::BasicPortAllocator(network_manager_.get(),
                                                        socket_factory_.get()));
  cricket::ServerAddresses stun_servers;
  stun_servers.insert({"stun.l.google.com", 19302});
  port_allocator_->SetConfiguration(stun_servers, std::vector<cricket::RelayServerConfig>(), 0, false);

  ice_transport_channel_ = base::MakeUnique<cricket::P2PTransportChannel>(
      "ice", 1, port_allocator_.get());
  // Hack for the hackathon. In reality we'd expose these as JS APIs.
  ice_transport_channel_->SetIceRole(cricket::ICEROLE_CONTROLLING);
  uint64_t tiebreaker;
  crypto::RandBytes(&tiebreaker, sizeof(tiebreaker));
  ice_transport_channel_->SetIceTiebreaker(tiebreaker);
  ice_transport_channel_->SetIceCredentials("foo", "bar");
  ice_transport_channel_->SetRemoteIceCredentials("foo", "bar");
  ice_transport_channel_->MaybeStartGathering();

  ice_transport_channel_->SignalRoleConflict.connect(this,
                                                   &IceTransport::OnRoleConflict);
  ice_transport_channel_->SignalReadPacket.connect(this,
                                                   &IceTransport::OnReadPacket);
  ice_transport_channel_->SignalCandidateGathered.connect(
      this, &IceTransport::OnCandidateGathered);
  ice_transport_channel_->SignalWritableState.connect(
      this, &IceTransport::OnWritableState);
}

IceTransport::~IceTransport(){
}

void IceTransport::set_quartc_session(net::QuartcSessionInterface* session) {
  quartc_session_ = session;
}

void IceTransport::AddRemoteCandidate(const blink::WebString& candidate_string) {
  cricket::Candidate candidate;
  if (!webrtc::SdpDeserializeCandidate("ice", candidate_string.Utf8(), &candidate, nullptr)) {
    LOG(ERROR) << "Failed to deserialize candidate.";
    return;
  }
  ice_transport_channel_->AddRemoteCandidate(candidate);
}

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

void IceTransport::OnRoleConflict(cricket::IceTransportInternal* ice_transport) {
  ice_transport_channel_->SetIceRole(
      (ice_transport_channel_->GetIceRole() == cricket::ICEROLE_CONTROLLING) ?
      cricket::ICEROLE_CONTROLLED : cricket::ICEROLE_CONTROLLING);
}

void IceTransport::OnReadPacket(rtc::PacketTransportInternal* transport,
                                const char* data,
                                size_t data_size,
                                const rtc::PacketTime& packet_time,
                                int flags) {
  if (!quartc_session_) {
    LOG(WARNING) << "Received data before hooked up to QuicTransport!";
    return;
  }
  quartc_session_->OnTransportReceived(data, data_size);
}

void IceTransport::OnCandidateGathered(
    cricket::IceTransportInternal* ice_transport,
    const cricket::Candidate& candidate) {
  DCHECK(delegate_);
  std::string serialized = webrtc::SdpSerializeCandidate(candidate);
  LOG(INFO) << "Candidate gathered: " << serialized;
  delegate_->OnCandidateGathered(blink::WebString::FromUTF8(serialized));
}

void IceTransport::OnWritableState(rtc::PacketTransportInternal* ice_transport) {
  if (ice_transport->writable()) {
    LOG(INFO) << "We're writable, yo";
    if (quartc_session_) {
      quartc_session_->OnTransportCanWrite();
    }
  }
}

}  // namespace content
