#pragma once

// A QObject exposed to QML: Q_PROPERTY for two-way-bindable state,
// Q_INVOKABLE for methods QML can call directly, and a free function to
// register the type with the QML engine.
//
// NOTE: the original version of this file ended with
//   qmlRegisterType<BackEnd>("com.mycompany.backend", 1, 0, "BackEnd");
// as a bare statement directly in the header -- not legal at namespace
// scope, so the header could not compile if included anywhere. It's now
// the body of registerQmlTypes(), declared here and called once from
// examples/console_demo.cpp before any QQmlEngine is created.
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

class BackEnd : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
  Q_PROPERTY(QStringList dataList READ dataList NOTIFY dataListChanged)

 public:
  explicit BackEnd(QObject* parent = nullptr);

  Q_INVOKABLE void processData(const QString& input);
  Q_INVOKABLE QVariant getData(int index) const;

  QString userName() const { return m_userName; }
  void setUserName(const QString& userName);

  QStringList dataList() const { return m_dataList; }

 signals:
  void userNameChanged();
  void dataListChanged();
  void processingFinished(const QString& result);

 private:
  QString m_userName;
  QStringList m_dataList;
};

// Registers BackEnd as a creatable QML type under the given URI. Call once,
// before constructing any QQmlEngine that needs to `import
// com.mycompany.backend 1.0`.
void registerQmlTypes();
