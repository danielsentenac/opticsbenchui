#include <ComediCounter.h>
#include <qwt_plot.h>
#include <QTime>

#define HISTORY 3000 // 1/10 seconds

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

    ComediCounterPlot( QWidget * = 0, ComediCounter * = 0, int = 0 );
    const QwtPlotCurve *comedicounterCurve( int id ) const
    {
        return data[id].curve;
    }

protected:
    void timerEvent( QTimerEvent *e );
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
    Comedi  *comedi;
    int output;
};
