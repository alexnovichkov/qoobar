#include "logging.h"

#ifndef QT_NO_DEBUG
#include <QElapsedTimer>
#include <QtDebug>
#define DO_TRACE
Trace::Trace(const QString &msg) : _msg(msg)
{
#ifdef DO_TRACE
    time = new QElapsedTimer();
    time->start();
    qDebug()<<"Entering "<<_msg;
#endif
}
Trace::~Trace()
{
#ifdef DO_TRACE
    qDebug()<<"Leaving  "<<_msg << "time" << time->nsecsElapsed()/1000 <<"mks";
    delete time;
#endif
}
#endif //ifndef QT_NO_DEBUG
