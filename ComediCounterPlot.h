#ifndef COMEDICOUNTERPLOT_H
#define COMEDICOUNTERPLOT_H

#include "Comedi.h"

#include <QTime>
#include <qwt_plot.h>

#define HISTORY 1200 // 1/10 seconds

class QwtPlotCurve;

class ComediCounterPlot : public QwtPlot
{
    Q_OBJECT
public:
    enum ComediCounterData
    {
        User,

        NComediCounterData
    };

    explicit ComediCounterPlot(QWidget *parent = nullptr,
                               Comedi *comedi = nullptr,
                               int output = 0);
    const QwtPlotCurve *comedicounterCurve( int id ) const
    {
        return data[id].curve;
    }

protected:
    void timerEvent( QTimerEvent *e ) override;
    QTime upTime() const;

private slots:
    void getOutputValues(void *comedivalues);
    void legendChecked( const QVariant &, bool on );

private:
    void showCurve( QwtPlotItem *, bool on );

    struct
    {
        QwtPlotCurve *curve;
        double data[HISTORY];
    } data[NComediCounterData];

    double timeData[HISTORY];
    double curValue;
    int dataCount;
    Comedi  *comedicounter;
    int output;
};

#endif
