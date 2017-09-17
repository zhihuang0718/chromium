// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/quartc/quic_data_stream.h"

namespace net {

void QuicDataStream::OnReceived(QuartcStreamInterface* stream,
                                const char* data,
                                size_t size) {}

void QuicDataStream::OnClose(QuartcStreamInterface* stream) {}

void QuicDataStream::OnCanWrite(QuartcStreamInterface* stream) {}

}  // namespace net
