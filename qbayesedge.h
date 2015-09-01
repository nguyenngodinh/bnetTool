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

#ifndef MYBAYESEDGE_H
#define MYBAYESEDGE_H

#include <QObject>
#include <QGraphicsItem>
#include <QPointF>

class QBayesNode;

class QBayesEdge : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    QBayesEdge(QBayesNode* parentNode, QBayesNode* childNode, QPointF parentPoint, QPointF childPoint);
    ~QBayesEdge();

    void setLine();
    QBayesNode* getParentNode() const;
    QBayesNode* getChildNode() const;
    QString toString() const;

public slots:
    void onParentNodePositionChanged(QPointF value);
    void onChildNodePositionChanged(QPointF value);
    void onNodeDeleted();

signals:
    void edgeDeleted();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void keyReleaseEvent(QKeyEvent *event);
    QPainterPath shape();
    QRectF boundingRect() const;
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    const qreal ARROW_WIDTH = 10;

    QBayesNode *parentNode, *childNode;
    bool isSelected;
    QPointF parentPoint, childPoint;
    QPointF start, end, arrow1, arrow2;

};

#endif // MYBAYESEDGE_H
