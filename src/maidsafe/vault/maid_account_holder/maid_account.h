/***************************************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#ifndef MAIDSAFE_VAULT_MAID_ACCOUNT_HOLDER_MAID_ACCOUNT_H_
#define MAIDSAFE_VAULT_MAID_ACCOUNT_HOLDER_MAID_ACCOUNT_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "maidsafe/common/node_id.h"
#include "maidsafe/common/types.h"
#include "maidsafe/nfs/pmid_registration.h"

#include "maidsafe/vault/db.h"
#include "maidsafe/vault/sync.h"
#include "maidsafe/vault/types.h"
#include "maidsafe/vault/pmid_account_holder/pmid_record.h"
#include "maidsafe/vault/maid_account_holder/maid_account_merge_policy.h"


namespace maidsafe {

namespace vault {

class Db;
class AccountDb;

namespace protobuf { class MaidAccount; }

namespace test {

class MaidAccountHandlerTest;

template<typename Data>
class MaidAccountHandlerTypedTest;

}  // namespace test


struct PmidTotals {
  PmidTotals();
  explicit PmidTotals(
      const nfs::PmidRegistration::serialised_type& serialised_pmid_registration_in);
  PmidTotals(const nfs::PmidRegistration::serialised_type& serialised_pmid_registration_in,
             const PmidRecord& pmid_record_in);
  PmidTotals(const PmidTotals& other);
  PmidTotals(PmidTotals&& other);
  PmidTotals& operator=(PmidTotals other);

  nfs::PmidRegistration::serialised_type serialised_pmid_registration;
  PmidRecord pmid_record;
};

class MaidAccount {
 public:
  enum class Status { kOk, kLowSpace };
  typedef MaidName name_type;
  typedef TaggedValue<NonEmptyString, struct SerialisedMaidAccountTag> serialised_type;

  // For client adding new account
  MaidAccount(const MaidName& maid_name, Db& db, const NodeId& this_node_id);
  // For creating new account via account transfer
  MaidAccount(const MaidName& maid_name,
              Db& db,
              const NodeId& this_node_id,
              const serialised_type& serialised_maid_account_details);

  MaidAccount(MaidAccount&& other);
  MaidAccount& operator=(MaidAccount&& other);
//  void ArchiveToDisk() const;
  serialised_type Serialise();

  void ApplyAccountTransfer(const serialised_type& serialised_maid_account_details);
  void RegisterPmid(const nfs::PmidRegistration& pmid_registration);
  void UnregisterPmid(const PmidName& pmid_name);
  void UpdatePmidTotals(const PmidTotals& pmid_totals);

  // headers and unresolved data
  NonEmptyString GetSyncData();
  void ApplySyncData();
  void ReplaceNode(const NodeId& old_node, const NodeId& new_node) {
    sync_.ReplaceNode(old_node, new_node);
  }

  void PutData(int32_t cost);
  // This offers the strong exception guarantee
  template<typename Data>
  void DeleteData(const typename Data::name_type& name) {
    total_put_data_ -= sync_.AllowDelete<Data>(name);
  }
  name_type name() const { return maid_name_; }

  friend class test::MaidAccountHandlerTest;
  template<typename Data>
  friend class test::MaidAccountHandlerTypedTest;

 private:
  MaidAccount(const MaidAccount&);
  MaidAccount& operator=(const MaidAccount&);

  std::vector<PmidTotals>::iterator Find(const PmidName& pmid_name);

  Status DoPutData(int32_t cost);

  name_type maid_name_;
  std::vector<PmidTotals> pmid_totals_;
  int64_t total_claimed_available_size_by_pmids_, total_put_data_;
  std::unique_ptr<AccountDb> account_db_;
  Sync<MaidAccountMergePolicy> sync_;
};

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_MAID_ACCOUNT_HOLDER_MAID_ACCOUNT_H_
