/***************************************************************************************************
 *  Copyright 2012 maidsafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use. The use of this code is governed by the licence file licence.txt found in the root of     *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit written *
 *  permission of the board of directors of MaidSafe.net.                                          *
 **************************************************************************************************/

#include "maidsafe/vault/maid_account_holder/maid_account.h"

#include <string>

#include "boost/variant/apply_visitor.hpp"

#include "maidsafe/common/utils.h"

#include "maidsafe/vault/account_db.h"
#include "maidsafe/vault/maid_account_holder/maid_account.pb.h"


namespace maidsafe {

namespace vault {

PmidTotals::PmidTotals() : serialised_pmid_registration(), pmid_record() {}

PmidTotals::PmidTotals(
    const nfs::PmidRegistration::serialised_type& serialised_pmid_registration_in)
        : serialised_pmid_registration(serialised_pmid_registration_in),
          pmid_record() {}

PmidTotals::PmidTotals(
    const nfs::PmidRegistration::serialised_type& serialised_pmid_registration_in,
    const PmidRecord& pmid_record_in)
        : serialised_pmid_registration(serialised_pmid_registration_in),
          pmid_record(pmid_record_in) {}

PmidTotals::PmidTotals(const PmidTotals& other)
    : serialised_pmid_registration(other.serialised_pmid_registration),
      pmid_record(other.pmid_record) {}

PmidTotals::PmidTotals(PmidTotals&& other)
    : serialised_pmid_registration(std::move(other.serialised_pmid_registration)),
      pmid_record(std::move(other.pmid_record)) {}

PmidTotals& PmidTotals::operator=(PmidTotals other) {
  using std::swap;
  swap(serialised_pmid_registration, other.serialised_pmid_registration);
  swap(pmid_record, other.pmid_record);
  return *this;
}

MaidAccount::MaidAccount(const MaidName& maid_name, Db& db, const NodeId& this_node_id)
    : maid_name_(maid_name),
      pmid_totals_(),
      total_claimed_available_size_by_pmids_(0),
      total_put_data_(0),
      account_db_(new AccountDb(db)),
      sync_(account_db_.get(), this_node_id) {}

MaidAccount::MaidAccount(const MaidName& maid_name,
                         Db& db,
                         const NodeId& this_node_id,
                         const serialised_type& serialised_maid_account_details)
    : maid_name_(maid_name),
      pmid_totals_(),
      total_claimed_available_size_by_pmids_(0),
      total_put_data_(0),
      account_db_(new AccountDb(db)),
      sync_(account_db_.get(), this_node_id) {
  ApplyAccountTransfer(serialised_maid_account_details);
}

MaidAccount::MaidAccount(MaidAccount&& other)
    : maid_name_(std::move(other.maid_name_)),
      pmid_totals_(std::move(other.pmid_totals_)),
      total_claimed_available_size_by_pmids_(std::move(
                                             other.total_claimed_available_size_by_pmids_)),
      total_put_data_(std::move(other.total_put_data_)),
      account_db_(std::move(other.account_db_)),
      sync_(std::move(other.sync_)) {}

MaidAccount& MaidAccount::operator=(MaidAccount&& other) {
  maid_name_ = std::move(other.maid_name_);
  pmid_totals_ = std::move(other.pmid_totals_);
  total_claimed_available_size_by_pmids_ = std::move(other.total_claimed_available_size_by_pmids_);
  total_put_data_ = std::move(other.total_put_data_);
  account_db_ = std::move(other.account_db_);
  sync_ = std::move(other.sync_);
  return *this;
}

MaidAccount::serialised_type MaidAccount::Serialise() {
  protobuf::MaidAccountDetails proto_maid_account_details;
  for (const auto& pmid_total : pmid_totals_) {
    proto_maid_account_details.add_serialised_pmid_registration(
        pmid_total.serialised_pmid_registration->string());
  }

  auto db_entries(account_db_->Get());
  GetTagValueAndIdentityVisitor type_and_name_visitor;
  for (const auto& db_entry : db_entries) {
    auto type_and_name(boost::apply_visitor(type_and_name_visitor, db_entry.first));
    auto proto_db_entry(proto_maid_account_details.add_db_entry());
    proto_db_entry->set_type(static_cast<int32_t>(type_and_name.first));
    proto_db_entry->set_name(type_and_name.second.string());
    protobuf::MaidAccountDbValue proto_db_value;
    if (!proto_db_value.ParseFromString(db_entry.second.string()))
      ThrowError(CommonErrors::parsing_error);
    proto_db_entry->mutable_value()->CopyFrom(proto_db_value);
  }

  auto unresolved_data(sync_.GetUnresolvedData());
  for (const auto& unresolved_entry : unresolved_data) {
    proto_maid_account_details.add_serialised_unresolved_entry(
        unresolved_entry.Serialise()->string());
  }

  return serialised_type(NonEmptyString(proto_maid_account_details.SerializeAsString()));
}

void MaidAccount::PutData(int32_t cost) {
  if (total_claimed_available_size_by_pmids_ < total_put_data_ + cost)
    ThrowError(VaultErrors::not_enough_space);
}

MaidAccount::Status MaidAccount::DoPutData(int32_t cost) {
  total_put_data_ += cost;
  if (total_put_data_ > (total_claimed_available_size_by_pmids_ / 10) * 9)
    return Status::kLowSpace;
  else
    return Status::kOk;
}

void MaidAccount::ApplyAccountTransfer(const serialised_type& serialised_maid_account_details) {
  protobuf::MaidAccountDetails proto_maid_account_details;
  if (!proto_maid_account_details.ParseFromString(serialised_maid_account_details->string()))
    ThrowError(CommonErrors::parsing_error);

  for (int i(0); i != proto_maid_account_details.serialised_pmid_registration_size(); ++i) {
    pmid_totals_.emplace_back(
        nfs::PmidRegistration::serialised_type(NonEmptyString(
            proto_maid_account_details.pmid_totals(i).serialised_pmid_registration())));
  }

  for (int i(0); i != proto_maid_account_details.db_entry_size(); ++i) {
    auto data_name(GetDataNameVariant(
        static_cast<DataTagValue>(proto_maid_account_details.db_entry(i).type()),
        Identity(proto_maid_account_details.db_entry(i).name())));
    int32_t average_cost(proto_maid_account_details.db_entry(i).value().average_cost());
    int32_t count(proto_maid_account_details.db_entry(i).value().count());
    MaidAndPmidUnresolvedEntry entry(std::make_pair(data_name, nfs::MessageAction::kPut),
                                      average_cost);
    for (int32_t i(0); i != count; ++i)
      sync_.AddUnresolvedEntry(entry);
  }

  for (int i(0); i != proto_maid_account_details.serialised_unresolved_entry_size(); ++i) {
    MaidAndPmidUnresolvedEntry entry(MaidAndPmidUnresolvedEntry::serialised_type(
        NonEmptyString(proto_maid_account_details.serialised_unresolved_entry(i))));
    sync_.AddUnresolvedEntry(entry);
  }
}

std::vector<PmidTotals>::iterator MaidAccount::Find(const PmidName& pmid_name) {
  return std::find_if(std::begin(pmid_totals_),
                      std::end(pmid_totals_),
                      [&pmid_name](const PmidTotals& pmid_totals) {
                        return pmid_name == pmid_totals.pmid_record.pmid_name;
                      });
}

void MaidAccount::RegisterPmid(const nfs::PmidRegistration& pmid_registration) {
  auto itr(Find(pmid_registration.pmid_name()));
  if (itr == std::end(pmid_totals_)) {
    nfs::PmidRegistration::serialised_type serialised_pmid_registration(
        pmid_registration.Serialise());
    pmid_totals_.emplace_back(serialised_pmid_registration,
                              PmidRecord(pmid_registration.pmid_name()));
  }
}

void MaidAccount::UnregisterPmid(const PmidName& pmid_name) {
  auto itr(Find(pmid_name));
  if (itr != std::end(pmid_totals_))
    pmid_totals_.erase(itr);
}

void MaidAccount::UpdatePmidTotals(const PmidTotals& pmid_totals) {
  auto itr(Find(pmid_totals.pmid_record.pmid_name));
  if (itr == std::end(pmid_totals_))
    ThrowError(CommonErrors::no_such_element);
  *itr = pmid_totals;
}

}  // namespace vault

}  // namespace maidsafe
