/**
 * @file   ysid.hpp
 *
 * @brief interface of ysid
 *
 *
 */
#ifndef FILE_267e7a9d_ccc9_4c79_b957_a6fe66919c08_H
#define FILE_267e7a9d_ccc9_4c79_b957_a6fe66919c08_H 1
#include <string>
#include <sstream>
#include <ctime>
#include <cstdint>
#ifdef WIN32
typedef std::streamsize ssize_t;
#else
#include <sys/time.h>
#endif
#include <gettimeofday.h>
#include "string.hpp"
#include <algorithm>

namespace ysid {

static inline std::string to_std_string(const string &s) {
  std::string ss;
  ss.insert(ss.begin(), s.begin(), s.end());
  return ss;
}

static inline string to_string(const string &s) {
  return s;
}

template <class T>
inline string to_string(const T &v) {
  std::ostringstream oss;
  oss << v;
  string s;
  std::string ss = oss.str();
  s.insert(s.begin(), ss.begin(), ss.end());
  std::fill(ss.begin(), ss.end(), 0);
  return s;
}

static inline string to_string(const std::string &ss) {
  string s;
  s.insert(s.begin(), ss.begin(), ss.end());
  return s;
}

static inline bool to_bool(const string &s, bool dft=false) {
  if (!s.size()) return dft;
  if (s[0] == 'f' || s[0] == 'F' || s[0] == '0') return false;
  if (s[0] == 't' || s[0] == 'T' || s[0] == '1') return true;
  return dft;
}

static inline long long to_int(const string &s) {
  if (s.size() == 0) return 0;
  return atoll(s.c_str());
}

static inline long long to_int(const char *c) {
  if (!c) return 0;
  return atoll(c);
}

#ifdef HAVE_CLOCK_GETTIME
static inline double get_drtime() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec + ts.tv_nsec / 1.e9;
}
#else
static inline double get_drtime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1.e6;
}
#endif

} // namespace ysid
#endif
