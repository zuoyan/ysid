/**
 * @file   GlobalHot.cpp
 *
 * @brief global hot impl
 *
 *
 */
#include "GlobalHot.hpp"
#include <QtGui>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <iostream>
#include "ysid_qt.hpp"

class GlobalHotImpl {
 protected:
  GlobalHotImpl();

 public:
  ~GlobalHotImpl();
  static GlobalHotImpl* instance();
  static bool eventFilter(void *message);

  bool register_hot(const QKeySequence &keys, GlobalHot *hot);
  bool unregister_hot(const QKeySequence &keys);
  int activate(quint32 keycode, quint32 mods);

 protected:
  bool register_hot(quint32 native_key, quint32 native_mods);
  bool unregister_hot(quint32 native_key, quint32 native_mods);

 private:
  QHash<QPair<quint32, quint32>, GlobalHot*> m_shortcuts;
};

GlobalHot::GlobalHot(const QKeySequence &keys, QObject *parent) : QObject(parent) {
  GlobalHotImpl::instance()->register_hot(keys, this);
  m_enable = true;
}

GlobalHot::~GlobalHot() {

}

GlobalHot::event_filter_func GlobalHot::get_event_filter() {
  return &GlobalHotImpl::eventFilter;
}

bool GlobalHot::enable() {
  return m_enable;
}

void GlobalHot::setEnable(bool v) {
  m_enable = v;
}

void GlobalHot::activate() {
  emit activated();
}

static int hot_error;

static int (*orig_x_err_handler)(Display* display, XErrorEvent* event);

static int hot_x_err_handler(Display* display, XErrorEvent *event) {
  Q_UNUSED(display);
  switch (event->error_code)
  {
    case BadAccess:
    case BadValue:
    case BadWindow:
      if (event->request_code == 33 /* X_GrabKey */ ||
          event->request_code == 34 /* X_UngrabKey */)
      {
        hot_error = true;
      }
    default:
      return 0;
  }
}

static quint32 native_modifiers(Qt::KeyboardModifiers modifiers) {
  // ShiftMask, LockMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, and Mod5Mask
  quint32 native = 0;
  if (modifiers & Qt::ShiftModifier) native |= ShiftMask;
  if (modifiers & Qt::ControlModifier) native |= ControlMask;
  if (modifiers & Qt::AltModifier) native |= Mod1Mask;
  if (modifiers & Qt::MetaModifier) native |= Mod4Mask;
  // TODO: resolve these?
  //if (modifiers & Qt::KeypadModifier)
  //if (modifiers & Qt::GroupSwitchModifier)
  return native;
}

static quint32 native_keycode(Qt::Key key) {
  auto qs = QKeySequence(key).toString(QKeySequence::NativeText);
  auto latin = qstring_to_utf8(qs);
  auto keysym = XStringToKeysym(latin.c_str());
  return XKeysymToKeycode(QX11Info::display(), keysym);
}

GlobalHotImpl::GlobalHotImpl() {}

GlobalHotImpl::~GlobalHotImpl() {
  // TODO: register all global hot keys
}

GlobalHotImpl* GlobalHotImpl::instance() {
  static GlobalHotImpl inst;
  return &inst;
}

bool GlobalHotImpl::eventFilter(void *message) {
  XEvent *event = static_cast<XEvent*>(message);
  if (event->type == KeyPress) {
    XKeyEvent *key = (XKeyEvent*)event;
    instance()->activate(
        key->keycode,
        key->state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask));
  }
  return false;
}

int GlobalHotImpl::activate(quint32 keycode, quint32 mods) {
  GlobalHot *hot = m_shortcuts.value(qMakePair(keycode, mods));
  if (hot && hot->enable()) {
    hot->activate();
  }
  return 0;
}

bool GlobalHotImpl::register_hot(quint32 native_key, quint32 native_mods) {
  Display* display = QX11Info::display();
  Window window = QX11Info::appRootWindow();
  Bool owner = True;
  int pointer = GrabModeAsync;
  int keyboard = GrabModeAsync;
  hot_error = false;
  orig_x_err_handler = XSetErrorHandler(hot_x_err_handler);
  XGrabKey(display, native_key, native_mods, window, owner, pointer, keyboard);
  XGrabKey(display, native_key, native_mods | Mod2Mask, window, owner, pointer, keyboard); // allow numlock
  XSync(display, False);
  XSetErrorHandler(orig_x_err_handler);
  return !hot_error;
}

bool GlobalHotImpl::unregister_hot(quint32 native_key, quint32 native_mods) {
  Display* display = QX11Info::display();
  Window window = QX11Info::appRootWindow();
  hot_error = false;
  orig_x_err_handler = XSetErrorHandler(hot_x_err_handler);
  XUngrabKey(display, native_key, native_mods, window);
  XUngrabKey(display, native_key, native_mods | Mod2Mask, window); // allow numlock
  XSync(display, False);
  XSetErrorHandler(orig_x_err_handler);
  return !hot_error;
}

bool GlobalHotImpl::register_hot(const QKeySequence &keys, GlobalHot *hot) {
  if (keys.isEmpty()) return false;
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
  quint32 code = native_keycode(Qt::Key((keys[0] ^ allMods) & keys[0]));
  quint32 mods = native_modifiers(Qt::KeyboardModifiers(keys[0] & allMods));
  bool r = register_hot(code, mods);
  m_shortcuts.insert(qMakePair(code, mods), hot);
  return r;
}

bool GlobalHotImpl::unregister_hot(const QKeySequence &keys) {
  if (keys.isEmpty()) return false;
  Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
  quint32 code = native_keycode(Qt::Key((keys[0] ^ allMods) & keys[0]));
  quint32 mods = native_modifiers(Qt::KeyboardModifiers(keys[0] & allMods));
  unregister_hot(code, mods);
  m_shortcuts.remove(qMakePair(code, mods));
  return true;
}
