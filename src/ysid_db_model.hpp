/**
 * @file   ysid_db_model.hpp
 *
 * @brief ysid db model
 *
 *
 */
#ifndef FILE_ac691340_bb53_438d_9850_d9dfd64fcf5f_H
#define FILE_ac691340_bb53_438d_9850_d9dfd64fcf5f_H 1
#include "ysid_db.hpp"
#include <unordered_map>
#include "item.hpp"
#include "kvstore.hpp"
#include "mt_random.hpp"
#include "fmtstr.hpp"
#include <QtGui>

class YsidDbModel : public QAbstractTableModel {
  Q_OBJECT;
 public:
  YsidDbModel(QObject *parent=0);
  virtual ~YsidDbModel();
  ysid::ysid_db* db();

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  ysid::item get_item(ysid::string uuid) const;
  ysid::item get_item(size_t x) const;

  ysid::string get_password(ysid::string uuid) const;

 signals:
  void error(const QString &s) const;
  void info(const QString &m) const;

 public slots:
  void sync_items();
  void filter_items(const QString &qs);
  bool login(const QString &path, const QString &dpri, const QString &dpub, const QString &password,
             const QString &spri, const QString &spub, const QString &truncate, const QString &merge);
  void logout();
  void add(const ysid::item &n);
  void erase(const ysid::string &uuid);

 private:
  ysid::ysid_db *m_db;
  std::vector<ysid::string> m_items;
  std::unordered_map<ysid::string, ysid::item> m_item_maps;
};

#endif
