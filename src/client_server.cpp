#include "qtcpp/client_server.h"

#include <QDebug>
#include <QHostAddress>
#include <QThreadPool>

namespace client_server {

MyServer::MyServer(QObject* parent) : QTcpServer(parent) {}

bool MyServer::StartServer(quint16 port) {
  if (listen(QHostAddress::Any, port)) {
    qDebug() << "server listening on port" << serverPort();
    return true;
  }

  qDebug() << "server failed to start:" << errorString();
  return false;
}

void MyServer::incomingConnection(qintptr socketDescriptor) {
  // Fix: the original file never overrode this, so accepted connections
  // were never turned into MyClient objects at all.
  auto* client = new MyClient(this);
  client->SetSocket(socketDescriptor);
}

MyClient::MyClient(QObject* parent) : QObject(parent) {
  QThreadPool::globalInstance()->setMaxThreadCount(15);
}

void MyClient::SetSocket(qintptr descriptor) {
  socket = new QTcpSocket(this);

  connect(socket, &QTcpSocket::connected, this, &MyClient::onConnected);
  connect(socket, &QTcpSocket::disconnected, this, &MyClient::onDisconnected);
  connect(socket, &QTcpSocket::readyRead, this, &MyClient::onReadyRead);

  socket->setSocketDescriptor(descriptor);
  qDebug() << "client connected, descriptor:" << descriptor;
}

void MyClient::onConnected() { qDebug() << "client connected event"; }

void MyClient::onDisconnected() { qDebug() << "client disconnected event"; }

void MyClient::onReadyRead() {
  qDebug() << "received:" << socket->readAll();

  // Offload the "heavy" work to the thread pool so the event loop (and
  // every other client's I/O) isn't blocked while it runs.
  auto* task = new MyTask();
  task->setAutoDelete(true);

  // Queued: MyTask::run() executes on a QThreadPool worker thread, but
  // onTaskResult() touches `socket`, which must only be touched from the
  // thread MyClient itself lives on. Qt::QueuedConnection posts the
  // delivery back onto that thread instead of calling directly.
  connect(task, &MyTask::Result, this, &MyClient::onTaskResult,
          Qt::QueuedConnection);

  QThreadPool::globalInstance()->start(task);
}

void MyClient::onTaskResult(int number) {
  QByteArray buffer;
  buffer.append("\r\nTask Result = ");
  buffer.append(QByteArray::number(number));

  socket->write(buffer);
}

void MyTask::run() {
  qDebug() << "task start";

  int total = 0;
  for (int i = 0; i < 100; ++i) {
    total += i;
  }

  qDebug() << "task done, result =" << total;
  emit Result(total);
}

}  // namespace client_server
