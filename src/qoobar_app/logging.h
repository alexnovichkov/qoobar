#ifndef TRACE_H
#define TRACE_H

#include <QString>

//#define DO_TRACE
#include <QObject>

#include <QElapsedTimer>

class Trace {
public:
    explicit Trace(const QString &msg);
    ~Trace();
private:
    Q_DISABLE_COPY(Trace)
//#if QT_VERSION >= QT_VERSION_CHECK(5,13,0)
//    Q_DISABLE_COPY_MOVE(Trace)
//#endif
    QString _msg;
    QElapsedTimer time;
};

#ifdef DO_TRACE

#define DD  Trace trace(QString(Q_FUNC_INFO));
#else
#define DD
#endif

#define DDD  Trace trace(QString(Q_FUNC_INFO));

#endif // TRACE_H
