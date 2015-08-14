#ifndef PLATFORMPROCESS_H
#define PLATFORMPROCESS_H

#include <QProcess>

class PlatformProcess : public QProcess
{
    Q_OBJECT
public:
    explicit PlatformProcess(QObject *parent = 0);
};

#endif // PLATFORMPROCESS_H
