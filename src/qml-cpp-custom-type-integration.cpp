#include "qtcpp/qml-cpp-custom-type-integration.h"

#include <QQmlEngine>

BackEnd::BackEnd(QObject* parent) : QObject(parent) {}

void BackEnd::processData(const QString& input) {
    QString result = input.trimmed().toUpper();
    m_dataList.append(result);
    emit dataListChanged();
    emit processingFinished(result);
}

QVariant BackEnd::getData(int index) const {
    if (index < 0 || index >= m_dataList.size()) return QVariant();
    return m_dataList.at(index);
}

void BackEnd::setUserName(const QString& userName) {
    if (m_userName == userName) return;
    m_userName = userName;
    emit userNameChanged();
}

void registerQmlTypes() { qmlRegisterType<BackEnd>("com.mycompany.backend", 1, 0, "BackEnd"); }
