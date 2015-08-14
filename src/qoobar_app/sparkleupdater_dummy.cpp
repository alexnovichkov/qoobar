#include "sparkleupdater.h"

SparkleUpdater::SparkleUpdater(QObject *parent) :
    QObject(parent)
{

}

SparkleUpdater::~SparkleUpdater()
{

}

void SparkleUpdater::checkNow(bool silent)
{
    Q_UNUSED(silent)
}
