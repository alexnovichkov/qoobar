#ifndef FILEICONPROVIDER_H
#define FILEICONPROVIDER_H

#include <QFileIconProvider>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#define ICON_PROVIDER QAbstractFileIconProvider
#else
#define ICON_PROVIDER QFileIconProvider
#endif


class FileIconProvider : public QFileIconProvider
{
public:
    FileIconProvider();
    virtual QIcon icon(ICON_PROVIDER::IconType type) const override;
//    virtual QIcon icon(const QFileInfo &info) const override;
};

#endif // FILEICONPROVIDER_H
