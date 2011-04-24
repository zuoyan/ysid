/**
 * @file   optionsdialog.hpp
 *
 * @brief options dialog
 *
 *
 */
#ifndef FILE_334cd3af_9ce3_4990_b842_444081dfbd61_H
#define FILE_334cd3af_9ce3_4990_b842_444081dfbd61_H 1
#include "ui_optionsdialog.h"

class OptionsDialog : public QDialog {
  Q_OBJECT;
 public:
  OptionsDialog(QWidget *parent=0);

  virtual void accept();

 private:
  Ui::OptionsDialog ui;
};

#endif
