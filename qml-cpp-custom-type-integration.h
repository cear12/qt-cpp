class BackEnd : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(QStringList dataList READ dataList NOTIFY dataListChanged)
    
public:
    explicit BackEnd(QObject *parent = nullptr);
    
    Q_INVOKABLE void processData(const QString &input);
    Q_INVOKABLE QVariant getData(int index) const;
    
    QString userName() const { return m_userName; }
    void setUserName(const QString &userName);
    
    QStringList dataList() const { return m_dataList; }
    
signals:
    void userNameChanged();
    void dataListChanged();
    void processingFinished(const QString &result);
    
private:
    QString m_userName;
    QStringList m_dataList;
};

// Регистрация в main():
qmlRegisterType<BackEnd>("com.mycompany.backend", 1, 0, "BackEnd");
