/**
 * @file   logindialog.cpp
 *
 * @brief login dialog
 *
 *
 */
#include "logindialog.hpp"
#include "ysid.hpp"
#include "ysid_qt.hpp"

ysid::string options_get_str(const ysid::string &key, const ysid::string dft);

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);
  ui.passwordEdit->setEchoMode(QLineEdit::Password);
  setModal(true);
  set_path(options_get_qstr("login/path"));
  set_dpri(options_get_qstr("login/dpri"));
  set_dpub(options_get_qstr("login/dpub"));
  set_spri(options_get_qstr("login/spri"));
  set_spub(options_get_qstr("login/spub"));
  set_password(options_get_qstr("login/password"));
  if (!get_path().size()) ui.pathEdit->setFocus();
  else if (!get_dpri().size()) ui.dpriEdit->setFocus();
  else if (!get_dpub().size()) ui.dpubEdit->setFocus();
  else ui.passwordEdit->setFocus();
}

void LoginDialog::set_path(const QString &path) {
  ui.pathEdit->setText(path);
}

QString LoginDialog::get_path() {
  return ui.pathEdit->text();
}

void LoginDialog::set_dpri(const QString &dpri) {
  ui.dpriEdit->setText(dpri);
}

QString LoginDialog::get_dpri() {
  return ui.dpriEdit->text();
}

void LoginDialog::set_dpub(const QString &dpub) {
  ui.dpubEdit->setText(dpub);
}

QString LoginDialog::get_dpub() {
  return ui.dpubEdit->text();
}

void LoginDialog::set_spri(const QString &spri) {
  ui.spriEdit->setText(spri);
}

QString LoginDialog::get_spri() {
  return ui.spriEdit->text();
}

void LoginDialog::set_spub(const QString &spub) {
  ui.spubEdit->setText(spub);
}

QString LoginDialog::get_spub() {
  return ui.spubEdit->text();
}

void LoginDialog::set_password(const QString &password) {
  ui.passwordEdit->setText(password);
}

QString LoginDialog::get_password() {
  return ui.passwordEdit->text();
}

void LoginDialog::set_truncate(const QString &truncate) {
  ui.truncateEdit->setText(truncate);
}

QString LoginDialog::get_truncate() {
  return ui.truncateEdit->text();
}

void LoginDialog::set_merge(const QString &merge) {
  ui.mergeEdit->setText(merge);
}

QString LoginDialog::get_merge() {
  return ui.mergeEdit->text();
}
