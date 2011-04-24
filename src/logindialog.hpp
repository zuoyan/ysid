/**
 * @file   logindialog.hpp
 *
 * @brief login dialog
 *
 *
 */
#ifndef FILE_5924f51b_8efb_4aa8_926a_84f7ab6a4655_H
#define FILE_5924f51b_8efb_4aa8_926a_84f7ab6a4655_H 1
#include "ui_logindialog.h"

class LoginDialog: public QDialog {
  Q_OBJECT;
 public:
  LoginDialog(QWidget *parent=0);

  void set_path(const QString &);
  QString get_path();
  void set_dpri(const QString &);
  QString get_dpri();
  void set_dpub(const QString &);
  QString get_dpub();
  void set_spri(const QString &);
  QString get_spri();
  void set_spub(const QString &);
  QString get_spub();
  void set_password(const QString &);
  QString get_password();
  void set_truncate(const QString &);
  QString get_truncate();
  void set_merge(const QString &);
  QString get_merge();

 private:
  Ui::LoginDialog ui;
};

#endif
