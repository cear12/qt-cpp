#pragma once

// Three ways to expose an asynchronous operation through QFuture:
// QtConcurrent::run() for a single fire-and-forget result, a manually
// driven QPromise for a multi-step operation that reports progress, and
// QFutureWatcher for turning either into signals a GUI can connect to.
//
// NOTE on QPromise capture: rather than moving a QPromise into a lambda
// capture (its move/copy guarantees have shifted across Qt6 minor
// releases -- see the repository README), results are reported through a
// std::shared_ptr<QPromise<T>> captured by value, sidestepping that
// question. Also fixed here: the original called
// promise.setProgressRange(0, total) on every loop iteration (redundant,
// and -- worse -- after that same iteration's setProgressValue() call, so
// the very first reported value could be clamped against a not-yet-set
// range); the range is now set once, before the loop starts.
#include <QByteArray>
#include <QFuture>
#include <QFutureWatcher>
#include <QList>
#include <QObject>
#include <QPromise>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QtConcurrent>
#include <chrono>
#include <memory>

class AsyncProcessor : public QObject {
  Q_OBJECT

 public:
  // Fire-and-forget async operation.
  QFuture<QString> processDataAsync(const QByteArray& data) {
    return QtConcurrent::run([data]() -> QString {
      QThread::msleep(2000);  // stand-in for real work
      return QString("Processed: %1 bytes").arg(data.size());
    });
  }

  // Multi-item operation with progress reporting and cancellation support.
  QFuture<QStringList> processMultipleAsync(const QList<QByteArray>& dataList) {
    auto promise = std::make_shared<QPromise<QStringList>>();
    auto future = promise->future();

    QtConcurrent::run([promise, dataList]() {
      QStringList results;
      int total = dataList.size();
      promise->setProgressRange(0, total);

      for (int i = 0; i < total; ++i) {
        if (promise->isCanceled()) {
          return;
        }

        promise->setProgressValue(i);
        results << QString("Item %1: %2 bytes").arg(i).arg(dataList[i].size());
        QThread::msleep(500);  // stand-in for real per-item work
      }

      promise->addResult(results);
      promise->finish();
    });

    return future;
  }

  // Bridges a QFuture to signals, for callers that would rather connect a
  // slot than block on or poll the future themselves.
  void watchFuture(const QFuture<QString>& future) {
    auto* watcher = new QFutureWatcher<QString>(this);

    connect(watcher, &QFutureWatcher<QString>::finished, this,
            [this, watcher]() {
              emit operationCompleted(watcher->result());
              watcher->deleteLater();
            });

    connect(watcher, &QFutureWatcher<QString>::progressValueChanged, this,
            &AsyncProcessor::progressChanged);

    watcher->setFuture(future);
  }

 signals:
  void operationCompleted(const QString& result);
  void progressChanged(int value);
};
