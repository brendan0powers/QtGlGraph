#include "glgraphwidget.h"
#include <QDebug>
#include "float.h"

GlGraphWidget::GlGraphWidget(QWidget *parent)
   : QGLWidget(parent)
   , m_axisColor(QColor::fromRgb(255,255,255,255))
   , m_gridColor(QColor::fromRgb(100,100,100))
   , m_lineColor(QColor::fromRgb(255,0,0))
   , m_bgColor(QColor::fromRgb(0,0,0))
   , m_fAxisLineWidth(2)
   , m_fGridLineWidth(1)
   , m_fLineWidth(1)
   , m_iTextureId(-1)
   , m_fMin(0)
   , m_fMax(0)
   , m_fYMin(-1)
   , m_fYMax(1)
   , m_bAutoScale(true)
   , m_bInitialized(false)
   , m_iGridSizeX(10)
   , m_iGridSizeY(10)
   , m_bLeftAlignedAxis(true)
{
    setAutoFillBackground(false);
    m_transformMatrix.setToIdentity();
    m_transformMatrix.scale(0.9);
}

void GlGraphWidget::setGridColor(const QColor &color)
{
    m_gridColor = color;
}

void GlGraphWidget::setLineColor(const QColor &color)
{
    m_lineColor = color;
}

void GlGraphWidget::setBgColor(const QColor &color)
{
    m_bgColor = color;
}

void GlGraphWidget::setLineWidth(float width)
{
    m_fLineWidth = width;
}

void GlGraphWidget::setGridLineWidth(float width)
{
    m_fGridLineWidth = width;
}

void GlGraphWidget::setAxisLineWidth(float width)
{
    m_fAxisLineWidth = width;
}

void GlGraphWidget::setLeftAlignedAxis(bool left)
{
    m_bLeftAlignedAxis = left;
    UpdateGridBuffer();
}

void GlGraphWidget::setData(const QVector<float> &data)
{
    //init x axis
    if(m_xAxis.size() != data.size())
    {
        float curX = -1;
        float stepSize = (float)2/(float)data.size();

        m_xAxis.resize(data.size());

        for(int i = 0; i < data.size(); i++)
        {
            //qDebug() << i << curX;
            m_xAxis[i] = curX;
            curX += stepSize;
        }

        //qDebug() << "LastX" << curX;
    }

    m_yAxis = data;
    m_fMax = FLT_MIN_EXP;
    m_fMin = FLT_MAX_EXP;

    for(int i = 0; i < data.size(); i++)
    {
        float tmp = data.constData()[i];
        if(tmp < m_fMin)
            m_fMin = tmp;
        if(tmp > m_fMax)
            m_fMax = tmp;
    }

    if(m_bInitialized)
    {
        update();
    }
}

void GlGraphWidget::setDataLimits(float min, float max)
{
    m_fYMin = min;
    m_fYMax = max;
    m_bAutoScale = false;
}

void GlGraphWidget::setAutoScale(bool scale)
{
    m_bAutoScale = scale;

    if(scale)
    {
        m_fYMin = -1.0;
        m_fYMax = 1.0;
    }
}

void GlGraphWidget::setGridSize(int x, int y)
{
    m_iGridSizeX = x;
    m_iGridSizeY = y;

    UpdateGridBuffer();
}

void GlGraphWidget::initializeGL()
{
    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    m_graphShader.addShaderFromSourceFile(QGLShader::Vertex, ":/graphshader.vert");
    m_graphShader.addShaderFromSourceFile(QGLShader::Fragment, ":/graphshader.frag");
    m_graphShader.link();
    m_graphShader.bind();

    m_gridShader.addShaderFromSourceFile(QGLShader::Vertex, ":/gridshader.vert");
    m_gridShader.addShaderFromSourceFile(QGLShader::Fragment, ":/gridshader.frag");
    m_gridShader.link();
    m_gridShader.bind();

    m_bInitialized = true;

    //If we have data, then set up the buffers
    if(m_yAxis.size() != 0)
        setData(m_yAxis);
}

void GlGraphWidget::paintEvent(QPaintEvent *event)
{
    makeCurrent();

    qglClearColor(m_bgColor);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawGrid();

    m_graphShader.bind();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_iTextureId);

    m_graphShader.setUniformValue("transform", m_transformMatrix);
    m_graphShader.setUniformValue("texture", 0);
    m_graphShader.setUniformValue("lineColor", m_lineColor);
    m_graphShader.setUniformValue("scaleFactor", getScaleFactor());
    m_graphShader.setUniformValue("yOffset", getYOffset());
    m_graphShader.enableAttributeArray("xAxis");
    m_graphShader.setAttributeArray("xAxis", (GLfloat *)m_xAxis.constData(), 1, 0);
    m_graphShader.enableAttributeArray("yAxis");
    m_graphShader.setAttributeArray("yAxis", (GLfloat *)m_yAxis.constData(), 1, 0);

    glLineWidth(m_fLineWidth);
    glDrawArrays(GL_LINE_STRIP, 0, m_xAxis.size());

    glLineWidth(1);

    swapBuffers();
}

void GlGraphWidget::drawGrid()
{

    m_gridShader.bind();
    m_gridShader.enableAttributeArray("vertex");
    m_gridShader.setUniformValue("transform", m_transformMatrix);
    m_gridShader.setAttributeArray("vertex", (GLfloat *)m_fvGridVBuffer.constData(), 2, 0);

    if(m_fvGridVBuffer.size() > 8)
    {
        //Draw grid
        m_gridShader.setUniformValue("lineColor", m_gridColor);
        glLineWidth(m_fGridLineWidth);
        glDrawArrays(GL_LINES, 4, (2*(m_iGridSizeX + 1)) + (2*(m_iGridSizeY + 1)));
    }

    //Draw Axis
    m_gridShader.setUniformValue("lineColor", m_axisColor);
    glLineWidth(m_fAxisLineWidth);
    glDrawArrays(GL_LINES, 0, 4);


    m_gridShader.disableAttributeArray("vertex");
    m_gridShader.release();
}

void GlGraphWidget::resizeGL(int width, int height)
{
    qDebug() << "resizeGL" << width << height;
    glViewport(0,0, width, height);
}

void GlGraphWidget::mousePressEvent(QMouseEvent *event)
{

}

void GlGraphWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void GlGraphWidget::showEvent(QShowEvent *event)
{

}

float GlGraphWidget::getScaleFactor()
{
    float dataWidth = m_fMax - m_fMin;
    float scaleWidth = m_fYMax - m_fYMin;

    if(m_bAutoScale)
        return 2/dataWidth;
    else
        return 2/scaleWidth;
}

float GlGraphWidget::getYOffset()
{
    float scaleFactor = getScaleFactor();
    float scaledMax = m_fYMax * scaleFactor;
    float scaledMin = m_fYMin * scaleFactor;

    return (scaledMin + ((scaledMax - scaledMin) / 2.0)) * -1;
}

void GlGraphWidget::UpdateGridBuffer()
{
    m_fvGridVBuffer.reserve(1024);
    m_fvGridVBuffer.clear();

    //Set up axis
    if(m_bLeftAlignedAxis)
    {
        m_fvGridVBuffer.append(-1.01);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(-1.01);
        m_fvGridVBuffer.append(1);

        m_fvGridVBuffer.append(-1.01);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(1);
        m_fvGridVBuffer.append(-1);
    }
    else
    {
        m_fvGridVBuffer.append(1.01);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(1.01);
        m_fvGridVBuffer.append(1);

        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(1.01);
        m_fvGridVBuffer.append(-1);
    }



    if(m_iGridSizeX == 0 || m_iGridSizeY == 0)
    {
        m_fvGridVBuffer.resize(8);
        return;
    }

    float gridWidth = 2.0/(float)m_iGridSizeX;
    float gridHeight = 2.0/(float)m_iGridSizeY;
    float startX = -1.0;
    float startY = -1.0;

    //X Grid Lines
    for(int i = 0; i <= m_iGridSizeX; i++)
    {
        m_fvGridVBuffer.append(startX);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(startX);
        m_fvGridVBuffer.append(1);

        qDebug() << QPointF(startX, -1) << QPointF(startX, 1);

        startX += gridWidth;
    }

    //Y Grid Lines
    for(int i = 0; i <= m_iGridSizeY; i++)
    {
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(startY);
        m_fvGridVBuffer.append(1);
        m_fvGridVBuffer.append(startY);

        qDebug() << QPointF(startX, -1) << QPointF(startX, 1);

        startY += gridHeight;
    }
}
