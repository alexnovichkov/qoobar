#ifndef TRACE_H
#define TRACE_H

#include <QString>

#ifndef QT_NO_DEBUG
class QTime;
class Trace {
public:
    Trace(const QString &msg);
    ~Trace();
private:
    QString _msg;
    QTime *time;
};

#define DD  Trace trace(Q_FUNC_INFO);
#else
#define DD
#endif

#endif // TRACE_H
