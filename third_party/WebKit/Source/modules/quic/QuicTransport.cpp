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

#include "modules/quic/QuicTransport.h"

#include "bindings/core/v8/ExceptionMessages.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExecutionContext.h"
#include "modules/quic/QuicStream.h"
#include "modules/quic/QuicStreamEvent.h"
#include "public/platform/Platform.h"
#include "public/platform/WebQuicTransport.h"

namespace blink {

QuicTransport* QuicTransport::Create(ExecutionContext* context,
                                     bool is_server,
                                     UdpTransport* transport,
                                     ExceptionState& exception_state) {
  if (!transport) {
    exception_state.ThrowDOMException(
        kTypeMismatchError,
        ExceptionMessages::ArgumentNullOrIncorrectType(1, "UdpTransport"));
    return nullptr;
  }
  return new QuicTransport(context, is_server, transport);
}

QuicTransport::QuicTransport(ExecutionContext* context, bool is_server, UdpTransport* transport)
    : SuspendableObject(context),
      is_server_(is_server),
      udp_transport_(transport),
      dispatch_scheduled_event_runner_(
          AsyncMethodRunner<QuicTransport>::Create(
              this,
              &QuicTransport::DispatchScheduledEvent)) {
  quic_transport_ = Platform::Current()->CreateQuicTransport(
      is_server, transport->web_udp_transport(), this);
}

QuicTransport::~QuicTransport() {
}

void QuicTransport::connect(ExceptionState& exception_state) {
  if (is_server_) {
    exception_state.ThrowDOMException(kInvalidAccessError, "Can't connect from the server side...");
    return;
  }
  quic_transport_->Connect();
}

QuicStream* QuicTransport::createStream(ScriptState* script_state, ExceptionState& exception_state) {
  WebQuicStream* web_stream = quic_transport_->CreateStream();
  if (!web_stream) {
    exception_state.ThrowDOMException(kOperationError, "Failed to create QuicStream; is transport connected?");
    return nullptr;
  }
  ExecutionContext* context = ExecutionContext::From(script_state);
  QuicStream* stream = new QuicStream(context, web_stream);
  web_stream->SetDelegate(stream);
  return stream;
}

void QuicTransport::OnIncomingStream(WebQuicStream* web_stream) {
  DCHECK(GetExecutionContext()->IsContextThread());

  QuicStream* stream = new QuicStream(GetExecutionContext(), web_stream);
  web_stream->SetDelegate(stream);
  ScheduleDispatchEvent(QuicStreamEvent::Create(EventTypeNames::stream,
                                                false, false, stream));
}

const AtomicString& QuicTransport::InterfaceName() const {
  return EventTargetNames::QuicTransport;
}

ExecutionContext* QuicTransport::GetExecutionContext() const {
  return SuspendableObject::GetExecutionContext();
}

void QuicTransport::ContextDestroyed(ExecutionContext*) {
}

void QuicTransport::Suspend() {
  dispatch_scheduled_event_runner_->Suspend();
}

void QuicTransport::Resume() {
  dispatch_scheduled_event_runner_->Resume();
}

bool QuicTransport::HasPendingActivity() const {
  // Prevents us from being garbage collected. Probably good enough for the
  // hackathon.
  return true;
}

void QuicTransport::ScheduleDispatchEvent(Event* event) {
  scheduled_events_.push_back(event);
  dispatch_scheduled_event_runner_->RunAsync();
}

void QuicTransport::DispatchScheduledEvent() {
  HeapVector<Member<Event>> events;
  events.swap(scheduled_events_);

  HeapVector<Member<Event>>::iterator it = events.begin();
  for (; it != events.end(); ++it) {
    DispatchEvent(it->Release());
  }

  events.clear();
}

DEFINE_TRACE(QuicTransport) {
  visitor->Trace(udp_transport_);
  visitor->Trace(dispatch_scheduled_event_runner_);
  visitor->Trace(scheduled_events_);
  EventTargetWithInlineData::Trace(visitor);
  SuspendableObject::Trace(visitor);
}

}  // namespace blink
