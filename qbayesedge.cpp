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
#include <QGraphicsSceneMouseEvent>
#include <QtGui>
#include <QDebug>

#include "qbayesnode.h"

QBayesEdge::QBayesEdge(QBayesNode* parentNode, QBayesNode* childNode, QPointF p1, QPointF p2)
    : mParentNode(parentNode), mChildNode(childNode), mParentPoint(p1), mChildPoint(p2)
{
    mIsSelected = false;
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
    this->mParentPoint = mParentPoint;
    this->mChildPoint = mChildPoint;
    double x1, y1, x2, y2;
    double arrowX1, arrowY1, arrowX2, arrowY2;

    if (mParentPoint.x() == mChildPoint.x()) {
        x1 = x2 = mParentPoint.x();
        if (mParentPoint.y() < mChildPoint.y()) {
            y1 = mParentPoint.y() + QBayesNode::RADIUS;
            y2 = mChildPoint.y() - QBayesNode::RADIUS;
        } else {
            y1 = mParentPoint.y() - QBayesNode::RADIUS;
            y2 = mChildPoint.y() + QBayesNode::RADIUS;
        }
    } else {
        double k, m;
        k = ( mChildPoint.y() - mParentPoint.y() ) / ( mChildPoint.x() - mParentPoint.x() );
        m = mParentPoint.y() - k*mParentPoint.x();

        double adder = QBayesNode::RADIUS / sqrt(1+k*k);
        if (mChildPoint.x() < mParentPoint.x()) {
            adder *= -1;
        }

        x1 = mParentPoint.x() + adder;
        x2 = mChildPoint.x() - adder;
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
    mStartPoint = QPointF(x1, y1);
    mEndPoint = QPointF(x2, y2);
    mArrow1 = QPointF(arrowX1, arrowY1);
    mArrow2 = QPointF(arrowX2, arrowY2);

    prepareGeometryChange();

    qDebug() << mParentPoint.x() << ";" << mParentPoint.y() << endl;
    qDebug() << mChildPoint.x() << ";" << mChildPoint.y() << endl;
    qDebug() << x1 << ";" << y1 << endl;
    qDebug() << x2 << ";" << y2 << endl;
    qDebug() << endl;

}

QBayesNode *QBayesEdge::getParentNode() const
{
    return mParentNode;
}

QBayesNode *QBayesEdge::getChildNode() const
{
    return mChildNode;
}

QString QBayesEdge::toString() const
{
    QString rs = "";

    rs += QString::number(mParentNode->getIndex());
    rs += "\t";
    rs += QString::number(mChildNode->getIndex());
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
    mParentPoint = value;
    setLine();
}

void QBayesEdge::onChildNodePositionChanged(QPointF value)
{
    mChildPoint = value;
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

    if (mIsSelected)
        painter->setPen(QPen(Qt::red));
    else
        painter->setPen(QPen(Qt::blue));

    painter->drawLine(mStartPoint, mEndPoint);
    painter->setBrush(QBrush(painter->pen().color()));
    painter->drawPolygon(QPolygonF(QVector<QPointF>() << mEndPoint << mArrow1 << mArrow2));
    painter->setBrush(Qt::NoBrush);
//    painter->drawRect(boundingRect());
}

QVariant QBayesEdge::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSelectedChange:
        mIsSelected = value.toBool();
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

    QPointF vec = mEndPoint-mStartPoint;
    double length = sqrt( vec.x()*vec.x() + vec.y()*vec.y() );
    vec = vec * (kClickTolerance/length);
    QPointF orthogonal(vec.y(), -vec.x());

    QPainterPath result(mStartPoint-vec+orthogonal);
    result.lineTo(mStartPoint-vec-orthogonal);
    result.lineTo(mEndPoint+vec-orthogonal);
    result.lineTo(mEndPoint+vec+orthogonal);
    result.closeSubpath();

    return result;
}

QRectF QBayesEdge::boundingRect() const
{
    static const qreal WIDTH = 10;

    if (mStartPoint.x() == mEndPoint.x()) {
        if (mStartPoint.y() < mEndPoint.y()) {
            return QRectF(mStartPoint.x()-WIDTH, mStartPoint.y(), 2*WIDTH, mEndPoint.y()-mStartPoint.y());
        } else {
            return QRectF(mEndPoint.x()-WIDTH, mEndPoint.y(), 2*WIDTH, -mEndPoint.y()+mStartPoint.y());
        }
    } else if (mStartPoint.y() == mEndPoint.y()) {
        if (mStartPoint.x() < mEndPoint.x()) {
            return QRectF(mStartPoint.x(), mStartPoint.y()-WIDTH, mEndPoint.x()-mStartPoint.x(), 2*WIDTH);
        } else {
            return QRectF(mEndPoint.x(), mEndPoint.y()-WIDTH, -mEndPoint.x()+mStartPoint.x(), 2*WIDTH);
        }
    }

//    QPoint dp(WIDTH, WIDTH);
    return QRectF(mStartPoint, mEndPoint).normalized().adjusted(-WIDTH,-WIDTH,WIDTH,WIDTH);
}

void QBayesEdge::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    auto mousePoint = event->scenePos();
    double distance;

    if (mParentPoint.x() == mChildPoint.x()) {
        distance = abs(mousePoint.x() - mParentPoint.x());
    } else {
        double k, m;
        k = ( mChildPoint.y() - mParentPoint.y() ) / ( mChildPoint.x() - mParentPoint.x() );
        m = mParentPoint.y() - k*mParentPoint.x();
        distance = abs(k*mousePoint.x() - mousePoint.y() + m) / sqrt(1+k*k);
    }

    if (distance < 10) {
        QGraphicsItem::mousePressEvent(event);
    } else {
        event->ignore();
    }
}
