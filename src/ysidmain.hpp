/**
 * @file   ysidmain.hpp
 *
 * @brief ysid main ui
 *
 *
 */
#ifndef FILE_067fcbb6_d5ef_47d6_a25c_91e2617d7c2c_H
#define FILE_067fcbb6_d5ef_47d6_a25c_91e2617d7c2c_H 1
#include "ui_ysidmain.h"
#include <QSystemTrayIcon>

class LoginDialog;
class YsidDbModel;
class YsidDbView;
class QTableView;
class QLineEdit;
class QSystemTrayIcon;
class QLabel;
class QToolBar;

class YsidMain : public QMainWindow {
  Q_OBJECT;
 public:
  explicit YsidMain(QWidget *parent=0);
  virtual void closeEvent(QCloseEvent *event);

 public slots:
  void info(const QString &mesg);
  void list_info(const QString &mesg);

 protected:
  virtual void keyPressEvent(QKeyEvent * e);

 private slots:
  void login();
  void logout();
  void quit();
  void about();
  void error(const QString &mesg);
  void add();
  void edit(const QString &uuid);
  void edit_selected();
  void erase_selected();
  void copy_password();
  void trayicon_activated(QSystemTrayIcon::ActivationReason reason);
  void item_context_menu(const QPoint &pos);
  void toggle_toolbar();
  void toggle_httpd();
  void list_next();
  void hot_activate();
  void options();
  void autotype();
  void autotype_username();
  void autotype_url();
  void autotype_password();
  void do_login();

 private:
  void type_str(const char *s);
  void autotype_str(const char *s);

  Ui::YsidMain ui;
  LoginDialog *m_logdlg;
  YsidDbView *m_db_view;
  YsidDbModel *m_db_model;
  QLineEdit *m_list;
  QSystemTrayIcon *m_trayicon;
  QLabel *m_list_info, *m_info;
  QToolBar *m_toolbar;
  void *m_httpd_ctx;
};

#endif
