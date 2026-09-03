#pragma once

// Observer pattern via native Qt signals/slots: DataSource emits events,
// any number of independent Observer instances can subscribe without
// DataSource knowing they exist.
#include <QDebug>
#include <QObject>
#include <QString>
#include <QTimer>

class DataSource : public QObject {
    Q_OBJECT
private:
    QTimer m_timer;
    int m_counter = 0;

public:
    explicit DataSource(QObject* parent = nullptr) : QObject(parent) {
        connect(&m_timer, &QTimer::timeout, this, [this]() {
            m_counter++;
            emit dataChanged(m_counter);
            if (m_counter % 5 == 0) {
                emit milestone(m_counter);
            }
        });
        m_timer.start(1000);
    }

signals:
    void dataChanged(int value);
    void milestone(int value);
};

class Observer : public QObject {
    Q_OBJECT
private:
    QString m_name;

public:
    explicit Observer(const QString& name, QObject* parent = nullptr) : QObject(parent), m_name(name) {}

public slots:
    void onDataChanged(int value) { qDebug() << m_name << "received data:" << value; }

    void onMilestone(int value) { qDebug() << m_name << "milestone reached:" << value; }
};

// Usage:
// auto source = new DataSource;
// auto observer1 = new Observer("Observer1");
// auto observer2 = new Observer("Observer2");
//
// connect(source, &DataSource::dataChanged, observer1, &Observer::onDataChanged);
// connect(source, &DataSource::milestone, observer2, &Observer::onMilestone);
// See examples/console_demo.cpp for a runnable version of this.
