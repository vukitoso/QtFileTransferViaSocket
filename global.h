#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

#define TCP_READ_TIMEOUT 5000

enum PacketType
{
    TYPE_NONE = 0,
    TYPE_MSG = 1,
    TYPE_FILE = 2,
};


class Tools {
public:
    static void printTime();
    static QString getTime();
};


#endif // GLOBAL_H
