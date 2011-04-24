/**
 * @file   ysiddbview.hpp
 *
 * @brief ysid db view
 *
 *
 */
#ifndef FILE_cc811b2b_ea51_4199_9060_7b0f2ca0c210_H
#define FILE_cc811b2b_ea51_4199_9060_7b0f2ca0c210_H 1
#include <QAbstractItemView>
#include <QTableView>

class QWidget;

class YsidDbView : public QTableView {
  Q_OBJECT;
 public:
  YsidDbView(QWidget *parent=0);
  virtual void keyPressEvent(QKeyEvent * e);

  ~YsidDbView() {}
};

#endif
