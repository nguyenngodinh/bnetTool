/**
 * \author trungvv1
 *
 * \date 3/26/2015
 * \class MyBayesEdge
 *
 * \brief write something about your class
 *
 *
 */

#include "qbayesedge.h"

#include <QtGui>
#include <QDebug>

#include "qbayesnode.h"

QBayesEdge::QBayesEdge(QBayesNode* parentNode, QBayesNode* childNode, QPointF p1, QPointF p2)
    : parentNode(parentNode), childNode(childNode), parentPoint(p1), childPoint(p2)
{
    isSelected = false;
    setLine();

    this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
}

QBayesEdge::~QBayesEdge()
{

}

/**
 * @brief MyBayesEdge::setLine
 * @param p1
 * @param p2
 * @param R - radius of nodes
 */
void QBayesEdge::setLine()
{
    this->parentPoint = parentPoint;
    this->childPoint = childPoint;
    double x1, y1, x2, y2;
    double arrowX1, arrowY1, arrowX2, arrowY2;

    if (parentPoint.x() == childPoint.x()) {
        x1 = x2 = parentPoint.x();
        if (parentPoint.y() < childPoint.y()) {
            y1 = parentPoint.y() + QBayesNode::RADIUS;
            y2 = childPoint.y() - QBayesNode::RADIUS;
        } else {
            y1 = parentPoint.y() - QBayesNode::RADIUS;
            y2 = childPoint.y() + QBayesNode::RADIUS;
        }
    } else {
        double k, m;
        k = ( childPoint.y() - parentPoint.y() ) / ( childPoint.x() - parentPoint.x() );
        m = parentPoint.y() - k*parentPoint.x();

        double adder = QBayesNode::RADIUS / sqrt(1+k*k);
        if (childPoint.x() < parentPoint.x()) {
            adder *= -1;
        }

        x1 = parentPoint.x() + adder;
        x2 = childPoint.x() - adder;
        y1 = k*x1 + m;
        y2 = k*x2 + m;

        adder *= (ARROW_WIDTH/QBayesNode::RADIUS);
        double px = x2 - adder;
        double py = k * px + m;
        double k1 = -1 / k;
        double m1 = py - k1*px;
        double adder1 = ARROW_WIDTH / sqrt(1+k1*k1);
        arrowX1 = px - adder1;
        arrowX2 = px + adder1;
        arrowY1 = k1*arrowX1 + m1;
        arrowY2 = k1*arrowX2 + m1;
    }
    start = QPointF(x1, y1);
    end = QPointF(x2, y2);
    arrow1 = QPointF(arrowX1, arrowY1);
    arrow2 = QPointF(arrowX2, arrowY2);

    prepareGeometryChange();

    qDebug() << parentPoint.x() << ";" << parentPoint.y() << endl;
    qDebug() << childPoint.x() << ";" << childPoint.y() << endl;
    qDebug() << x1 << ";" << y1 << endl;
    qDebug() << x2 << ";" << y2 << endl;
    qDebug() << endl;

}

QBayesNode *QBayesEdge::getParentNode() const
{
    return parentNode;
}

QBayesNode *QBayesEdge::getChildNode() const
{
    return childNode;
}

QString QBayesEdge::toString() const
{
    QString rs = "";

    rs += QString::number(parentNode->getIndex());
    rs += "\t";
    rs += QString::number(childNode->getIndex());
    rs += "\t";
//    rs += QString::number(parentPoint.x());
//    rs += "\t";
//    rs += QString::number(parentPoint.y());
//    rs += "\t";
//    rs += QString::number(childPoint.x());
//    rs += "\t";
//    rs += QString::number(childPoint.y());
//    rs += "\t";

    return rs;
}

void QBayesEdge::onParentNodePositionChanged(QPointF value)
{
    parentPoint = value;
    setLine();
}

void QBayesEdge::onChildNodePositionChanged(QPointF value)
{
    childPoint = value;
    setLine();
}

void QBayesEdge::onNodeDeleted()
{
    /// nothing to do 'cause graphicscene is responsible for this
}

void QBayesEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (isSelected)
        painter->setPen(QPen(Qt::red));
    else
        painter->setPen(QPen(Qt::blue));

    painter->drawLine(start, end);
    painter->setBrush(QBrush(painter->pen().color()));
    painter->drawPolygon(QPolygonF(QVector<QPointF>() << end << arrow1 << arrow2));
    painter->setBrush(Qt::NoBrush);
//    painter->drawRect(boundingRect());
}

QVariant QBayesEdge::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSelectedChange:
        isSelected = value.toBool();
        break;
    default:
        break;
    }
    return QGraphicsItem::itemChange(change, value);
}

void QBayesEdge::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        emit edgeDeleted();
    }
    return QGraphicsItem::keyReleaseEvent(event);
}

QPainterPath QBayesEdge::shape()
{
    static const qreal kClickTolerance = 2;

    QPointF vec = end-start;
    double length = sqrt( vec.x()*vec.x() + vec.y()*vec.y() );
    vec = vec * (kClickTolerance/length);
    QPointF orthogonal(vec.y(), -vec.x());

    QPainterPath result(start-vec+orthogonal);
    result.lineTo(start-vec-orthogonal);
    result.lineTo(end+vec-orthogonal);
    result.lineTo(end+vec+orthogonal);
    result.closeSubpath();

    return result;
}

QRectF QBayesEdge::boundingRect() const
{
    static const qreal WIDTH = 10;

    if (start.x() == end.x()) {
        if (start.y() < end.y()) {
            return QRectF(start.x()-WIDTH, start.y(), 2*WIDTH, end.y()-start.y());
        } else {
            return QRectF(end.x()-WIDTH, end.y(), 2*WIDTH, -end.y()+start.y());
        }
    } else if (start.y() == end.y()) {
        if (start.x() < end.x()) {
            return QRectF(start.x(), start.y()-WIDTH, end.x()-start.x(), 2*WIDTH);
        } else {
            return QRectF(end.x(), end.y()-WIDTH, -end.x()+start.x(), 2*WIDTH);
        }
    }

//    QPoint dp(WIDTH, WIDTH);
    return QRectF(start, end).normalized().adjusted(-WIDTH,-WIDTH,WIDTH,WIDTH);
}

void QBayesEdge::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    auto mousePoint = event->scenePos();
    double distance;

    if (parentPoint.x() == childPoint.x()) {
        distance = abs(mousePoint.x() - parentPoint.x());
    } else {
        double k, m;
        k = ( childPoint.y() - parentPoint.y() ) / ( childPoint.x() - parentPoint.x() );
        m = parentPoint.y() - k*parentPoint.x();
        distance = abs(k*mousePoint.x() - mousePoint.y() + m) / sqrt(1+k*k);
    }

    if (distance < 10) {
        QGraphicsItem::mousePressEvent(event);
    } else {
        event->ignore();
    }
}
