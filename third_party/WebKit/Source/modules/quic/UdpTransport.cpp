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

#include "modules/quic/UdpTransport.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExecutionContext.h"
#include "public/platform/Platform.h"
#include "public/platform/WebUdpTransport.h"

namespace blink {

UdpTransport* UdpTransport::Create(ExecutionContext* context) {
  UdpTransport* transport = new UdpTransport(context);
  transport->SuspendIfNeeded();
  return transport;
}

UdpTransport::UdpTransport(ExecutionContext* context)
    : SuspendableObject(context) {
  transport_ = Platform::Current()->CreateUdpTransport();
}

UdpTransport::~UdpTransport() {
}

String UdpTransport::address() const {
  return transport_->Address();
}

void UdpTransport::setDestination(const String& address,
                                  ExceptionState&) {
  transport_->SetDestination(address);
}

WebUdpTransport* UdpTransport::web_udp_transport() {
  return transport_.get();
}

void UdpTransport::ContextDestroyed(ExecutionContext*) {
  transport_.reset(nullptr);
}

void UdpTransport::Suspend() {
  // Suspend/resume event queue if we have one.
}

void UdpTransport::Resume() {
}

bool UdpTransport::HasPendingActivity() const {
  // Prevents us from being garbage collected. Probably good enough for the
  // hackathon.
  return true;
}

DEFINE_TRACE(UdpTransport) {
  SuspendableObject::Trace(visitor);
}

}  // namespace blink
