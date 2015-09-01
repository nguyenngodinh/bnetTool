/**
 * \author trungvv1
 *
 * \date 3/25/2015
 * \class MyGraphicsScene
 *
 * \brief write something about your class
 *
 *
 */

#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QMap>
#include <QList>

class QBayesNode;
class QBayesEdge;

class BnGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    static constexpr double RADIUS = 20;

    BnGraphicsScene(QObject *parent = 0);
    ~BnGraphicsScene();

    void setCurrentNodeLabel(QString label);
    bool hasNodeSelected();
    QBayesNode* getSelectedNode();
    QBayesEdge* getSelectedEdge();
    QBayesNode* getNodeAt(uint index) const;
    QString toString() const;

    void addNode(QString label, QPointF pos);
    void addEdge(QBayesNode *parentNode, QBayesNode *childNode);
    void loadFromText(QString *text);
    void clearNetwork();

public slots:
    void onNodeDeleted();
    void onEdgeDeleted();

signals:
    void nodeAdded();
    void nodeDeleted(uint index);
    void edgeAdded(uint parentIndex, uint childIndex);
    void edgeDeleted(uint parentIndex, uint childIndex);
    void nodePositionChanged();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    uint numNodes;
    QMap<QBayesNode*, QList<QBayesEdge*>> mapBayes;   /// map each node to all edge related to it
    QList<QBayesEdge*> listEdges;
};

#endif // MYGRAPHICSSCENE_H
