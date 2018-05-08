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

#ifndef MYBAYESNODE_H
#define MYBAYESNODE_H

#include <QObject>
#include <QGraphicsItem>
#include <QFontMetricsF>

class QBayesNode : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    static const qreal RADIUS;

    QBayesNode(uint mIndex, QString mLabel);
    ~QBayesNode();

    uint getIndex() const;
    void setIndex(uint value);
    void setLabel(QString mLabel);
    QString getLabel() const;
    bool getIsEvidence() const;
    void setIsEvidence(bool value);
    bool getIsSelected() const;
    void setIsSelected(bool value);
    uint getEvidenceValue() const;
    void setEvidenceValue(uint value);
    QString toString() const;

    bool operator <(const QBayesNode& other) const;

signals:
    void positionChanged(QPointF value);
    void nodeDeleted();

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void keyReleaseEvent(QKeyEvent *event);

private:
    uint mIndex;
    QString mLabel;
    bool mIsEvidence;
    uint mEvidenceValue;
    bool mIsSelected;

    /// auxiliary member
    QFont mFont;
    QFontMetricsF *mFontMetrics;
    QColor mNormalColor, mEvidenceColor;
    QColor mNormalTextColor, mEvidenceTextColor;
    QRectF mRect, mLabelRect;
};

#endif // MYBAYESNODE_H
