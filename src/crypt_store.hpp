/**
 * @file   crypt_store.hpp
 *
 * @brief crypt store
 *
 *
 */
#ifndef FILE_f059c66d_8f7f_4dd8_893b_1e78f64b2bb2_H
#define FILE_f059c66d_8f7f_4dd8_893b_1e78f64b2bb2_H 1
namespace ysid {

class kvstore;
class cipher;

kvstore *new_crypt_store(cipher *c, kvstore *s);

} // namespace ysid
#endif
