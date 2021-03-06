#include "canvas.h"
#include "primitives/boxprimitive.h"
#include "primitives/boxprimitivemarker.h"
#include "tools.h"
#include "scene.h"
#include "primitives/rotateprimitivemarker.h"
#include "primitives/circleprimitive.h"
#include "primitives/circleprimitivemarker.h"
#include "primitives/polygonprimitive.h"
#include "primitives/polygonprimitivemarker.h"
#include "body.h"

#include <QPainter>
#include <QMouseEvent>


#include <cmath>
#ifndef M_PI
#define M_PI 3.14
#endif
Canvas::Canvas(QWidget *parent) :
    QWidget(parent), m_tools(0),m_scene(0)
{
    m_drawGrid = true;
    m_gridSize = 10;
    m_snapToGrid = false;
    m_border = 64;
}

void Canvas::init(Tools* tools,Scene* scene) {
    m_tools = tools;
    m_scene = scene;
    setFixedSize(size());
    connect(scene,SIGNAL(zoomChanged()),this,SLOT(onZoomChanged()));
    setMouseTracking(true);
}

void Canvas::onZoomChanged() {
    setFixedSize(size());
    repaint();
}


QSize   Canvas::size() const {
    const QSizeF& sz = m_scene->worldSize();
    return QSize( sz.width()*zoom()+m_border*2, sz.height()*zoom()+m_border*2 );
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
    QSizeF sz = m_scene->worldSize();
    painter.fillRect(rect(),Qt::black);
    painter.fillRect(0,0,width(),m_border,Qt::lightGray);
    painter.fillRect(0,m_border,m_border,height()-m_border*2,Qt::lightGray);
    painter.fillRect(width()-m_border,m_border,m_border,height()-m_border*2,Qt::lightGray);
    painter.fillRect(0,height()-m_border,width(),m_border,Qt::lightGray);

    painter.setTransform(QTransform().translate(m_border,m_border));
    qreal z = zoom();
    sz*=z;
    if (m_drawGrid) {
        painter.setPen(QColor(32,32,32));
        //painter.setPen(QPen(QBrush(QColor(42,42,42)),1,Qt::DotLine));
        for (int x=m_gridSize*z;x<sz.width();x+=m_gridSize*z*2) {
            painter.drawLine(x,0,x,sz.height()-1);
        }
        for (int y=m_gridSize*z;y<sz.height();y+=m_gridSize*z*2) {
            painter.drawLine(0,y,sz.width()-1,y);
        }
        painter.setPen(QColor(42,42,42));
        for (int x=m_gridSize*z*2;x<sz.width();x+=m_gridSize*z*2) {
            painter.drawLine(x,0,x,sz.height()-1);
        }
        for (int y=m_gridSize*z*2;y<sz.height();y+=m_gridSize*z*2) {
            painter.drawLine(0,y,sz.width()-1,y);
        }
    }

    painter.setPen(QColor(255,255,255));
    painter.setBrush(QBrush(QColor(0,0,255,64)));
    m_scene->Draw(this,&painter);
    painter.setPen(QColor(255,64,64));
    painter.setBrush(QBrush(QColor(0,0,255,64)));
    m_tools->Draw(this,&painter);



}

void Canvas::BeginDraw(const Body* body, QPainter* painter) const {
    painter->save();
    qreal z = zoom();
    painter->translate(body->position()*z);
    painter->rotate( body->angle() * 180.0 / M_PI );
    //painter->drawEllipse(body->toLocal(m_last_mouse_pos)*z,2,2);

}
void Canvas::Draw(const DynamicBody* body, QPainter* painter) const {
    qreal z = zoom();
    qreal r = 10 * z;
    painter->drawEllipse(body->position(),r,r);
}
void Canvas::EndDraw(const Body* body, QPainter* painter) const {
    painter->restore();
}

void Canvas::Draw(const BoxPrimitive *box, QPainter* painter) const {

    qreal a = box->angle();
    qreal z = zoom();

    if (a!=0) {
        QPointF pos = box->position() * z;
        painter->save();
        painter->translate(pos);
        painter->rotate(a * 180.0 / M_PI );
        painter->translate(-pos);
    }

    const QRectF& rect = box->rect();
    QRectF r = QRectF(rect.x()*z,rect.y()*z,rect.width()*z,rect.height()*z);
    painter->drawRect(r);
    QPointF c = rect.center()*z;
    if (r.height()>10)
        painter->drawLine(c.x(),c.y()-5,c.x(),c.y()+5);
    if (r.width()>10)
        painter->drawLine(c.x()-5,c.y(),c.x()+5,c.y());

    if (a!=0) {
        painter->restore();
    }
}

void Canvas::Draw(const CirclePrimitive* circle, QPainter* painter) const {
    qreal z = zoom();
    QPointF pos = circle->position() * z;
    qreal r = circle->r()*z;
    painter->drawEllipse(pos,r,r);
    if (r>10) {
        painter->drawLine(pos.x(),pos.y()-5,pos.x(),pos.y()+5);
        painter->drawLine(pos.x()-5,pos.y(),pos.x()+5,pos.y());
    }
}

void Canvas::Draw(const PolygonPrimitive* polygon, QPainter* painter, bool creating) const {
    qreal z = zoom();
    QPointF pos = polygon->position() * z;
    QSizeF sz = polygon->size()*z;

    QVector<QPointF> points = polygon->points();
    for (int i=0;i<points.size();i++) {
        points[i]*=z;
    }

    if (creating) {
        painter->drawPolyline(QPolygonF(points));
        if (polygon->pointsAmount()>1) {
            painter->drawEllipse(polygon->point(0)*z,polygon->connectWidth()*z,polygon->connectWidth()*z);
        }
    } else {
        painter->drawPolygon(QPolygonF(points));
        const PolygonPrimitive::Diagonals& diags = polygon->diagonals();
        for (int i=0;i<diags.size();i++) {
            painter->drawLine(points[diags[i].first],points[diags[i].second]);
        }
    }

    if (sz.width()>10 && sz.height()>10) {
        painter->drawLine(pos.x(),pos.y()-5,pos.x(),pos.y()+5);
        painter->drawLine(pos.x()-5,pos.y(),pos.x()+5,pos.y());
    }
}

void Canvas::Draw(const PolygonPrimitiveMarker* marker, QPainter* painter) const {
    qreal z = zoom();
    QPointF pos = marker->position() * z;
    painter->drawEllipse(pos,marker->width()*z,marker->width()*z);
}

void Canvas::Draw(const CirclePrimitiveMarker* marker, QPainter* painter) const {
    qreal z = zoom();
    QPointF pos = marker->primitive()->position() * z;
    qreal r1 = marker->primitive()->r()*z;
    qreal r2 = r1;
    if (marker->xAlign()==PrimitiveMarkerXAlign_Right) {
        r2 = r1 + marker->width()*z;
    } else if (marker->xAlign()==PrimitiveMarkerXAlign_Left) {
        r1 = r2 - marker->width()*z;
    }
    QPainterPath path1;
    path1.addEllipse(pos,r1,r1);
    QPainterPath path2;
    path2.addEllipse(pos,r2,r2);
    painter->drawPath(path2.subtracted(path1));
}

void Canvas::Draw(const BoxPrimitiveMarker *marker, QPainter* painter) const {
    const BoxPrimitive* box = marker->primitive();
    qreal z = zoom();
    qreal a = box->angle();
    if (a!=0) {
        QPointF pos = marker->position() * z;
        painter->save();
        painter->translate(pos);
        painter->rotate(a * 180.0 / M_PI );
        painter->translate(-pos);
    }
    QPointF pos = marker->position();
    QSizeF size = marker->size();
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

    if (a!=0) {
        painter->restore();
    }
}

void Canvas::Draw(const RotatePrimitiveMarker* marker, QPainter* painter) const {
    qreal z = zoom();
    QPointF pos = marker->position()*z;
    qreal r = marker->radius() * z;
    painter->drawEllipse(pos,r,r);

    if (marker->activated()) {
        painter->drawLine(pos,marker->startPoint()*z);
        painter->drawLine(pos,marker->endPoint()*z);
    }
}

void Canvas::processPos(QPointF& pos) const {
    pos-=QPointF(m_border,m_border);
    pos*=(1.0f/zoom());
    if (m_snapToGrid) {
        qreal x = floorf(pos.x()/m_gridSize) * m_gridSize;
        qreal y = floorf(pos.y()/m_gridSize) * m_gridSize;
        pos.setX(x);
        pos.setY(y);
    }
}
void Canvas::mousePressEvent(QMouseEvent * event) {
    QPointF p = event->localPos();
    processPos(p);
    m_last_mouse_pos = p;
    if (m_tools->onMousePress(p)) {
        repaint();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    QPointF p = event->localPos();
    processPos(p);
    m_last_mouse_pos = p;
    if (m_tools->onMouseRelease(p)) {
        repaint();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    QPointF p = event->localPos();
    processPos(p);
    m_last_mouse_pos = p;
    if (m_tools->onMouseMove(p)) {
        repaint();
    }
}
