#ifndef TRACE_H
#define TRACE_H

#include <QString>

#ifndef QT_NO_DEBUG
class QElapsedTimer;
class Trace {
public:
    Trace(const QString &msg);
    ~Trace();
private:
    QString _msg;
    QElapsedTimer *time;
};

#define DD  Trace trace(QString(Q_FUNC_INFO));
#else
#define DD
#endif

#endif // TRACE_H
