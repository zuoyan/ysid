/**
 * @file   crypt.hpp
 *
 * @brief crypt interface
 *
 *
 */
#ifndef FILE_4b23ca83_d29b_4ccb_9209_a065d047ba22_H
#define FILE_4b23ca83_d29b_4ccb_9209_a065d047ba22_H 1
#include "string.hpp"
#include <functional>

namespace ysid {

struct cipher {
  virtual string encrypt(const string &plain) = 0;
  virtual string decrypt(const string &cipher) = 0;
  virtual void load_public(const string &pub) = 0;
  virtual void load_private(const string &pri, std::function<string(int)> cb) = 0;
  virtual void clear_private() {} // clear private information
  virtual void clear_public() {} // clear public information
  virtual ~cipher() { };
};

cipher* new_rsa_cipher();
unsigned char *rand_bytes(unsigned char *buf, size_t l);
unsigned rand_uint();
int rand_range(int min, int max);

}
#endif
