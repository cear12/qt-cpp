#pragma once

// Uses Qt's runtime meta-object system (Q_PROPERTY / QMetaObject) together
// with compile-time C++ metaprogramming (if constexpr, SFINAE via
// std::enable_if_t, static_assert) to build a small type-safe(ish) property
// accessor and a signal-connect helper over an arbitrary QObject subclass.
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QRect>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <type_traits>

template <typename T>
class PropertyWrapper {
  static_assert(std::is_base_of_v<QObject, T>, "T must inherit from QObject");

  T* m_object;

 public:
  explicit PropertyWrapper(T* obj) : m_object(obj) {}

  template <typename U>
  bool setProperty(const QString& name, const U& value) {
    static_assert(std::is_convertible_v<U, QVariant>,
                  "Value must be convertible to QVariant");
    return m_object->setProperty(name.toLocal8Bit().constData(),
                                 QVariant::fromValue(value));
  }

  template <typename U>
  U property(const QString& name) const {
    QVariant variant = m_object->property(name.toLocal8Bit().constData());
    if constexpr (std::is_same_v<U, QString>) {
      return variant.toString();
    } else if constexpr (std::is_integral_v<U>) {
      return static_cast<U>(variant.toInt());
    } else {
      return variant.value<U>();
    }
  }

  // SFINAE-gated: only enabled for a zero-argument-invocable receiver
  // (e.g. a lambda with no parameters). Relies on Qt's signal/slot
  // argument-count reduction -- connecting a signal that carries
  // arguments (e.g. T::dataChanged(int)) to a receiver that ignores them
  // is valid and common in Qt.
  template <typename Func>
  auto connectToSignal(Func&& func)
      -> std::enable_if_t<std::is_invocable_v<Func>, bool> {
    return QObject::connect(m_object, &T::dataChanged, m_object,
                            std::forward<Func>(func));
  }
};

// Specialization for plain QWidget: no dataChanged signal to hook into, so
// this offers direct widget property access instead.
template <>
class PropertyWrapper<QWidget> {
  QWidget* m_widget;

 public:
  explicit PropertyWrapper(QWidget* widget) : m_widget(widget) {}

  void setVisible(bool visible) { m_widget->setVisible(visible); }
  void setGeometry(const QRect& rect) { m_widget->setGeometry(rect); }
};
