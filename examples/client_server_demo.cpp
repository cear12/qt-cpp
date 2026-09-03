// client_server_demo.cpp - Real loopback proof that the client_server.h
// signal/slot chain actually works end to end: start MyServer on an
// OS-assigned localhost port, connect a plain QTcpSocket to it, send a
// line, and verify the computed "Task Result = 4950" comes back over the
// same connection. No external network access required.
#include "qtcpp/client_server.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    client_server::MyServer server(nullptr);
    if (!server.StartServer(0)) {  // port 0 = let the OS pick a free port
        std::cout << "[FAIL] server failed to start\n";
        return 1;
    }
    std::cout << "server listening on 127.0.0.1:" << server.serverPort() << "\n";

    auto client = std::make_unique<QTcpSocket>();
    QByteArray received;
    bool sawExpectedResult = false;

    QObject::connect(client.get(), &QTcpSocket::readyRead, [&]() {
        received += client->readAll();
        if (received.contains("Task Result = 4950")) {
            sawExpectedResult = true;
            QCoreApplication::quit();
        }
    });
    QObject::connect(client.get(), &QTcpSocket::connected,
                      [&]() { client->write("hello from the demo client\n"); });

    QTimer::singleShot(5000, &app, &QCoreApplication::quit);  // safety timeout

    client->connectToHost(QHostAddress::LocalHost, server.serverPort());
    int exitCode = app.exec();
    Q_UNUSED(exitCode)

    std::cout << "received: " << received.toStdString() << "\n";
    std::cout << "[" << (sawExpectedResult ? "PASS" : "FAIL")
               << "] client received the expected background-task result over the real socket\n";

    return sawExpectedResult ? 0 : 1;
}
