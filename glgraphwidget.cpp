#include "glgraphwidget.h"
#include <QDebug>
#include <QPointF>
#include <QMouseEvent>
#include "float.h"
#include "math.h"

GlGraphWidget::GlGraphWidget(QWidget *parent)
   : QGLWidget(parent)
   , m_axisColor(QColor::fromRgb(255,255,255,255))
   , m_gridColor(QColor::fromRgb(100,100,100))
   , m_lineColor(QColor::fromRgb(255,0,0))
   , m_bgColor(QColor::fromRgb(0,0,0))
   , m_fAxisLineWidth(2)
   , m_fGridLineWidth(1)
   , m_fLineWidth(1)
   , m_fMin(0)
   , m_fMax(0)
   , m_fYMin(-1)
   , m_fYMax(1)
   , m_bAutoScale(true)
   , m_bInitialized(false)
   , m_iGridSizeX(10)
   , m_iGridSizeY(10)
   , m_axisStyle(LeftAxis)
   , m_fZoomStepSize(0.1)
{
    setAutoFillBackground(false);
    m_transformMatrix.setToIdentity();
    m_transformMatrix.scale(0.9);
    m_zoomMatrix.setToIdentity();
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

void GlGraphWidget::setAxisStyle(AxisStyle style)
{
    m_axisStyle = style;
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
            m_xAxis[i] = curX;
            curX += stepSize;
        }
    }

    //Find limits of Y axis
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

    //Request an update
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

void GlGraphWidget::zoom(float zoomFactor, const QPointF &offset)
{
    m_zoomMatrix.translate(offset.x(), offset.y());
    m_zoomMatrix.scale(zoomFactor);
    m_zoomMatrix.translate(offset.x() * -zoomFactor, offset.y() * -zoomFactor);
}

void GlGraphWidget::resetZoom()
{
    m_zoomMatrix.setToIdentity();
}

void GlGraphWidget::setZoomStepSize(float stepSize)
{
    m_fZoomStepSize = stepSize;
}

void GlGraphWidget::initializeGL()
{
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
}

void GlGraphWidget::paintEvent(QPaintEvent *event)
{
    makeCurrent(); //Make the GL context current

    qglClearColor(m_bgColor);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawGrid();

    //Set up the graph shader
    m_graphShader.bind();
    m_graphShader.setUniformValue("transform", m_transformMatrix);
    m_graphShader.setUniformValue("zoom", m_zoomMatrix);
    m_graphShader.setUniformValue("texture", 0);
    m_graphShader.setUniformValue("lineColor", m_lineColor);
    m_graphShader.setUniformValue("scaleFactor", getScaleFactor());
    m_graphShader.setUniformValue("yOffset", getYOffset());
    m_graphShader.enableAttributeArray("xAxis");
    m_graphShader.setAttributeArray("xAxis", (GLfloat *)m_xAxis.constData(), 1, 0);
    m_graphShader.enableAttributeArray("yAxis");
    m_graphShader.setAttributeArray("yAxis", (GLfloat *)m_yAxis.constData(), 1, 0);


    //Calculate the clippring region
    QPointF bottomLeft = m_transformMatrix.map(QPointF(-1,-1));
    QPointF topRight = m_transformMatrix.map(QPointF(1,1));
    bottomLeft.setX((bottomLeft.x() + 1) * width()/2);
    bottomLeft.setY((bottomLeft.y() + 1) * height()/2);
    topRight.setX((topRight.x() + 1) * width()/2);
    topRight.setY((topRight.y() + 1) * height()/2);

    //Enable clipping
    glEnable(GL_SCISSOR_TEST);
    glScissor(bottomLeft.x(), bottomLeft.y(), topRight.x() - bottomLeft.x(), topRight.y() - bottomLeft.y());

    //Draw the graph
    glLineWidth(m_fLineWidth);
    glDrawArrays(GL_LINE_STRIP, 0, m_xAxis.size());

    //Clean up
    glDisable(GL_SCISSOR_TEST);
    glLineWidth(1);

    swapBuffers();
}

void GlGraphWidget::drawGrid()
{
    m_gridShader.bind();
    m_gridShader.setUniformValue("transform", m_transformMatrix);
    m_gridShader.enableAttributeArray("vertex");
    m_gridShader.setAttributeArray("vertex", (GLfloat *)m_fvGridVBuffer.constData(), 2, 0);

    //The offset for drawing the grid is 4 if we are drawing an axis, 0 otherwise
    int bufferStart = 4;
    if(m_axisStyle == NoAxis)
        bufferStart = 0;

    if(m_fvGridVBuffer.size() > 8)
    {
        //Draw grid
        m_gridShader.setUniformValue("lineColor", m_gridColor);
        glLineWidth(m_fGridLineWidth);
        glDrawArrays(GL_LINES, bufferStart, (2*(m_iGridSizeX + 1)) + (2*(m_iGridSizeY + 1)));
    }

    if(m_axisStyle != NoAxis)
    {
        //Draw Axis
        m_gridShader.setUniformValue("lineColor", m_axisColor);
        glLineWidth(m_fAxisLineWidth);
        glDrawArrays(GL_LINES, 0, 4);
    }


    m_gridShader.disableAttributeArray("vertex");
    m_gridShader.release();
}

void GlGraphWidget::resizeGL(int width, int height)
{
    //qDebug() << "resizeGL" << width << height;
    glViewport(0,0, width, height);
}

void GlGraphWidget::mousePressEvent(QMouseEvent *event)
{

}

void GlGraphWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void GlGraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF pos((((float)event->pos().x()/width()) * 2.0) - 1.0,(((float)event->pos().y()/height()) * 2.0) - 1.0);
    pos = m_transformMatrix.inverted().map(pos);
    //pos.setX(pos.x() * -1);
    pos.setY(pos.y() * -1);
    if(fabs((float)pos.x()) > 1 || fabs((float)pos.y()) >1)
        return;

    if(event->button() == Qt::LeftButton)
    {
        zoom(1.0 + m_fZoomStepSize, pos);
    }
    else if(event->button() == Qt::RightButton)
    {
        zoom(1.0 - m_fZoomStepSize, QPointF(0,0));
    }
    else if(event->button() == Qt::MiddleButton)
    {
        resetZoom();
    }
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
    if(m_axisStyle == LeftAxis)
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
    else if(m_axisStyle == RightAxis)
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

        startX += gridWidth;
    }

    //Y Grid Lines
    for(int i = 0; i <= m_iGridSizeY; i++)
    {
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(startY);
        m_fvGridVBuffer.append(1);
        m_fvGridVBuffer.append(startY);

        startY += gridHeight;
    }
}
