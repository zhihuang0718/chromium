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
#include "public/platform/Platform.h"

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
      udp_transport_(transport) {
}

QuicTransport::~QuicTransport() {
}

void QuicTransport::ContextDestroyed(ExecutionContext*) {
}

void QuicTransport::Suspend() {
  // Suspend/resume event queue if we have one.
}

void QuicTransport::Resume() {
}

bool QuicTransport::HasPendingActivity() const {
  // Prevents us from being garbage collected. Probably good enough for the
  // hackathon.
  return true;
}

DEFINE_TRACE(QuicTransport) {
  visitor->Trace(udp_transport_);
  SuspendableObject::Trace(visitor);
}

}  // namespace blink
