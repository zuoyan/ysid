/**
 * @file   item.hpp
 *
 * @brief ysid item
 *
 *
 */
#ifndef FILE_88271a93_688a_4bfb_9c27_e2d87ef86116_H
#define FILE_88271a93_688a_4bfb_9c27_e2d87ef86116_H
#include "string.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ysid {

struct item {
  static string new_uuid();

  item();
  item(const string &text);
  string fields_string() const {
    return string();
  }

#ifdef WIN32
  string fields_string(const string &a) const {
    string s;
    auto it = m_fields.find(a);
    if (it != m_fields.end()) {
      s += a + ":" + it->second;
      s += "\n";
    }
    return s;
  }

  string fields_string(const string &a, const string &b) const {
    return fields_string(a) + fields_string(b);
  }

  string fields_string(const string &a, const string &b, const string &c) const {
    return fields_string(a) + fields_string(b, c);
  }

  string fields_string(const string &a, const string &b, const string &c, const string &d) const {
    return fields_string(a) + fields_string(b, c, d);
  }

  string fields_string(const string &a, const string &b, const string &c, const string &d, const string &e) const {
    return fields_string(a) + fields_string(b, c, d, e);
  }

  string fields_string(const string &a, const string &b,
                       const string &c, const string &d,
                       const string &e, const string &f) const {
    return fields_string(a) + fields_string(b, c, d, e, f);
  }
#else
  template <class ...T>
  string fields_string(const string &n, const T&...a) const {
    string s;
    auto it = m_fields.find(n);
    if (it != m_fields.end()) {
      s += n + ":" + it->second;
      s += "\n";
    }
    return s + fields_string(a...);
  }
#endif

  string to_string() const;
  string field(const string &n) const;
  void set_field(const string &n, const string &v);

  string uuid() const;
  string username() const;
  string password() const;
  string url() const;
  string tags() const;
  string notes() const;

  void uuid(const string &s);
  void username(const string &s);
  void password(const string &s);
  void url(const string &s);
  void tags(const string &s);
  void notes(const string &s);

  void update(const item &r);
  void update_to(item &r) const;

  void generate_password(const string &cfg);

  std::unordered_set<string> split_tags() const {
    auto v = tags();
    size_t p = 0;
    std::unordered_set<string> r;
    while (p <  v.size()) {
      size_t n = v.find(',', p);
      if (n == string::npos) n = v.size();
      r.insert(v.substr(p, n - p));
      p = n + 1;
    }
    return r;
  }

  std::unordered_map<string, string> m_fields;
};

struct item_filter {
  explicit item_filter(const string &s) {
    string f, v = s;
    auto d = s.find(':');
    auto one_of = [](const string &p, std::initializer_list<const char*> l) -> bool {
      for (auto it = l.begin(); it != l.end(); ++it) {
        if (p == *it) return true;
      }
      return false;
    };
    m_exact = false;
    m_prefix = false;
    m_reverse = false;
    if (d != string::npos && d > 0) {
      f = s.substr(0, d);
      v = s.substr(d + 1);
      if (f[0] == '-') {
        m_reverse = true;
        f = f.substr(1);
      }
      if (one_of(f, {"u", "user", "username"})) f = "username";
      else if (one_of(f, {"l", "url"})) f = "url";
      else if (one_of(f, {"t", "tag", "tags"})) f = "tags";
      else if (one_of(f, {"n", "note", "notes"})) f = "notes";
      else if (one_of(f, {"U", "USER", "USERNAME"})) {
        f = "username";
        m_exact = true;
      } else if (one_of(f, {"User", "Username"})) {
        f = "username";
        m_prefix = true;
      } else {
        v = s;
        f = "";
      }
    }
    m_field = f;
    m_value = v;
  }

  bool match(const item &n) {
    bool r = do_match(n);
    if (m_reverse) r = !r;
    return r;
  }

 private:

  bool do_match(const item &n) {
    if (m_field.size()) {
      string fv;
      if (m_field == "tags") {
        item t;
        t.tags(m_value);
        auto tt = t.split_tags();
        auto tags = n.split_tags();
        if (m_exact) return tt == tags;
        for (auto it = tt.begin(); it != tt.end(); ++it) {
          if (!tags.count(*it)) return false;
        }
        return true;
      }
      if (m_field == "username") fv = n.username();
      if (m_field == "url") fv = n.url();
      if (m_field == "notes") fv = n.notes();
      if (m_exact) return fv == m_value;
      if (m_prefix) return fv.size() >= m_value.size() && fv.substr(0, m_value.size()) == m_value;
      return fv.find(m_value) != string::npos;
    }
    return n.username().find(m_value) != string::npos
        || n.url().find(m_value) != string::npos
        || n.tags().find(m_value) != string::npos
        || n.notes().find(m_value) != string::npos;
  }

  string m_field, m_value;
  bool m_exact, m_prefix;
  bool m_reverse;
};

} // namespace ysid
#endif
