/**
 * @file   ysid_httpd_server.hpp
 *
 * @brief httpd server interface
 *
 *
 */
#ifndef FILE_9024ed87_cdab_4491_b27b_e20617231567_H
#define FILE_9024ed87_cdab_4491_b27b_e20617231567_H 1
#include "ysid.hpp"

namespace ysid {
class ysid_db;

void* start_server(ysid_db *db, string www, string port, int *pqf);
void stop_server(void *ctx);

} // namespace ysid
#endif
