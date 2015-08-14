#ifndef SPARKLEUPDATER_H
#define SPARKLEUPDATER_H

#include <QObject>



class SparkleUpdater : public QObject
{
    Q_OBJECT
public:
    explicit SparkleUpdater(QObject *parent = 0);
    ~SparkleUpdater();
public Q_SLOTS:
    void checkNow(bool silent = true);
private:
    class Private;
    Private* d;
};

#endif // SPARKLEUPDATER_H
