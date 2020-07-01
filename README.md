# QtFileTransferViaSocket
Передача файла в Qt через QTcpSocket

2 варианта передачи файла на клиенте:

TcpClient::socketSendMessageFile() - Асинхронная отправка данных

TcpClient::socketSendMessageFile_block() - Блокирующая сокет отправка

MyTcpServer::slotReadyRead - Асинхронное получение данных

MyTcpServer::slotReadyRead_block - Блокирующее сокет получение данных
