/**
 * @file   ysiddbview.cpp
 *
 * @brief ysid db view implementation
 *
 *
 */
#include <QtGui>
#include "ysiddbview.hpp"
#include <iostream>

YsidDbView::YsidDbView(QWidget *parent) : QTableView(parent) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  //setWordWrapModel(QTextOption::WrapAnywhere);
  setWordWrap(true);
}

void YsidDbView::keyPressEvent(QKeyEvent * e) {
  auto cur = this->currentIndex();
  int x = 0;
  if (cur.isValid()) x = cur.row();
  if (e->text() == "j") {
    this->selectRow(x + 1);
  } else if (e->text() == "k") {
    this->selectRow(x - 1);
  } else {
    e->ignore();
  }
}



