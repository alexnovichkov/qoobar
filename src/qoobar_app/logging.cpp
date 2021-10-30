#include "logging.h"

#include <QtDebug>
Trace::Trace(const QString &msg) : _msg(msg)
{
    time.start();
    qDebug()<<"Entering "<<_msg;
}
Trace::~Trace()
{
    qDebug()<<"Leaving  "<<_msg << "time" << time.nsecsElapsed()/1000 <<"mks";
}

