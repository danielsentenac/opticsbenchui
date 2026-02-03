#ifndef COMEDICOUNTERPLOT_H
#define COMEDICOUNTERPLOT_H

#include "Comedi.h"

#include <QTime>
#include <qwt_plot.h>

#define HISTORY 1200 // 1/10 seconds

class QwtPlotCurve;

/// \ingroup ui
/// Qwt plot widget for Comedi counter data.
class ComediCounterPlot : public QwtPlot
{
    Q_OBJECT
public:
    /// Curve identifiers for counter data.
    enum ComediCounterData
    {
        User,

        NComediCounterData
    };

    /// Construct a counter plot for a Comedi output.
    /// \param parent Parent widget.
    /// \param comedi Comedi controller instance.
    /// \param output Output channel index.
    explicit ComediCounterPlot(QWidget *parent = nullptr,
                               Comedi *comedi = nullptr,
                               int output = 0);
    /// Return a curve by id.
    /// \param id Curve identifier.
    const QwtPlotCurve *comedicounterCurve( int id ) const
    {
        return data[id].curve;
    }

protected:
    /// Timer callback for plot updates.
    /// \param e Timer event.
    void timerEvent( QTimerEvent *e ) override;
    /// Return the elapsed uptime.
    QTime upTime() const;

private slots:
    /// Receive output values for plotting.
    /// \param comedivalues Pointer to device-specific values.
    void getOutputValues(void *comedivalues);
    /// Toggle a curve visibility from the legend.
    /// \param item Legend item.
    /// \param on True to show, false to hide.
    void legendChecked( const QVariant &item, bool on );

private:
    /// Show or hide a curve.
    /// \param item Plot item.
    /// \param on True to show, false to hide.
    void showCurve( QwtPlotItem *item, bool on );

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
