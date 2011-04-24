/**
 * @file   test_kvstore.cpp
 *
 * @brief test kvstore module
 *
 *
 */
#include "ysid_config.h"
#include "ysid.hpp"
#include "crypt.hpp"
#include "kvstore.hpp"
#include <memory>
#include <iostream>

using namespace ysid;

int main(int argc, char *argv[]) {
  string path = argv[1];
  std::unique_ptr<kvstore> store(new_file_kvstore(path));
  int idx = 2;
  while (idx < argc) {
    string cmd = argv[idx];
    if (cmd == "put") {
      string key = argv[idx + 1];
      string val = argv[idx + 2];
      store->put(key, val);
      idx += 3;
      continue;
    }
    if (cmd == "has") {
      string key = argv[idx + 1];
      std::cout << store->has(key) << std::endl;
      idx += 2;
      continue;
    }
    if (cmd == "get") {
      string key = argv[idx + 1];
      std::cout << store->get(key) << std::endl;
      idx += 2;
      continue;
    }
    std::cerr << "invalid command " << cmd << std::endl;
    return 1;
  }
  return 0;
}
