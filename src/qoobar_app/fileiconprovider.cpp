#include "fileiconprovider.h"
#include <QApplication>
#include <QStyle>

FileIconProvider::FileIconProvider() : QFileIconProvider()
{

}

QIcon FileIconProvider::icon(IconType type) const
{
    switch (type) {
    case ICON_PROVIDER::Folder:
        return qApp->style()->standardIcon(QStyle::SP_DirIcon);
    case ICON_PROVIDER::File:
        return qApp->style()->standardIcon(QStyle::SP_FileIcon);
    case ICON_PROVIDER::Computer:
        return qApp->style()->standardIcon(QStyle::SP_ComputerIcon);
    case ICON_PROVIDER::Desktop:
        return qApp->style()->standardIcon(QStyle::SP_DesktopIcon);
    case ICON_PROVIDER::Trashcan:
        return qApp->style()->standardIcon(QStyle::SP_TrashIcon);
    case ICON_PROVIDER::Network:
        return qApp->style()->standardIcon(QStyle::SP_DriveNetIcon);
    case ICON_PROVIDER::Drive:
        return qApp->style()->standardIcon(QStyle::SP_DriveHDIcon);
    default: break;

    }

    return QFileIconProvider::icon(type);
}

//QIcon FileIconProvider::icon(const QFileInfo &info) const
//{
//    if (info.isDir()) return qApp->style()->standardIcon(QStyle::SP_DirIcon);
//    if (info.isFile()) return qApp->style()->standardIcon(QStyle::SP_FileIcon);
//    return QFileIconProvider::icon(info);
//}
