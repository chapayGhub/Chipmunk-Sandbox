#include "canvas.h"
#include "primitives/boxprimitive.h"
#include "primitives/boxprimitivemarker.h"
#include "tools.h"
#include "scene.h"


#include <QPainter>
#include <QMouseEvent>

#include <cmath>

Canvas::Canvas(Tools* tools,Scene* scene,QWidget *parent) :
    QWidget(parent), m_tools(tools),m_scene(scene)
{
    setFixedSize(size());
    m_drawGrid = true;
    m_gridSize = 10;
    m_snapToGrid = false;
    connect(scene,SIGNAL(zoomChanged()),this,SLOT(onZoomChanged()));
}

void Canvas::onZoomChanged() {
    setFixedSize(size());
    repaint();
}


QSize   Canvas::size() const {
    const QSizeF& sz = m_scene->worldSize();
    return QSize( sz.width()*zoom(), sz.height()*zoom() );
}

qreal Canvas::zoom() const {
    return m_scene->zoom();
}


bool Canvas::drawGrid() const {
    return m_drawGrid;
}
void Canvas::setDrawGrid(bool draw) {
    m_drawGrid = draw;
    repaint();
}

bool Canvas::snapToGrid() const {
    return m_snapToGrid;
}

void Canvas::setSnapToGrid(bool snap) {
    m_snapToGrid = snap;
}



void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(),Qt::black);

    qreal z = zoom();
    if (m_drawGrid) {
        painter.setPen(QColor(32,32,32));
        //painter.setPen(QPen(QBrush(QColor(42,42,42)),1,Qt::DotLine));
        for (int x=m_gridSize*z;x<width();x+=m_gridSize*z*2) {
            painter.drawLine(x,0,x,height());
        }
        for (int y=m_gridSize*z;y<height();y+=m_gridSize*z*2) {
            painter.drawLine(0,y,width(),y);
        }
        painter.setPen(QColor(42,42,42));
        for (int x=m_gridSize*z*2;x<width();x+=m_gridSize*z*2) {
            painter.drawLine(x,0,x,height());
        }
        for (int y=m_gridSize*z*2;y<height();y+=m_gridSize*z*2) {
            painter.drawLine(0,y,width(),y);
        }
    }

    painter.setPen(QColor(255,255,255));
    painter.setBrush(QBrush(QColor(0,0,255,64)));
    m_scene->Draw(this,&painter);
    painter.setPen(QColor(255,64,64));
    painter.setBrush(QBrush(QColor(0,0,255,64)));
    m_tools->Draw(this,&painter);


    painter.setPen(QColor(255,64,64));
    painter.setBrush(QBrush(QColor(255,255,255,64)));
    m_scene->DrawSelected(this,&painter);

    painter.setBrush(QBrush(QColor(0,0,0,64)));
    painter.setPen(QColor(255,255,255,128));
    m_scene->DrawMarkers(this,&painter);

    painter.setPen(QColor(255,64,64));
    painter.setBrush(Qt::NoBrush);
    m_scene->DrawSelected(this,&painter);
}

void Canvas::Draw(const BoxPrimitive *box, QPainter* painter) const {
    const QRectF& rect = box->rect();
    qreal z = zoom();
    QRect r = QRect(rect.x()*z,rect.y()*z,rect.width()*z,rect.height()*z);
    painter->drawRect(r);
}

void Canvas::Draw(const BoxPrimitiveMarker *marker, QPainter* painter) const {
    QPointF pos = marker->position();
    QSizeF size = marker->size();
    qreal z = zoom();
    QRect r = QRect( pos.x()*z, pos.y()*z ,size.width()*z,size.height()*z);
    if (marker->xAlign()==PrimitiveMarkerXAlign_Right)
        r.moveLeft(r.left()-r.width());
    else if (marker->xAlign()==PrimitiveMarkerXAlign_Center)
        r.moveLeft(r.left()-r.width()/2);
    if (marker->yAlign()==PrimitiveMarkerYAlign_Bottom)
        r.moveTop(r.top()-r.height());
    else if (marker->yAlign()==PrimitiveMarkerYAlign_Center)
        r.moveTop(r.top()-r.height()/2);

    painter->drawRect(r);
}


void Canvas::processPos(QPointF& pos) const {
    pos*=(1.0f/zoom());
    if (m_snapToGrid) {
        qreal x = round(pos.x()/m_gridSize) * m_gridSize;
        qreal y = round(pos.y()/m_gridSize) * m_gridSize;
        pos.setX(x);
        pos.setY(y);
    }
}
void Canvas::mousePressEvent(QMouseEvent * event) {
    QPointF p = event->posF();
    processPos(p);
    if (m_tools->onMousePress(p)) {
        repaint();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    QPointF p = event->posF();
    processPos(p);
    if (m_tools->onMouseRelease(p)) {
        repaint();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    QPointF p = event->posF();
    processPos(p);
    if (m_tools->onMouseMove(p)) {
        repaint();
    }
}
