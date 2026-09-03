#ifndef QTCPP_CLIENT_SERVER_H
#define QTCPP_CLIENT_SERVER_H

// A tiny TCP server: each accepted connection gets a MyClient, and every
// line of input received is handed off to the global QThreadPool as a
// MyTask, whose result is delivered back to the client asynchronously.
//
// NOTE -- this file had several real bugs in its original version, all
// fixed here (see client_server.cpp for the fixes in context):
//   1. MyServer never overrode incomingConnection(), so accepted
//      connections were never actually turned into MyClient objects --
//      the server would listen() successfully and then do nothing.
//   2. MyClient/MyServer had no Q_OBJECT and used the old string-based
//      SIGNAL()/SLOT() connect() overload for methods that weren't even
//      declared under a `slots:` section -- without moc-generated
//      metadata, those connections would fail at runtime.
//   3. One connect() call wired QTcpSocket's (nonexistent) `Result()`
//      signal to TaskResult(int) -- a copy/paste leftover that could never
//      have compiled correctly once Q_OBJECT was added, since QTcpSocket
//      has no such signal.
//   4. MyTask (a QRunnable) declared a Q_SIGNALS section, but QRunnable is
//      not a QObject -- a plain QRunnable cannot have signals at all.
//   5. MyTask::run() computed a result and then never emitted it (the
//      `emit Result(...)` line was commented out), so the result of every
//      background task was silently discarded.
//   6. SetSocket(int) used `int` for a socket descriptor; Qt5/6 use
//      `qintptr` (QTcpServer::incomingConnection's actual parameter type),
//      which matters on platforms where a native socket handle doesn't fit
//      in 32 bits.
//
// Per the repository README, none of this has been compiled locally (no
// Qt installation in the sandbox this was written in) -- see
// examples/client_server_demo.cpp for a real loopback client/server test
// that CI runs to verify the whole chain actually works end to end.
#include <QObject>
#include <QRunnable>
#include <QTcpServer>
#include <QTcpSocket>

namespace client_server {

class MyServer : public QTcpServer {
  Q_OBJECT

 public:
  explicit MyServer(QObject* parent);

  bool StartServer(quint16 port = 1234);

 protected:
  void incomingConnection(qintptr socketDescriptor) override;
};

class MyClient : public QObject {
  Q_OBJECT

 public:
  explicit MyClient(QObject* parent);

  void SetSocket(qintptr descriptor);

 private slots:
  void onConnected();
  void onDisconnected();
  void onReadyRead();
  void onTaskResult(int number);

 private:
  QTcpSocket* socket = nullptr;
};

class MyTask : public QObject, public QRunnable {
  Q_OBJECT

 public:
  void run() override;

 signals:
  void Result(int number);
};

}  // namespace client_server

#endif  // QTCPP_CLIENT_SERVER_H
