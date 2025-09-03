#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <memory>

// Abstract Service Interface
class IDataService {
public:
    virtual ~IDataService() = default;
    virtual QFuture<QJsonObject> fetchData(const QString& id) = 0;
    virtual QFuture<bool> saveData(const QString& id, const QJsonObject& data) = 0;
};

// Real Implementation
class NetworkDataService : public QObject, public IDataService {
    Q_OBJECT
    std::unique_ptr<QNetworkAccessManager> m_manager;
    
public:
    explicit NetworkDataService(QObject* parent = nullptr) 
        : QObject(parent), m_manager(std::make_unique<QNetworkAccessManager>(this)) {}
    
    QFuture<QJsonObject> fetchData(const QString& id) override {
        auto promise = QPromise<QJsonObject>();
        auto future = promise.future();
        
        auto url = QUrl(QString("https://api.example.com/data/%1").arg(id));
        auto reply = m_manager->get(QNetworkRequest(url));
        
        connect(reply, &QNetworkReply::finished, [promise = std::move(promise), reply]() mutable {
            auto data = reply->readAll();
            auto doc = QJsonDocument::fromJson(data);
            promise.addResult(doc.object());
            promise.finish();
            reply->deleteLater();
        });
        
        return future;
    }
    
    QFuture<bool> saveData(const QString& id, const QJsonObject& data) override {
        // Implementation for saving data
        auto promise = QPromise<bool>();
        auto future = promise.future();
        
        // Async save logic here
        QtConcurrent::run([promise = std::move(promise)]() mutable {
            QThread::msleep(1000); // Simulate network delay
            promise.addResult(true);
            promise.finish();
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
    explicit CachedDataService(std::unique_ptr<IDataService> service, QObject* parent = nullptr)
        : QObject(parent), m_realService(std::move(service)) {
        
        m_cacheCleanup.setInterval(std::chrono::minutes(5));
        connect(&m_cacheCleanup, &QTimer::timeout, [this]() {
            m_cache.clear();
            qDebug() << "Cache cleared";
        });
        m_cacheCleanup.start();
    }
    
    QFuture<QJsonObject> fetchData(const QString& id) override {
        if (m_cache.contains(id)) {
            qDebug() << "Cache hit for:" << id;
            auto promise = QPromise<QJsonObject>();
            promise.addResult(m_cache[id]);
            promise.finish();
            return promise.future();
        }
        
        auto future = m_realService->fetchData(id);
        auto watcher = new QFutureWatcher<QJsonObject>(this);
        
        connect(watcher, &QFutureWatcher<QJsonObject>::finished, [this, watcher, id]() {
            m_cache[id] = watcher->result();
            qDebug() << "Cached result for:" << id;
            watcher->deleteLater();
        });
        
        watcher->setFuture(future);
        return future;
    }
    
    QFuture<bool> saveData(const QString& id, const QJsonObject& data) override {
        m_cache.remove(id); // Invalidate cache
        return m_realService->saveData(id, data);
    }
};
