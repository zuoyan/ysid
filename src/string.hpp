#ifndef FILE_3807aa2c_91c6_4757_96da_4800934333cc_H
#define FILE_3807aa2c_91c6_4757_96da_4800934333cc_H

#include <string>
#include <cstring>

namespace ysid {

template <class T>
class zero_allocator : public std::allocator<T> {
 public:
  template<class U> struct rebind { typedef zero_allocator<U> other; };
  zero_allocator() throw() {}
  zero_allocator(const zero_allocator &) throw() {}
  template <class U> zero_allocator(const zero_allocator<U>&) throw() {}

  typedef typename std::allocator<T>::pointer pointer;
  typedef typename std::allocator<T>::size_type size_type;

  void deallocate(pointer p, size_type num) {
    // todo: avoid optimization out
    memset((void*)p, 0, num);
    std::allocator<T>::deallocate(p, num);
  }
};

typedef std::basic_string<char, std::char_traits<char>, zero_allocator<char> > string;

static inline string operator+(string &l, std::string &r) {
  string s = l;
  s.insert(s.end(), r.begin(), r.end());
  return s;
}

static inline string operator+(std::string &l, string &r) {
  string s(l.begin(), l.end());
  s.insert(s.end(), r.begin(), r.end());
  return s;
}

} // namespace ysid

namespace std {

template<>
struct hash<ysid::string> {
  inline size_t operator()(const ysid::string &s) const {
    size_t h = 2166136261U;
    for (size_t i = 0; i < s.size(); ++i) {
      h = h ^ s[i];
      h = h * 16777619;
    }
    return h;
  }
};

} // namespace std

#endif
