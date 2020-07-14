#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDataStream>
#include <QThread>
#include <QFileInfo>

MyTcpServer::MyTcpServer(const quint16 &serverPort, QObject *parent)
    : QObject(parent), serverPort(serverPort)
{
    packetType = PacketType::TYPE_NONE;
    filePath.clear();
    fileSize = 0;
    testStr.clear();
    sizeReceivedData = 0;
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

    if(!mTcpServer->listen(QHostAddress::AnyIPv4, this->serverPort)) {
        qFatal("SERVER: server is not started");
    }

    qDebug() << Tools::getTime() << "SERVER: server is started";
}

MyTcpServer::~MyTcpServer()
{
    qDebug() << "MyTcpServer::~MyTcpServer()";
    delete mTcpSocket;
    delete mTcpServer;
}

void MyTcpServer::slotNewConnection()
{
    mTcpSocket = mTcpServer->nextPendingConnection();
    mTcpSocket->write("Hello, World!!! I am server!");

//    connect(mTcpSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotReadyRead);
    connect(mTcpSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotReadyRead_block);
    connect(mTcpSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
}


void MyTcpServer::slotClientDisconnected()
{
    mTcpSocket->close();
}


// Асинхронное чтение данных из сокета
void MyTcpServer::slotReadyRead()
{
    if (!mTcpSocket || !mTcpSocket->bytesAvailable())
        return;

//    qDebug() << Tools::getTime() << "SERVER: --------------------new-----------------------";
//    qDebug() << Tools::getTime() << "SERVER: slotReadyRead(): bytesAvailable" << mTcpSocket->bytesAvailable();

    QDataStream stream(mTcpSocket);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

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
                    //                        qDebug() << Tools::getTime() << "SERVER: tmpBlock - FAIL commitTransaction";
                    break;
                }

                qint64 toFile = file.write(tmpBlock);
                //                    qDebug() << Tools::getTime() << "SERVER: toFile    : " << toFile;

                sizeReceivedData += toFile;
                countSend++;
                //                    qDebug() << Tools::getTime() << "SERVER: countSend: " << countSend;
                //                    qDebug() << Tools::getTime() << "SERVER: sizeReceivedData: " << sizeReceivedData;
                //                    qDebug() << Tools::getTime() << "SERVER: -------------------------------------------------" << endl;

                tmpBlock.clear();

                if (sizeReceivedData == fileSize) {
                    qDebug() << Tools::getTime() << "SERVER: sizeReceivedData END: " << sizeReceivedData;
                    qDebug() << Tools::getTime() << "SERVER fileSize ORIG:" << fileSize;
                    qDebug() << Tools::getTime() << "SERVER: countSend FINAL: " << countSend;
                    break;
                }

            } // while (!stream.atEnd())

            file.close();

        } // if (sizeReceivedData != fileSize)

        if (sizeReceivedData != fileSize)
            return;

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

    exit(0);
}


// Чтение из сокета через блокировку сокета
void MyTcpServer::slotReadyRead_block()
{
    if (!mTcpSocket || !mTcpSocket->bytesAvailable())
        return;

    qDebug() << Tools::getTime() << "SERVER: --------------------new-----------------------";
    qDebug() << Tools::getTime() << "SERVER: slotReadyRead_block(): bytesAvailable" << mTcpSocket->bytesAvailable();

    QDataStream stream(mTcpSocket);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    qDebug() << Tools::getTime() << "SERVER: forever -------: bytesAvailable" << mTcpSocket->bytesAvailable();

    // Считывание PacketType
    forever {
        stream.startTransaction();
        stream >> packetType;
        if (!stream.commitTransaction()) {
            qDebug() << Tools::getTime() << "SERVER: packetType - FAIL commitTransaction";
            if (!mTcpSocket->waitForReadyRead(TCP_READ_TIMEOUT)) {
                qDebug() << Tools::getTime() << "SERVER: ERROR! readyRead timeout - packetType!!!";
                return;
            }
            continue;
        }
        qDebug() << Tools::getTime() << "SERVER: type:" << packetType;
        break;
    }

    if (packetType == PacketType::TYPE_MSG)
    {
        //
    }
    else if (packetType == PacketType::TYPE_FILE)
    {
        //====================================================
        // Получение filePath
        // Получение fileSize

        forever {
            stream.startTransaction();
            stream >> filePath;
            stream >> fileSize;
            if (!stream.commitTransaction()) {
                qDebug() << Tools::getTime() << "SERVER: filePath, fileSize - FAIL commitTransaction";
                if (!mTcpSocket->waitForReadyRead(TCP_READ_TIMEOUT)) {
                    qDebug() << Tools::getTime() << "SERVER: ERROR! readyRead timeout - filePath, fileSize!!!";
                    return;
                }
                continue;
            }
            qDebug() << Tools::getTime() << "SERVER: filePath:" << filePath;
            qDebug() << Tools::getTime() << "SERVER: fileSize:" << fileSize;
            break;
        }


        //====================================================
        // Получение файла

        filePath = this->fileCopy; // временная замена имени файла
        QFile file(filePath);
        file.open(QFile::Append);

        qDebug() << Tools::getTime() << "SERVER: FILE bytesAvailable" << mTcpSocket->bytesAvailable();

//        while (sizeReceivedData < fileSize)
        forever
        {
            qDebug() << Tools::getTime() << "SERVER: FILE forever -------: bytesAvailable" << mTcpSocket->bytesAvailable();

            stream.startTransaction();
            stream >> tmpBlock;
            if (!stream.commitTransaction()) {
                qDebug() << Tools::getTime() << "SERVER: tmpBlock - FAIL commitTransaction";
                if (!mTcpSocket->waitForReadyRead(TCP_READ_TIMEOUT)) {
                    qDebug() << Tools::getTime() << "SERVER: ERROR! readyRead timeout - tmpBlock!!!";
                    return;
                }
                continue;
            }

            qint64 toFile = file.write(tmpBlock);
            qDebug() << Tools::getTime() << "SERVER: toFile    : " << toFile;

            sizeReceivedData += toFile;
            countSend++;
            qDebug() << Tools::getTime() << "SERVER: countSend: " << countSend;
            qDebug() << Tools::getTime() << "SERVER: sizeReceivedData: " << sizeReceivedData;
            qDebug() << Tools::getTime() << "SERVER: ------------------------FILE write block-------------------------" << endl;

            tmpBlock.clear();

            if (sizeReceivedData == fileSize) {
                qDebug() << Tools::getTime() << "SERVER: sizeReceivedData END: " << sizeReceivedData;
                qDebug() << Tools::getTime() << "SERVER: fileSize ORIG:" << fileSize;
                qDebug() << Tools::getTime() << "SERVER: countSend FINAL: " << countSend;
                break;
            }
        }

        file.close();
        qDebug() << Tools::getTime() << "SERVER: file.close()";

        //====================================================
        // Получение testStr

        forever {
            stream.startTransaction();
            stream >> testStr;
            if (!stream.commitTransaction()) {
                qDebug() << Tools::getTime() << "SERVER: testStr - FAIL commitTransaction";
                if (!mTcpSocket->waitForReadyRead(TCP_READ_TIMEOUT)) {
                    qDebug() << Tools::getTime() << "SERVER: ERROR! readyRead timeout - testStr!!!";
                    return;
                }
                continue;
            }
            qDebug() << Tools::getTime() << "SERVER: testStr:" << testStr;
            break;
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

    exit(0);

} // MyTcpServer::slotReadyRead_block()

