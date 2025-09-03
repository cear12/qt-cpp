class AdvancedNetworkManager : public QObject {
    Q_OBJECT
    
public:
    explicit AdvancedNetworkManager(QObject *parent = nullptr)
        : QObject(parent) {
        
        m_manager = new QNetworkAccessManager(this);
        m_cache = new QNetworkDiskCache(this);
        m_cache->setCacheDirectory("./cache");
        m_manager->setCache(m_cache);
        
        // Настраиваем SSL
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        QSslConfiguration::setDefaultConfiguration(sslConfig);
        
        connect(m_manager, &QNetworkAccessManager::finished,
                this, &AdvancedNetworkManager::requestFinished);
        connect(m_manager, &QNetworkAccessManager::sslErrors,
                this, &AdvancedNetworkManager::handleSslErrors);
    }
    
public slots:
    void makeRequest(const QString &url, const QVariantMap &headers = {}) {
        QNetworkRequest request(QUrl(url));
        
        // Устанавливаем заголовки
        for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
            request.setRawHeader(it.key().toUtf8(), it.value().toByteArray());
        }
        
        // Устанавливаем User-Agent
        request.setHeader(QNetworkRequest::UserAgentHeader, "AdvancedApp/1.0");
        
        // Делаем запрос
        QNetworkReply *reply = m_manager->get(request);
        m_activeRequests.insert(reply, QDateTime::currentDateTime());
        
        // Таймаут
        QTimer::singleShot(30000, reply, [reply]() {
            if (reply->isRunning()) {
                reply->abort();
            }
        });
    }
    
signals:
    void requestCompleted(const QString &url, const QByteArray &data);
    void requestFailed(const QString &url, const QString &error);
    
private slots:
    void requestFinished(QNetworkReply *reply) {
        reply->deleteLater();
        m_activeRequests.remove(reply);
        
        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(reply->url().toString(), reply->errorString());
            return;
        }
        
        QByteArray data = reply->readAll();
        emit requestCompleted(reply->url().toString(), data);
    }
    
    void handleSslErrors(QNetworkReply *reply, const QList<QSslError> &errors) {
        // Логируем SSL ошибки
        for (const QSslError &error : errors) {
            qWarning() << "SSL Error:" << error.errorString();
        }
        
        // В продакшене здесь должна быть более строгая проверка
        reply->ignoreSslErrors();
    }
    
private:
    QNetworkAccessManager *m_manager;
    QNetworkDiskCache *m_cache;
    QHash<QNetworkReply*, QDateTime> m_activeRequests;
};
