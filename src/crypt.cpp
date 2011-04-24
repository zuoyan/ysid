/**
 * @file   crypt.cpp
 *
 * @brief encrypt and decrypt implementation
 *
 *
 */
#include "ysid_config.h"
#include "crypt.hpp"
#include "ffile.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <functional>
#include <climits>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>

using ysid::string;

extern "C" {

static int password_function_cb(
    char *buf, int size, int rwflag, void *u) {
  std::function<string(int)> *func;
  func = (decltype(func))u;
  int times = 0;
  while (times < 3) {
    string s = (*func)(times++);
    if (s.size() > (size_t)size) continue;
    memcpy(buf, s.data(), s.size());
    return s.size();
    break;
  }
  return 0;
}

} // end of extern "C"

namespace ysid {
namespace { // namespace anonymous

class rsa_cipher : public cipher {
 public:
  string encrypt(const string &value);
  string decrypt(const string &value);

  rsa_cipher();
  virtual ~rsa_cipher();
  void load_public(const string &pub);
  void load_private(const string &pri, std::function<string(int)> cb);
  void clear_private();
  void clear_public();

 private:
  std::string get_error();
  void load_key_error(const string &role, const string &fn);

  RSA *m_pri_key, *m_pub_key;
};

rsa_cipher::rsa_cipher() {
  m_pri_key = NULL;
  m_pub_key = NULL;
}

rsa_cipher::~rsa_cipher() {
  if (m_pri_key) RSA_free(m_pri_key);
  if (m_pub_key) RSA_free(m_pub_key);
}

void rsa_cipher::clear_private() {
  if (m_pri_key) {
    RSA_free(m_pri_key);
    m_pri_key = NULL;
  }
}

void rsa_cipher::clear_public() {
  if (m_pub_key) {
    RSA_free(m_pub_key);
    m_pub_key = NULL;
  }
}


std::string rsa_cipher::get_error() {
  unsigned long e = ERR_get_error();
  char buf[121];
  return ERR_error_string(e, buf);
}

string rsa_cipher::encrypt(const string &value) {
  if (m_pub_key == 0) throw std::runtime_error("encrypt error, no public key");
  size_t rs = RSA_size(m_pub_key);
  int iblk = rs - 2 * (20 + 1);
  string to(rs, (char)0);
  string ret;
  for (size_t pos = 0, rlen = 0; pos < value.size(); pos += rlen) {
    rlen = std::min(value.size(), pos + iblk) - pos;
    int elen = RSA_public_encrypt(rlen,
                                  (const unsigned char*)&value[pos],
                                  (unsigned char*)&to[0],
                                  m_pub_key, RSA_PKCS1_OAEP_PADDING);
    if (elen < 0) {
      throw std::runtime_error("encrypt error:" + get_error());
    } else {
      ret.append(&to[0], &to[0] + elen);
    }
  }
  return ret;
}

string rsa_cipher::decrypt(const string &value) {
  if (m_pri_key == 0) throw std::runtime_error("encrypt error, no private key");
  size_t rs = RSA_size(m_pri_key);
  string to(rs, (char)0);
  string ret;
  for (size_t pos = 0, rlen = 0; pos < value.size(); pos += rlen) {
    rlen = std::min(value.size(), pos + rs) - pos;
    int dlen = RSA_private_decrypt(rlen,
                                   (const unsigned char*)&value[pos],
                                   (unsigned char*)&to[0],
                                   m_pri_key, RSA_PKCS1_OAEP_PADDING);
    if (dlen < 0) {
      throw std::runtime_error(
          "decrypt error " + get_error());
    } else {
      ret.append(&to[0], &to[0] + dlen);
    }
  }
  return ret;
}

void rsa_cipher::load_key_error(const string &role, const string &fn) {
  string sn = fn;
  if (sn.size() > 100) sn = sn.substr(0, 100) + "...";
  throw std::runtime_error(
      "load " + role + " key from (" + sn + ") failed:" + get_error());
}

void rsa_cipher::load_public(const string &pubkey) {
  clear_public();
  string k = pubkey;
  if (k.find("--BEGIN ") == string::npos) {
    k = ffile_read(k);
  }
  if (!k.size()) return;
  BIO *b = BIO_new_mem_buf((void*)k.data(), k.size());
  EVP_PKEY *key = PEM_read_bio_PUBKEY(
      b, NULL, NULL, NULL);
  if (key == NULL) load_key_error("pub", k);
  m_pub_key = EVP_PKEY_get1_RSA(key);
  EVP_PKEY_free(key);
  if (m_pub_key == NULL) load_key_error("pub rsa", k);
  BIO_free(b);
}

void rsa_cipher::load_private(const string &prikey,
                              std::function<string(int)> password_cb) {
  clear_private();
  string k = prikey;
  if (k.find("--BEGIN ") == string::npos) {
    k = ffile_read(k);
  }
  if (!k.size()) return;
  BIO *b = BIO_new_mem_buf((void*)k.data(), k.size());
  EVP_PKEY *key = PEM_read_bio_PrivateKey(
      b, NULL,
      &password_function_cb, &password_cb);
  if (key == NULL) load_key_error("pri", k);
  m_pri_key = EVP_PKEY_get1_RSA(key);
  EVP_PKEY_free(key);
  if (m_pri_key == NULL) load_key_error("pri rsa", k);
  BIO_free(b);
}

} // namespace anonymous

cipher* new_rsa_cipher() {
  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();
  return new rsa_cipher();
}

unsigned char *rand_bytes(unsigned char *buf, size_t l) {
  RAND_bytes(buf, l);
  return buf;
}

unsigned rand_uint() {
  unsigned u;
  rand_bytes((unsigned char *)&u, sizeof(u));
  return u;
}

int rand_range(int min, int max) {
  unsigned u;
  unsigned r = max - min + 1;
  unsigned ub = UINT_MAX - (UINT_MAX % r);
  do {
    u = rand_uint();
  } while (u > ub);
  return min + u % r;
}

} // namespace ysid
