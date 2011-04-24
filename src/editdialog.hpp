/**
 * @file   editdialog.hpp
 *
 * @brief edit dialog
 *
 *
 */
#ifndef FILE_c85fcb61_7195_425e_bdae_a956994f8d63_H
#define FILE_c85fcb61_7195_425e_bdae_a956994f8d63_H 1
#include "ui_editdialog.h"

class EditDialog: public QDialog {
  Q_OBJECT;
 public:
  EditDialog(QWidget *parent=0);
  void set_username(const QString &);
  QString get_username();
  void set_url(const QString &);
  QString get_url();
  void set_tags(const QString &);
  QString get_tags();
  void set_notes(const QString &);
  QString get_notes();
  void set_password(const QString &);
  QString get_password();

 private slots:
  void check_username();
  void check_password();

 private:
  Ui::EditDialog ui;
};

#endif
