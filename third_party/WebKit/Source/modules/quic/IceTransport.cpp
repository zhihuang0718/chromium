/*
 * Copyright (C) 2017 Google Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "modules/quic/IceTransport.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExecutionContext.h"
#include "modules/peerconnection/RTCIceCandidate.h"
#include "modules/peerconnection/RTCPeerConnectionIceEvent.h"
#include "modules/peerconnection/RTCIceCandidateInit.h"
#include "public/platform/Platform.h"
#include "public/platform/WebRTCICECandidate.h"
#include "public/platform/WebIceTransport.h"

namespace blink {

IceTransport* IceTransport::Create(ExecutionContext* context) {
  IceTransport* transport = new IceTransport(context);
  transport->SuspendIfNeeded();
  return transport;
}

IceTransport::IceTransport(ExecutionContext* context)
    : SuspendableObject(context),
      dispatch_scheduled_event_runner_(
          AsyncMethodRunner<IceTransport>::Create(
              this,
              &IceTransport::DispatchScheduledEvent)) {
  transport_ = Platform::Current()->CreateIceTransport(this);
}

IceTransport::~IceTransport() {
}

void IceTransport::addRemoteCandidate(const String& candidate, ExceptionState&) {
  transport_->AddRemoteCandidate(candidate);
}

WebIceTransport* IceTransport::web_ice_transport() {
  return transport_.get();
}

void IceTransport::OnCandidateGathered(const WebString& candidate_string) {
  // Just re-use the PeerConnection ICE candidate event in the interest of
  // time.
  DCHECK(GetExecutionContext()->IsContextThread());
  WebRTCICECandidate web_candidate;
  web_candidate.Initialize(candidate_string, "ice", 0);
  RTCIceCandidate* candidate = RTCIceCandidate::Create(web_candidate);
  ScheduleDispatchEvent(
      RTCPeerConnectionIceEvent::Create(false, false, candidate));
}

const AtomicString& IceTransport::InterfaceName() const {
  return EventTargetNames::IceTransport;
}

ExecutionContext* IceTransport::GetExecutionContext() const {
  return SuspendableObject::GetExecutionContext();
}

void IceTransport::ContextDestroyed(ExecutionContext*) {
  transport_.reset(nullptr);
}

void IceTransport::Suspend() {
  dispatch_scheduled_event_runner_->Suspend();
}

void IceTransport::Resume() {
  dispatch_scheduled_event_runner_->Resume();
}

bool IceTransport::HasPendingActivity() const {
  // Prevents us from being garbage collected. Probably good enough for the
  // hackathon.
  return true;
}

void IceTransport::ScheduleDispatchEvent(Event* event) {
  scheduled_events_.push_back(event);
  dispatch_scheduled_event_runner_->RunAsync();
}

void IceTransport::DispatchScheduledEvent() {
  HeapVector<Member<Event>> events;
  events.swap(scheduled_events_);

  HeapVector<Member<Event>>::iterator it = events.begin();
  for (; it != events.end(); ++it) {
    DispatchEvent(it->Release());
  }

  events.clear();
}

DEFINE_TRACE(IceTransport) {
  visitor->Trace(dispatch_scheduled_event_runner_);
  visitor->Trace(scheduled_events_);
  EventTargetWithInlineData::Trace(visitor);
  SuspendableObject::Trace(visitor);
}

}  // namespace blink
