#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "global.h"
#include <QFile>


class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

public slots:
    void startServer();

private slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotReadyRead();

private:
    QTcpServer * mTcpServer;
    QTcpSocket * mTcpSocket;
    qint64 sizeReceivedData;
    QString fileCopy; // Путь файла для сохранение
    PacketType packetType;

    QString filePath;
    qint64 fileSize;
    QString testStr;
    QByteArray tmpBlock;
    int countSend;
};

#endif // MYTCPSERVER_H
