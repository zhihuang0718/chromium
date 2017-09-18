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

#include "modules/quic/QuicStream.h"

#include "bindings/core/v8/ExceptionMessages.h"
#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/ExecutionContext.h"
#include "public/platform/Platform.h"
#include "public/platform/WebQuicStream.h"

namespace blink {

QuicStream::QuicStream(ExecutionContext* context, WebQuicStream* quic_stream)
    : SuspendableObject(context),
      quic_stream_(quic_stream) {
}

QuicStream::~QuicStream() {
}

void QuicStream::write(NotShared<DOMUint8Array> data) {
  // Going from unsigned char to char.
  quic_stream_->Write(reinterpret_cast<const char*>(data.View()->Data()), data.View()->length());
}

NotShared<DOMUint8Array> QuicStream::read() {
  NotShared<DOMUint8Array> data(
     DOMUint8Array::Create(received_data_.data(), received_data_.size()));
  received_data_.clear();
  return data;
}

void QuicStream::OnRead(const char* data, size_t length) {
  received_data_.insert(received_data_.end(), data, data + length);
}

void QuicStream::ContextDestroyed(ExecutionContext*) {
  quic_stream_.reset(nullptr);
}

void QuicStream::Suspend() {
  // Suspend/resume event queue if we have one.
}

void QuicStream::Resume() {
}

bool QuicStream::HasPendingActivity() const {
  // Prevents us from being garbage collected. Probably good enough for the
  // hackathon.
  return true;
}

DEFINE_TRACE(QuicStream) {
  SuspendableObject::Trace(visitor);
}

}  // namespace blink
