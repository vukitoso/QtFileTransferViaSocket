#include <QCoreApplication>
#include "mytcpserver.h"
#include "tcpclient.h"
#include <QThread>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MyTcpServer *server = new MyTcpServer;
    QThread *threadServer = new QThread;
    QObject::connect(threadServer, &QThread::started, server, &MyTcpServer::startServer);
    QObject::connect(threadServer, &QThread::finished, server, &MyTcpServer::deleteLater);
    server->moveToThread(threadServer);
    threadServer->start();

    QThread::msleep(100);

    TcpClient *client = new TcpClient("127.0.0.1", 1111);
    QThread *threadClient = new QThread;
    QObject::connect(threadClient, &QThread::started, client, &TcpClient::startClient);
    QObject::connect(threadClient, &QThread::finished, client, &TcpClient::deleteLater);
    client->moveToThread(threadClient);
    threadClient->start();

    return a.exec();
}
