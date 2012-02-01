/**
 * @file   item.cpp
 *
 * @brief item
 *
 *
 */
#include "ysid_config.h"
#include "item.hpp"
#include "mt_random.hpp"
#include <ctime>
#include <sstream>
#include "crypt.hpp"
#include "ysid.hpp"

namespace ysid {

static mt_random random;
static int mt_seeded;

static unsigned merge_random() {
  if (!mt_seeded) {
    auto t = time(NULL);
    unsigned x;
    rand_bytes((unsigned char*)&x, sizeof(x));
    random.seed(t ^ x);
    mt_seeded = 1;
  }
  unsigned x = random(), y;
  rand_bytes((unsigned char*)&y, sizeof(y));
  return x ^ y;
}

item::item() {
  m_fields["uuid"] = new_uuid();
}

item::item(const string &text) {
  size_t off = 0;
  while (off < text.size()) {
    auto nl = text.find('\n', off);
    if (nl == string::npos) nl = text.size();
    auto fp = text.find(':', off);
    if (fp >= nl) break;
    string f = text.substr(off, fp - off);
    string v = text.substr(fp + 1, nl - fp - 1);
    set_field(f, v);
    off = nl + 1;
  }
  if (!m_fields.count("uuid")) m_fields["uuid"] = new_uuid();
}

string item::new_uuid() {
  string uuid(32, 0);
  for (int i = 0; i < 32; ++i) {
    if (i == 12 || i == 16) continue;
    int v = merge_random() % 16;
    if (v < 0) v += 16;
    if (v >= 10) {
      v = 'a' + v - 10;
    } else {
      v = '0' + v;
    }
    uuid[i] = v;
  }
  uuid[12] = '4';
  int v = merge_random() % 4;
  if (v < 2) uuid[16] = '8' + v;
  else uuid[16] = 'a' + v - 2;
  return uuid;
}

string item::to_string() const {
  return fields_string("uuid", "username", "password", "url", "tags", "notes");
}

string item::field(const string &n) const {
  string v;
  auto it = m_fields.find(n);
  if (it != m_fields.end()) v = it->second;
  return v;
}

void item::set_field(const string &n, const string &v) {
  m_fields[n] = v;
}

string item::uuid() const {
  return field("uuid");
}

void item::uuid(const string &s) {
  set_field("uuid", s);
}

string item::username() const {
  return field("username");
}

void item::username(const string &s) {
  set_field("username", s);
}

string item::password() const {
  return field("password");
}

void item::password(const string &p) {
  set_field("password", p);
}

void item::generate_password(const string &cfg) {
  auto rand_graph = [](size_t l) -> string {
    string s(l, 0);
    for (size_t i = 0; i < l; ++i) {
      unsigned char c;
      do {
        c = 33 + merge_random() % (126 - 33);
      } while (!isgraph(c));
      s[i] = c;
    }
    return s;
  };
  auto rand_pop = [](const std::string &pop, size_t l) -> string {
    string s(l, 0);
    for (size_t i = 0; i < l; ++i) {
      s[i] = pop[merge_random() % pop.size()];
    }
    return s;
  };
  const char *d = "0123456789";
  const char *a = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char *u = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char *l = "abcdefghijklmnopqrstuvwxyz";
  const char *ad = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const char *ld = "abcdefghijklmnopqrstuvwxyz0123456789";
  const char *ud = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const char *h = "0123456789abcdef";
  const char *H = "0123456789ABCDEF";
  const char *hh = "0123456789ABCDEFabcdef";
  std::string pop = "";
  size_t len = 15;
  if (cfg.size()) {
    std::istringstream iss(to_std_string(cfg));
    while (iss.good()) {
      std::string p;
      iss >> p;
      if (iss.fail()) break;
      if (p.size() && isdigit(p[0])) {
        std::istringstream iss(p);
        iss >> len;
        continue;
      }
      if (p == "d") pop = d;
      else if (p == "l") pop = l;
      else if (p == "u") pop = u;
      else if (p == "a") pop = a;
      else if (p == "ad") pop = ad;
      else if (p == "ld") pop = ld;
      else if (p == "ud") pop = ud;
      else if (p == "h") pop = h;
      else if (p == "H") pop = H;
      else if (p == "hh") pop = hh;
      else if (p == "p" || p == "g") pop = "";
      else pop = p;
    }
  }
  if (!len) len = 15;
  if (!pop.size()) password(rand_graph(len));
  else password(rand_pop(pop, len));
}

string item::url() const {
  return field("url");
}

void item::url(const string &s) {
  set_field("url", s);
}

string item::tags() const {
  return field("tags");
}

void item::tags(const string &s) {
  set_field("tags", s);
}

string item::notes() const {
  return field("notes");
}

void item::notes(const string &s) {
  set_field("notes", s);
}

void item::update(const item &r) {
  for (auto it = r.m_fields.begin(); it != r.m_fields.end(); ++it) {
    m_fields[it->first] = it->second;
  }
}

void item::update_to(item &r) const {
  r.update(*this);
}

} // namespace ysid
