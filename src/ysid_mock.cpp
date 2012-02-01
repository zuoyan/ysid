/**
 * @file   ysid_mock.cpp
 *
 * @brief generating mock data.
 *
 *
 */
#include <string>
#include "string.hpp"
#include "ysid_db.hpp"
#include "item.hpp"
#include "ffile.hpp"
#include "kvstore.hpp"
#include "option_parser.hpp"
#include "mt_random.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <ctime>

using namespace ysid;

string random_string(mt_random &m, const string &pop, size_t n) {
  string r(n, 0);
  for (size_t i = 0; i < n; ++i) {
    r[i] = pop[m() % pop.size()];
  }
  return r;
}

int main(int argc, char *argv[]) {
  string path;
  string spri;
  string spub;
  string dpri;
  string dpub;
  string password;
  size_t n;

  mt_random random;
  random.seed(time(NULL));

  option_parser parser;
  parser
      .add_option("h|help", [&](int idx, int &argc, char *argv[]) -> int {parser.help(); return 1;})
      .add_value_option("path", &path, "path")
      .add_value_option("spri", &spri, "spri")
      .add_value_option("spub", &spub, "spub")
      .add_value_option("dpri", &dpri, "dpri")
      .add_value_option("dpub", &dpub, "dpub")
      .add_value_option("password", &password, "password")
      .add_value_option("num", &n, "num. of mocked")
      ;
  int idx = parser.parse(argc, argv);
  if (idx != argc) parser.help();
  if (!password.size()) {
    char *pass = getpass("Enter password for disk private key:");
    password = pass;
    memset(pass, 0, password.size());
  }

  ysid_db *db = new ysid_db();
  db->set_path(path);
  db->set_spub_key(spub);
  db->set_spri_key(spri);
  db->set_dpri_key(dpri);
  db->set_dpub_key(dpub);
  db->set_password(password);
  db->load_session_private();
  const char *pop = "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "0123456789";
  for (size_t i = 0; i < n; ++i) {
    item n;
    n.username(random_string(random, pop, random() % 5 + 2));
    n.password(random_string(random, pop, random() % 10 + 4));
    n.url("http://" + random_string(random, pop, random() % 10 + 2) + ".com");
    n.notes(random_string(random, pop, random() % 20 + 10));
    n.tags(random_string(random, pop, random() % 3 + 1));
    db->db()->put(n.uuid(), n.to_string());
  }

  delete db;
  return 0;
}
