// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_SERVICE_WORKER_CONTEXT_H_
#define CONTENT_PUBLIC_BROWSER_SERVICE_WORKER_CONTEXT_H_

#include <set>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "content/public/browser/service_worker_usage_info.h"
#include "url/gurl.h"

namespace content {

class ServiceWorkerContextObserver;

enum class ServiceWorkerCapability {
  NO_SERVICE_WORKER,
  SERVICE_WORKER_NO_FETCH_HANDLER,
  SERVICE_WORKER_WITH_FETCH_HANDLER,
};

// Used for UMA. Append only.
enum class StartServiceWorkerForNavigationHintResult {
  // The service worker started successfully.
  STARTED = 0,
  // The service worker was already running.
  ALREADY_RUNNING = 1,
  // There was no service worker registration for the given URL.
  NO_SERVICE_WORKER_REGISTRATION = 2,
  // There was no active service worker for the given URL.
  NO_ACTIVE_SERVICE_WORKER_VERSION = 3,
  // The service worker for the given URL had no fetch event handler.
  NO_FETCH_HANDLER = 4,
  // Something failed.
  FAILED = 5,
  // Add new result to record here.
  NUM_TYPES
};

// Represents the per-StoragePartition service worker data.
//
// See service_worker_context_wrapper.cc for the implementation
// of ServiceWorkerContext and ServiceWorkerContextWrapper (the
// primary implementation of this abstract class).
class ServiceWorkerContext {
 public:
  using ResultCallback = base::OnceCallback<void(bool success)>;

  using GetUsageInfoCallback = base::OnceCallback<void(
      const std::vector<ServiceWorkerUsageInfo>& usage_info)>;

  using CheckHasServiceWorkerCallback =
      base::OnceCallback<void(ServiceWorkerCapability capability)>;

  using CountExternalRequestsCallback =
      base::OnceCallback<void(size_t external_request_count)>;

  using StartServiceWorkerForNavigationHintCallback =
      base::Callback<void(StartServiceWorkerForNavigationHintResult result)>;

  using StartActiveWorkerCallback =
      base::OnceCallback<void(int process_id, int thread_id)>;

  // Registers the header name which should not be passed to the ServiceWorker.
  // Must be called from the IO thread.
  CONTENT_EXPORT static void AddExcludedHeadersForFetchEvent(
      const std::set<std::string>& header_names);

  // Returns true if the header name should not be passed to the ServiceWorker.
  // Must be called from the IO thread.
  static bool IsExcludedHeaderNameForFetchEvent(const std::string& header_name);

  // Returns true if |url| is within the service worker |scope|.
  CONTENT_EXPORT static bool ScopeMatches(const GURL& scope, const GURL& url);

  // Observer methods are always dispatched on the UI thread.
  virtual void AddObserver(ServiceWorkerContextObserver* observer) = 0;
  virtual void RemoveObserver(ServiceWorkerContextObserver* observer) = 0;

  // Equivalent to calling navigator.serviceWorker.register(script_url, {scope:
  // scope}). |callback| is passed true when the JS promise is fulfilled or
  // false when the JS promise is rejected.
  //
  // The registration can fail if:
  //  * |script_url| is on a different origin from |scope|
  //  * Fetching |script_url| fails.
  //  * |script_url| fails to parse or its top-level execution fails.
  //  * Something unexpected goes wrong, like a renderer crash or a full disk.
  //
  // This function can be called from any thread, but the callback will always
  // be called on the UI thread.
  virtual void RegisterServiceWorker(const GURL& scope,
                                     const GURL& script_url,
                                     ResultCallback callback) = 0;

  // Equivalent to calling ServiceWorkerRegistration#unregister on the
  // registration for |scope|. |callback| is passed true when the JS promise is
  // fulfilled or false when the JS promise is rejected.
  //
  // Unregistration can fail if:
  //  * No registration exists for |scope|.
  //  * Something unexpected goes wrong, like a renderer crash.
  //
  // This function can be called from any thread, but the callback will always
  // be called on the UI thread.
  virtual void UnregisterServiceWorker(const GURL& scope,
                                       ResultCallback callback) = 0;

  // Mechanism for embedder to increment/decrement ref count of a service
  // worker.
  // Embedders can call StartingExternalRequest() while it is performing some
  // work with the worker. The worker is considered to be working until embedder
  // calls FinishedExternalRequest(). This ensures that content/ does not
  // shut the worker down while embedder is expecting the worker to be kept
  // alive.
  //
  // Must be called from the IO thread. Returns whether or not changing the ref
  // count succeeded.
  virtual bool StartingExternalRequest(int64_t service_worker_version_id,
                                       const std::string& request_uuid) = 0;
  virtual bool FinishedExternalRequest(int64_t service_worker_version_id,
                                       const std::string& request_uuid) = 0;
  // Returns the pending external request count for the worker with the
  // specified |origin| via |callback|. Must be called from the UI thread.
  virtual void CountExternalRequestsForTest(
      const GURL& origin,
      CountExternalRequestsCallback callback) = 0;

  // Must be called from the IO thread.
  virtual void GetAllOriginsInfo(GetUsageInfoCallback callback) = 0;

  // This function can be called from any thread, but the callback will always
  // be called on the IO thread.
  virtual void DeleteForOrigin(const GURL& origin_url,
                               ResultCallback callback) = 0;

  // Returns ServiceWorkerCapability describing existence and properties of a
  // Service Worker registration matching |url|. Found service worker
  // registration must also encompass the |other_url|, otherwise it will be
  // considered non existent by this method. Note that the longest matching
  // registration for |url| is described, which is not necessarily the longest
  // matching registration for |other_url|. In case the service worker is being
  // installed as of calling this method, it will wait for the installation to
  // finish before coming back with the result.
  //
  // This function can be called from any thread, but the callback will always
  // be called on the UI thread.
  virtual void CheckHasServiceWorker(
      const GURL& url,
      const GURL& other_url,
      CheckHasServiceWorkerCallback callback) = 0;

  // Stops all running service workers and unregisters all service worker
  // registrations. This method is used in LayoutTests to make sure that the
  // existing service worker will not affect the succeeding tests.
  //
  // This function can be called from any thread, but the callback will always
  // be called on the UI thread.
  virtual void ClearAllServiceWorkersForTest(base::OnceClosure callback) = 0;

  // Starts the active worker of the registration whose scope is |pattern|.
  // |info_callback| is passed the worker's render process id and thread id.
  //
  // Must be called on IO thread.
  virtual void StartActiveWorkerForPattern(
      const GURL& pattern,
      StartActiveWorkerCallback info_callback,
      base::OnceClosure failure_callback) = 0;

  // Starts the service worker for |document_url|. Called when a navigation to
  // that URL is predicted to occur soon. Must be called from the UI thread. The
  // |callback| will always be called on the UI thread.
  virtual void StartServiceWorkerForNavigationHint(
      const GURL& document_url,
      const StartServiceWorkerForNavigationHintCallback& callback) = 0;

  // Stops all running workers on the given |origin|.
  //
  // This function can be called from any thread.
  virtual void StopAllServiceWorkersForOrigin(const GURL& origin) = 0;

 protected:
  ServiceWorkerContext() {}
  virtual ~ServiceWorkerContext() {}
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_SERVICE_WORKER_CONTEXT_H_
