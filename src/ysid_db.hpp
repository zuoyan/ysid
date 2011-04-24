/**
 * @file   ysid_db.hpp
 *
 * @brief ysid db
 *
 *
 */
#ifndef FILE_8f445760_656d_4f5c_8a70_3af8242425f4_H
#define FILE_8f445760_656d_4f5c_8a70_3af8242425f4_H 1
#include <string>

namespace ysid {

using std::string;

struct kvstore;
struct ysid_db_impl;

struct ysid_db {
  ysid_db();
  ~ysid_db();

  void load_session_private();
  void clear_session_private();
  kvstore* db();
  void set_dpri_key(const string &key);
  void set_dpub_key(const string &key);
  void set_password(const string &p);
  void set_spri_key(const string &key);
  void set_spub_key(const string &key);
  void set_path(const string &p);
  void set_truncate_log(ssize_t x);
  void clear();

 private:
  ysid_db_impl *pimpl;
};

} // namespace ysid
#endif
