#include "fileiconprovider.h"
#include <QApplication>
#include <QStyle>

FileIconProvider::FileIconProvider() : QFileIconProvider()
{

}

QIcon FileIconProvider::icon(IconType type) const
{
    switch (type) {
    case QAbstractFileIconProvider::Folder:
        return qApp->style()->standardIcon(QStyle::SP_DirIcon);
    case QAbstractFileIconProvider::File:
        return qApp->style()->standardIcon(QStyle::SP_FileIcon);
    case QAbstractFileIconProvider::Computer:
        return qApp->style()->standardIcon(QStyle::SP_ComputerIcon);
    case QAbstractFileIconProvider::Desktop:
        return qApp->style()->standardIcon(QStyle::SP_DesktopIcon);
    case QAbstractFileIconProvider::Trashcan:
        return qApp->style()->standardIcon(QStyle::SP_TrashIcon);
    case QAbstractFileIconProvider::Network:
        return qApp->style()->standardIcon(QStyle::SP_DriveNetIcon);
    case QAbstractFileIconProvider::Drive:
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
