/**
 * @file   ffile.hpp
 *
 * @brief full/flush file ...
 *
 *
 */
#ifndef FILE_6ea78d92_04fd_4e99_a3f2_01d70db2cd44_H
#define FILE_6ea78d92_04fd_4e99_a3f2_01d70db2cd44_H 1
#include "ysid.hpp"
namespace ysid {

string ffile_read(const string &file_name);
string ffile_write(const string &file_name, const string &mesg);
bool file_newer_than(const string &afile, const string &bfile);

} // namespace ysid
#endif
