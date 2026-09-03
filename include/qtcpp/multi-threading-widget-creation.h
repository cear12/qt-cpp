#pragma once

// The one safe way to create a QWidget from a background thread: don't.
// WorkerThread does the heavy lifting off the GUI thread and emits a
// signal; WidgetCreator's slot (which runs on the GUI thread, since it's
// default-constructed there and Qt queues the connection across threads)
// is the only place that actually calls `new QWidget`.
//
// NOTE: the original version of this file had `WidgetCreator creator;
// WorkerThread worker; QObject::connect(...)` as bare statements sitting
// directly in the header -- a statement outside of any function is not
// legal C++ at namespace scope, so the header could not compile if
// included anywhere. That wiring now lives in
// examples/console_demo.cpp::runWidgetCreationDemo(), which is where it
// belongs (and which actually starts the thread and waits for the result).
#include <QObject>
#include <QString>
#include <QThread>
#include <QWidget>

class WidgetCreator : public QObject {
    Q_OBJECT

public slots:
    void createWidgetSafely(const QString& data) {
        // This slot runs on whatever thread `this` lives on -- the GUI
        // thread, as long as WidgetCreator was constructed there and the
        // connection to it is a (default or explicit) queued connection
        // from a different thread.
        QWidget* widget = new QWidget();
        widget->setWindowTitle(data);
        widget->show();
        emit widgetCreated();
    }

signals:
    void widgetCreated();
};

class WorkerThread : public QThread {
    Q_OBJECT

signals:
    void requestWidgetCreation(const QString& data);

protected:
    void run() override {
        QString result = performHeavyCalculation();
        emit requestWidgetCreation(result);
    }

private:
    // Stand-in for whatever the real background computation is.
    QString performHeavyCalculation() { return QStringLiteral("Computed on background thread"); }
};
