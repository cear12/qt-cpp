class WidgetCreator : public QObject {
    Q_OBJECT
    
public slots:
    void createWidgetSafely(const QString& data) {
        // Этот слот выполняется в главном потоке
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
        // Выполняем тяжелые вычисления
        QString result = performHeavyCalculation();
        // Запрашиваем создание виджета в главном потоке
        emit requestWidgetCreation(result);
    }
};

// В main():
WidgetCreator creator;
WorkerThread worker;
QObject::connect(&worker, &WorkerThread::requestWidgetCreation,
                &creator, &WidgetCreator::createWidgetSafely);
