#pragma once

// Proxy pattern: CachedDataService wraps any IDataService implementation
// (here, a real NetworkDataService) and transparently adds a time-limited
// in-memory cache in front of it, without callers needing to know caching
// is happening at all -- they just hold an IDataService*.
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPromise>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QtConcurrent>
#include <chrono>
#include <memory>

// Abstract Service Interface
class IDataService {
 public:
  virtual ~IDataService() = default;
  virtual QFuture<QJsonObject> fetchData(const QString& id) = 0;
  virtual QFuture<bool> saveData(const QString& id,
                                 const QJsonObject& data) = 0;
};

// Real Implementation
class NetworkDataService : public QObject, public IDataService {
  Q_OBJECT
  std::unique_ptr<QNetworkAccessManager> m_manager;

 public:
  explicit NetworkDataService(QObject* parent = nullptr)
      : QObject(parent),
        m_manager(std::make_unique<QNetworkAccessManager>(this)) {}

  QFuture<QJsonObject> fetchData(const QString& id) override {
    auto promise = std::make_shared<QPromise<QJsonObject>>();
    auto future = promise->future();

    auto url = QUrl(QString("https://api.example.com/data/%1").arg(id));
    auto reply = m_manager->get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [promise, reply]() {
      auto data = reply->readAll();
      auto doc = QJsonDocument::fromJson(data);
      promise->addResult(doc.object());
      promise->finish();
      reply->deleteLater();
    });

    return future;
  }

  QFuture<bool> saveData(const QString& /*id*/,
                         const QJsonObject& /*data*/) override {
    auto promise = std::make_shared<QPromise<bool>>();
    auto future = promise->future();

    // Real save logic would serialize `data` and PUT/POST it to the
    // service for `id`; this demo only simulates the async round trip.
    QtConcurrent::run([promise]() {
      QThread::msleep(1000);  // simulated network latency
      promise->addResult(true);
      promise->finish();
    });

    return future;
  }
};

// Caching Proxy
class CachedDataService : public QObject, public IDataService {
  Q_OBJECT
  std::unique_ptr<IDataService> m_realService;
  QHash<QString, QJsonObject> m_cache;
  QTimer m_cacheCleanup;

 public:
  explicit CachedDataService(std::unique_ptr<IDataService> service,
                             QObject* parent = nullptr)
      : QObject(parent), m_realService(std::move(service)) {
    m_cacheCleanup.setInterval(std::chrono::minutes(5));
    connect(&m_cacheCleanup, &QTimer::timeout, this, [this]() {
      m_cache.clear();
      qDebug() << "Cache cleared";
    });
    m_cacheCleanup.start();
  }

  QFuture<QJsonObject> fetchData(const QString& id) override {
    if (m_cache.contains(id)) {
      qDebug() << "Cache hit for:" << id;
      QPromise<QJsonObject> promise;
      auto future = promise.future();
      promise.addResult(m_cache[id]);
      promise.finish();
      return future;
    }

    auto future = m_realService->fetchData(id);
    auto watcher = new QFutureWatcher<QJsonObject>(this);

    connect(watcher, &QFutureWatcher<QJsonObject>::finished, this,
            [this, watcher, id]() {
              m_cache[id] = watcher->result();
              qDebug() << "Cached result for:" << id;
              watcher->deleteLater();
            });

    watcher->setFuture(future);
    return future;
  }

  QFuture<bool> saveData(const QString& id, const QJsonObject& data) override {
    m_cache.remove(id);  // Invalidate cache
    return m_realService->saveData(id, data);
  }
};
