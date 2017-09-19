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

#ifndef QuicTransport_h
#define QuicTransport_h

#include <memory>

#include "core/dom/SuspendableObject.h"
#include "modules/EventTargetModules.h"
#include "modules/ModulesExport.h"
#include "modules/quic/UdpTransport.h"
#include "modules/quic/IceTransport.h"
#include "platform/AsyncMethodRunner.h"
#include "platform/bindings/ActiveScriptWrappable.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/GarbageCollected.h"
#include "public/platform/WebQuicTransportDelegate.h"

namespace blink {

class ExceptionState;
class QuicStream;
class WebQuicStream;
class WebQuicTransport;

class MODULES_EXPORT QuicTransport : public EventTargetWithInlineData,
                                     public ActiveScriptWrappable<QuicTransport>,
                                     public SuspendableObject,
                                     public WebQuicTransportDelegate {
  DEFINE_WRAPPERTYPEINFO();
  USING_GARBAGE_COLLECTED_MIXIN(QuicTransport);

 public:
  static QuicTransport* Create(ExecutionContext*, bool, UdpTransport*, ExceptionState&);
  static QuicTransport* Create(ExecutionContext*, bool, IceTransport*, ExceptionState&);
  ~QuicTransport() override;

  void connect(ExceptionState&);
  QuicStream* createStream(ScriptState*, ExceptionState&);

  DEFINE_ATTRIBUTE_EVENT_LISTENER(stream);

  // WebQuicTransportDelegate implementation.
  void OnIncomingStream(WebQuicStream* web_stream) override;

  // EventTarget
  const AtomicString& InterfaceName() const override;
  ExecutionContext* GetExecutionContext() const override;

  // SuspendableObject functions.
  void ContextDestroyed(ExecutionContext*) override;
  void Suspend() override;
  void Resume() override;

  // ScriptWrappable functions.
  bool HasPendingActivity() const final;

  DECLARE_VIRTUAL_TRACE();

 private:
  QuicTransport(ExecutionContext*, bool, UdpTransport*);
  QuicTransport(ExecutionContext*, bool, IceTransport*);

  void ScheduleDispatchEvent(Event*);
  void DispatchScheduledEvent();

  bool is_server_;
  Member<UdpTransport> udp_transport_;
  Member<IceTransport> ice_transport_;
  HeapVector<Member<QuicStream>> quic_streams_;
  std::unique_ptr<WebQuicTransport> quic_transport_;

  Member<AsyncMethodRunner<QuicTransport>> dispatch_scheduled_event_runner_;
  HeapVector<Member<Event>> scheduled_events_;
};

}  // namespace blink

#endif  // QuicTransport_h
