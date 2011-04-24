/**
 * @file   pair_store.cpp
 *
 * @brief pari store
 *
 *
 */
#include "ysid_config.h"
#include "ysid.hpp"
#include "kvstore.hpp"
#include <unordered_set>
#include <functional>

namespace ysid {
struct pair_store : kvstore {
  pair_store(
      kvstore *cds, kvstore *cms)
      : m_cds(cds), m_cms(cms) {
    m_cds->fold([&](const string &k, const string &v, void *) {
        m_cms->put(k, v);
      }, 0);
  }

  bool has(const string &k) {
    return m_cms->has(k);
  }

  int erase(const string &k) {
    return m_cms->erase(k) && m_cds->erase(k);
  }

  string get(const string &k) {
    return m_cms->get(k);
  }

  int put(const string &k, const string &v) {
    m_cms->put(k, v);
    int r = m_cds->put(k, v);
    return r;
  }

  int sync() {
    m_cms->sync();
    return m_cds->sync();
  }

  void fold(
      std::function<void (const string &key, const string &value, void *data)> func,
      void *data) {
    m_cms->fold(func, data);
  }

  void fold_keys(
      std::function<void (const string &key, void *data)> func,
      void *data) {
    m_cms->fold_keys(func, data);
  }

  void fold_log(
      std::function<void(const string &key, const string &value, int status)> func) {
    m_cds->fold_log(func);
  }

  size_t size() {
    return m_cms->size();
  }

  size_t logs() {
    return m_cds->logs();
  }

  int truncate_log(size_t x) {
    int r = m_cds->truncate_log(x);
    if (r < 0) return r;
    m_cds->fold([&](const string &k, const string &v, void *) {
        m_cms->put(k, v);
      }, 0);
    if (x != 0) {
      std::unordered_set<string> dels;
      m_cms->fold([&](const string &k, const string &v, void *) {
          if (!m_cds->has(k)) dels.insert(k);
        }, 0);
      for (auto it = dels.begin(); it != dels.end(); ++it) {
        m_cms->erase(*it);
      }
    }
    return r;
  }

  int merge() {
    int r = m_cms->merge();
    if (r >= 0) r = m_cds->merge();
    return r;
  }

  ~pair_store() {}

  kvstore *m_cds, *m_cms;
};

kvstore* new_pair_store(kvstore *d, kvstore *m) {
  return new pair_store(d, m);
}

} // namespace ysid

