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

namespace ysid {

typedef std::string string;

template <class F>
static inline string to_string(const F &f) {
  std::ostringstream oss;
  oss << f;
  return oss.str();
}

static inline bool to_bool(const string &s, bool dft=false) {
  if (!s.size()) return dft;
  if (s[0] == 'f' || s[0] == 'F' || s[0] == '0') return false;
  if (s[0] == 't' || s[0] == 'T' || s[0] == '1') return true;
  return dft;
}

static inline long long to_int(const string &s) {
  if (s.size() == 0) return 0;
  long long l;
  std::istringstream iss(s);
  iss >> l;
  return iss.fail() ? 0 : l;
}

static inline long long to_int(const char *c) {
  if (!c) return 0;
  return to_int(string(c));
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
