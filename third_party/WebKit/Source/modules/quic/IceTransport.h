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

#ifndef IceTransport_h
#define IceTransport_h

#include "core/dom/SuspendableObject.h"
#include "modules/ModulesExport.h"
#include "platform/bindings/ActiveScriptWrappable.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/GarbageCollected.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class WebIceTransport;

class MODULES_EXPORT IceTransport : public GarbageCollectedFinalized<IceTransport>,
                                    public ScriptWrappable,
                                    public ActiveScriptWrappable<IceTransport>,
                                    public SuspendableObject {
  DEFINE_WRAPPERTYPEINFO();
  USING_GARBAGE_COLLECTED_MIXIN(IceTransport);

 public:
  static IceTransport* Create(ExecutionContext*);
  ~IceTransport() override;

  // Implementation of IDL interface.
  bool ready_to_send() const;
  void startGathering();
  //DOMString getLocalCandidates();

  // Methods used by QuicTransport.
  WebIceTransport* web_ice_transport();

  // SuspendableObject functions.
  void ContextDestroyed(ExecutionContext*) override;
  void Suspend() override;
  void Resume() override;

  // ScriptWrappable functions.
  bool HasPendingActivity() const final;

  DECLARE_VIRTUAL_TRACE();

 private:
  explicit IceTransport(ExecutionContext*);

  std::unique_ptr<WebIceTransport> transport_;
};

}  // namespace blink

#endif  // IceTransport_h
