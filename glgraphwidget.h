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
    explicit GlGraphWidget(QWidget *parent = 0);

    void setGridColor(const QColor &color);
    void setLineColor(const QColor &color);
    void setBgColor(const QColor &color);
    void setLineWidth(float width);
    void setAxisLineWidth(float width);
    void setGridLineWidth(float width);

    void setData(const QVector<float> &data);
    void setDataLimits(float min, float max);
    void setAutoScale(bool scale);
    
    void setLeftAlignedAxis(bool left);

    void setGridSize(int x, int y);

protected:
    virtual void initializeGL();
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeGL(int width, int height);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void showEvent(QShowEvent *event);
    
private:
    void drawGrid();
    float getScaleFactor();
    float getYOffset();
    void UpdateGridBuffer();

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
    GLuint m_iTextureId;
    float m_fMin;
    float m_fMax;
    float m_fYMin;
    float m_fYMax;
    bool m_bAutoScale;
    bool m_bInitialized;

    int m_iGridSizeX;
    int m_iGridSizeY;

    bool m_bLeftAlignedAxis;

    QMatrix4x4 m_transformMatrix;
};

#endif // GLGRAPHWIDGET_H
