#include <QObject>
#include <QFuture>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QPromise>
#include <QThread>
#include <chrono>

class AsyncProcessor : public QObject {
    Q_OBJECT
    
public:
    // Async operation с QFuture
    QFuture<QString> processDataAsync(const QByteArray& data) {
        return QtConcurrent::run([data]() -> QString {
            // Имитация длительной операции
            QThread::msleep(2000);
            return QString("Processed: %1 bytes").arg(data.size());
        });
    }
    
    // Promise-based API
    QFuture<QStringList> processMultipleAsync(const QList<QByteArray>& dataList) {
        auto promise = QPromise<QStringList>();
        auto future = promise.future();
        
        QtConcurrent::run([promise = std::move(promise), dataList]() mutable {
            QStringList results;
            int total = dataList.size();
            
            for (int i = 0; i < total; ++i) {
                if (promise.isCanceled()) {
                    return;
                }
                
                // Progress reporting
                promise.setProgressValue(i);
                promise.setProgressRange(0, total);
                
                results << QString("Item %1: %2 bytes").arg(i).arg(dataList[i].size());
                QThread::msleep(500);
            }
            
            promise.addResult(results);
            promise.finish();
        });
        
        return future;
    }
    
    // Watcher pattern для мониторинга
    void watchFuture(const QFuture<QString>& future) {
        auto watcher = new QFutureWatcher<QString>(this);
        
        connect(watcher, &QFutureWatcher<QString>::finished, [this, watcher]() {
            emit operationCompleted(watcher->result());
            watcher->deleteLater();
        });
        
        connect(watcher, &QFutureWatcher<QString>::progressValueChanged, 
                this, &AsyncProcessor::progressChanged);
                
        watcher->setFuture(future);
    }
    
signals:
    void operationCompleted(const QString& result);
    void progressChanged(int value);
};
