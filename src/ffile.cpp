/**
 * @file   ffile.hpp
 *
 * @brief full/flush file ...
 *
 *
 */
#include "ysid_config.h"
#include "ysid.hpp"
#include <fstream>
#include <cstdint>

namespace ysid {

string ffile_read(const string &file_name) {
  string s;
  std::ifstream ifs(file_name, std::ios_base::binary);
  while (ifs.good()) {
    char buf[4096];
    ifs.read(buf, sizeof(buf));
    ssize_t c = ifs.gcount();
    if (c <= 0) break;
    s.insert(s.end(), buf, &buf[c]);
  }
  return s;
}

void ffile_write(const string &file_name, const string &mesg) {
  std::ofstream ofs(file_name, std::ios_base::binary);
  if (ofs.good()) ofs.write(mesg.data(), mesg.size());
}

bool file_newer_than(const string &afile, const string &bfile) {
  return 0;
}

} // namespace ysid
