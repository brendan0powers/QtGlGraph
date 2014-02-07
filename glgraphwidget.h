#ifndef GLGRAPHWIDGET_H
#define GLGRAPHWIDGET_H

#include <QGLWidget>
#include <QGLShaderProgram>
#include <QColor>
#include <QVector>

class GlGraphWidget : public QGLWidget
{
    Q_OBJECT
public:
    enum AxisStyle
    {
        LeftAxis,
        RightAxis,
        NoAxis
    };

    explicit GlGraphWidget(QWidget *parent = 0);

    void setGridColor(const QColor &color);
    void setLineColor(const QColor &color);
    void setBgColor(const QColor &color);
    void setLineWidth(float width);
    void setAxisLineWidth(float width);
    void setGridLineWidth(float width);

    void setData(const QVector<float> &data);
    void setYAxisLimits(float min, float max);
    void setXAxisLimits(float min, float max);
    void setAutoScale(bool scale);
    
    void setAxisStyle(AxisStyle style);
    void setGridSize(int x, int y);

    void zoom(float zoomFactor, const QPointF &offset);
    void resetZoom();
    void setZoomStepSize(float stepSize);

    void setHeaderText(const QString &text);
    void setHeaderFont(const QFont &font);
    void setHeaderColor(const QColor &color);
    void setFooterText(const QString &text);
    void setFooterFont(const QFont &font);
    void setFooterColor(const QColor &color);
    void setAxisFont(const QFont &font);
    void setAxisTextColor(const QColor &color);

    void setMargins(int margin);
    void setMargins(int left, int top, int right, int bottom);

protected:
    virtual void initializeGL();
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeGL(int width, int height);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    
private:
    void drawAxis();
    void drawGrid();
    void drawText();
    float getScaleFactor();
    float getYOffset();
    void UpdateGrid();
    void CreateGridBuffer();
    void UpdateMargins();
    void CalculateMargins();
    QPointF ToScreenCoords(const QPointF &point);

    QGLShaderProgram m_gridShader;
    QVector<float> m_fvGridVBuffer;
    QGLShaderProgram m_graphShader;

    QColor m_axisColor;
    QColor m_gridColor;
    QColor m_lineColor;
    QColor m_bgColor;
    float m_fAxisLineWidth;
    float m_fGridLineWidth;
    float m_fLineWidth;

    QVector<float> m_xAxis;
    QVector<float> m_yAxis;
    float m_fMin;
    float m_fMax;
    float m_fYMin;
    float m_fYMax;
    float m_fXMin;
    float m_fXMax;
    bool m_bAutoScale;
    bool m_bInitialized;

    int m_iGridSizeX;
    int m_iGridSizeY;

    AxisStyle m_axisStyle;

    QMatrix4x4 m_transformMatrix;
    QMatrix4x4 m_zoomMatrix;
    float m_fZoomStepSize;

    bool m_bRecalcMargins;
    bool m_bUpdateGridBuffer;

    bool m_bHeaderEnabled;
    QString m_sHeaderText;
    QFont m_fntHeaderFont;
    QColor m_cHeaderColor;
    bool m_bFooterEnabled;
    QString m_sFooterText;
    QFont m_fntFooterFont;
    QColor m_cFooterColor;
    QFont m_fntAxisFont;
    QColor m_cAxisTextColor;
    QMargins m_margins;

    QRect m_headerRect;
    QRect m_footerRect;
    QRect m_xAxisRect;
    QRect m_yAxisRect;

};

#endif // GLGRAPHWIDGET_H
