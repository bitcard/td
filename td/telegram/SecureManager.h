//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/net/NetQuery.h"
#include "td/telegram/SecureValue.h"
#include "td/telegram/UserId.h"

#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"

#include "td/actor/actor.h"
#include "td/actor/PromiseFuture.h"

#include "td/utils/Container.h"
#include "td/utils/Status.h"

#include <map>
#include <memory>
#include <unordered_map>
#include <utility>

namespace td {

class Td;

using TdApiSecureValue = td_api::object_ptr<td_api::PassportElement>;
using TdApiSecureValues = td_api::object_ptr<td_api::passportElements>;
using TdApiAuthorizationForm = td_api::object_ptr<td_api::passportAuthorizationForm>;

class SecureManager : public NetQueryCallback {
 public:
  explicit SecureManager(ActorShared<> parent);

  void get_secure_value(std::string password, SecureValueType type, Promise<TdApiSecureValue> promise);
  void get_all_secure_values(std::string password, Promise<TdApiSecureValues> promise);
  void set_secure_value(string password, SecureValue secure_value, Promise<TdApiSecureValue> promise);
  void delete_secure_value(SecureValueType type, Promise<Unit> promise);
  void set_secure_value_errors(Td *td, tl_object_ptr<telegram_api::InputUser> input_user,
                               vector<tl_object_ptr<td_api::inputPassportElementError>> errors, Promise<Unit> promise);

  void on_get_secure_value(SecureValueWithCredentials value);

  void get_passport_authorization_form(string password, UserId bot_user_id, string scope, string public_key,
                                       string nonce, Promise<TdApiAuthorizationForm> promise);
  void send_passport_authorization_form(int32 authorization_form_id, std::vector<SecureValueType> types,
                                        Promise<> promise);

  void get_preferred_country_code(string country_code, Promise<td_api::object_ptr<td_api::text>> promise);

 private:
  ActorShared<> parent_;
  int32 refcnt_{1};
  std::map<SecureValueType, ActorOwn<>> set_secure_value_queries_;
  std::map<SecureValueType, SecureValueWithCredentials> secure_value_cache_;

  struct AuthorizationForm {
    UserId bot_user_id;
    string scope;
    string public_key;
    string nonce;
    bool is_received;
    std::map<SecureValueType, SuitableSecureValue> options;
  };

  std::unordered_map<int32, AuthorizationForm> authorization_forms_;
  int32 max_authorization_form_id_{0};

  void hangup() override;
  void hangup_shared() override;
  void dec_refcnt();
  void on_delete_secure_value(SecureValueType type, Promise<Unit> promise, Result<Unit> result);
  void on_get_passport_authorization_form(
      int32 authorization_form_id, Promise<TdApiAuthorizationForm> promise,
      Result<std::pair<std::map<SecureValueType, SuitableSecureValue>, TdApiAuthorizationForm>> r_authorization_form);

  void on_result(NetQueryPtr query) override;
  Container<Promise<NetQueryPtr>> container_;
  void send_with_promise(NetQueryPtr query, Promise<NetQueryPtr> promise);
};

}  // namespace td
