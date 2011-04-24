/**
 * @file   option_parser.cpp
 *
 * @brief stupid option parser implementation
*
 *
 */
#include <functional>
#include <climits>
#include "option_parser.hpp"
#include <iostream>

namespace ysid {

int option_parser::parse(int argc, char *argv[]) {
  int idx = 1;
  while (idx < argc) {
    std::string opt = argv[idx];
    bool found = false;
    if (!opt.size() || opt[0] != '-') {
      for (size_t i = 0; i < m_positions.size(); ++i) {
        if (m_positions[i].limit > 0) {
          m_positions[i].limit --;
          m_positions[i].func(opt);
          found = true;
          break;
        }
      }
      if (!found) help_at(idx);
      continue;
    }
    if (opt == "--") return idx + 1;
    while (opt.size() && opt[0] == '-') opt = opt.substr(1);
    if (!opt.size()) help_at(idx);
    for (size_t i = 0; i < m_options.size(); ++i) {
      auto &option = m_options[i];
      if (is_opt(opt, option.choices)) {
        idx = option.func(idx, argc, argv);
        found = true;
        break;
      }
    }
    if (found) continue;
    break;
  }
  return idx;
}

option_parser& option_parser::add_position(
    int limit,
    std::function<void(const std::string &opt)> func,
    const std::string &doc) {
  position_type pos(limit, func, doc);
  m_positions.push_back(pos);
  return *this;
}

option_parser& option_parser::add_position(
    std::function<void(const std::string &opt)> func,
    const std::string &doc) {
  position_type pos(INT_MAX, func, doc);
  m_positions.push_back(pos);
  return *this;
}

option_parser& option_parser::add_option(
    const std::string &choices,
    std::function<int(int, int &argc, char *argv[])> func,
    const std::string &doc) {
  option_type option(choices, func, doc);
  m_options.push_back(option);
  return *this;
}



void option_parser::help(int status) {
  std::cerr << "option_parser help" << std::endl;
  std::cerr << "Usage prog [option]";
  for (auto it = m_positions.begin(); it != m_positions.end(); ++it) {
    auto &pos = *it;
    if (pos.limit > 0) {
      std::cerr << ' ';
      if (pos.limit > 1) {
        std::cerr << '[' << pos.doc << "]+";
      } else {
        std::cerr << '\'' << pos.doc << '\'';
      }
    }
  }
  std::cerr << std::endl;
  if (m_options.size()) {
    std::cerr << "Options:" << std::endl;
    for (auto it = m_options.begin(); it != m_options.end(); ++it) {
      auto &opt = *it;
      std::cerr << " --" << opt.choices << '\t' << opt.doc << std::endl;
    }
  }
  exit(status);
}

void option_parser::help_at(int idx, int status) {
  std::cerr << "option_parser help at " << idx << std::endl;
  help(status);
}

int option_parser::is_opt(const std::string &opt, const std::string &choices) {
  size_t off = 0;
  do {
    auto nl = choices.find('|', off);
    if (nl == std::string::npos) nl = choices.size();
    if (choices.compare(off, nl - off, &opt[0], opt.size()) == 0) return 1;
    off = nl + 1;
  } while (off < choices.size());
  return false;
}

} // namespace ysid
