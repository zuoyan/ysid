/**
 * @file   test_crypt.cpp
 *
 * @brief test crypt module
 *
 *
 */
#undef NDEBUG
#include <cassert>
#include "ysid_config.h"
#include <ysid.hpp>
#include <memory>
#include "crypt.hpp"

using ysid::string;

string const_password;

void test_cipher(ysid::cipher *cipher, const string &plain) {
  string crypted = cipher->encrypt(plain);
  assert(cipher->decrypt(crypted) == plain);
}

void test_cipher(ysid::cipher *cipher, const string &s, size_t times) {
  string plain = s;
  for (size_t i = 1; i < times; ++i) {
    plain += s;
  }
  test_cipher(cipher, plain);
}

string password_callback(int ) {
  return const_password;
}

int main(int argc, char *argv[]) {
  string prikey(argv[1]);
  string pubkey(argv[2]);
  const_password = argv[3];

  std::unique_ptr<ysid::cipher> cipher(ysid::new_rsa_cipher());
  cipher->load_private(prikey, password_callback);
  cipher->load_public(pubkey);

  test_cipher(cipher.get(), "");
  test_cipher(cipher.get(), "0");
  test_cipher(cipher.get(), "1");
  test_cipher(cipher.get(), "0", 100);
  test_cipher(cipher.get(), "10", 100);
  test_cipher(cipher.get(), "10", 1000);
  test_cipher(cipher.get(), "101", 10000);
  test_cipher(cipher.get(), "test crypt");
  test_cipher(cipher.get(), "test crypt", 201);

  return 0;
}
