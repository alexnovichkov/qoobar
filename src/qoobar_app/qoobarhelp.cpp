#include "qoobarhelp.h"
#include "applicationpaths.h"
#include <QDesktopServices>
#include <QUrl>
#include "enums.h"

void Qoobar::showHelp(const QString &section)
{//DD;
    QString path = ApplicationPaths::documentationPath();
    if (!QFileInfo::exists(path))
        path = QSL("http://qoobar.sourceforge.net/en/documentation.htm");
    else path = QString("file:///%1").arg(path);
    if (!section.isEmpty())
        path.append(QSL("#") + section);
    QDesktopServices::openUrl(QUrl(path));
}
