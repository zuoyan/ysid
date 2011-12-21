/**
 * @file   ysid.cpp
 *
 * @brief ysid main implementation
 *
 *
 */
#include "ysid_httpd_server.hpp"
#include "option_parser.hpp"
#include <memory>
#include "ysid_db.hpp"
#include "fmtstr.hpp"

using namespace ysid;

namespace {

string www_root = "www";string port = "127.0.0.1:8081";

}

int main(int argc, char *argv[]) {
  option_parser parser;
  parser
      .add_option("h|help", [&](int idx, int &argc, char *argv[]) -> int {parser.help(); return 1;})
      .add_value_option("port", &port, "port")
      .add_value_option("www", &www_root, "www docuement root")
      ;
  int idx = parser.parse(argc, argv);
  if (idx < argc) {
    parser.help();
  }
  int quit_flag = 0;
  std::unique_ptr<ysid_db> pdb(new ysid_db());
  void *ctx = start_server(pdb.get(), www_root, port, &quit_flag);
  if (!ctx) {
    fmterrlnt("start server with port %s failed", port);
    return 1;
  }
#ifdef WIN32
  HWND hWnd = GetConsoleWindow();
  ShowWindow(hWnd, SW_HIDE); // hide the window
#endif
  while (!quit_flag) sleep(1);
  stop_server(ctx);
  return 0;
}

