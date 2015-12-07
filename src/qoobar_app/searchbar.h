#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QToolBar>

#include "tagger.h"

class QMenu;
class QWidgetAction;
class QPushButton;
class SearchLineEdit;
class QLineEdit;
class QProgressIndicatorSpinning;

class Searcher : public QObject {
    Q_OBJECT

public:
    Searcher();
    ~Searcher();

public Q_SLOTS:
    void setSearchParametres(bool caseSensitive, bool useRegularExpressions,
                 bool wholeWord, const QString &text);
    void setPaths(const QStringList &searchPaths) {paths = searchPaths;}
    void process();
    void stop();
    void reset();
Q_SIGNALS:
    void finished();
    void error(QString err);
    void found(const Tag &tag);
    void pathChanged(const QString&);
private:
    void searchDir(const QString &file);
    void maybeAdd(const QString &file);
    bool stopped;
    QStringList paths;
    QStringList filters;
    QString text;
    QRegExp regExp;
};

class SearchBar : public QToolBar
{
    Q_OBJECT
public:
    explicit SearchBar(QWidget *parent = 0);
    ~SearchBar();
    void retranslateUI();
    SearchLineEdit *textEdit;
Q_SIGNALS:
    void addFile(const Tag &tag);
    void reset();
public Q_SLOTS:

private Q_SLOTS:
    void removePath();
    void addPath();
    void updateStartSearchButton();
    void toggleSearch();
    void quitSearch();

    void startSearch();
    void stopSearch();
    void resetSearch();
private:
    QMenu *createPathsMenu();
    QWidgetAction *createAction(const QString &text);
    QPushButton *pathsButton;
    QAction *addPathAct;
    QMenu *pathsMenu;

    QPushButton *startSearchButton;
    QAction *closeSearchPanel;
    QLineEdit *currentPathLabel;

    QThread* thread;
    Searcher* searcher;
    QProgressIndicatorSpinning *progress;

    QStringList paths;
};

#endif // SEARCHBAR_H
