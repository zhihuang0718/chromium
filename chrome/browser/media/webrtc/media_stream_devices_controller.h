// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_MEDIA_WEBRTC_MEDIA_STREAM_DEVICES_CONTROLLER_H_
#define CHROME_BROWSER_MEDIA_WEBRTC_MEDIA_STREAM_DEVICES_CONTROLLER_H_

#include <map>
#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "components/content_settings/core/common/content_settings.h"
#include "content/public/common/media_stream_request.h"

class MediaStreamDevicesController;
class Profile;
class TabSpecificContentSettings;

namespace content {
class WebContents;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace policy {
class MediaStreamDevicesControllerBrowserTest;
}

namespace test {
class MediaStreamDevicesControllerTestApi;
}

class MediaStreamDevicesController {
 public:
  static void RequestPermissions(
      const content::MediaStreamRequest& request,
      const content::MediaResponseCallback& callback);

  // Registers the prefs backing the audio and video policies.
  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

  ~MediaStreamDevicesController();

  bool IsAskingForAudio() const;
  bool IsAskingForVideo() const;

  // Called when a permission prompt is answered through the PermissionManager.
  void PromptAnsweredGroupedRequest(
      const std::vector<ContentSetting>& responses);

#if defined(OS_ANDROID)
  // Called when the Android OS-level prompt is answered.
  void AndroidOSPromptAnswered(bool allowed);
#endif  // defined(OS_ANDROID)

  // Called when the request is finished and no prompt is required.
  void RequestFinishedNoPrompt();

 private:
  friend class MediaStreamDevicesControllerTest;
  friend class test::MediaStreamDevicesControllerTestApi;
  friend class policy::MediaStreamDevicesControllerBrowserTest;

  MediaStreamDevicesController(content::WebContents* web_contents,
                               const content::MediaStreamRequest& request,
                               const content::MediaResponseCallback& callback);

  bool IsAllowedForAudio() const;
  bool IsAllowedForVideo() const;

  // Returns a list of devices available for the request for the given
  // audio/video permission settings.
  content::MediaStreamDevices GetDevices(ContentSetting audio_setting,
                                         ContentSetting video_setting);

  // Runs |callback_| with the current audio/video permission settings.
  void RunCallback();

  // Called when the permission has been set to update the
  // TabSpecificContentSettings.
  void UpdateTabSpecificContentSettings(ContentSetting audio_setting,
                                        ContentSetting video_setting) const;

  // Returns the content settings for the given content type and request.
  ContentSetting GetContentSetting(
      ContentSettingsType content_type,
      const content::MediaStreamRequest& request,
      content::MediaStreamRequestResult* denial_reason) const;

  // Returns true if clicking allow on the dialog should give access to the
  // requested devices.
  bool IsUserAcceptAllowed(ContentSettingsType content_type) const;

  // The current state of the audio/video content settings which may be updated
  // through the lifetime of the request.
  ContentSetting audio_setting_;
  ContentSetting video_setting_;
  content::MediaStreamRequestResult denial_reason_;

  content::WebContents* web_contents_;

  // The owner of this class needs to make sure it does not outlive the profile.
  Profile* profile_;

  // Weak pointer to the tab specific content settings of the tab for which the
  // MediaStreamDevicesController was created. The tab specific content
  // settings are associated with a the web contents of the tab. The
  // MediaStreamDeviceController must not outlive the web contents for which it
  // was created.
  TabSpecificContentSettings* content_settings_;

  // The original request for access to devices.
  const content::MediaStreamRequest request_;

  // The callback that needs to be Run to notify WebRTC of whether access to
  // audio/video devices was granted or not.
  content::MediaResponseCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(MediaStreamDevicesController);
};

#endif  // CHROME_BROWSER_MEDIA_WEBRTC_MEDIA_STREAM_DEVICES_CONTROLLER_H_
