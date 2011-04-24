/**
 * @file   optionsdialog.cpp
 *
 * @brief options dialog
 *
 *
 */
#include "optionsdialog.hpp"
#include "ysid_qt.hpp"

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent) {
  ui.setupUi(this);
  setModal(true);
  ui.hotEdit->setText(options_get_qstr("globalhot", "Meta+Y"));
  ui.passwordEdit->setChecked(options_get_bool("remember_password", false));
  ui.toolbarEdit->setChecked(options_get_bool("toolbar", true));
  ui.httpdEdit->setChecked(options_get_bool("httpd/start", false));
  ui.wwwEdit->setText(options_get_qstr("httpd/www", "www"));
  ui.portEdit->setText(options_get_qstr("httpd/port", "127.0.0.1:8081"));
  ui.delayEdit->setText(options_get_qstr("autotype/delay", "12000"));
}

void OptionsDialog::accept() {
  options_set_qstr("globalhot", ui.hotEdit->text());
  options_set_bool("remember_password", ui.passwordEdit->isChecked());
  options_set_bool("toolbar", ui.toolbarEdit->isChecked());
  options_set_bool("httpd/start", ui.httpdEdit->isChecked());
  options_set_qstr("httpd/www", ui.wwwEdit->text());
  options_set_qstr("httpd/port", ui.portEdit->text());
  options_set_qstr("autotype/delay", ui.delayEdit->text());

  QDialog::accept();
}
