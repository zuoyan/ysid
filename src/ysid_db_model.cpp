/**
 * @file   ysid_db_model.cpp
 *
 * @brief ysid db model implementation
 *
 *
 */
#include "ysid_db_model.hpp"
#include <algorithm>

struct scope_session {
  scope_session(ysid::ysid_db *db)
      : db(db) {
    db->load_session_private();
  }

  ~scope_session() {
    db->clear_session_private();
  }

  ysid::ysid_db *db;
};

YsidDbModel::YsidDbModel(QObject *parent) : QAbstractTableModel(parent) {
  m_db = new ysid::ysid_db();
}

YsidDbModel::~YsidDbModel() {
  if (m_db) delete m_db;
}

ysid::ysid_db* YsidDbModel::db() {
  return m_db;
}

int YsidDbModel::rowCount(const QModelIndex &parent) const {
  return m_items.size();
}

int YsidDbModel::columnCount(const QModelIndex &parent) const {
  return 4;
}

QVariant YsidDbModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();
  if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  if (role == Qt::DisplayRole) {
    int x = index.row(), y = index.column();
    QString s;
    ysid::string uuid = m_items[x];
    ysid::item n = get_item(uuid);
    if (y == 0) { // username
      s = s.fromUtf8(n.username().c_str());
    }
    if (y == 1) { // url
      s = s.fromUtf8(n.url().c_str());
    }
    if (y == 2) { // tags
      s = s.fromUtf8(n.tags().c_str());
    }
    if (y == 3) { // notes
      s = s.fromUtf8(n.notes().c_str());
    }
    return s;
  }
  return QVariant();
}

QVariant YsidDbModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole) {
      if (section == 0) return QString("username");
      if (section == 1) return QString("url");
      if (section == 2) return QString("tags");
      if (section == 3) return QString("notes");
    }
  }
  return QVariant();
}

void YsidDbModel::sync_items() {
  try {
    scope_session session(m_db);
    m_item_maps.clear();
    emit info(tr("loading items"));
    m_db->db()->fold([&](const ysid::string k, const ysid::string v, void *ignore) {
        ysid::item n(v);
        n.password("");
        m_item_maps[n.uuid()] = n;
      }, 0);
    emit info(QString("loaded %1 items").arg(m_item_maps.size()));
    filter_items("");
  } catch (std::exception &e) {
    emit error(tr(e.what()));
  }
}

void YsidDbModel::filter_items(const QString &qs) {
  auto qb = qs.toUtf8();
  ysid::string s(qb.begin(), qb.end());
  std::vector<ysid::item_filter> filters;
  size_t p = 0;
  while (p < s.size()) {
    auto n = s.find(' ', p);
    if (n == ysid::string::npos) n = s.size();
    if (n != p) filters.push_back(ysid::item_filter(s.substr(p, n - p)));
    p = n + 1;
  }
  m_items.resize(0);
  for (auto it = m_item_maps.begin(); it != m_item_maps.end(); ++it) {
    bool f = true;
    for (size_t i = 0; i < filters.size() && f; i++) {
      f = filters[i].match(it->second);
    }
    if (f) m_items.push_back(it->first);
  }
  std::sort(
      m_items.begin(), m_items.end(),
      [&](const ysid::string &a, const ysid::string &b) -> bool {
        ysid::string av, bv;
        av = m_item_maps[a].url();
        bv = m_item_maps[b].url();
        return av < bv;
      });
  emit info(QString("# %1/%2").arg(m_items.size()).arg(m_item_maps.size()));
  reset();
}

ysid::item YsidDbModel::get_item(ysid::string uuid) const {
  auto it = m_item_maps.find(uuid);
  if (it != m_item_maps.end()) return it->second;
  return ysid::item();
}

ysid::item YsidDbModel::get_item(size_t x) const {
  if (x < m_items.size()) {
    return get_item(m_items[x]);
  }
  return ysid::item();
}

ysid::string YsidDbModel::get_password(ysid::string uuid) const {
  try {
    scope_session session(m_db);
    if (m_db->db()->has(uuid)) {
      return ysid::item(m_db->db()->get(uuid)).password();
    }
    auto msg = ysid::string("get_password: item uuid=") + uuid + " not exists";
    emit error(tr(msg.c_str()));
  } catch (std::exception &e) {
    emit error(tr(e.what()));
  }
  return ysid::string();
}

bool YsidDbModel::login(const QString &path, const QString &dpri, const QString &dpub, const QString &password,
                        const QString &spri, const QString &spub, const QString &truncate, const QString &merge) {
  try {
    if (truncate.size()) m_db->set_truncate_log(ysid::to_int(truncate.toStdString()));
    else m_db->set_truncate_log(0);
    m_db->set_path(path.toStdString());
    m_db->set_dpri_key(dpri.toStdString());
    m_db->set_dpub_key(dpub.toStdString());
    m_db->set_password(password.toStdString());
    m_db->set_spri_key(spri.toStdString());
    m_db->set_spub_key(spub.toStdString());
    if (ysid::to_bool(merge.toStdString())) m_db->db()->merge();
    sync_items();
    return true;
  } catch (std::exception &e) {
    emit error(tr(e.what()));
    return false;
  }
}

void YsidDbModel::logout() {
  try {
    m_db->clear();
    m_items.resize(0);
    m_item_maps.clear();
    reset();
  } catch (std::exception &e) {
    emit error(tr(e.what()));
  }
}

void YsidDbModel::add(const ysid::item &n) {
  try {
    scope_session session(m_db);
    ysid::item c = n;
    if (!n.password().size() && m_db->db()->has(n.uuid())) {
      ysid::item o(m_db->db()->get(n.uuid()));
      c.password(o.password());
    }
    if (!c.password().size()) {
      c.generate_password("p 20");
    }
    m_db->db()->put(c.uuid(), c.to_string());
    m_db->db()->sync();
    c.password("");
    m_item_maps[c.uuid()] = c;
    reset();
  } catch (std::exception &e) {
    emit error(tr(e.what()));
  }
}

void YsidDbModel::erase(const ysid::string &uuid) {
  try {
    scope_session session(m_db);
    m_db->db()->erase(uuid);
    m_db->db()->sync();
    m_item_maps.erase(uuid);
    filter_items("");
  } catch (std::exception &e) {
    emit error(tr(e.what()));
  }
}

