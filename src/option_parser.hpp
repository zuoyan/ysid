/**
 * @file   option_parser.hpp
 *
 * @brief stupid option parser
 *
 *
 */
#ifndef FILE_2cd8cf7f_2094_4078_b8b6_9d1336e0be3d_H
#define FILE_2cd8cf7f_2094_4078_b8b6_9d1336e0be3d_H 1

#include <functional>
#include <sstream>
#include <vector>
#include <string>

namespace ysid {

class option_parser {
  template <class T>
  struct value_option_func {
    value_option_func(option_parser *parser, T *dest)
        : m_parser(parser), m_dest(dest) {}
    int operator()(int idx, int &argc, char *argv[]) {
      if (idx + 1 >= argc) m_parser->help_at(idx);
      std::string v = argv[idx + 1];
      std::istringstream iss(v);
      iss >> *m_dest;
      if (iss.fail()) m_parser->help_at(idx);
      return idx + 2;
    }
    option_parser *m_parser;
    T *m_dest;
  };

  template <class T>
  struct value_position_func {
    value_position_func(option_parser *parser, T *dest)
        : m_parser(parser), m_dest(dest) {}
    void operator()(const std::string &v) {
      std::istringstream iss(v);
      iss >> *m_dest;
      if (iss.fail()) m_parser->help();
    }
    option_parser *m_parser;
    T *m_dest;
  };

  struct option_type {
    std::string choices;
    std::function<int(int,int &argc, char *argv[])> func;
    std::string doc;
    option_type(const std::string &c,
                std::function<int(int,int &argc, char *argv[])> f,
                const std::string &d)
        : choices(c), func(f), doc(d) {}
  };

  struct position_type {
    int limit;
    std::function<void(std::string &opt)> func;
    std::string doc;
    position_type(int l,
                  std::function<void(std::string &opt)> f,
                  const std::string &d)
        : limit(l), func(f), doc(d){}
  };

 public:
  int parse(int argc, char *argv[]);

  option_parser& add_position(
      int limit,
      std::function<void(const std::string &opt)> func,
      const std::string &doc);

  option_parser& add_position(
      std::function<void(const std::string &opt)> func,
      const std::string &doc);

  template <class T>
  option_parser& add_value_position(
      int limit, T *dest, const std::string &doc) {
    value_position_func<T> func(this, dest);
    return add_position(1, func, doc);
  }

  template <class T>
  option_parser& add_value_position(
      T *dest, const std::string &doc) {
    return add_value_position(1, dest, doc);
  }

  option_parser& add_option(
      const std::string &choices,
      std::function<int(int, int &argc, char *argv[])> func,
      const std::string &doc="");

  template <class T>
  option_parser& add_value_option(const std::string &choices, T *dest, const std::string &doc="") {
    return add_option(choices, value_option_func<T>(this, dest), doc);
  }

  void help(int status=1);

 private:

  void help_at(int idx, int status=1);
  int is_opt(const std::string &opt, const std::string &choices);

  std::vector< option_type > m_options;
  std::vector< position_type > m_positions;
};

} // namespace ysid
#endif
