/**
 * @file   ysid_qt.hpp
 *
 * @brief ysid qt header
 *
 *
 */
#ifndef FILE_16851210_466f_4a2c_b042_bd7bb3de5fd0_H
#define FILE_16851210_466f_4a2c_b042_bd7bb3de5fd0_H 1
#include "ysid.hpp"
#include <QtCore>

ysid::string qstring_to_local8bit(const QString &qs);
ysid::string qstring_to_utf8(const QString &qs);
ysid::string options_get_str(const ysid::string &key, const ysid::string &dft="");
QString options_get_qstr(const ysid::string &key, const QString &dft="");

void options_set_str(const ysid::string &key, const ysid::string &value);
void options_set_qstr(const ysid::string &key, const QString &value);
bool options_get_bool(const ysid::string &key, bool dft);
void options_set_bool(const ysid::string &key, bool value);
int options_get_int(const ysid::string &key, int dft);
void options_set_int(const ysid::string &key, int value);

#endif
