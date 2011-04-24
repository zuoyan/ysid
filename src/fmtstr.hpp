/**
 * @file   fmtstr.hpp
 *
 * @brief format variadic args to string, like sprintf in C
 *
 * only %s is supported now
 *
 */
#ifndef FILE_555b65b3_4a15_4168_9714_0ca3f9062ac5_H
#define FILE_555b65b3_4a15_4168_9714_0ca3f9062ac5_H 1
#include <iosfwd>
#include <string>
#include <iostream>
#include <stdexcept>

namespace ysid {

namespace {

#ifdef WIN32
std::string fmtstr(const std::string &fmt) {
  return fmt;
}

template <class T0>
std::string fmtstr_one(std::string &fmt, const T0 &t0) {
  std::string ret;
  size_t p = fmt.find('%');
  if (p == std::string::npos || p + 1 >= fmt.size()) {
    std::cerr << "fmtstr: '" << fmt << "', epxect '%'" << std::endl;
    abort();
  }
  if (fmt[p + 1] == '%') {
    ret = fmt.substr(0, p + 1);
    fmt = fmt.substr(p + 2);
    return ret + fmtstr_one(fmt, t0);
  }
  ret = fmt.substr(0, p);
  if (fmt[p + 1] == 's') {
    fmt = fmt.substr(p + 2);
    return ret + to_string(t0);
  }
  if (fmt[p + 1] == 'd') {
    fmt = fmt.substr(p + 2);
    return ret + to_string(t0);
  }
  if (fmt[p + 1] == 'f') {
    fmt = fmt.substr(p + 2);
    return ret + to_string(t0);
  }
  if ((fmt[p + 1] == 'z' || fmt[p + 1] == 'l')
      && p + 2 < fmt.size() && fmt[p + 2] == 'd') {
    fmt = fmt.substr(p + 3);
    return ret + to_string(t0);
  }
  if (fmt[p + 1] == 'l'
      && p + 2 < fmt.size() && fmt[p + 2] == 'f') {
    fmt = fmt.substr(p + 3);
    return ret + to_string(t0);
  }
  throw std::runtime_error(std::string("fmtstr: '" + fmt + "', epxect 's'"));
}

template<class T0>
std::string fmtstr(const std::string &fmt, const T0 &t0) {
  std::string ret, ff = fmt;
  ret = fmtstr_one(ff, t0);
  return ret + ff;
}

template<class T0, class T1>
std::string fmtstr(const std::string &fmt, const T0 &t0, const T1 &t1) {
  std::string ret, ff = fmt;
  ret = fmtstr_one(ff, t0);
  ret += fmtstr_one(ff, t1);
  return ret + ff;
}

template<class T0, class T1, class T2>
std::string fmtstr(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2) {
  std::string ret, ff = fmt;
  ret = fmtstr_one(ff, t0);
  ret += fmtstr_one(ff, t1);
  ret += fmtstr_one(ff, t2);
  return ret + ff;
}

template<class T0, class T1, class T2, class T3>
std::string fmtstr(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3) {
  std::string ret, ff = fmt;
  ret = fmtstr_one(ff, t0);
  ret += fmtstr_one(ff, t1);
  ret += fmtstr_one(ff, t2);
  ret += fmtstr_one(ff, t3);
  return ret + ff;
}

template<class T0, class T1, class T2, class T3, class T4>
std::string fmtstr(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3,
                   const T4 &t4) {
  std::string ret, ff = fmt;
  ret = fmtstr_one(ff, t0);
  ret += fmtstr_one(ff, t1);
  ret += fmtstr_one(ff, t2);
  ret += fmtstr_one(ff, t3);
  ret += fmtstr_one(ff, t4);
  return ret + ff;
}


template<class T0, class T1, class T2, class T3, class T4, class T5>
std::string fmtstr(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3,
                   const T4 &t4, const T5 &t5) {
  std::string ret, ff = fmt;
  ret = fmtstr_one(ff, t0);
  ret += fmtstr_one(ff, t1);
  ret += fmtstr_one(ff, t2);
  ret += fmtstr_one(ff, t3);
  ret += fmtstr_one(ff, t4);
  ret += fmtstr_one(ff, t5);
  return ret + ff;
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6>
std::string fmtstr(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3,
                   const T4 &t4, const T5 &t5, const T6 &t6) {
  std::string ret, ff = fmt;
  ret = fmtstr_one(ff, t0);
  ret += fmtstr_one(ff, t1);
  ret += fmtstr_one(ff, t2);
  ret += fmtstr_one(ff, t3);
  ret += fmtstr_one(ff, t4);
  ret += fmtstr_one(ff, t5);
  ret += fmtstr_one(ff, 6);
  return ret + ff;
}

decltype(std::cerr)& fmterrlnt(const std::string &fmt) {
  return std::cerr << "[" << get_drtime() << "]" << fmt << std::endl;
}

template <class T0>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, const T0 &t0) {
  return std::cerr << "[" << get_drtime() << "]" << fmtstr(fmt, t0) << std::endl;
}

template <class T0, class T1>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, const T0 &t0, const T1 &t1) {
  return std::cerr << "[" << get_drtime() << "]" << fmtstr(fmt, t0, t1) << std::endl;
}

template <class T0, class T1, class T2>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2) {
  return std::cerr << "[" << get_drtime() << "]" << fmtstr(fmt, t0, t1, t2) << std::endl;
}

template <class T0, class T1, class T2, class T3>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3) {
  return std::cerr << "[" << get_drtime() << "]" << fmtstr(fmt, t0, t1, t2, t3) << std::endl;
}

template <class T0, class T1, class T2, class T3, class T4>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
  return std::cerr << "[" << get_drtime() << "]" << fmtstr(fmt, t0, t1, t2, t3, t4) << std::endl;
}

template <class T0, class T1, class T2, class T3, class T4, class T5>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5) {
  return std::cerr << "[" << get_drtime() << "]" << fmtstr(fmt, t0, t1, t2, t3, t4, t5) << std::endl;
}

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3,
                               const T4 &t4, const T5 &t5, const T6 &t6) {
  return std::cerr << "[" << get_drtime() << "]" << fmtstr(fmt, t0, t1, t2, t3, t4, t5, t6) << std::endl;
}

#else

static inline std::string fmtstr(const std::string &fmt) {
  return fmt;
}

template <class ...A>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, A...a);

template<class H, class ...A>
std::string fmtstr(const std::string &fmt, const H &h, const A&...a) {
  size_t p = fmt.find('%');
  if (p == std::string::npos || p + 1 >= fmt.size()) {
    std::cerr << "fmtstr: '" << fmt << "', epxect '%'" << std::endl;
    abort();
  }
  if (fmt[p + 1] == '%') {
    return fmt.substr(0, p + 1) + fmtstr(fmt.substr(p + 2), a...);
  }
  if (fmt[p + 1] == 's') {
    return fmt.substr(0, p) + to_string(h)
      + fmtstr(fmt.substr(p + 2), a...);
  }
  if (fmt[p + 1] == 'd') {
    return fmt.substr(0, p) + to_string(h)
        + fmtstr(fmt.substr(p + 2), a...);
  }
  if (fmt[p + 1] == 'f') {
    return fmt.substr(0, p) + to_string(h)
        + fmtstr(fmt.substr(p + 2), a...);
  }
  if ((fmt[p + 1] == 'z' || fmt[p + 1] == 'l')
      && p + 2 < fmt.size() && fmt[p + 2] == 'd') {
    return fmt.substr(0, p) + to_string(h)
        + fmtstr(fmt.substr(p + 3), a...);
  }
  if (fmt[p + 1] == 'l'
      && p + 2 < fmt.size() && fmt[p + 2] == 'f') {
    return fmt.substr(0, p) + to_string(h)
        + fmtstr(fmt.substr(p + 3), a...);
  }
  throw std::runtime_error(std::string("fmtstr: '" + fmt + "', epxect 's'"));
}

template <class ...A>
decltype(std::cout)& fmtout(const std::string &fmt, A...a) {
  return std::cout << fmtstr(fmt, a...);
}

template <class ...A>
decltype(std::cerr)& fmterr(const std::string &fmt, A...a) {
  return std::cerr << fmtstr(fmt, a...);
}

template <class ...A>
decltype(std::cout)& fmtoutln(const std::string &fmt, A...a) {
  return fmtout(fmt, a...) << std::endl;
}

template <class ...A>
decltype(std::cerr)& fmterrln(const std::string &fmt, A...a) {
  return fmterr(fmt, a...) << std::endl;
}

template <class ...A>
decltype(std::cerr)& fmterrlnt(const std::string &fmt, A...a) {
  return fmterrln("[time %s]" + fmt, get_drtime(), a...);
}

#endif

}
} // namespace ysid
#endif
