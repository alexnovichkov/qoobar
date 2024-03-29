#ifndef QPROGRESSINDICATORSPINNING_H
#define QPROGRESSINDICATORSPINNING_H

#include <QWidget>
#include <QPointer>

class QProgressIndicatorSpinningPrivate;
class QProgressIndicatorSpinning : public QWidget
{
    Q_OBJECT
public:
    // Matches NSProgressIndicatorThickness
    enum Thickness {
       Default = 14,
       Small   = 10,
       Large   = 18,
       Aqua    = 12
    };

    explicit QProgressIndicatorSpinning(QWidget *parent,
                                        Thickness thickness = Default);
public Q_SLOTS:
    void animate(bool animate = true);
private:
    friend class QProgressIndicatorSpinningPrivate;
    QPointer<QProgressIndicatorSpinningPrivate> pimpl;
};

struct QProgressIndicatorSpinningHandle
{
    QProgressIndicatorSpinningHandle(QProgressIndicatorSpinning *indicator) : indicator(indicator)
    {
        if (indicator) indicator->animate();
    }
    ~QProgressIndicatorSpinningHandle()
    {
        if (indicator) indicator->animate(false);
    }
    QProgressIndicatorSpinning *indicator;
};

#endif // QPROGRESSINDICATORSPINNING_H
