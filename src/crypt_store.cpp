/**
 * @file   crypt_store.cpp
 *
 * @brief crypt store
 *
 *
 */
#include "ysid_config.h"
#include "ysid.hpp"
#include "crypt.hpp"
#include "kvstore.hpp"

namespace ysid {

struct crypt_store : kvstore {
  crypt_store(cipher *c, kvstore *db)
      : m_cipher(c), m_store(db) {}

  bool has(const string &k) {
    return m_store->has(k);
  }

  string get(const string &k) {
    string v;
    if (m_store->has(k)) {
      v = m_cipher->decrypt(m_store->get(k));
    }
    return v;
  }

  int put(const string &k, const string &v) {
    string e = m_cipher->encrypt(v);
    return m_store->put(k, e);
  }

  int sync() {
    return m_store->sync();
  }

  void fold(
      std::function<void (const string &key, const string &value, void *data)> func,
      void *data) {
    auto ifunc = [&](const string &key, const string &value, void *data) {
      string v = m_cipher->decrypt(value);
      func(key, v, data);
    };
    m_store->fold(ifunc, data);
  }

  void fold_keys(
      std::function<void (const string &key, void *data)> func,
      void *data) {
    m_store->fold_keys(func, data);
  }

  void fold_log(
      std::function<void(const string &key, const string &value, int status)> func) {
    auto ifunc = [&](const string &key, const string &value, int status) {
      string v = m_cipher->decrypt(value);
      func(key, v, status);
    };
    m_store->fold_log(ifunc);
  }

  size_t size() {
    return m_store->size();
  }

  int erase(const string &k) {
    return m_store->erase(k);
  }

  size_t logs() {
    return m_store->logs();
  }

  int truncate_log(size_t x) {
    return m_store->truncate_log(x);
  }

  int merge() {
    return m_store->merge();
  }

  ~crypt_store() {}

  cipher *m_cipher;
  kvstore *m_store;
};

kvstore *new_crypt_store(cipher *c, kvstore *s) {
  return new crypt_store(c, s);
}

}
