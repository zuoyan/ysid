/**
 * @file   editdialog.cpp
 *
 * @brief editdialog
 *
 *
 */
#include "editdialog.hpp"
#include "item.hpp"

static ysid::string qstring_to_utf8(const QString &qs) {
  auto qb = qs.toUtf8();
  return ysid::string(qb.begin(), qb.end());
}

EditDialog::EditDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);
  setModal(true);
  ui.passwordEdit->setEchoMode(QLineEdit::Password);
  connect(ui.passwordEdit, SIGNAL(editingFinished()), this, SLOT(check_password()));
  connect(ui.usernameEdit, SIGNAL(editingFinished()), this, SLOT(check_username()));
}

void EditDialog::check_username() {
  auto s = qstring_to_utf8(get_username());
  if (s.size() && s[0] == ' ') {
    ysid::item n;
    s = ysid::string("l 6 ") + s;
    n.generate_password(s);
    set_username(tr(n.password().c_str()));
  }
}

void EditDialog::check_password() {
  auto s = qstring_to_utf8(get_password());
  if (s.size() && s[0] == ' ') {
    ysid::item n;
    s = ysid::string("p 20 ") + s;
    n.generate_password(s);
    set_password(tr(n.password().c_str()));
  }
}

void EditDialog::set_username(const QString &username) {
  ui.usernameEdit->setText(username);
}

QString EditDialog::get_username() {
  return ui.usernameEdit->text();
}

void EditDialog::set_url(const QString &url) {
  ui.urlEdit->setText(url);
}

QString EditDialog::get_url() {
  return ui.urlEdit->text();
}

void EditDialog::set_tags(const QString &tags) {
  ui.tagsEdit->setText(tags);
}

QString EditDialog::get_tags() {
  return ui.tagsEdit->text();
}

void EditDialog::set_notes(const QString &notes) {
  ui.notesEdit->setText(notes);
}

QString EditDialog::get_notes() {
  return ui.notesEdit->text();
}

void EditDialog::set_password(const QString &password) {
  ui.passwordEdit->setText(password);
}

QString EditDialog::get_password() {
  return ui.passwordEdit->text();
}
