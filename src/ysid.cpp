/**
 * @file   ysid.cpp
 *
 * @brief ysid main implementation
 *
 *
 */
#include "ysid_config.h"
#include "ysid.hpp"
#include <memory>
#include "mongoose.h"

using namespace ysid;

namespace {
string pri_key;
string pub_key;
string password;
string path;

int port;

string password_callback(int times) {
  return password;
}
}

int main(int argc, char *argv[]) {
  option_parser parser;
  parser
      .add_value_option("pri|private|private-key", &pri_key, "private key")
      .add_value_option("password", &password, "passphrase")
      .add_value_option("pub|public|public-key", &pub_key, "public key")
      .add_value_option("spub", &spub_key, "session public key")
      .add_value_option("path", &path, "path")
      .add_value_option("port", &port, "port")
      .add_value_position(1, &path, "path")
      .add_value_position(1, &pri_key, "private key")
      .add_value_position(1, &pub_key, "public key")
      ;
  parser.parse(argc, argv);

  std::unique_ptr<ysid::cipher> cipher(new_rsa_cipher(prikey, pubkey, &password_callback));
  std::unique_ptr<ysid::kvstore> ds(new_file_kvstore(path));
  std::unique_ptr<ysid::kvstore> ms(new_mem_kvstore());

  return 0;
}
