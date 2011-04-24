/**
 * @file   ysid_from_pgman.cpp
 *
 * @brief convert pgman to ysid format
 *
 *
 */
#include <string>
#include "ysid_db.hpp"
#include "item.hpp"
#include "ffile.hpp"
#include "kvstore.hpp"
#include "option_parser.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>

using namespace ysid;
using std::string;

int main(int argc, char *argv[]) {
  string path;
  string spri;
  string spub;
  string dpri;
  string dpub;
  string password;

  option_parser parser;
  parser
      .add_option("h|help", [&](int idx, int &argc, char *argv[]) -> int {parser.help(); return 1;})
      .add_value_option("path", &path, "path")
      .add_value_option("spri", &spri, "spri")
      .add_value_option("spub", &spub, "spub")
      .add_value_option("dpri", &dpri, "dpri")
      .add_value_option("dpub", &dpub, "dpub")
      .add_value_option("password", &password, "password")
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

  string line;
  item n;
  while (std::cin.good()) {
    std::getline(std::cin, line);
    if (line == "") {
      if (n.username().size() || n.password().size() || n.url().size()) {
        db->db()->put(n.uuid(), n.to_string());
      }
      n = item();
      continue;
    }
    auto f = line.find(':');
    if (f == string::npos) {
      std::cerr << "error line " << line << std::endl;
      continue;
    }
    string field = line.substr(0, f);
    string value = line.substr(f + 1);
    if (!value.size()) continue;
    for (size_t i = 0; i < field.size(); ++i) field[i] = tolower(field[i]);
    if (field == "id") {
      field = "uuid";
      std::cerr << "Adding " << value << std::endl;
    }
    n.set_field(field, value);
  }
  if (n.username().size() || n.password().size() || n.url().size()) {
    db->db()->put(n.uuid(), n.to_string());
  }

  delete db;
  return 0;
}
