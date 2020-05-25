#include "global.h"
#include <QTime>
#include <QDebug>

void Tools::printTime()
{
    qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz");
}


QString Tools::getTime()
{
    return QTime::currentTime().toString("hh:mm:ss.zzz");
}
