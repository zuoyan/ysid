/**
 * @file   mt_random.hpp
 *
 * @brief mersenne twist random
 *
 *
 */
#ifndef FILE_61864ff2_0799_4a1b_a466_57e48897e769_H
#define FILE_61864ff2_0799_4a1b_a466_57e48897e769_H 1

namespace ysid {

class mt_random {
 public:
  mt_random() {
    seed(19850214);
  }

  mt_random(int s)  {
    seed(s);
  }

  void seed(int seed) {
    m_mt[0] = seed;
    for (int i = 1; i < mt_size(); ++i) {
      m_mt[i] = 0x6c078965UL * (m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) + i;
    }
    m_index = 0;
  }

  void seed(int *s, size_t len) {
    if (len <= 0) return;
    seed(*s);
    for (size_t i = 1; i < len; ++i) {
      m_mt[i % mt_size()] ^= s[i];
    }
  }

  int operator()() {
    if (m_index == 0) {
      refresh();
    }
    int y = m_mt[m_index];
    y ^= y >> 11;
    y ^= (y << 7) & 0x9d2c5680;
    y ^= (y << 15) & 0xefc60000;
    y ^= y >> 18;
    m_index += 1;
    if (m_index >= mt_size()) m_index -= mt_size();
    return y;
  }

  void refresh() {
    for (int i = 0; i < mt_size(); ++i) {
      int y = (m_mt[i] >> 31) + (m_mt[(i + 1) % mt_size()] & 0x7fffffff);
      m_mt[i] = m_mt[(i + 397) % mt_size()] ^ (y >> 1);
      if (y & 1) {
        m_mt[i] = m_mt[i] ^ 0x9908b0df;
      }
    }
  }

 private:
  int m_mt[624];
  int m_index;

  int mt_size() {
    return sizeof(m_mt) / sizeof(m_mt[0]);
  }
};

} // namespace ysid
#endif
