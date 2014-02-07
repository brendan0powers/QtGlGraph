#include "glgraphwidget.h"
#include <QDebug>
#include <QPointF>
#include <QMouseEvent>
#include "float.h"
#include "math.h"

#define TEXT_MARGIN 10

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
   , m_fZoomStepSize((float)0.1)
   , m_bRecalcMargins(true)
   , m_bUpdateGridBuffer(true)
   , m_bHeaderEnabled(false)
   , m_sHeaderText("")
   , m_fntHeaderFont(QFont("Arial", 12))
   , m_cHeaderColor(QColor::fromRgb(255,255,255,255))
   , m_bFooterEnabled(false)
   , m_sFooterText("")
   , m_fntFooterFont(QFont("Arial", 8))
   , m_cFooterColor(QColor::fromRgb(255,255,255,255))
   , m_fntAxisFont(QFont("Arial", 9))
   , m_cAxisTextColor(QColor::fromRgb(255,255,255,255))
   , m_margins(QMargins(20,10,20,10))
{
    setAutoFillBackground(false);
    m_transformMatrix.setToIdentity();
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
    UpdateGrid();
    UpdateMargins();
}

void GlGraphWidget::setData(const QVector<float> &data)
{
    //init x axis
    if(m_xAxis.size() != data.size())
    {
        float curX = -1.0;
        float stepSize = (float)2.0/(float)data.size();

        m_xAxis.resize(data.size());

        for(int i = 0; i < data.size(); i++)
        {
            curX += stepSize;
            m_xAxis[i] = curX;
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

void GlGraphWidget::setYAxisLimits(float min, float max)
{
    m_fYMin = min;
    m_fYMax = max;
    m_bAutoScale = false;
}

void GlGraphWidget::setXAxisLimits(float min, float max)
{
    m_fXMin = min;
    m_fXMax = max;
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

    UpdateGrid();
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
    Q_UNUSED(event)
    makeCurrent(); //Make the GL context current

    CalculateMargins();
    CreateGridBuffer();

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
    glDisable(GL_SCISSOR_TEST);

    drawAxis();

    //Clean up
    glLineWidth(1);

    drawText();
}

void GlGraphWidget::drawAxis()
{
    if(m_axisStyle == NoAxis)
        return;

    m_gridShader.bind();
    m_gridShader.setUniformValue("transform", m_transformMatrix);
    m_gridShader.enableAttributeArray("vertex");
    m_gridShader.setAttributeArray("vertex", (GLfloat *)m_fvGridVBuffer.constData(), 2, 0);

    //Draw Axis
    m_gridShader.setUniformValue("lineColor", m_axisColor);
    glLineWidth(m_fAxisLineWidth);
    glDrawArrays(GL_LINES, 0, 4);

    m_gridShader.disableAttributeArray("vertex");
    m_gridShader.release();
}

void GlGraphWidget::drawGrid()
{
    if(m_fvGridVBuffer.size() <= 8)
        return;

    m_gridShader.bind();
    m_gridShader.setUniformValue("transform", m_transformMatrix);
    m_gridShader.enableAttributeArray("vertex");
    m_gridShader.setAttributeArray("vertex", (GLfloat *)m_fvGridVBuffer.constData(), 2, 0);

    //The offset for drawing the grid is 4 if we are drawing an axis, 0 otherwise
    int bufferStart = 4;
    if(m_axisStyle == NoAxis)
        bufferStart = 0;

    //Draw grid
    m_gridShader.setUniformValue("lineColor", m_gridColor);
    glLineWidth(m_fGridLineWidth);
    glDrawArrays(GL_LINES, bufferStart, (2*(m_iGridSizeX + 1)) + (2*(m_iGridSizeY + 1)));

    m_gridShader.disableAttributeArray("vertex");
    m_gridShader.release();
}

void GlGraphWidget::drawText()
{
    QPainter p(this);
    p.beginNativePainting();

    if(m_bHeaderEnabled)
    {
        p.setFont(m_fntHeaderFont);
        p.setPen(m_cHeaderColor);
        p.drawText(m_headerRect, Qt::AlignCenter, m_sHeaderText);
        //p.fillRect(m_headerRect, QColor::fromRgb(255,0,0));
    }

    if(m_bFooterEnabled)
    {
        p.setFont(m_fntFooterFont);
        p.setPen(m_cFooterColor);
        p.drawText(m_footerRect, Qt::AlignLeft | Qt::AlignVCenter, m_sFooterText);
        //p.fillRect(m_footerRect, QColor::fromRgb(255,0,0));
    }

    if(m_axisStyle != NoAxis)
    {
        p.setFont(m_fntAxisFont);
        p.setPen(m_cAxisTextColor);

        QPoint textPos = m_yAxisRect.topLeft();
        QFontMetrics metrics(m_fntAxisFont);
        int textSpacing = m_yAxisRect.height() / m_iGridSizeY;
        int height = metrics.ascent() - metrics.descent();

        float yMin = m_fMin, yMax = m_fMax;
        if(!m_bAutoScale)
        {
            yMin = m_fYMin;
            yMax = m_fYMax;
        }

        float numStart = yMax;
        float numSpacing = (yMax - yMin) / m_iGridSizeY;

        for(int i = 0; i <= m_iGridSizeY; i++)
        {
            QPoint drawPoint = textPos;
            drawPoint.setY(drawPoint.y() + (height/2));

            p.drawText(drawPoint, QString::number(numStart, 'f', 4));
            textPos.setY(textPos.y() + textSpacing);
            numStart -= numSpacing;
        }

        // X AXIS
        textPos = m_xAxisRect.bottomLeft();
        textSpacing = m_xAxisRect.width() / m_iGridSizeY;
        numStart = m_fXMin;
        numSpacing = (m_fXMax - m_fXMin) / m_iGridSizeY;

        for(int i = 0; i <= m_iGridSizeY; i++)
        {
            QString text = QString::number(numStart, 'f', 0);
            QPoint drawPoint = textPos;
            drawPoint.setX(drawPoint.x() - (metrics.width(text)/2));

            p.drawText(drawPoint, text);
            textPos.setX(textPos.x() + textSpacing);
            numStart += numSpacing;
        }

        //p.fillRect(m_yAxisRect, QColor::fromRgb(255,0,0));
        //p.fillRect(m_xAxisRect, QColor::fromRgb(255,0,0));
    }


    p.endNativePainting();
}

void GlGraphWidget::resizeGL(int width, int height)
{
    //qDebug() << "resizeGL" << width << height;
    glViewport(0,0, width, height);
    UpdateMargins();
}

void GlGraphWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void GlGraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
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

void GlGraphWidget::UpdateGrid()
{
    m_bUpdateGridBuffer = true;
}

void GlGraphWidget::CreateGridBuffer()
{
    if(!m_bUpdateGridBuffer)
        return;

    m_bUpdateGridBuffer = false;

    m_fvGridVBuffer.reserve(1024);
    m_fvGridVBuffer.clear();

    //Set up axis
    if(m_axisStyle == LeftAxis)
    {
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(1);

        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(1);
        m_fvGridVBuffer.append(-1);
    }
    else if(m_axisStyle == RightAxis)
    {
        m_fvGridVBuffer.append(1);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(1);
        m_fvGridVBuffer.append(1);

        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(-1);
        m_fvGridVBuffer.append(1);
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

void GlGraphWidget::UpdateMargins()
{
    m_bRecalcMargins = true;
}

void GlGraphWidget::CalculateMargins()
{
    if(!m_bRecalcMargins)
        return;

    m_bRecalcMargins = false;

    m_transformMatrix.setToIdentity();

    QMargins margins = m_margins;

    if(m_bHeaderEnabled)
    {
        QFontMetrics metrics(m_fntHeaderFont);
        margins.setTop(margins.top() + metrics.height() + TEXT_MARGIN);

        m_headerRect = QRect(m_margins.left(), m_margins.top(), width() - (m_margins.right() + m_margins.left()), metrics.height());
    }

    if(m_bFooterEnabled)
    {
        QFontMetrics metrics(m_fntFooterFont);
        margins.setBottom(margins.bottom() + metrics.height() + TEXT_MARGIN);

        m_footerRect = QRect(m_margins.left(), height() - (margins.bottom() - TEXT_MARGIN), width() - (m_margins.right() + m_margins.left()), metrics.height());
    }

    //X axis
    if(m_axisStyle != NoAxis)
    {
        QFontMetrics metrics(m_fntAxisFont);
        margins.setBottom(margins.bottom() + metrics.height());

        m_xAxisRect = QRect(m_margins.left(), height() - (margins.bottom()), width() - (m_margins.right() + m_margins.left()), metrics.height());
    }

    if(m_axisStyle == LeftAxis)
    {
        QFontMetrics metrics(m_fntAxisFont);
        margins.setLeft(margins.left() + metrics.width("-0.0000") + TEXT_MARGIN);

        m_yAxisRect = QRect(m_margins.left(), margins.top(), metrics.width("0.0000"), height() - (margins.top() + margins.bottom()));
        m_xAxisRect.setLeft(margins.left());
    }
    else if(m_axisStyle == RightAxis)
    {
        QFontMetrics metrics(m_fntFooterFont);
        margins.setRight(margins.right() + metrics.width("-0.0000") + TEXT_MARGIN);

        m_yAxisRect = QRect(width() - (margins.right() - TEXT_MARGIN), margins.top(), metrics.width("0.0000"), height() - (margins.top() + margins.bottom()));
        m_xAxisRect.setRight(margins.right());
    }

    QRect screen = QRect(QPoint(0,0), size());
    QRect marginRect = screen.marginsRemoved(margins);
    QPointF translate = ToScreenCoords(screen.center()) - ToScreenCoords(marginRect.center());
    QSizeF scale;
    scale.setWidth((float)marginRect.width() / screen.width());
    scale.setHeight((float)marginRect.height() / screen.height());

    m_transformMatrix.translate(translate.x(), translate.y());
    m_transformMatrix.scale(scale.width(), scale.height());
}

QPointF GlGraphWidget::ToScreenCoords(const QPointF &point)
{
    QPointF result;
    result.setX((point.x() * (-2.0/width())) + 1.0);
    result.setY((point.y() * (2.0/height())) - 1.0);
    return result;
}

void GlGraphWidget::setHeaderText(const QString &text)
{
    m_sHeaderText = text;
    m_bHeaderEnabled = !text.isEmpty();
    UpdateMargins();
}

void GlGraphWidget::setHeaderFont(const QFont &font)
{
    m_fntHeaderFont = font;
    UpdateMargins();
}

void GlGraphWidget::setHeaderColor(const QColor &color)
{
    m_cHeaderColor = color;
    UpdateMargins();
}

void GlGraphWidget::setFooterText(const QString &text)
{
    m_sFooterText = text;
    m_bFooterEnabled = !text.isEmpty();
    UpdateMargins();
}

void GlGraphWidget::setFooterFont(const QFont &font)
{
    m_fntFooterFont = font;
    UpdateMargins();
}

void GlGraphWidget::setFooterColor(const QColor &color)
{
    m_cFooterColor = color;
    UpdateMargins();
}

void GlGraphWidget::setAxisFont(const QFont &font)
{
    m_fntAxisFont = font;
    UpdateMargins();
}

void GlGraphWidget::setAxisTextColor(const QColor &color)
{
    m_cAxisTextColor = color;
    UpdateMargins();
}

void GlGraphWidget::setMargins(int margin)
{
    m_margins = QMargins(margin, margin, margin, margin);
    UpdateMargins();
}

void GlGraphWidget::setMargins(int left, int top, int right, int bottom)
{
    m_margins = QMargins(left, top, right, bottom);
    UpdateMargins();
}
