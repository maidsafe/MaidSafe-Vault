/*  Copyright 2015 MaidSafe.net limited

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

#ifndef MAIDSAFE_VAULT_DATA_MANAGER_H_
#define MAIDSAFE_VAULT_DATA_MANAGER_H_

#include "maidsafe/common/types.h"
#include "maidsafe/routing/types.h"

namespace maidsafe {

namespace vault {


template <typename Child>
class DataManager {
 public:
  DataManager() {}

  template <typename DataType>
  void HandleGet(routing::SourceAddress from, Identity data_name);
  template <typename DataType>
  void HandlePut(routing::SourceAddress /* from */, Identity /* data_name */, DataType /* data */);
  void HandleChurn(routing::CloseGroupDifference);
};

}  // namespace vault

}  // namespace maidsafe

#endif // MAIDSAFE_VAULT_DATA_MANAGER_H_
