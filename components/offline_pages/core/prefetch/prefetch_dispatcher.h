// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_DISPATCHER_H_
#define COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_DISPATCHER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "components/offline_pages/core/client_id.h"
#include "components/offline_pages/core/offline_page_item.h"
#include "components/offline_pages/core/prefetch/prefetch_types.h"

namespace offline_pages {
class PrefetchService;

// Serves as the entry point for external signals into the prefetching system.
// It listens to these events, converts them to the appropriate internal tasks
// and manage their execution and inter-dependencies.
// Tasks are generally categorized as one of the follwoing types:
// 1. Event handlers. These react to incoming events, such as new URLs coming
//    from suggestion service or network request finished and response is
//    available. Typical task of this type would capture the incoming data into
//    pipeline and indicate that more processing of pipeline is needed by
//    calling PrefetchDispatcher::SchedulePipelineProcessing().
// 2. Reconcilers. These are tasks that are invoked on periodic wakeup
//    (BackgroundTask) and are responsible for checking status of ongoing
//    operations, such as network request or a download. If the failure
//    condition found (as a result of Chrome being killed in the middle of
//    network request for example), they make necessary adjustments and
//    optionally call PrefetchDispatcher::SchedulePipelineProcessing()
// 3. Actions. These are expecting the prefetch items database looking for
//    items that are ready for some applied actions - for example, to start
//    a network request or download, or to be expired etc. These tasks are
//    scheduled as a bundle when TaskQueue becomes empty in response to
//    PrefetchDispatcher::SchedulePipelineProcessing(), and also once after
//    Reconcilers during BackgroundTask processing.
class PrefetchDispatcher {
 public:
  // A |ScopedBackgroundTask| is created when we are running in a background
  // task.  Destroying this object should notify the system that we are done
  // processing the background task.
  class ScopedBackgroundTask {
   public:
    ScopedBackgroundTask() = default;
    virtual ~ScopedBackgroundTask() = default;

    // Used on destruction to inform the system about whether rescheduling with
    // or without backoff is required.
    virtual void SetNeedsReschedule(bool reschedule, bool backoff) = 0;
  };

  virtual ~PrefetchDispatcher() = default;

  // Initializes the dispatcher with its respective service instance. This must
  // be done before any other methods are called.
  virtual void SetService(PrefetchService* service) = 0;

  // Called by an Event Handler or Reconciler Task in case it modified state
  // of one or more prefetch items and needs Actions tasks to examine/process
  // pipeline again.
  virtual void SchedulePipelineProcessing() = 0;

  // Called by Event Handler tasks to ensure that we will wake up in the
  // background to process data produced by the event handler.  For example, if
  // new URLs are added to the system, this is called to process them later in
  // the background.  If we are in a background task, requests rescheduling at
  // the end of the task.  Otherwise, updates the task in the OS scheduler.
  // Does not update any existing task, so will not affect backoff.
  virtual void EnsureTaskScheduled() = 0;

  // Called when a client has candidate URLs for the system to prefetch, along
  // with the client's unique namespace. URLs that are currently in the system
  // for this client are acceptable but ignored.
  virtual void AddCandidatePrefetchURLs(
      const std::string& name_space,
      const std::vector<PrefetchURL>& prefetch_urls) = 0;

  // Called when all existing suggestions are no longer considered valid for a
  // given namespace.  The prefetch system should remove any URLs that
  // have not yet started downloading within that namespace.
  virtual void RemoveAllUnprocessedPrefetchURLs(
      const std::string& name_space) = 0;

  // Called to invalidate a single PrefetchURL entry identified by |client_id|.
  // If multiple have the same |client_id|, they will all be removed.
  virtual void RemovePrefetchURLsByClientId(const ClientId& client_id) = 0;

  // Called when Android OS has scheduled us for background work.  When
  // destroyed, |task| will call back and inform the OS that we are done work
  // (if required).  |task| also manages rescheduling behavior.
  virtual void BeginBackgroundTask(
      std::unique_ptr<ScopedBackgroundTask> task) = 0;

  // Called when a task must stop immediately due to system constraints. After
  // this call completes, the system will reschedule the task based on whether
  // SetNeedsReschedule has been called.
  virtual void StopBackgroundTask() = 0;

  // Called when the GCM app handler receives a GCM message with an embeddeed
  // operation name.
  virtual void GCMOperationCompletedMessageReceived(
      const std::string& operation_name) = 0;

  // Called when a download service is up and notifies us about the ongoing and
  // success downloads. The prefetch system should clean up the database to
  // be consistent with those reported from the download system.
  // Note that the failed downloads are not reported. The cleanup task can
  // easily figure this out from both sides and take care of them.
  virtual void CleanupDownloads(
      const std::set<std::string>& outstanding_download_ids,
      const std::map<std::string, std::pair<base::FilePath, int64_t>>&
          success_downloads) = 0;

  // Called when a download is completed successfully or fails.
  virtual void DownloadCompleted(
      const PrefetchDownloadResult& download_result) = 0;

  // Called when an archive import is completed successfully or fails.
  virtual void ImportCompleted(int64_t offline_id, bool success) = 0;

  // Used by the test to signal the completion of the background task.
  virtual void RequestFinishBackgroundTaskForTest() = 0;
};

}  // namespace offline_pages

#endif  // COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_PREFETCH_DISPATCHER_H_
