#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H


#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QThread>
#include <QRunnable>


namespace client_server
{
    //
    class MyServer : public QTcpServer
    {
    public:
        MyServer(QObject *parent);

        void StartServer();
        //void MyServer::incommingConnection(int handle);
    };

    //
    class MyClient : public QObject
    {
    public:
        MyClient(QObject *parent);

        void SetSocket(int Descriptor);
        void connected();
        void disconnected();
        void readyRead();
        void TaskResult(int Number);

    private:
        QTcpSocket* socket;
    };

    //
    class MyTask : public QRunnable
    {
    public:
        void run();

    Q_SIGNALS:
        void Result( int );
    };

    //
    void test();
}


#endif // CLIENT_SERVER_H
