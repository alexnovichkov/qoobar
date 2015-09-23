#include "logging.h"

#ifndef QT_NO_DEBUG
#include <QTime>
#include <QtDebug>
#define DO_TRACE
Trace::Trace(const QString &msg) : _msg(msg)
{
#ifdef DO_TRACE
    time = new QTime();
    time->start();
    qDebug()<<"Entering "<<_msg;
#endif
}
Trace::~Trace()
{
#ifdef DO_TRACE
    qDebug()<<"Leaving  "<<_msg << "time" << time->elapsed();
    delete time;
#endif
}
#endif
