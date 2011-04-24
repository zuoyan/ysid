/**
 * @file   ysidmain.cpp
 *
 * @brief ysid main ui
 *
 *
 */
#include <QtGui>
#include "ysid_qt.hpp"
#include "ysidmain.hpp"
#include "logindialog.hpp"
#include "editdialog.hpp"
#include <iostream>
#include "ysid_db_model.hpp"
#include "ysiddbview.hpp"
#include "ysid_httpd_server.hpp"
#include "optionsdialog.hpp"
#include "GlobalHot.hpp"
#include "xdo.h"

#include <QX11Info>

YsidMain::YsidMain(QWidget *parent) : QMainWindow(parent) {
  ui.setupUi(this);
  setWindowTitle("ysid Yun's security information dictionary");

  connect(ui.actionLogin, SIGNAL(triggered()), this, SLOT(login()));
  connect(ui.actionLogout, SIGNAL(triggered()), this, SLOT(logout()));
  connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(quit()));
  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  connect(ui.actionAdd_Item, SIGNAL(triggered()), this, SLOT(add()));
  connect(ui.actionEdit, SIGNAL(triggered()), this, SLOT(edit_selected()));
  connect(ui.actionErase, SIGNAL(triggered()), this, SLOT(erase_selected()));
  connect(ui.actionCopy_password, SIGNAL(triggered()), this, SLOT(copy_password()));
  connect(ui.actionToolbar, SIGNAL(triggered()), this, SLOT(toggle_toolbar()));
  connect(ui.actionHttpd, SIGNAL(triggered()), this, SLOT(toggle_httpd()));
  connect(ui.actionOptions, SIGNAL(triggered()), this, SLOT(options()));
  connect(ui.actionAutotype, SIGNAL(triggered()), this, SLOT(autotype()));
  connect(ui.actionAutotype_username, SIGNAL(triggered()), this, SLOT(autotype_username()));
  connect(ui.actionAutotype_password, SIGNAL(triggered()), this, SLOT(autotype_password()));
  connect(ui.actionAutotype_url, SIGNAL(triggered()), this, SLOT(autotype_url()));

  m_toolbar = addToolBar(tr("ysid"));
  m_toolbar->addAction(ui.actionAutotype);
  m_toolbar->addAction(ui.actionAutotype_username);
  m_toolbar->addAction(ui.actionAutotype_password);
  m_toolbar->addAction(ui.actionAutotype_url);
  m_toolbar->addAction(ui.actionCopy_password);
  m_toolbar->addAction(ui.actionEdit);
  m_toolbar->addAction(ui.actionAdd_Item);
  m_toolbar->addAction(ui.actionErase);
  m_toolbar->addAction(ui.actionLogin);
  m_toolbar->addAction(ui.actionLogout);
  m_toolbar->addAction(ui.actionExit);
  if (!options_get_bool("toolbar", true)) {
    toggle_toolbar();
  }

  m_logdlg = new LoginDialog(this);

  auto statusbar = statusBar();
  m_list_info = new QLabel(statusbar);
  m_info = new QLabel(statusbar);
  statusbar->addWidget(m_list_info);
  statusbar->addWidget(m_info);

  m_list = new QLineEdit(this);
  m_db_view = new YsidDbView(this);
  m_db_model = new YsidDbModel(this);
  m_db_view->setModel(m_db_model);
  m_db_view->setSelectionMode(QAbstractItemView::SingleSelection);
  m_db_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_db_view->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  m_db_view->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  QWidget *central = new QWidget(this);
  QVBoxLayout *vb = new QVBoxLayout(central);
  vb->addWidget(m_list);
  vb->addWidget(m_db_view);
  setCentralWidget(central);
  connect(m_list, SIGNAL(textChanged(const QString &)), m_db_model, SLOT(filter_items(const QString &)));
  connect(m_list, SIGNAL(returnPressed()), this, SLOT(list_next()));
  connect(m_db_model, SIGNAL(error(const QString &)), this, SLOT(error(const QString &)));
  connect(m_db_model, SIGNAL(info(const QString &)), this, SLOT(list_info(const QString &)));

  m_trayicon = new QSystemTrayIcon(QIcon(":/imgs/trayicon.png"));
  m_trayicon->setToolTip("ysid Yun's security information dictionary");
  m_trayicon->show();
  connect(m_trayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(trayicon_activated(QSystemTrayIcon::ActivationReason)));
  connect(m_db_view, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(item_context_menu(const QPoint&)));

  m_httpd_ctx = 0;
  if (options_get_bool("httpd/start", false)) {
    toggle_httpd();
  }
}

void YsidMain::item_context_menu(const QPoint &pos) {
  QMenu *menu = new QMenu;
  menu->addAction(ui.actionAutotype);
  menu->addAction(ui.actionAutotype_username);
  menu->addAction(ui.actionAutotype_password);
  menu->addAction(ui.actionAutotype_url);
  menu->addAction(ui.actionCopy_password);
  menu->addAction(ui.actionEdit);
  menu->addAction(ui.actionErase);
  menu->addAction(ui.actionAdd_Item);
  menu->exec(m_db_view->mapToGlobal(pos));
  delete menu;
}

void YsidMain::toggle_toolbar() {
  if (m_toolbar->isHidden()) {
    options_set_bool("toolbar", false);
    m_toolbar->show();
    ui.actionToolbar->setText(tr("Hide &Toolbar"));
  } else {
    options_set_bool("toolbar", true);
    m_toolbar->hide();
    ui.actionToolbar->setText(tr("Show &Toolbar"));
  }
}

void YsidMain::toggle_httpd() {
  if (m_httpd_ctx) {
    ysid::stop_server(m_httpd_ctx);
    m_httpd_ctx = 0;
    ui.actionHttpd->setText(tr("Start &Httpd"));
    options_set_bool("httpd/start", false);
  } else {
    auto www = options_get_str("httpd/www");
    auto port = options_get_str("httpd/port");
    m_httpd_ctx = ysid::start_server(m_db_model->db(), www, port, 0);
    if (m_httpd_ctx == 0) error(tr("start httpd failed"));
    else ui.actionHttpd->setText(tr("Stop &Httpd"));
    options_set_bool("httpd/start", true);
  }
}

void YsidMain::closeEvent(QCloseEvent *event) {
  hide();
  event->ignore();
}

void YsidMain::trayicon_activated(QSystemTrayIcon::ActivationReason reason) {
  if (isHidden()) {
    show();
    activateWindow();
  } else {
    hide();
  }
}

void YsidMain::login() {
  m_logdlg->show();
  auto r = m_logdlg->exec();
  if (r == QDialog::Accepted) {
    // QTimer::singleShot(0, this, SLOT(do_login()));
    QtConcurrent::run(this, &YsidMain::do_login);
  } else m_logdlg->set_password(QString(""));
}

void YsidMain::do_login() {
  if (m_db_model->login(
          m_logdlg->get_path(), m_logdlg->get_dpri(), m_logdlg->get_dpub(), m_logdlg->get_password(),
          m_logdlg->get_spri(), m_logdlg->get_spub(), m_logdlg->get_truncate(), m_logdlg->get_merge())) {
    options_set_qstr("login/path", m_logdlg->get_path());
    options_set_qstr("login/dpri", m_logdlg->get_dpri());
    options_set_qstr("login/dpub", m_logdlg->get_dpub());
    options_set_qstr("login/spri", m_logdlg->get_spri());
    options_set_qstr("login/spub", m_logdlg->get_spub());
    if (options_get_bool("remember_password", false)) {
      int r = QMessageBox::question(
          this, "Remember the master password?",
          "You configed to remember password, But it's not safe, and are you sure to mean that?",
          QMessageBox::Yes, QMessageBox::No);
      if (r == QMessageBox::Yes) {
        options_set_qstr("login/password", m_logdlg->get_password());
      }
    }
  }
  m_logdlg->set_password(QString(""));
}

void YsidMain::logout() {
  m_db_model->logout();
}

void YsidMain::add() {
  edit("");
}

void YsidMain::edit_selected() {
  auto cur = m_db_view->currentIndex();
  if (!cur.isValid()) return;
  size_t x = cur.row();
  ysid::item n = m_db_model->get_item(x);
  QString s(n.uuid().c_str());
  edit(s);
}

void YsidMain::erase_selected() {
  auto cur = m_db_view->currentIndex();
  if (!cur.isValid()) return;
  size_t x = cur.row();
  ysid::item n = m_db_model->get_item(x);
  ysid::string m = "Are you sure to delete ";
  m += n.username() + "@" + n.url() + " " + n.tags();
  m += "<br />\n" + n.notes();
  int r = QMessageBox::question(
      this, tr("Are you sure to delete?"), tr(m.c_str()),
      QMessageBox::Yes, QMessageBox::No);
  if (r == QMessageBox::Yes) {
    m_db_model->erase(n.uuid());
    info("erased");
  }
}

void YsidMain::copy_password() {
  auto cur = m_db_view->currentIndex();
  if (!cur.isValid()) return;
  size_t x = cur.row();
  ysid::item n = m_db_model->get_item(x);
  auto clip = QApplication::clipboard();
  auto pas = m_db_model->get_password(n.uuid());
  clip->setText(tr(pas.c_str()));
  info("copied");
}

void YsidMain::autotype() {
  auto cur = m_db_view->currentIndex();
  if (!cur.isValid()) {
    const char *q = "No_User\tNo_item";
    autotype_str(q);
    return;
  }
  size_t x = cur.row();
  ysid::item n = m_db_model->get_item(x);
  auto s = n.username() + '\t';
  s += m_db_model->get_password(n.uuid());
  autotype_str(s.c_str());
}

void YsidMain::autotype_username() {
  auto cur = m_db_view->currentIndex();
  if (!cur.isValid()) return;
  size_t x = cur.row();
  ysid::item n = m_db_model->get_item(x);
  autotype_str(n.username().c_str());
}

void YsidMain::autotype_url() {
  auto cur = m_db_view->currentIndex();
  if (!cur.isValid()) return;
  size_t x = cur.row();
  ysid::item n = m_db_model->get_item(x);
  autotype_str(n.url().c_str());
}

void YsidMain::autotype_password() {
  auto cur = m_db_view->currentIndex();
  if (!cur.isValid()) return;
  size_t x = cur.row();
  ysid::item n = m_db_model->get_item(x);
  autotype_str(m_db_model->get_password(n.uuid()).c_str());
}

void YsidMain::autotype_str(const char *s) {
  hide();
  usleep(500000);
  type_str(s);
  show();
  activateWindow();
  QTimer::singleShot(200, this, SLOT(hide()));
}

void YsidMain::type_str(const char *s) {
  xdo_t *x = xdo_new_with_opened_display(QX11Info::display(), NULL, 0);
  xdo_type(x, CURRENTWINDOW, (char*)s, options_get_int("autotype/delay", 12000));
  xdo_free(x);
}

void YsidMain::edit(const QString &uuid) {
  EditDialog dlg(this);
  dlg.set_password("");
  ysid::item n;
  if (uuid.size()) {
    n.uuid(qstring_to_utf8(uuid));
    n = ysid::item(m_db_model->get_item(n.uuid()));
    dlg.set_username(QString::fromUtf8(n.username().c_str()));
    dlg.set_url(QString::fromUtf8(n.url().c_str()));
    dlg.set_tags(QString::fromUtf8(n.tags().c_str()));
    dlg.set_notes(QString::fromUtf8(n.notes().c_str()));
  }
  dlg.show();
  auto r = dlg.exec();
  if (r != QDialog::Accepted) return;
  n.username(qstring_to_utf8(dlg.get_username()));
  n.url(qstring_to_utf8(dlg.get_url()));
  n.tags(qstring_to_utf8(dlg.get_tags()));
  n.notes(qstring_to_utf8(dlg.get_notes()));
  n.password(qstring_to_utf8(dlg.get_password()));
  m_db_model->add(n);
  m_db_model->filter_items(m_list->text());
  info("edited");
}

void YsidMain::error(const QString &mesg) {
  QMessageBox::warning(this, tr("ysid error"), mesg);
}

void YsidMain::list_info(const QString &mesg) {
  m_list_info->setText(mesg);
}

void YsidMain::info(const QString &mesg) {
  m_info->setText(mesg);
}

void YsidMain::quit() {
  exit(0);
}

void YsidMain::about() {
  QMessageBox::about(
      this,
      tr("Ysid"),
      tr("<h2>Ysid Yun's security information dictionary</h2>"
         "<p>Copyright &copy;  2010 Changsheng Jiang(jiangzuoyan@gmail.com)</p>"
         "<p>Ysid save/query passphrases<p>"
         "<p>Ysid is wrote by Changsheng Jiang(jiangzuoyan@gmail.com) for Ariel Wang."
         "It's free, using it at your own risk.<p>"
         ));
}

void YsidMain::list_next() {
  auto cur = m_db_view->currentIndex();
  int x = 0;
  if (cur.isValid()) x = cur.row() + 1;
  if (x >= m_db_model->rowCount(QModelIndex())) x = 0;
  m_db_view->selectRow(x);
  m_db_view->setFocus();
}

void YsidMain::keyPressEvent(QKeyEvent * e) {
  if (e->text() == "j" || e->text() == "k") {
    auto cur = m_db_view->currentIndex();
    int x = 0;
    if (cur.isValid()) x = cur.row();
    if (e->text() == "j") x = x + 1;
    else x = x - 1;
    m_db_view->selectRow(x);
  } else if (e->text() == "/") {
    m_list->setFocus();
  } else {
    QMainWindow::keyPressEvent(e);
  }
}

void YsidMain::hot_activate() {
  show();
  activateWindow();
}

void YsidMain::options() {
  OptionsDialog dlg(this);
  dlg.exec();
}

