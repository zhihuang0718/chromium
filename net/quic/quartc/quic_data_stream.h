// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_QUARTC_QUIC_DATA_STREAM
#define NET_QUIC_QUARTC_QUIC_DATA_STREAM

#include "net/quic/quartc/quartc_stream_interface.h"

namespace net {

class QuicDataStream : public QuartcStreamInterface::Delegate {
 public:
  // Delegate overrides.
  void OnReceived(QuartcStreamInterface* stream,
                  const char* data,
                  size_t size) override;

  void OnClose(QuartcStreamInterface* stream) override;

  void OnCanWrite(QuartcStreamInterface* stream) override;

  bool writable() { return writable_; }

  void Write(const char* data, size_t size);

  void Read(char* data, size_t* size);

  class Delegate {
   public:
    virtual ~Delegate() = 0;
    virtual void OnDataReceived(const char* data, size_t size) = 0;
  };

 private:
  bool writable_ = false;
  // QuratcSession takes the ownership of |stream_|.
  QuartcStreamInterface* stream_ = nullptr;
};

}  // namespace net
#endif  // NET_QUIC_QUARTC_QUIC_DATA_STREAM
