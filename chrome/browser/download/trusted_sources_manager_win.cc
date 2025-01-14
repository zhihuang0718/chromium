// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/download/trusted_sources_manager.h"

#include <urlmon.h>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/scoped_comptr.h"
#include "url/gurl.h"

namespace {

class TrustedSourcesManagerWin : public TrustedSourcesManager {
 public:
  TrustedSourcesManagerWin();
  ~TrustedSourcesManagerWin() override;

  // TrustedSourcesManager methods:
  bool IsFromTrustedSource(const GURL& url) const override;
};

TrustedSourcesManagerWin::TrustedSourcesManagerWin() = default;
TrustedSourcesManagerWin::~TrustedSourcesManagerWin() = default;

bool TrustedSourcesManagerWin::IsFromTrustedSource(const GURL& url) const {
  base::win::ScopedComPtr<IInternetSecurityManager> security_manager;
  HRESULT hr = ::CoInternetCreateSecurityManager(
      NULL, security_manager.GetAddressOf(), NULL);
  // URLZONE_LOCAL_MACHINE 0
  // URLZONE_INTRANET      1
  // URLZONE_TRUSTED       2
  // URLZONE_INTERNET      3
  // URLZONE_UNTRUSTED     4
  DWORD zone = 0;
  base::string16 url16 = base::ASCIIToUTF16(url.spec());
  hr = security_manager->MapUrlToZone(url16.c_str(), &zone, 0);
  if (FAILED(hr)) {
    LOG(ERROR) << "security_manager->MapUrlToZone failed with hr: " << std::hex
               << hr;
    return false;
  }
  return zone <= static_cast<DWORD>(URLZONE_TRUSTED);
}

}  // namespace

// static
TrustedSourcesManager* TrustedSourcesManager::Create() {
  return new TrustedSourcesManagerWin;
}
