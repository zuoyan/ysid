/**
 * @file   ysid_qt.cpp
 *
 * @brief ysid qt version
 *
 *
 */
#include "ysid_qt.hpp"
#include "logindialog.hpp"
#include "editdialog.hpp"
#include "ysidmain.hpp"
#include <QApplication>
#include "ysid_db.hpp"
#include <memory>
#include <QTextCodec>
#include "GlobalHot.hpp"

ysid::string qstring_to_local8bit(const QString &qs) {
  auto qb = qs.toLocal8Bit();
  return ysid::string(qb.begin(), qb.end());
}

ysid::string qstring_to_utf8(const QString &qs) {
  auto qb = qs.toUtf8();
  return ysid::string(qb.begin(), qb.end());
}

ysid::string options_get_str(const ysid::string &key, const ysid::string &dft) {
  QSettings settings;
  return qstring_to_utf8(settings.value(QString(key.c_str()), QString(dft.c_str())).toString());
}

QString options_get_qstr(const ysid::string &key, const QString &dft) {
  QSettings settings;
  return settings.value(QString(key.c_str()), dft).toString();
}

void options_set_str(const ysid::string &key, const ysid::string &value) {
  QSettings settings;
  settings.setValue(QString(key.c_str()), QString(value.c_str()));
}

void options_set_qstr(const ysid::string &key, const QString &value) {
  QSettings settings;
  settings.setValue(QString(key.c_str()), value);
}

bool options_get_bool(const ysid::string &key, bool dft) {
  return ysid::to_bool(options_get_str(key, ysid::to_string(dft)));
}

void options_set_bool(const ysid::string &key, bool value) {
  QSettings settings;
  settings.setValue(QString(key.c_str()), value);
}

int options_get_int(const ysid::string &key, int dft) {
  return ysid::to_int(options_get_str(key, ysid::to_string(dft)));
}

void options_set_int(const ysid::string &key, int value) {
  return options_set_str(key, ysid::to_string(value));
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("Changsheng Jiang Inc.");
  QCoreApplication::setOrganizationDomain("jiangzuoyan.at.gmail.com");
  QCoreApplication::setApplicationName("ysid");
  QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
  GlobalHot *hot = NULL;
  ysid::string hot_key = options_get_str("globalhot");
  if (hot_key.size()) {
    hot = new GlobalHot(QKeySequence(QString(hot_key.c_str())));
    QAbstractEventDispatcher::instance()->setEventFilter(hot->get_event_filter());
  }
  YsidMain win;
  win.show();
  if (hot) win.connect(hot, SIGNAL(activated()), &win, SLOT(hot_activate()));
  int r = app.exec();
  if (hot) delete hot;
  return r;
}
