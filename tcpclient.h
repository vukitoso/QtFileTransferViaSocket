#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <QSslSocket>
#include <QTcpSocket>
#include "global.h"
#include <QFile>


class TcpClient : public QObject
{
    Q_OBJECT

public:
    TcpClient();
    ~TcpClient();

private:
    QTcpSocket* m_pTcpSocket;
    quint16     m_nNextBlockSize;
    int countSend;
    qint64 fileSize;
    qint64 sizeSendData;
    QFile *sendFile;

    void socketSendMessageFile_block();
    void socketSendMessageFile();

signals:
    void disconnected(void);
    void endSendFile();

private slots:
    void slotReadyRead   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotConnected   (                            );
    void sendPartOfFile();
    void slotEndSendFile();

public slots:
    void startClient();
};

#endif /* TCPCLIENT_H_ */
