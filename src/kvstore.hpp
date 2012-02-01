/**
 * @file   kvstore.hpp
 *
 * @brief  kvstore interface
 *
 *
 */
#ifndef FILE_58ffcf4e_36be_432c_af01_a5ccaf040e0a_H
#define FILE_58ffcf4e_36be_432c_af01_a5ccaf040e0a_H 1
#include "ysid.hpp"
#include <functional>
#include "string.hpp"

namespace ysid {

struct kvstore {
  virtual void fold(
      std::function<void (const string &key, const string &value, void *data)> func,
      void *data) = 0;
  virtual void fold_keys(
      std::function<void (const string &key, void *data)> func,
      void *data) = 0;
  // status: -2: deleted -1: deleted-old 1:exist-old 2: exist-current
  virtual void fold_log(
      std::function<void(const string &key, const string &value, int status)> func) = 0;
  virtual size_t logs() = 0; // return the number of logs
  virtual int truncate_log(size_t x) = 0; // perform assume only log[0, x) exist
  virtual int put(const string &key, const string &value) = 0;
  virtual int erase(const string &key) = 0;
  virtual bool has(const string &key) = 0;
  virtual string get(const string &key) = 0;
  virtual size_t size() = 0;
  virtual int sync() = 0;
  virtual int merge() = 0;
  virtual ~kvstore();
};

kvstore* new_file_kvstore(const string &path);
kvstore* new_mem_kvstore();
} // namespace ysid
#endif
