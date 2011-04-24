/**
 * @file   ysid_db.cpp
 *
 * @brief ysid db
 *
 *
 */
#include "ysid_config.h"
#include "crypt_store.hpp"
#include "pair_store.hpp"
#include "ysid.hpp"
#include "ysid_db.hpp"
#include "crypt.hpp"
#include "kvstore.hpp"

#include <stdexcept>
#include <memory>

#ifdef WIN32

typedef HANDLE pthread_mutex_t;

static int pthread_mutex_init(pthread_mutex_t *mutex, void *unused) {
  unused = NULL;
  *mutex = CreateMutex(NULL, FALSE, NULL);
  return *mutex == NULL ? -1 : 0;
}

static int pthread_mutex_destroy(pthread_mutex_t *mutex) {
  return CloseHandle(*mutex) == 0 ? -1 : 0;
}

static int pthread_mutex_lock(pthread_mutex_t *mutex) {
  return WaitForSingleObject(*mutex, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
}

static int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  return ReleaseMutex(*mutex) == 0 ? -1 : 0;
}

namespace std {

struct mutex {
  mutex() {
    pthread_mutex_init(&m_mutex, 0);
  }

  ~mutex(){
    pthread_mutex_destroy(&m_mutex);
  }

  int lock() {
    return pthread_mutex_lock(&m_mutex);
  }

  int unlock() {
    return pthread_mutex_unlock(&m_mutex);
  }
 private:
  mutex(const mutex &r);
  pthread_mutex_t m_mutex;
};

template <class M>
struct lock_guard {
  lock_guard(M &m)
      : m_mutex(m) {
    m_mutex.lock();
  }

  ~lock_guard() {
    m_mutex.unlock();
  }
 private:
  lock_guard(const lock_guard &r);
  M &m_mutex;
};

}

#else
#include <mutex>
#endif

namespace ysid {

struct ysid_db_impl {
  ysid_db_impl() {
    m_scipher.reset(new_rsa_cipher());
    m_dcipher.reset(new_rsa_cipher());
    m_truncate_log = 0;
  }

  void load_session_private() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!spri_key.size())
      throw std::runtime_error("no session private key given");
    m_scipher->load_private(spri_key, [](int) { return string(); });
  }

  void clear_session_private() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_scipher->clear_private();
  }

  void set_truncate_log(ssize_t x) {
    m_truncate_log = x;
  }

  kvstore* db() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_db.get()) {
      m_db.reset(new_pair_store(cds(), cms()));
      if (m_truncate_log) {
        ssize_t x = m_truncate_log;
        if (x < 0) m_truncate_log = m_db.get()->logs() - x;
        m_db.get()->truncate_log(x);
      }
      m_dcipher->clear_private();
    }
    return m_db.get();
  }

  void set_dpri_key(const string &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dpri_key = key;
    if (password.size()) {
      m_dcipher->load_private(dpri_key, [&](int) { return password; });
      password = "";
    }
  }

  void set_dpub_key(const string &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dpub_key = key;
    m_dcipher->load_public(dpub_key);
  }

  void set_password(const string &p) {
    std::lock_guard<std::mutex> lock(m_mutex);
    password = p;
    if (dpri_key.size()) {
      m_dcipher->load_private(dpri_key, [&](int) { return password; });
      password = "";
    }
  }

  void set_spri_key(const string &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    spri_key = key;
    m_scipher->load_private(spri_key, [&](int) { return string(); });
  }

  void set_spub_key(const string &key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    spub_key = key;
    m_scipher->load_public(spub_key);
  }

  void set_path(const string &p) {
    std::lock_guard<std::mutex> lock(m_mutex);
    path = p;
  }

  void clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ds.reset();
    m_ms.reset();
    m_cds.reset();
    m_cms.reset();
    m_db.reset();
    m_scipher.get()->clear_private();
    m_scipher.get()->clear_public();
    m_dcipher.get()->clear_private();
    m_dcipher.get()->clear_public();
    path = "";
    password = "";
  }

 private:
  kvstore* cds() {
    if (!m_cds.get()) {
      if (!m_ds.get()) {
        if (!path.size()) throw std::runtime_error("no path given");
        m_ds.reset(new_file_kvstore(path));
      }
      // check private and public key is loaded
      try {
        if (m_dcipher->decrypt(m_dcipher->encrypt("test")) != "test")
          throw std::runtime_error("cds decrypt/encrypt(test) != test");
      } catch (std::runtime_error &e) {
        throw std::runtime_error(string("cds(): got exception ") + e.what());
      }
      if (m_ds.get()) m_cds.reset(new_crypt_store(m_dcipher.get(), m_ds.get()));
    }
    if (m_cds.get()) return m_cds.get();
    throw std::runtime_error("can't fetch disk file");
  }

  kvstore* cms() {
    if (!m_cms.get()) {
      if (!m_ms.get()) m_ms.reset(new_mem_kvstore());
      m_cms.reset(new_crypt_store(m_scipher.get(), m_ms.get()));
    }
    return m_cms.get();
  }

  string path;
  string password;
  string dpri_key;
  string dpub_key;
  string spri_key;
  string spub_key;

  std::unique_ptr<kvstore> m_db;
  std::unique_ptr<kvstore> m_cds, m_cms;
  std::unique_ptr<kvstore> m_ds, m_ms;
  std::unique_ptr<cipher> m_scipher;
  std::unique_ptr<cipher> m_dcipher;
  std::mutex m_mutex;
  ssize_t m_truncate_log;
};

ysid_db::ysid_db() {
  pimpl = new ysid_db_impl();
}

ysid_db::~ysid_db() {
  if (pimpl) delete pimpl;
  pimpl = 0;
}

void ysid_db::load_session_private() {
  pimpl->load_session_private();
}

void ysid_db::clear_session_private() {
  pimpl->clear_session_private();
}

kvstore* ysid_db::db() {
  return pimpl->db();
}

void ysid_db::set_dpri_key(const string &key) {
  pimpl->set_dpri_key(key);
}

void ysid_db::set_dpub_key(const string &key) {
  pimpl->set_dpub_key(key);
}

void ysid_db::set_password(const string &p) {
  pimpl->set_password(p);
}

void ysid_db::set_spri_key(const string &key) {
  pimpl->set_spri_key(key);
}

void ysid_db::set_spub_key(const string &key) {
  pimpl->set_spub_key(key);
}

void ysid_db::set_path(const string &p) {
  pimpl->set_path(p);
}

void ysid_db::clear()  {
  pimpl->clear();
}

void ysid_db::set_truncate_log(ssize_t x) {
  pimpl->set_truncate_log(x);
}

} // namespace ysid
