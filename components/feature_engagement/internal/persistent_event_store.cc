// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/feature_engagement/internal/persistent_event_store.h"

#include <vector>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "components/feature_engagement/internal/stats.h"

namespace feature_engagement {
namespace {
// Corresponds to a UMA suffix "LevelDBOpenResults" in histograms.xml.
// Please do not change.
const char kDatabaseUMAName[] = "FeatureEngagementTrackerEventStore";

using KeyEventPair = std::pair<std::string, Event>;
using KeyEventList = std::vector<KeyEventPair>;

void NoopUpdateCallback(bool success) {
  stats::RecordDbUpdate(success, stats::StoreType::EVENTS_STORE);
}

}  // namespace

PersistentEventStore::PersistentEventStore(
    const base::FilePath& storage_dir,
    std::unique_ptr<leveldb_proto::ProtoDatabase<Event>> db)
    : storage_dir_(storage_dir),
      db_(std::move(db)),
      ready_(false),
      weak_ptr_factory_(this) {}

PersistentEventStore::~PersistentEventStore() = default;

void PersistentEventStore::Load(const OnLoadedCallback& callback) {
  DCHECK(!ready_);

  db_->Init(kDatabaseUMAName, storage_dir_,
            base::Bind(&PersistentEventStore::OnInitComplete,
                       weak_ptr_factory_.GetWeakPtr(), callback));
}

bool PersistentEventStore::IsReady() const {
  return ready_;
}

void PersistentEventStore::WriteEvent(const Event& event) {
  DCHECK(IsReady());
  std::unique_ptr<KeyEventList> entries = base::MakeUnique<KeyEventList>();
  entries->push_back(KeyEventPair(event.name(), event));

  db_->UpdateEntries(std::move(entries),
                     base::MakeUnique<std::vector<std::string>>(),
                     base::Bind(&NoopUpdateCallback));
}

void PersistentEventStore::DeleteEvent(const std::string& event_name) {
  DCHECK(IsReady());
  auto deletes = base::MakeUnique<std::vector<std::string>>();
  deletes->push_back(event_name);

  db_->UpdateEntries(base::MakeUnique<KeyEventList>(), std::move(deletes),
                     base::Bind(&NoopUpdateCallback));
}

void PersistentEventStore::OnInitComplete(const OnLoadedCallback& callback,
                                          bool success) {
  stats::RecordDbInitEvent(success, stats::StoreType::EVENTS_STORE);

  if (!success) {
    callback.Run(false, base::MakeUnique<std::vector<Event>>());
    return;
  }

  db_->LoadEntries(base::Bind(&PersistentEventStore::OnLoadComplete,
                              weak_ptr_factory_.GetWeakPtr(), callback));
}

void PersistentEventStore::OnLoadComplete(
    const OnLoadedCallback& callback,
    bool success,
    std::unique_ptr<std::vector<Event>> entries) {
  stats::RecordEventDbLoadEvent(success, *entries.get());
  ready_ = success;
  callback.Run(success, std::move(entries));
}

}  // namespace feature_engagement
