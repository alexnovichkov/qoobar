#ifndef REPLAYGAINER_H
#define REPLAYGAINER_H

#include <QObject>
#include <QHash>
#include "enums.h"
#include "tagger.h"
#include <QStringList>
#include <QUrl>

#ifdef WITH_DECODING
extern "C" {
#include "input.h"
#ifdef __MINGW32__
#  define _FLOAT_H___
#endif
#include "filetree.h"
#include "scanner-common.h"
#include <glib.h>
#include "scanner-tag.h"
}

struct RgNode {
    ReplayGainInfo rg;
    int index;
};
#endif



class Model;
class QProcess;
class WorkerThread;

enum RG_OPERATION {
    RG_SCAN_AS_FILES=1,
    RG_SCAN_AS_ALBUM=2,
    RG_SCAN_AS_ALBUMS_BY_TAGS=3,
    RG_SCAN_AS_ALBUMS_BY_FOLDERS=4,
    RG_SCAN_REMOVE=5
};

const struct ProgramOption {
    const char *type;
    const char *program;
    const char *trackOptions;
    const char *albumOptions;
} options[] = {
    #ifdef Q_OS_WIN
    {"MP3","aacgain", "-e.-s.r.-s.a.-q", "-s.r.-s.a.-q"},
    #else
    {"MP3","mp3gain", "-e.-s.r.-s.a", "-s.r.-s.a"},
    #endif
    {"OGG","vorbisgain","-sn","-asn"},
    {"WMA",0,0,0},
    {"FLAC","metaflac", "--add-replay-gain", "--add-replay-gain"},
    {"M4A","aacgain", "-e.-s.r.-q", "-s.r.-q"},
    {"MPC","mpcgain", 0, 0},
    {"WV","wvgain","-i","-ai"},
    {"WAV",0,0,0},
    {"SPX",0,0,0},
    {"TTA",0,0,0},
    {"OGA",0,0,0},
    {"APE",0,0,0},
    {"AIFF",0,0,0},
    {"OPUS",0,0,0},
    {"MPC","replaygain","--auto","--auto"} //mpc v7
};

class ReplayGainer : public QObject
{
    Q_OBJECT
public:
    explicit ReplayGainer(Model *model, QObject *parent = 0);
    void setSkip(bool skip) {this->skip = skip;}
    void setDryRun(bool dryRun) {this->dryRun = dryRun;}
    QList<ReplayGainInfo> getNewRgInfo() {return rgInfo;}
    bool start(int operation);
Q_SIGNALS:
    void message(int type, const QString &message);
    void textRead(const QString &text);
    void tick();
private Q_SLOTS:
    void messages(const QStringList &messages);
private:
    QHash<int, QVector<int> > sortByFileType(int operation);
    void scanGroup(int fileType, const QVector<int> &indexes, const int operation);
    void scan(int fileType, const QVector<int> &indexes, int operation);
    QMap<QString,QVector<int> > sort(const QVector<int> &indexes, bool byFolder);
    void removeRG(int fileType, const QVector<int> &indexes);
    QStringList getFileNames(const QVector<int> &indexes);

    QList<ReplayGainInfo> scanWithUtilities(int fileType, const QVector<int> &indexes, int operation);
    QList<ReplayGainInfo> scanWithDecoding(int fileType, const QVector<int> &indexes, int operation);

    //bool write(int fileType, const QVector<int> &indexes, int operation, const QList<ReplayGainInfo> &rgs);

    void init();
    void deinit();

    bool checkFileType(int fileType);

    Model *m;
    bool skip;
    bool dryRun;

    QHash<QString,QString> copiedFiles;
    QList<ReplayGainInfo> rgInfo;

    QProcess * m_process;
};

#endif // REPLAYGAINER_H
