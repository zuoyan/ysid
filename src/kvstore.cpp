/**
 * @file   kvstore.cpp
 *
 * @brief kvstore, key-value store, in file, or in memory
 *
 *
 */
#include "ysid.hpp"
#include "kvstore.hpp"
#include "ffile.hpp"

#include <unordered_map>
#include <memory>
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>

namespace ysid {

struct io_char {
  static inline void read(std::istream &is, char &n) {
    is.read(&n, 1);
  }

  static inline char read(std::istream &is) {
    char t;
    read(is, t);
    return t;
  }

  static inline void write(std::ostream &os, char n) {
    os.write(&n, 1);
  }

  static size_t binary_size() {
    return 1;
  }

  static size_t binary_size(const char &n) {
    return 1;
  }
};

struct io_size {
  static inline void read(std::istream &is, size_t &n) {
    n = 0;
    uint8_t l;
    int b = 0;
    do {
      l = is.get();
      if (is.fail()) break;
      n += (((size_t)(l & 0x7f)) << b);
      b += 7;
    } while (l & 0x80);
  }

  static inline size_t read(std::istream &is) {
    size_t t;
    read(is, t);
    return t;
  }

  static inline void write(std::ostream &os, size_t n) {
    uint8_t l;
    do {
      l = n & 0x7f;
      n >>= 7;
      if (n) l |= 0x80;
      os.put(l);
    } while (n);
  }

  static inline size_t binary_size(size_t n) {
    size_t c = 0;
    do {
      n >>= 7;
      c += 1;
    } while (n);
    return c;
  }
};

struct io_string {
  static void read(std::istream &is, string &v) {
    size_t sz;
    io_size::read(is, sz);
    v.resize(sz);
    is.read(&v[0], sz);
  }

  static void skip(std::istream &is) {
    size_t sz;
    io_size::read(is, sz);
    is.seekg(sz, std::ios_base::cur);
  }

  static void write(std::ostream &os, const string &v) {
    io_size::write(os, v.size());
    os.write(&v[0], v.size());
  }

  static inline size_t binary_size(const string &s) {
    return io_size::binary_size(s.size()) + s.size();
  }
};

kvstore::~kvstore() {}

struct mem_kvstore : public kvstore {
  virtual void fold(
      std::function<void(const string &key, const string &value, void *data)> func,
      void *data) {
    for (auto it = m_map.begin(); it != m_map.end(); ++it) {
      const string &key = it->first;
      const string &value = it->second;
      func(key, value, data);
    }
  }

  virtual size_t logs() {
    return 0;
  }

  virtual int truncate_log(size_t) {
    return -1;
  }

  virtual void fold_log(
      std::function<void(const string &key, const string &value, int status)> func) {
    auto ifunc = [&](const string &key, const string &value, void *data){
      func(key, value, 2);
    };
    fold(ifunc, 0);
  }

  virtual void fold_keys(
      std::function<void(const string &key, void *data)> func,
      void *data) {
    for (auto it = m_map.begin(); it != m_map.end(); ++it) {
      const string &key = it->first;
      func(key, data);
    }
  }

  int sync() {
    return 1;
  }

  int put(const string &key, const string &value) {
    m_map[key] = value;
    return 1;
  }

  string get(const string &key) {
    return m_map[key];
  }

  int erase(const string &key) {
    return m_map.erase(key);
  }

  bool has(const string &key) {
    return m_map.count(key);
  }

  size_t size() {
    return m_map.size();
  }

  int merge() {
    return 1;
  }

 private:
  std::unordered_map<string, string> m_map;
};

struct file_kvstore : public kvstore {
  struct hint_type {
    size_t pos;
  };

  file_kvstore() {}

  file_kvstore(const std::string &path) {
    std::string hint_fn = path + ".hint";
    size_t c = 0;
    if (file_newer_than(hint_fn.c_str(), path.c_str())) {
      c = read_hint(hint_fn);
    }
    if (!c) c = read_data(path, true);
    if (!m_ios.get()) ios(path);
    m_path = path;
  }

  file_kvstore(const std::shared_ptr<std::iostream> &d)
      : m_ios(d) {
    read_data(ios());
  }

  file_kvstore(const std::shared_ptr<std::iostream> &d, std::istream &hint)
      : m_ios(d) {
    read_hint(hint);
  }

  std::iostream& ios() {
    return *m_ios;
  }

  std::iostream& ios(std::shared_ptr<std::iostream> s) {
    m_ios = s;
    return ios();
  }

  std::iostream& ios(const std::string &path) {
    m_ios = std::shared_ptr<std::iostream>(
        new std::fstream(
            path,
            std::ios_base::binary | std::ios_base::app | std::ios_base::in | std::ios_base::out));
    return ios();
  }

  std::ostream &os() {
    return ios();
  }

  std::istream &is() {
    return ios();
  }

  string get(const string &key) {
    string value;
    if (!m_ios.get()) return value;
    bool found;
    hint_type h; h.pos = 0;
    {
      auto it = m_key_hint.find(key);
      found = (it != m_key_hint.end());
      if (found) h.pos = it->second.pos;
    }
    if (found) {
      is().seekg(
          h.pos + 1 + io_string::binary_size(key),
          std::ios_base::beg);
      io_string::read(is(), value);
    }
    return value;
  }

  int put(const string &key, const string &value) {
    if (!m_ios.get()) return -1;
    hint_type h;
    write_log(h, key, value);
    m_key_hint[key] = h;
    return 1;
  }

  bool has(const string &key) {
    return m_key_hint.count(key);
  }

  int erase(const string &key) {
    if (!m_ios.get()) return -1;
    hint_type h;
    {
      auto it = m_key_hint.find(key);
      if (it == m_key_hint.end()) return 0;
      h = it->second;
    }
    write_log(h, key);
    m_key_hint.erase(key);
    return 1;
  }

  size_t size() {
    return m_key_hint.size();
  }

  int merge() {
    return merge(m_path);
  }

  int merge(const std::string &path) {
    if (!m_ios.get()) return -1;
    std::string tmp = path + ".tmp." + to_string(time(0));
    merge_write(tmp);
    m_ios.reset();
    unlink(path.c_str());
    rename(tmp.c_str(), path.c_str());
    ios(path);
    m_path = path;
    return 1;
  }

  virtual size_t logs() {
    auto &is = ios();
    is.seekg(0, std::ios_base::beg);
    size_t c = 0;
    while (is.good()) {
      auto pos = is.tellg();
      if (pos < 0) break;
      char crc = io_char::read(is);
      string key, value;
      if (is.fail()) break;
      io_string::skip(is);
      if (is.fail()) break;
      c += 1;
      if (crc == 1) continue;
      io_string::skip(is);
      if (is.fail()) break;
    }
    return c;
  }

  virtual int truncate_log(size_t x) {
    m_key_hint.clear();
    auto &is = ios();
    is.seekg(0, std::ios_base::beg);
    read_data(is, x);
    return 1;
  }

  virtual void fold_keys(
      std::function<void(const string &key, void *data)> func,
      void *data) {
    for (auto it = m_key_hint.begin(); it != m_key_hint.end(); ++it) {
      func(it->first, data);
    }
  }

  virtual void fold(
      std::function<void(const string &key, const string &value, void *data)> func,
      void *data) {
    auto &is = ios();
    is.seekg(0, std::ios_base::beg);
    while (is.good()) {
      auto pos = is.tellg();
      if (pos < 0) break;
      char crc = io_char::read(is);
      if (is.fail()) break;
      if (crc == 1) {
        io_string::skip(is);
        continue;
      }
      string key;
      io_string::read(is, key);
      if (is.fail()) break;
      auto it = m_key_hint.find(key);
      if (it != m_key_hint.end()
          && it->second.pos == (size_t)pos) {
        string v;
        io_string::read(is, v);
        if (is.fail()) break;
        func(key, v, data);
      } else {
        io_string::skip(is);
      }
    }
    is.clear();
  }

  // status: -2: deleted -1: deleted-old 1:exist-old 2: exist-current
  virtual void fold_log(
      std::function<void(const string &key, const string &value, int status)> func) {
    auto &is = ios();
    is.seekg(0, std::ios_base::beg);
    while (is.good()) {
      auto pos = is.tellg();
      if (pos < 0) break;
      char crc = io_char::read(is);
      string key, value;
      if (is.fail()) break;
      io_string::read(is, key);
      if (is.fail()) break;
      if (crc == 1) {
        func(key, value, -2);
        continue;
      }
      io_string::read(is, value);
      if (is.fail()) break;
      auto it = m_key_hint.find(key);
      if (it == m_key_hint.end()) {
        func(key, value, -1);
      } else {
        if (it->second.pos == (size_t)pos) {
          func(key, value, 2);
        } else {
          func(key, value, 1);
        }
      }
    }
    is.clear();
  }

  int merge_write(const std::string &path) {
    std::ofstream ofs(path, std::ios_base::trunc | std::ios_base::binary);
    return merge_write(ofs);
  }

  int merge_write(std::ostream &os) {
    auto &kh = m_key_hint;
    auto write_func = [&](const string &k, const string &v, void*) {
      auto &h = kh[k];
      this->write_log(os, h, k, v);
    };
    is().seekg(0, std::ios_base::beg);
    fold(write_func, 0);
    return 1;
  }

  int sync() {
    if (!m_ios.get()) return -1;
    os().flush();
    return 1;
  }

  int close() {
    if (!m_ios.get()) return -1;
    m_ios.reset();
    m_key_hint.clear();
    return 1;
  }

  inline void write_log(
      std::ostream &fs,
      hint_type &h,
      const string &key, const string &value) {
    h.pos = fs.tellp();
    io_char::write(fs, 0);
    io_string::write(fs, key);
    io_string::write(fs, value);
  }

  inline void write_log(
      hint_type &h,
      const string &key, const string &value) {
    write_log(os(), h, key, value);
  }

  void write_log(std::ostream &fs, hint_type &h, const string &key) { // erase
    io_char::write(fs, 1);
    io_string::write(fs, key);
  }

  void write_log(hint_type &h, const string &key) { // erase
    write_log(os(), h, key);
  }

  inline void write_hint(std::ostream &os) {
    for (auto it = m_key_hint.begin(); it != m_key_hint.end(); ++it) {
      auto &h = it->second;
      io_size::write(os, h.pos);
      io_string::write(os, it->first);
    }
  }

  inline void write_hint(const std::string &fn) {
    std::ofstream fs(fn, std::ios_base::trunc | std::ios_base::binary);
    write_hint(fs);
  }

  inline size_t read_hint(const std::string &fn) {
    std::ifstream fs(fn, std::ios_base::binary);
    return read_hint(fs);
  }

  inline size_t read_hint(std::istream &is) {
    hint_type h;
    size_t c = 0;
    if (!is.good()) return c;
    while (is.good()) {
      if (!is.good()) break;
      io_size::read(is, h.pos);
      string key;
      io_string::read(is, key);
      if (!is.good()) break;
      m_key_hint[key] = h;
      ++c;
    }
    is.clear();
    return c;
  }

  inline size_t read_data(
      const std::string &fn,
      bool dest=false) {
    std::shared_ptr<std::iostream >
        pfs(new std::fstream(
                fn,
                std::ios_base::binary | std::ios_base::app | std::ios_base::in | std::ios_base::out));
    size_t c = read_data(*pfs);
    if (dest) ios(pfs);
    return c;
  }

  inline size_t read_data(std::istream &is, size_t log_limit=0) {
    //crc key value
    hint_type h;
    size_t c = 0;
    while (is.good()) {
      h.pos = is.tellg();
      char crc = io_char::read(is);
      if (is.fail()) break;
      string key;
      io_string::read(is, key);
      if (is.fail()) break;
      if (crc == 1) {
        m_key_hint.erase(key);
      } else {
        io_string::skip(is);
        if (is.fail()) break;
        m_key_hint[key] = h;
      }
      ++c;
      if (log_limit && c >= log_limit) break;
    }
    is.clear();
    return c;
  }

 protected:
  std::unordered_map<string, hint_type> m_key_hint;
  std::shared_ptr<std::iostream> m_ios;
  string m_path;
};


kvstore* new_file_kvstore(const std::string &path) {
  return new file_kvstore(path);
}

kvstore* new_mem_kvstore() {
  return new mem_kvstore();
}

} // namespace ysid
