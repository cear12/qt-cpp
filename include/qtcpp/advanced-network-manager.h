#pragma once

// A QNetworkAccessManager wrapper with disk caching, a minimum TLS version
// policy, a per-request timeout, and centralized SSL-error handling.
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSslConfiguration>
#include <QSslError>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>

class AdvancedNetworkManager : public QObject {
    Q_OBJECT

public:
    explicit AdvancedNetworkManager(QObject* parent = nullptr) : QObject(parent) {
        m_manager = new QNetworkAccessManager(this);
        m_cache = new QNetworkDiskCache(this);
        m_cache->setCacheDirectory("./cache");
        m_manager->setCache(m_cache);

        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        QSslConfiguration::setDefaultConfiguration(sslConfig);

        connect(m_manager, &QNetworkAccessManager::finished, this, &AdvancedNetworkManager::requestFinished);
        connect(m_manager, &QNetworkAccessManager::sslErrors, this, &AdvancedNetworkManager::handleSslErrors);
    }

public slots:
    void makeRequest(const QString& url, const QVariantMap& headers = {}) {
        QNetworkRequest request((QUrl(url)));

        for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
            request.setRawHeader(it.key().toUtf8(), it.value().toByteArray());
        }

        request.setHeader(QNetworkRequest::UserAgentHeader, "AdvancedApp/1.0");

        QNetworkReply* reply = m_manager->get(request);
        m_activeRequests.insert(reply, QDateTime::currentDateTime());

        // Give any single request 30s before we give up on it.
        QTimer::singleShot(30000, reply, [reply]() {
            if (reply->isRunning()) {
                reply->abort();
            }
        });
    }

signals:
    void requestCompleted(const QString& url, const QByteArray& data);
    void requestFailed(const QString& url, const QString& error);

private slots:
    void requestFinished(QNetworkReply* reply) {
        reply->deleteLater();
        m_activeRequests.remove(reply);

        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(reply->url().toString(), reply->errorString());
            return;
        }

        QByteArray data = reply->readAll();
        emit requestCompleted(reply->url().toString(), data);
    }

    void handleSslErrors(QNetworkReply* reply, const QList<QSslError>& errors) {
        for (const QSslError& error : errors) {
            qWarning() << "SSL Error:" << error.errorString();
        }

        // NOTE: ignoring SSL errors unconditionally is convenient for a demo
        // against self-signed/test endpoints but is NOT safe for production
        // use -- a real app must inspect `errors` and only ignore the
        // specific, expected ones (e.g. a pinned self-signed cert), never
        // blanket-ignore everything.
        reply->ignoreSslErrors();
    }

private:
    QNetworkAccessManager* m_manager;
    QNetworkDiskCache* m_cache;
    QHash<QNetworkReply*, QDateTime> m_activeRequests;
};
