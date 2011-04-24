/**
 * @file   ysid.cpp
 *
 * @brief ysid main implementation
 *
 *
 */
#include "ysid_config.h"
#include <QApplication>
#include <QLabel>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QLabel *label = new QLabel("hello world");
  label->show();
  return app->exec();
}
