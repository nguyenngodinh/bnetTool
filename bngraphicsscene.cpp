/**
 * \author trungvv1
 *
 * \date 3/25/2015
 * \class BnGraphicsScene
 *
 * \brief write something about your class
 *
 *
 */

#include "bngraphicsscene.h"

#include <QtGui>
#include <QGraphicsSceneMouseEvent>
#include "utility.h"
#include "qbayesnode.h"
#include "qbayesedge.h"

void BnGraphicsScene::loadFromText(QString *text)
{
    this->clearNetwork();

    QTextStream strScene(text);
    uint nodeIndex;
    double px, py;
    QString label;

    numNodes = 0;
    strScene >> numNodes;
    for (uint i = 0; i < numNodes; ++i) {
        strScene >> nodeIndex;
        strScene >> label;
        strScene >> px;
        strScene >> py;

        if (label == "_")
            label = "";

        /// add node
        auto newNode = new QBayesNode(i, label);
        newNode->setPos(px, py);
        this->addItem(newNode);
//        newNode->setIsSelected(false);
        connect(newNode, SIGNAL(nodeDeleted()), this, SLOT(onNodeDeleted()));
        connect(newNode, SIGNAL(positionChanged(QPointF)), this, SIGNAL(nodePositionChanged()));
        mapBayes.insert(newNode, QList<QBayesEdge*>());
    }

    uint nodeIndex1, nodeIndex2;
    uint numEdges = 0;
    strScene >> numEdges;
    for (uint i = 0; i < numEdges; ++i) {
        strScene >> nodeIndex1;
        strScene >> nodeIndex2;        

        /// add edge
        auto parentNode = this->getNodeAt(nodeIndex1);
        auto childNode = this->getNodeAt(nodeIndex2);
        auto newEdge = new QBayesEdge(parentNode, childNode, parentNode->scenePos(), childNode->scenePos());
        newEdge->setPos(0,0);
        this->addItem(newEdge);
        connect(parentNode, SIGNAL(positionChanged(QPointF)), newEdge, SLOT(onParentNodePositionChanged(QPointF)));
        connect(childNode, SIGNAL(positionChanged(QPointF)), newEdge, SLOT(onChildNodePositionChanged(QPointF)));
        connect(parentNode, SIGNAL(nodeDeleted()), newEdge, SLOT(onNodeDeleted()));
        connect(childNode, SIGNAL(nodeDeleted()), newEdge, SLOT(onNodeDeleted()));
        connect(newEdge, SIGNAL(edgeDeleted()), this, SLOT(onEdgeDeleted()));
        mapBayes[parentNode].append(newEdge);
        mapBayes[childNode].append(newEdge);
        listEdges.push_back(newEdge);
    }
}

void BnGraphicsScene::clearNetwork()
{
    if (!listEdges.isEmpty()) {
        qDeleteAll(listEdges);
        listEdges.clear();
    }
    if (!mapBayes.isEmpty()) {
        foreach (auto list, mapBayes.values()) {
            list.clear();
        }
        foreach (auto key, mapBayes.keys()) {
            delete key;
        }
        mapBayes.clear();
    }
    numNodes = 0;
    clear();
    clearFocus();
    clearSelection();
}

BnGraphicsScene::BnGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{
    numNodes = 0;
}

BnGraphicsScene::~BnGraphicsScene()
{
    clearNetwork();
}

void BnGraphicsScene::setCurrentNodeLabel(QString label)
{
    auto item = getSelectedNode();
    item->setLabel(label);
}

bool BnGraphicsScene::hasNodeSelected()
{
    bool result = !this->selectedItems().isEmpty();
    if (result)
        result = (dynamic_cast<QBayesNode*>(this->selectedItems()[0]) != NULL);
    return result;
}

QBayesNode *BnGraphicsScene::getSelectedNode()
{
    if (hasNodeSelected())
        return (QBayesNode*) (this->selectedItems()[0]);
    else
        return nullptr;
}

QBayesEdge *BnGraphicsScene::getSelectedEdge()
{
    if (!this->selectedItems().isEmpty())
        return dynamic_cast<QBayesEdge*>(this->selectedItems()[0]);
    else
        return nullptr;
}

QBayesNode *BnGraphicsScene::getNodeAt(uint index) const
{
    foreach (auto node, mapBayes.keys()) {
        if (node->getIndex() == index) {
            return node;
        }
    }
    return nullptr;
}

QString BnGraphicsScene::toString() const
{
    QString rs = "";
    rs += QString::number(numNodes);
    rs += "\t";
    rs += "\n";
    auto listKeys = mapBayes.keys();
    qSort(listKeys.begin(), listKeys.end(), PtrLess<QBayesNode>());
    foreach (QBayesNode* node, listKeys) {
        rs += node->toString();
        rs += "\n";
    }
    rs += QString::number(listEdges.size());
    rs += "\t";
    rs += "\n";
    foreach (QBayesEdge* edge, listEdges) {
        rs += edge->toString();
        rs += "\n";
    }

    return rs;
}

void BnGraphicsScene::onNodeDeleted()
{
    auto dNode = getSelectedNode();

    /// delete related edges
    foreach (QBayesEdge* edge, mapBayes[dNode]) {
        mapBayes[edge->getParentNode()].removeOne(edge);
        mapBayes[edge->getChildNode()].removeOne(edge);
        listEdges.removeOne(edge);
        this->removeItem(edge);
    }
    mapBayes[dNode].clear();
    mapBayes.remove(dNode);
    ///

    emit nodeDeleted(dNode->getIndex());

    /// decrease node's index
    auto index = dNode->getIndex();
    foreach (QBayesNode* node, mapBayes.keys()) {
        if (node->getIndex() > index) {
            node->setIndex(node->getIndex()-1);
        }
    }
    numNodes--;
    ///

    this->removeItem(dNode);
    update();
}

void BnGraphicsScene::onEdgeDeleted()
{
    auto dEdge = getSelectedEdge();
    mapBayes[dEdge->getParentNode()].removeOne(dEdge);
    mapBayes[dEdge->getChildNode()].removeOne(dEdge);
    listEdges.removeOne(dEdge);
    this->removeItem(dEdge);
    auto paIndex = dEdge->getParentNode()->getIndex();
    auto cIndex = dEdge->getChildNode()->getIndex();
    emit edgeDeleted(paIndex, cIndex);
    update();
}

void BnGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    auto mousePos = event->scenePos();
//    auto ellipse = this->itemAt(mousePos);
    auto ellipse = this->itemAt(mousePos, QTransform());

    if (!ellipse) {
        addNode("", mousePos);
    } else {
        //        QGraphicsScene::sendEvent(ellipse, event);
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

void BnGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        auto mousePos = event->scenePos();
//        auto childNode = dynamic_cast<QBayesNode*>(this->itemAt(mousePos));
        auto childNode = dynamic_cast<QBayesNode*>(this->itemAt(mousePos, QTransform()));
        auto parentNode = getSelectedNode();
        if (childNode && parentNode) {
            /// check edge alredy exist
            foreach (QBayesEdge* edge, mapBayes[parentNode]) {
                if (edge->getChildNode() == childNode) {
                    return;
                }
            }
            ///
            addEdge(parentNode, childNode);
        }
        return;
    } else if (event->modifiers() & Qt::ControlModifier) {
        return;
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

void BnGraphicsScene::addNode(QString label, QPointF pos)
{
    QBayesNode *newNode = new QBayesNode(numNodes, label);
    newNode->setPos(pos);
    this->addItem(newNode);
    newNode->setSelected(true);
    connect(newNode, SIGNAL(nodeDeleted()), this, SLOT(onNodeDeleted()));
    connect(newNode, SIGNAL(positionChanged(QPointF)), this, SIGNAL(nodePositionChanged()));
    mapBayes.insert(newNode, QList<QBayesEdge*>());
    ++numNodes;
    emit nodeAdded();
}

void BnGraphicsScene::addEdge(QBayesNode* parentNode, QBayesNode* childNode)
{
    QBayesEdge* newEdge = new QBayesEdge(parentNode, childNode, parentNode->scenePos(), childNode->scenePos());
    newEdge->setPos(0,0);
    this->addItem(newEdge);
    newEdge->setSelected(false);
    connect(parentNode, SIGNAL(positionChanged(QPointF)), newEdge, SLOT(onParentNodePositionChanged(QPointF)));
    connect(childNode, SIGNAL(positionChanged(QPointF)), newEdge, SLOT(onChildNodePositionChanged(QPointF)));
    connect(parentNode, SIGNAL(nodeDeleted()), newEdge, SLOT(onNodeDeleted()));
    connect(childNode, SIGNAL(nodeDeleted()), newEdge, SLOT(onNodeDeleted()));
    connect(newEdge, SIGNAL(edgeDeleted()), this, SLOT(onEdgeDeleted()));
    mapBayes[parentNode].append(newEdge);
    mapBayes[childNode].append(newEdge);
    listEdges.push_back(newEdge);
    emit edgeAdded(parentNode->getIndex(), childNode->getIndex());
}

