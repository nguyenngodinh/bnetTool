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
#include <QGraphicsScene>
#include <QtGui>

const qreal QBayesNode::RADIUS = 25;

QBayesNode::QBayesNode(uint index, QString label)
    : mIndex(index), mLabel(label), mEvidenceValue(0)
{
    mRect = QRectF(-RADIUS, -RADIUS, RADIUS*2, RADIUS*2);
    mNormalColor = Qt::gray;
    mNormalTextColor = Qt::white;
    mEvidenceColor = Qt::green;
    mEvidenceTextColor = Qt::red;
    mIsSelected = false;
    mIsEvidence = false;
    mFont = QFont();
    mFontMetrics = new QFontMetricsF(mFont);

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
    return mRect.united(mLabelRect).adjusted(-RADIUS/5,-RADIUS/5,RADIUS/5,RADIUS/5);
}

void QBayesNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (mIsEvidence) {
        painter->setBrush(QBrush(mEvidenceColor));
        painter->setPen(mEvidenceTextColor);
    } else {
        painter->setBrush(QBrush(mNormalColor));
        painter->setPen(mNormalTextColor);
    }
    painter->drawEllipse(mRect);
    auto indexStr = QString::number(mIndex);
    auto textRect = mFontMetrics->boundingRect(indexStr);
    painter->drawText(QPointF(-textRect.width()/2, textRect.height()/4), indexStr);

    painter->setPen(Qt::black);
    /// draw label
    if (!mLabel.isEmpty()) {
        mLabelRect = mFontMetrics->boundingRect(mLabel);
        painter->drawText(QPointF(-mLabelRect.width()/2, -mLabelRect.height()/2-RADIUS), mLabel);
        mLabelRect.moveTo(-mLabelRect.width()/2, -mLabelRect.height()-RADIUS);
    } else {
        mLabelRect = QRectF(0,0,0,0);
    }
    if (mIsSelected) {
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(mRect.adjusted(-5,-5,5,5));
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
