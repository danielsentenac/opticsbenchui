/**
 * @file ComediCounterPlot.cpp
 * @brief Comedi Counter Plot source file.
 */
#include <qpainter.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_plot_canvas.h>
#include "ComediCounterPlot.h"

/// Scale draw that formats axis labels as elapsed time.
class TimeScaleDraw: public QwtScaleDraw
{
public:
    /// Construct with a base time.
    TimeScaleDraw( const QTime &base ):
        baseTime( base )
    {
    }
    /// Format the label for a given value.
    QwtText label( double v ) const override
    {
        QTime upTime = baseTime.addSecs( static_cast<int>( v ) );
        return upTime.toString();
    }
private:
    QTime baseTime;
};

/// Background plot item that paints alternating bands.
class Background: public QwtPlotItem
{
public:
    /// Construct the background item.
    Background()
    {
        setZ( 0.0 );
    }

    /// Return the runtime type information.
    int rtti() const override
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    /// Draw the background bands.
    void draw( QPainter *painter,
        const QwtScaleMap &, const QwtScaleMap &yMap,
        const QRectF &canvasRect ) const override
    {
        QColor c( Qt::white );
        QRectF r = canvasRect;

        for ( int i = 100; i > 0; i -= 10 )
        {
            r.setBottom( yMap.transform( i - 10 ) );
            r.setTop( yMap.transform( i ) );
            painter->fillRect( r, c );

            c = c.dark( 110 );
        }
    }
};

/// Plot curve with translucent fill for Comedi counter data.
class ComediCounterCurve: public QwtPlotCurve
{
public:
    /// Construct a curve with the given title.
    ComediCounterCurve( const QString &title ):
        QwtPlotCurve( title )
    {
        setRenderHint( QwtPlotItem::RenderAntialiased );
    }

    /// Set the curve color with alpha.
    void setColor( const QColor &color )
    {
        QColor c = color;
        c.setAlpha( 150 );

        setPen( QPen( Qt::NoPen ) );
        setBrush( c );
    }
};

ComediCounterPlot::ComediCounterPlot(QWidget *parent, Comedi *comedi, int output)
    : QwtPlot(parent),
      curValue(0.0),
      dataCount(0),
      comedicounter(comedi),
      output(output) {
    if (comedicounter) {
        connect(comedicounter, SIGNAL(getOutputValues(void*)),
                this, SLOT(getOutputValues(void*)));
    }
    setAutoReplot( false );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setBorderRadius( 10 );

    setCanvas( canvas );

    plotLayout()->setAlignCanvasToScales( true );

    QwtLegend *legend = new QwtLegend;
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    insertLegend( legend, QwtPlot::RightLegend );

    setAxisTitle( QwtPlot::xBottom, " Time [1/10 sec]" );
    //setAxisScaleDraw( QwtPlot::xBottom,
    //   new TimeScaleDraw( upTime() ) );
    setAxisScale( QwtPlot::xBottom, 0, HISTORY );
    setAxisLabelRotation( QwtPlot::xBottom, -50.0 );
    setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom );

    /*
     In situations, when there is a label at the most right position of the
     scale, additional space is needed to display the overlapping part
     of the label would be taken by reducing the width of scale and canvas.
     To avoid this "jumping canvas" effect, we add a permanent margin.
     We don't need to do the same for the left border, because there
     is enough space for the overlapping label below the left scale.
     */

    QwtScaleWidget *scaleWidget = axisWidget( QwtPlot::xBottom );
    const int fmh = QFontMetrics( scaleWidget->font() ).height();
    scaleWidget->setMinBorderDist( 0, fmh / 2 );

    setAxisTitle( QwtPlot::yLeft, "Counts [TTL/s]" );
    setAxisAutoScale( QwtPlot::yLeft, true );

    Background *bg = new Background();
    bg->attach( this );

    ComediCounterCurve *curve;

    curve = new ComediCounterCurve( tr("Output %1").arg(output) );
    curve->setColor( Qt::blue );
    curve->setZ( curve->z() - 1 );
    curve->attach( this );
    data[User].curve = curve;

    showCurve( data[User].curve, true );

    for ( int i = 0; i < HISTORY; i++ )
        timeData[HISTORY - 1 - i] = i;
    
    ( void )startTimer( 100 ); // 0.1 second

    connect( legend, SIGNAL( checked( const QVariant &, bool, int ) ),
        SLOT( legendChecked( const QVariant &, bool ) ) );

}

QTime ComediCounterPlot::upTime() const
{
    QTime t( 0, 0, 0 );
    for ( int i = 0; i < NComediCounterData; i++ )
        t = t.addSecs( 1. );

    return t;
}
void ComediCounterPlot::getOutputValues(void *comedivalues) {
  // Cast in expected type
  QVector<double>* vfcomedivalues;
  vfcomedivalues = (QVector<double>*) comedivalues;
  curValue = vfcomedivalues->at(output);
}

void ComediCounterPlot::timerEvent( QTimerEvent * )
{
    for ( int i = dataCount; i > 0; i-- )
    {
        for ( int c = 0; c < NComediCounterData; c++ )
        {
            if ( i < HISTORY )
                data[c].data[i] = data[c].data[i-1];
        }
    }

    data[User].data[0] =  curValue;

    if ( dataCount < HISTORY )
        dataCount++;

    for ( int j = 0; j < HISTORY; j++ )
        timeData[j]++;

    setAxisScale( QwtPlot::xBottom,
        timeData[HISTORY - 1], timeData[0] );

    for ( int c = 0; c < NComediCounterData; c++ )
    {
        data[c].curve->setRawSamples(
            timeData, data[c].data, dataCount );
    }

    replot();
}

void ComediCounterPlot::legendChecked( const QVariant &itemInfo, bool on )
{
    QwtPlotItem *plotItem = infoToItem( itemInfo );
    if ( plotItem )
        showCurve( plotItem, on );
}

void ComediCounterPlot::showCurve( QwtPlotItem *item, bool on )
{
    item->setVisible( on );

    QwtLegend *lgd = qobject_cast<QwtLegend *>( legend() );

    QList<QWidget *> legendWidgets = 
        lgd->legendWidgets( itemToInfo( item ) );

    if ( legendWidgets.size() == 1 )
    {
        QwtLegendLabel *legendLabel =
            qobject_cast<QwtLegendLabel *>( legendWidgets[0] );

        if ( legendLabel )
            legendLabel->setChecked( on );
    }

    replot();
}
