/**
 * @file   ysid_httpd_server.cpp
 *
 * @brief   httpd server
 *
 *
 */
#include "ysid_config.h"
#include "ysid.hpp"
#include "kvstore.hpp"
#include "fmtstr.hpp"
#include "item.hpp"
#include "mt_random.hpp"
#include "ysid_db.hpp"
#include "clipboard.hpp"

#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cctype>

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <unistd.h>
#endif

#include "mongoose.h"

using namespace ysid;

namespace {

int *pquit_flag;

struct scope_session {
  scope_session(ysid_db *db)
      : db(db) {
    db->load_session_private();
  }

  ~scope_session() {
    db->clear_session_private();
  }

  ysid_db *db;
};

bool str_slash_extract(const string &s, const string &fmt) {
  auto mark = s.find('?');
  if (!mark == string::npos) {
    return s.substr(0, mark) == fmt;
  }
  return s == fmt;
}

bool str_slash_extract_one(
    string &s, string &fmt, string &v) {
  auto mark = s.find('?');
  if (mark != string::npos) {
    s = s.substr(0, mark);
  }
  auto p = fmt.find('*');
  if (p == string::npos) return false;
  if (s.compare(0, p, fmt, 0, p) != 0) return false;
  if (p + 1 == fmt.size()) {
    v = s.substr(p);
    s = "";
    fmt = "";
    return true;
  }
  auto sp = s.find(fmt[p + 1], p + 1);
  if (sp == string::npos) return false;
  v = s.substr(p, sp - p);
  s = s.substr(sp);
  fmt = fmt.substr(p + 1);
  return true;
}

bool str_slash_extract(
    const string &s, const string &fmt, string &v) {
  string ss = s;
  string ff = fmt;
  return str_slash_extract_one(ss, ff, v) && ss == ff;
}

bool str_slash_extract(
    const string &s, const string &fmt, string &a, string &b) {
  string ss = s, ff = fmt;
  return str_slash_extract_one(ss, ff, a)
      && str_slash_extract_one(ss, ff, b)
      && ss == ff;
}

bool str_slash_extract(
    const string &s, const string &fmt, string &a, string &b, string &c) {
  string ss = s, ff = fmt;
  return str_slash_extract_one(ss, ff, a)
      && str_slash_extract_one(ss, ff, b)
      && str_slash_extract_one(ss, ff, c)
      && ss == ff;
}

string url_decode(const string &src, bool is_form_url_encoded=true) {
  auto hextoi = [](int x) -> int {
    if (isdigit(x))  return x - '0';
    return x - 'a' + 10;
  };
  string dst;
  size_t off = 0;
  while (off < src.size()) {
    if (src[off] == '%'
        && off + 2 < src.size()
        && isxdigit(src[off + 1])
        && isxdigit(src[off + 2])) {
      int a = tolower(src[off + 1]);
      int b = tolower(src[off + 2]);
      dst.push_back((hextoi(a) << 4) | hextoi(b));
      off += 3;
      continue;
    }
    if (is_form_url_encoded && src[off] == '+') {
      dst.push_back(' ');
    } else {
      dst.push_back(src[off]);
    }
    off += 1;
  }
  return dst;
}

void query_str_to_map(const string &q, std::unordered_map<string, string> &map) {
  size_t off = 0;
  while (off < q.size()) {
    auto nt = q.find('&', off);
    if (nt == string::npos) nt = q.size();
    auto eq = q.find('=', off);
    if (eq == string::npos || eq >= nt) break;
    string name = q.substr(off, eq - off);
    string value = q.substr(eq + 1, nt - eq - 1);
    map[name] = url_decode(value);
    off = nt + 1;
  }
}

void data_to_map(
    struct mg_connection *conn, const struct mg_request_info *request_info,
    std::unordered_map<string, string> &map) {
  if (request_info->query_string) query_str_to_map(request_info->query_string, map);
  if (!strcmp(request_info->request_method, "POST")
      || !strcmp(request_info->request_method, "PUT")) {
    const char *cl = mg_get_header(conn, "Content-Length");
    if (cl) {
      size_t buf_len = (size_t)to_int(cl);
      if (buf_len) {
        string buf(buf_len, 0);
        mg_read(conn, (void*)buf.data(), buf_len);
        query_str_to_map(buf, map);
      }
    }
  }
}

std::unordered_map<string, string> data_to_map(
    struct mg_connection *conn, const struct mg_request_info *request_info) {
  std::unordered_map<string, string> map;
  data_to_map(conn, request_info, map);
  return map;
}

string str_html_encode(const string &s) {
  string r;
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '&') {
      r += "&amp;";
      continue;
    }
    if (s[i] == '<') {
      r += "&lt;";
      continue;
    }
    if (s[i] == '>') {
      r += "&gt;";
      continue;
    }
    r += s[i];
  }
  return r;
}

const char *text_plain = "text/plain; charset=utf-8";

void* ysid_httpd_write_gen(
    struct mg_connection *conn, const struct mg_request_info *request_info,
    int status, const string &status_text,
    const string &mimetype, const string &content) {
  static void *done = (void*)"done";
  const char *date_fmt = "%a, %d %b %Y %H:%M:%S";
  time_t curtime = time(NULL);
  struct tm t;
  char date[128];
  memset(date, 0, sizeof(date));
#ifdef WIN32
  localtime_s(&t, &curtime);
#else
  localtime_r(&curtime, &t);
#endif
  strftime(&date[0], sizeof(date), date_fmt, &t);

  mg_printf(conn, "HTTP/1.1 %d %s\r\n", status, status_text.c_str());
  mg_printf(conn, "Server: ysid_httpd\r\n");
  mg_printf(conn, "Content-Type: %s\r\n", mimetype.c_str());
  mg_printf(conn, "Content-Transfer-Encoding: binary\r\n");
  mg_printf(conn, "Content-Length: %zd\r\n", content.size());
  mg_printf(conn, "Expires: Tue, 03 Jul 2001 06:00:00 GMT\r\n");
  mg_printf(conn, "Cache-Control: no-store, no-cache, must-revalidate, max-age=0\r\n");
  mg_printf(conn, "Cache-Control: post-check=0, pre-check=0\r\n");
  mg_printf(conn, "Pragma: no-cache\r\n");
  mg_printf(conn, "Date: %s\r\n", &date[0]);
  mg_printf(conn, "\r\n");
  if (content.size()) mg_write(conn, content.data(), content.size());
  return done;
}

void* ysid_httpd_write_html_pre(
    struct mg_connection *conn, const struct mg_request_info *request_info,
    int status, const string &status_text, const string &content) {
  string r = "<!DOCTYPE html><html lang='en-US'>"
      "<head>"
      "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>"
      "<title> ysid Yun's security information dictionary</title></head><body>";
  r += str_html_encode(content);
  r += "</body></html>";
  return ysid_httpd_write_gen(conn, request_info, status, status_text,
                              "text/html", r);
}

void* ysid_httpd_write_text(
    struct mg_connection *conn, const struct mg_request_info *request_info,
    int status, const string &status_text, const string &content) {
  return ysid_httpd_write_gen(conn, request_info, status, status_text, text_plain, content);
}

void *ysid_httpd_write_ok(
    struct mg_connection *conn, const struct mg_request_info *request_info,
    const string &mimetype, const string &content) {
  return ysid_httpd_write_gen(conn, request_info, 200, "OK", mimetype, content);
}

void* ysid_httpd_write_notfound(
    struct mg_connection *conn, const struct mg_request_info *request_info) {
  return ysid_httpd_write_text(conn, request_info, 404, "Not Found", "Page not Found");
}

void *do_ysid_httpd_handler(
    enum mg_event event,
    struct mg_connection *conn,
    const struct mg_request_info *request_info) {
  if (!event == MG_NEW_REQUEST) return 0;
  ysid_db *db = (ysid_db*)request_info->user_data;
  string uri = request_info->uri;
  auto map = data_to_map(conn, request_info);
  {
    string id;
    if (str_slash_extract(uri, "/get/*/", id)) {
      if (db->db()->has(id)) {
        scope_session session(db);
        string value = db->db()->get(id);
        if (!map.count("password") || !to_bool(map["password"])) {
          item n(value);
          value = n.fields_string("uuid", "username", "url", "tags", "notes");
        }
        return ysid_httpd_write_ok(conn, request_info, text_plain, value);
      } else {
        return ysid_httpd_write_notfound(conn, request_info);
      }
    }
  }
  {
    string id, field;
    if (str_slash_extract(uri, "/get/*/*/", id, field)) {
      if (db->db()->has(id)) {
        scope_session session(db);
        item n(db->db()->get(id));
        return ysid_httpd_write_ok(conn, request_info, text_plain, n.field(field));
      }
      return ysid_httpd_write_notfound(conn, request_info);
    }
  }
  {
    if (str_slash_extract(uri, "/put/")) {
      scope_session session(db);
      item n;
      n.password(map["password"]);
      if (map["uuid"].size()) {
        n.uuid(map["uuid"]);
        if (!n.password().size()) {
          n.password(item(db->db()->get(n.uuid())).password());
        }
      }
      if (!n.password().size() || map["genpassword"].size()) {
        n.generate_password(map["genpassword"]);
      }
      n.username(map["username"]);
      n.url(map["url"]);
      n.notes(map["notes"]);
      n.tags(map["tags"]);
      db->db()->put(n.uuid(), n.to_string());
      db->db()->sync();
      return ysid_httpd_write_ok(conn, request_info, text_plain, n.uuid());
    }
  }
  {
    if (str_slash_extract(uri, "/list/")) {
      string s;
      if (to_bool(map["fields"])) {
        scope_session session(db);
        bool hold_password = to_bool(map["password"]);
        db->db()->fold([&](const string k, const string v, void *ignore) {
              item n(v);
              if (!hold_password) n.password("");
              s += n.to_string() + "\n";
          }, 0);
      } else {
        db->db()->fold_keys([&](const string &k, void *ignore) {
            s += k + "\n";
          }, 0);
      }
      return ysid_httpd_write_ok(conn, request_info, text_plain, s);
    }
  }
  {
    string id;
    if (str_slash_extract(uri, "/erase/*/", id)) {
      scope_session session(db);
      if (db->db()->has(id)) {
        db->db()->erase(id);
        return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
      }
      return ysid_httpd_write_notfound(conn, request_info);
    }
  }
  {
    if (str_slash_extract(uri, "/login/")) {
      db->clear();
      string truncate = map["truncate"];
      if (truncate.size()) db->set_truncate_log(to_int(truncate));
      else db->set_truncate_log(0);
      db->set_path(map["path"]);
      db->set_password(map["password"]);
      db->set_dpri_key(map["dpri"]);
      db->set_dpub_key(map["dpub"]);
      db->set_spri_key(map["spri"]);
      db->set_spub_key(map["spub"]);
      db->db(); // force load
      bool do_merge = to_bool(map["merge"]);
      if (do_merge) db->db()->merge();
      return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
    }
  }
  if (str_slash_extract(uri, "/logs/")) {
    size_t x = db->db()->logs();
    return ysid_httpd_write_ok(conn, request_info, text_plain, to_string(x));
  }
  {
    if (str_slash_extract(uri, "/logout/")) {
      db->clear();
      return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
    }
  }
  if (str_slash_extract(uri, "/quit/")) {
    db->clear();
    if (*pquit_flag) *pquit_flag = 1;
    return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
  }
  if (str_slash_extract(uri, "/random/")) {
    string pop = map["pop"];
    string len = map["len"];
    item n;
    n.generate_password(pop + " " + len);
    string gen = n.password();
    if (to_bool(map["html"]))
      return ysid_httpd_write_html_pre(conn, request_info, 200, "OK", gen);
    else
      return ysid_httpd_write_ok(conn, request_info, text_plain, gen);
  }
  if (str_slash_extract(uri, "/uuid/")) {
    item n;
    return ysid_httpd_write_ok(conn, request_info, text_plain, n.uuid());
  }
  {
    string uuid;
    if (str_slash_extract(uri, "/copy/*/password/", uuid)) {
      scope_session session(db);
      item n(db->db()->get(uuid));
      if (n.uuid() == uuid) {
        int r = clipboard_copy(n.password());
        if (r != -1) return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
        return ysid_httpd_write_gen(conn, request_info, 501, "Server error", text_plain, "Not implemented");
      } else {
        return ysid_httpd_write_notfound(conn, request_info);
      }
    }
  }
  if (str_slash_extract(uri, "/copy/")) {
    string text = map["text"];
    int r = clipboard_copy(text);
    if (r != -1) return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
    return ysid_httpd_write_gen(conn, request_info, 501, "Server error", text_plain, "Not implemented");
  }
#ifdef WIN32
  if (str_slash_extract(uri, "/windows/show/")) {
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_RESTORE); // show the window
    return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
  }
  if (str_slash_extract(uri, "/windows/hide/")) {
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE); // hide the window
    return ysid_httpd_write_ok(conn, request_info, text_plain, "OK");
  }
#endif

  return NULL;
}

void *ysid_httpd_handler(
    enum mg_event event,
    struct mg_connection *conn,
    const struct mg_request_info *request_info) {
  try {
    return do_ysid_httpd_handler(event, conn, request_info);
  } catch (std::runtime_error &e) {
    return ysid_httpd_write_text(conn, request_info, 403, "Forbidden",
                                 "server error: " + string(e.what()));
  }
  return NULL;
}

} // namespace anonymous

namespace ysid {

void* start_server(ysid_db *db, string www, string port, int *pqf) {
  pquit_flag = pqf;
  const char *mgoptions[] = {
    "listening_ports", port.c_str(),
    "num_threads", "4",
    "document_root", www.c_str(),
    NULL
  };
  return (void*)mg_start(&ysid_httpd_handler, db, mgoptions);
}

void stop_server(void *ctx) {
  if (ctx) {
    mg_stop((mg_context*)ctx);
  }
}

} // namespace ysid
