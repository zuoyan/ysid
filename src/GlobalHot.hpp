/**
 * @file   GlobalHot.hpp
 *
 * @brief global hot
 *
 *
 */
#ifndef FILE_0c7a3ab5_a8fb_4f12_94c0_d9eb7f26a517_H
#define FILE_0c7a3ab5_a8fb_4f12_94c0_d9eb7f26a517_H 1

#include <QtCore>
#include <QMainWindow>

class GlobalHot : public QObject {
  Q_OBJECT;
 public:
  GlobalHot(const QKeySequence &keys, QObject *parent=0);
  virtual ~GlobalHot();

  bool enable();
  void setEnable(bool v);
  typedef bool (*event_filter_func)(void *);
  static event_filter_func get_event_filter();

  void activate(void);

 signals:
  void activated(void);

 private:
  bool m_enable;
};

#endif
