#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QWidget>

class QLineEdit;
class ClearLineEdit;
class SearchLineEdit;

class SearchPanel : public QWidget
{
    Q_OBJECT
public:
    explicit SearchPanel(QWidget *parent = 0);
    bool caseSensitive() const;
    bool wholeWord() const;
    bool useRegularExpressions() const;
    void startSearch(const QString &searchWhat);
    const QString replaceBy();
    const QString searchWhat();
Q_SIGNALS:
    void find(const bool forward);
    void replaceAll();
    void replace();
    void replaceAndFind();
public Q_SLOTS:
private Q_SLOTS:
    void findNext();
    void findPrev();
private:
    SearchLineEdit *searchWhatEdit;
    ClearLineEdit *replaceByEdit;
};

#endif // SEARCHPANEL_H
