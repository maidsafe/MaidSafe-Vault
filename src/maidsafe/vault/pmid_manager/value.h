/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_VAULT_PMID_MANAGER_VALUE_H_
#define MAIDSAFE_VAULT_PMID_MANAGER_VALUE_H_

#include <cstdint>
#include <string>
#include <vector>

#include "maidsafe/data_types/data_name_variant.h"
#include "boost/variant/variant.hpp"

namespace maidsafe {
namespace vault {

class PmidManagerValue {
 public:
  PmidManagerValue();
  explicit PmidManagerValue(const std::string& serialised_pmid_manager_value);
  PmidManagerValue(const PmidManagerValue& other);
  PmidManagerValue(PmidManagerValue&& other);
  PmidManagerValue& operator=(PmidManagerValue other);

  std::string Serialise() const;

  void Delete(const DataNameVariant& data_name);
  void Add(const DataNameVariant& data_name, int32_t size);

  friend void swap(PmidManagerValue& lhs, PmidManagerValue& rhs);
  friend bool operator==(const PmidManagerValue& lhs, const PmidManagerValue& rhs);

 private:
  typedef std::pair<DataNameVariant, int32_t> ValueType;
  std::set<ValueType, std::function<bool(const ValueType&, const ValueType&)>> data_elements_;
};

bool operator==(const PmidManagerValue& lhs, const PmidManagerValue& rhs);

}  // namespace vault
}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_PMID_MANAGER_VALUE_H_
