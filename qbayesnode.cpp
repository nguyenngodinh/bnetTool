/**
 * \author trungvv1
 *
 * \date 3/25/2015
 * \class MyBayesNode
 *
 * \brief write something about your class
 *
 *
 */

#include "qbayesnode.h"
#include <QtGui>

const qreal QBayesNode::RADIUS = 25;

QBayesNode::QBayesNode(uint index, QString label)
    : mIndex(index), mLabel(label), mEvidenceValue(0)
{
    rect = QRectF(-RADIUS, -RADIUS, RADIUS*2, RADIUS*2);
    normalColor = Qt::gray;
    normalTextColor = Qt::white;
    evidenceColor = Qt::green;
    evidenceTextColor = Qt::red;
    mIsSelected = false;
    mIsEvidence = false;
    font = QFont();
    fontMetrics = new QFontMetricsF(font);

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable
             | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsFocusable);
    setAcceptDrops(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    //    setCursor(Qt::OpenHandCursor);
}

QBayesNode::~QBayesNode()
{

}

uint QBayesNode::getIndex() const
{
    return mIndex;
}

void QBayesNode::setIndex(uint value)
{
    mIndex = value;
}

QString QBayesNode::getLabel() const
{
    return mLabel;
}

void QBayesNode::setLabel(QString value)
{
    mLabel = value;
    update();
}

bool QBayesNode::getIsSelected() const
{
    return mIsSelected;
}

void QBayesNode::setIsSelected(bool value)
{
    mIsSelected = value;
}

bool QBayesNode::getIsEvidence() const
{
    return mIsEvidence;
}

void QBayesNode::setIsEvidence(bool value)
{
    mIsEvidence = value;
    update();
}

QRectF QBayesNode::boundingRect() const
{
    return rect.united(labelRect).adjusted(-RADIUS/5,-RADIUS/5,RADIUS/5,RADIUS/5);
}

void QBayesNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (mIsEvidence) {
        painter->setBrush(QBrush(evidenceColor));
        painter->setPen(evidenceTextColor);
    } else {
        painter->setBrush(QBrush(normalColor));
        painter->setPen(normalTextColor);
    }
    painter->drawEllipse(rect);
    auto indexStr = QString::number(mIndex);
    auto textRect = fontMetrics->boundingRect(indexStr);
    painter->drawText(QPointF(-textRect.width()/2, textRect.height()/4), indexStr);

    painter->setPen(Qt::black);
    /// draw label
    if (!mLabel.isEmpty()) {
        labelRect = fontMetrics->boundingRect(mLabel);
        painter->drawText(QPointF(-labelRect.width()/2, -labelRect.height()/2-RADIUS), mLabel);
        labelRect.moveTo(-labelRect.width()/2, -labelRect.height()-RADIUS);
    } else {
        labelRect = QRectF(0,0,0,0);
    }
    if (mIsSelected) {
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(rect.adjusted(-5,-5,5,5));
    }

//    painter->drawRect(boundingRect());
}

QVariant QBayesNode::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSelectedChange:
        mIsSelected = value.toBool();
        if (mIsSelected)
            setFocus();
        break;
    case QGraphicsItem::ItemPositionChange:
    {
        if (scene()) {
            // value is the new position.
            QPointF newPos = value.toPointF();
            QRectF sceneRect = scene()->sceneRect();
            if (!sceneRect.contains(newPos)) {
                // Keep the item inside the scene rect.
                newPos.setX(qMin(sceneRect.right(), qMax(newPos.x(), sceneRect.left())));
                newPos.setY(qMin(sceneRect.bottom(), qMax(newPos.y(), sceneRect.top())));
            }
            emit positionChanged(newPos);
            update();
            return QGraphicsItem::itemChange(change, newPos);
        }
        break;
    }
    default:
        break;
    }
    return QGraphicsItem::itemChange(change, value);
}

void QBayesNode::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        emit nodeDeleted();
    }
    return QGraphicsItem::keyReleaseEvent(event);
}

QString QBayesNode::toString() const
{
    QString rs = "";
    rs += QString::number(mIndex);
    rs += "\t";
    rs += (mLabel.isEmpty() ? "_" : mLabel);
    rs += "\t";
    auto nodePos = pos();
    rs += QString::number(nodePos.x());
    rs += "\t";
    rs += QString::number(nodePos.y());
    rs += "\t";

    return rs;
}

bool QBayesNode::operator <(const QBayesNode &other) const
{
    return (this->mIndex < other.getIndex());
}

uint QBayesNode::getEvidenceValue() const
{
    return mEvidenceValue;
}

void QBayesNode::setEvidenceValue(uint value)
{
    mEvidenceValue = value;
}
