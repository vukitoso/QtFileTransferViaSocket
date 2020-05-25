#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDataStream>
#include <QThread>
#include <QFileInfo>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    packetType = PacketType::TYPE_NONE;
    filePath.clear();
    fileSize = 0;
    testStr.clear();
    sizeReceivedData = 0;
    length = 0;
    tmpBlock.clear();
    countSend = 0;

    this->fileCopy = "/mnt/d/1_copy.png"; // временная замена имени файла
    if (QFile::exists(this->fileCopy))
        if (!QFile::remove(this->fileCopy))
            qFatal("File not remove!");
}

void MyTcpServer::startServer()
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 1111)){
        qFatal("SERVER: server is not started");
    }

    qDebug() << Tools::getTime() << "SERVER: server is started";
}

MyTcpServer::~MyTcpServer()
{
    qDebug() << "MyTcpServer::~MyTcpServer()";
    delete mTcpServer;
    delete mTcpSocket;
}

void MyTcpServer::slotNewConnection()
{
    mTcpSocket = mTcpServer->nextPendingConnection();
    mTcpSocket->write("Hello, World!!! I am server!");

    connect(mTcpSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotReadyRead);
    connect(mTcpSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}


void MyTcpServer::slotClientDisconnected()
{
    mTcpSocket->close();
}


void MyTcpServer::slotReadyRead()
{
    if (!mTcpSocket || !mTcpSocket->bytesAvailable())
        return;

    qDebug() << Tools::getTime() << "SERVER: --------------------new-----------------------";
    qDebug() << Tools::getTime() << "SERVER: onSocketReceiveMessage: bytesAvailable" << mTcpSocket->bytesAvailable();

    QDataStream stream(mTcpSocket);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

//    while (!mTcpSocket->atEnd())
//    {
        // Считывание PacketType
        if (packetType == PacketType::TYPE_NONE) {
            stream.startTransaction();
            stream >> packetType;
            if (!stream.commitTransaction()) {
                qDebug() << Tools::getTime() << "SERVER: packetType - FAIL commitTransaction";
                return;
            }
            qDebug() << Tools::getTime() << "SERVER: type:" << packetType;
        }

        if (packetType == PacketType::TYPE_MSG)
        {
            //
        }
        else if (packetType == PacketType::TYPE_FILE)
        {
            //====================================================
            // Получение filePath

            if (filePath.isEmpty()) {
                stream.startTransaction();
                stream >> filePath;
                if (!stream.commitTransaction()) {
                    qDebug() << Tools::getTime() << "SERVER: filePath - FAIL commitTransaction";
                    return;
                }
                qDebug() << Tools::getTime() << "SERVER: filePath:" << filePath;
            }


            //====================================================
            // Получение fileSize

            if (!fileSize) {
                stream.startTransaction();
                stream >> fileSize;
                if (!stream.commitTransaction()) {
                    qDebug() << Tools::getTime() << "SERVER: fileSize - FAIL commitTransaction";
                    return;
                }
                qDebug() << Tools::getTime() << "SERVER: fileSize:" << fileSize;
            }

            //====================================================
            // Получение файла

            if (sizeReceivedData != fileSize)
            {
                filePath = this->fileCopy; // временная замена имени файла
                QFile file(filePath);
                file.open(QFile::Append);

                // Работа с файлом в цикле "пока в сокете есть данные"
                while (!stream.atEnd())
                {
                    //====================================================
                    // Получение tmpBlock

                    stream.startTransaction();
                    stream >> tmpBlock;
                    if (!stream.commitTransaction()) {
                        qDebug() << Tools::getTime() << "SERVER: tmpBlock - FAIL commitTransaction";
                        break;
                    }

                    qint64 toFile = file.write(tmpBlock);
                    qDebug() << Tools::getTime() << "SERVER: toFile    : " << toFile;

                    sizeReceivedData += toFile;
                    countSend++;
                    qDebug() << Tools::getTime() << "SERVER: countSend: " << countSend;
//                    qDebug() << Tools::getTime() << "SERVER: sizeReceivedData: " << sizeReceivedData;
//                    qDebug() << Tools::getTime() << "SERVER: -------------------------------------------------" << endl;

                    length = 0;
                    tmpBlock.clear();

                    if (sizeReceivedData == fileSize)
                        break;

                } // while (!stream.atEnd())

                file.close();

            } // if (sizeReceivedData != fileSize)

            if (sizeReceivedData != fileSize)
                return;

            qDebug() << Tools::getTime() << "SERVER: sizeReceivedData END: " << sizeReceivedData;
            qDebug() << Tools::getTime() << "SERVER fileSize ORIG:" << fileSize;
            qDebug() << "SERVER: countSend FINAL: " << countSend;


            //====================================================
            // Получение testStr

            if (testStr.isEmpty()) {
                stream.startTransaction();
                stream >> testStr;
                if (!stream.commitTransaction()) {
                    qDebug() << Tools::getTime() << "SERVER: testStr - FAIL commitTransaction";
                    return;
                }
                qDebug() << Tools::getTime() << "SERVER: testStr:" << testStr;
            }

            qDebug() << Tools::getTime() << "SERVER: END - bytesAvailable:" << mTcpSocket->bytesAvailable();

            // Очистка переменных
            filePath.clear();
            fileSize = 0;
            tmpBlock.clear();
            sizeReceivedData = 0;
            testStr.clear();
            countSend = 0;

        } // else if (packetType == PacketType::TYPE_FILE)

        packetType = PacketType::TYPE_NONE;

//    } // while (!mTcpSocket->atEnd())
}
