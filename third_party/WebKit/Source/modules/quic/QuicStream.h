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

#ifndef QuicStream_h
#define QuicStream_h

#include <memory>
#include <vector>

#include "core/dom/SuspendableObject.h"
#include "core/typed_arrays/ArrayBufferViewHelpers.h"
#include "core/typed_arrays/DOMTypedArray.h"
#include "modules/ModulesExport.h"
#include "platform/bindings/ActiveScriptWrappable.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/GarbageCollected.h"
#include "public/platform/WebQuicStreamDelegate.h"

namespace blink {

class ExceptionState;
class WebQuicStream;

class MODULES_EXPORT QuicStream : public GarbageCollectedFinalized<QuicStream>,
                                  public ScriptWrappable,
                                  public ActiveScriptWrappable<QuicStream>,
                                  public SuspendableObject,
                                  public WebQuicStreamDelegate {
  DEFINE_WRAPPERTYPEINFO();
  USING_GARBAGE_COLLECTED_MIXIN(QuicStream);

 public:
  QuicStream(ExecutionContext* context, WebQuicStream* quic_stream);
  ~QuicStream() override;

  void write(NotShared<DOMUint8Array> data);
  NotShared<DOMUint8Array> read();

  // WebQuicStreamDelegate implementation.
  void OnRead(const char* data, size_t length) override;

  // SuspendableObject functions.
  void ContextDestroyed(ExecutionContext*) override;
  void Suspend() override;
  void Resume() override;

  // ScriptWrappable functions.
  bool HasPendingActivity() const final;

  DECLARE_VIRTUAL_TRACE();

 private:
  std::unique_ptr<WebQuicStream> quic_stream_;
  // Buffer of data that's been received but not yet read by JS.
  std::vector<uint8_t> received_data_;
};

}  // namespace blink

#endif  // QuicStream_h
