#include <QSslKey>
#include <QSslCertificate>
#include "tcpclient.h"
#include <QDataStream>
#include <QThread>
#include <QFileInfo>


TcpClient::TcpClient()
{
    m_nNextBlockSize = 0;
    countSend = 0;
    sizeSendData = 0;
    fileSize = 0;
}

TcpClient::~TcpClient()
{
    qDebug() << "TcpClient::~TcpClient()";
    delete m_pTcpSocket;
    delete sendFile;
}

void TcpClient::startClient()
{
    m_pTcpSocket = new QTcpSocket(this);

    QString ip("127.0.0.1");
    m_pTcpSocket->connectToHost(ip, 1111); // 127.0.0.1

    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
}

void TcpClient::slotConnected()
{
    this->socketSendMessageFile(); // Асинхронная отправка
//    this->socketSendMessageFile_block(); // Блокирующая сокет отправка
}

void TcpClient::slotReadyRead()
{
    qDebug() << "_CLIENT:" << m_pTcpSocket->readAll();
}


void TcpClient::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(m_pTcpSocket->errorString())
                    );
    qDebug() << "Link::slotError:" << strError;
}



void TcpClient::socketSendMessageFile()
{
    QDataStream stream(m_pTcpSocket);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    stream << PacketType::TYPE_FILE;

    QString fileName("/mnt/d/1.png");
    QFile file(fileName);
    QFileInfo fileInfo(file);
    fileSize = fileInfo.size();

    stream << fileName;
    stream << fileSize;

    sendFile = new QFile(fileName);

    if(sendFile->open(QFile::ReadOnly)) {
        connect(m_pTcpSocket, &QTcpSocket::bytesWritten, this, &TcpClient::sendPartOfFile);
        connect(this, &TcpClient::endSendFile, this, &TcpClient::slotEndSendFile);
        sendPartOfFile();
    } else {
        qFatal("_CLIENT: File not open!");
    }
}


void TcpClient::sendPartOfFile()
{
    QDataStream stream(m_pTcpSocket);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    if(!sendFile->atEnd()) {
        QByteArray data = sendFile->read(1024*250);
//        QByteArray data = sendFile->read(1024*1000*10);
        stream << data;
        m_pTcpSocket->waitForBytesWritten();
//        qDebug() << Tools::getTime() << "_CLIENT: slot sendPartOfFile() | write data";
    } else {
        qDebug() << Tools::getTime() << "_CLIENT: slot sendPartOfFile() | File end!";
        sendFile->close();
        sendFile = NULL;
        disconnect(m_pTcpSocket, &QTcpSocket::bytesWritten, this, &TcpClient::sendPartOfFile);

        emit endSendFile(); // Завершаем отправку данных

        return;
    }

    countSend++;
    qDebug() << Tools::getTime() << "_CLIENT: slot sendPartOfFile() | countSend++" << "countSend:" << countSend;
}


// Отправка текстовой строки после отправки файла
void TcpClient::slotEndSendFile()
{
    qDebug() << Tools::getTime() << "_CLIENT: TcpClient::slotEndSendFile() | File send FINISH!";
    qDebug() << Tools::getTime() << "_CLIENT: TcpClient::slotEndSendFile() | countSend FINAL: " << countSend;

    QDataStream stream(m_pTcpSocket);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    QString testStr("TEST_MESSAGE");
    stream << testStr;
}


void TcpClient::socketSendMessageFile_block()
{
    QDataStream stream(m_pTcpSocket);
    stream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    stream << PacketType::TYPE_FILE;

    // send File
    QString fileName("/mnt/d/1.png");
    QFile file(fileName);
    QFileInfo fileInfo(file);
    fileSize = fileInfo.size();

    stream << fileName;
    stream << fileSize;

    if (file.open(QFile::ReadOnly))
    {
        while(!file.atEnd())
        {
            QByteArray data = file.read(1024*250);
//            QByteArray data = file.read(1024*1000*10);
            stream << data;
            m_pTcpSocket->waitForBytesWritten();
            countSend++;
        }
        qDebug() << Tools::getTime() << "_CLIENT: ------------------------ countSend FINAL: " << countSend;
    }

    file.close();

    qDebug() << Tools::getTime() << "_CLIENT: send file ok";

    QString testStr("TEST_MESSAGE");
    stream << testStr;
}

