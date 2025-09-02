#include "client_server.h"


#include <QThreadPool>


namespace client_server
{
    //
    MyServer::MyServer(QObject *parent)
        : QTcpServer(parent)
    { }

    void MyServer::StartServer()
    {
        if(listen(QHostAddress::Any,1234))
        {
            qDebug()<<"started";
        }
        else
        {
            qDebug()<<"not strated";
        }
    }
    /*void MyServer::incommingConnection(int handle)
    {
        MyClient *client=new MyClient(this);
        client->SetSocket(handle);
    }*/

    //
    MyClient::MyClient(QObject *parent)
        : QObject(parent)
    {
        QThreadPool::globalInstance()->setMaxThreadCount(15);
    }

    void MyClient::SetSocket(int Descriptor)
    {
        socket = new QTcpSocket(this);

        connect(socket,SIGNAL(connected()), this,SLOT(connected()));
        connect(socket,SIGNAL(disconnected()), this,SLOT(disconnected()));
        connect(socket,SIGNAL(readyRead()), this,SLOT(readyRead()));
        connect(socket,SIGNAL(Result()), this,SLOT(TaskResult(int)), Qt::QueuedConnection);

       socket->setSocketDescriptor(Descriptor);
       qDebug() << "client connected";
    }

    void MyClient::connected()
    {
        qDebug() << "client connected event";
    }

    void MyClient::disconnected()
    {
        qDebug() << "client disconnected event";
    }

    void MyClient::readyRead()
    {
        qDebug() << socket->readAll();

        //time ocnsumer - deadlock

        MyTask *mytask=new MyTask();
        mytask->setAutoDelete(true);
        //QObject::connect(mytask,SIGNAL(Result(int)), this,SLOT(TaskResult(int)));
        QThreadPool::globalInstance()->start( mytask );
    }

    void MyClient::TaskResult(int Number)
    {
        //rh
        QByteArray Buffer;
        Buffer.append("\r\nTask Result = ");
        Buffer.append(QString::number(Number));

        socket->write(Buffer);
    }

    //
    void MyTask::run()
    { //time consumer
        qDebug()<<"Task Start";
        int iNumber=0; for(int i=0;i<100;i++)
        {
            iNumber+=i;
        }
        qDebug()<<"Task Done";

        //emit Result(iNumber);
    }

    //
    void test()
    {
        MyServer server( nullptr );
        server.StartServer();
    };

}
