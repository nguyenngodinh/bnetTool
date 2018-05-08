/**
 * \author trungvv1
 *
 * \date 3/24/2015
 * \class MainWindow
 *
 * \brief write something about your class
 *
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsEllipseItem>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QStringList>
#include <QDebug>

#include "utility.h"
#include "cptmodel.h"
#include "qbayesnode.h"
#include "bngraphicsscene.h"
#include "bayesnet.h"
#include "learnbayes.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mFilename(""),
    mTitle("Bayesian Network Tools"),
    mCurrPath(QDir::currentPath() + "/data/"),
    mIsSaved(true)
{
    ui->setupUi(this);
    initSplitter();
    QDir currDir;
    currDir.mkpath(mCurrPath);

    mBayesNet = new BayesNet();
    mQuickPredictDialog = new DialogQuickPredict();
    mConvertData = new ConvertBayesData();

    /// scene
    mScene = new BnGraphicsScene();
    mScene->setSceneRect(0,0,2000,2000);
    ui->graphicsView->setScene(mScene);
    ui->graphicsView->centerOn(0, 0);
//    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    connect(mScene, SIGNAL(selectionChanged()), this, SLOT(onSceneSelectionChanged()));
    connect(mScene, SIGNAL(nodeAdded()), this, SLOT(onNodeAdded()));
    connect(mScene, SIGNAL(nodeDeleted(uint)), this, SLOT(onNodeDeleted(uint)));
    connect(mScene, SIGNAL(edgeAdded(uint,uint)), this, SLOT(onEdgeAdded(uint,uint)));
    connect(mScene, SIGNAL(edgeDeleted(uint,uint)), this, SLOT(onEdgeDeleted(uint,uint)));
//    connect(scene, SIGNAL(changed(QList<QRectF>)), this, SLOT(onSceneChanged()));
    connect(mScene, SIGNAL(nodePositionChanged()), this, SLOT(onNodePositionChanged()));
    ///

    /// model for CPT table
    mModelCpt = new CptModel();
    connect(mModelCpt, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onCptDataChanged(QModelIndex,QModelIndex)));
    ui->tblCpt->setModel(mModelCpt);
    ui->tblCpt->setSelectionMode(QAbstractItemView::SingleSelection);
//    ui->tblCpt->horizontalHeader()->setSResizeMode(QHeaderView::Stretch);
    ui->tblCpt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ///

    /// init PPT graph
    ui->plot->setBackground(Qt::white);
    ui->plot->plotLayout()->insertRow(0);
    auto plotTitle = new QCPPlotTitle(ui->plot, "");
//    plotTitle->setTextColor(Qt::gray);
    plotTitle->setFont(QFont());
    ui->plot->plotLayout()->addElement(0, 0, plotTitle);

    ui->plot->yAxis->setRange(0.0, 1.0);
    ui->plot->yAxis->setAutoTicks(false);
//    ui->plot->yAxis->setTickVector({0,.1,.2,.3,.4,.5,.6,.7,.8,.9,1});
    ui->plot->yAxis->setTickVector({0,.2,.4,.6,.8,1});

    ui->plot->xAxis->setAutoTicks(false);
    ui->plot->xAxis->setAutoTickLabels(false);
    ui->plot->xAxis->setTickVector(QVector<double>());
    ui->plot->xAxis->setTickLabelRotation(0);
    ui->plot->xAxis->setSubTickCount(0);
    ui->plot->xAxis->setTickLength(0, 0);

    QCPBars *bar = new QCPBars(ui->plot->xAxis, ui->plot->yAxis);
    ui->plot->addPlottable(bar);
    bar->setPen(QPen(QBrush(Qt::black), 1));
    bar->setBrush(Qt::darkCyan);
//    plotBar("Test", {0.01, 0.3, 0.4, 0.28});
    ///

    /// disable text
    ui->groupBox->setEnabled(false);
    ui->txtIndex->setEnabled(false);
    ui->chbIsEvidence->setChecked(false);
    ui->txtEvidenceValue->setEnabled(false);
    ///

    setWindowTitle(mTitle);
}

MainWindow::~MainWindow()
{    
//    scene->disconnect(this);
    mListPpts.clear();
    delete mQuickPredictDialog;
    delete mConvertData;
    delete mScene;
    delete ui;
    delete mSplitter;
    delete mModelCpt;
    delete mBayesNet;
}

void MainWindow::onSceneSelectionChanged()
{
    auto node = mScene->getSelectedNode();
    if ( node && mBayesNet->hasNode(node->getIndex()) ) {
        ui->txtIndex->setText(QString::number(node->getIndex()));
        ui->txtLabel->setText(node->getLabel());
        ui->txtNumValues->setValue(mBayesNet->getNumNodeValues(node->getIndex()));
        ui->txtEvidenceValue->setMaximum(ui->txtNumValues->value()-1);
        if (node->getIsEvidence()) {
            ui->txtEvidenceValue->setValue(node->getEvidenceValue());
        }
        ui->chbIsEvidence->setChecked(node->getIsEvidence());
        loadSelectedCpt(node->getIndex());
        loadSelectedPpt(node->getIndex());
        ui->groupBox->setEnabled(true);
    } else {
        clearInfo();
        ui->groupBox->setEnabled(false);
    }
}

void MainWindow::onNodeAdded()
{
    mBayesNet->addNode(ui->txtNumValues->value());
    auto nodeIndex = mBayesNet->getNumNodes()-1;
    loadSelectedCpt(nodeIndex);
    mIsSaved = false;
    setWindowTitle(mTitle + " - " + mFilename + "*");
}

void MainWindow::onNodeDeleted(uint index)
{
    mBayesNet->removeNode(index);
    mIsSaved = false;
    setWindowTitle(mTitle + " - " + mFilename + "*");
}

void MainWindow::onEdgeAdded(uint parentIndex, uint childIndex)
{
    mBayesNet->addEdge(parentIndex, childIndex);
    mIsSaved = false;
    setWindowTitle(mTitle + " - " + mFilename + "*");
}

void MainWindow::onEdgeDeleted(uint parentIndex, uint childIndex)
{
    mBayesNet->removeEdge(parentIndex, childIndex);
    mIsSaved = false;
    setWindowTitle(mTitle + " - " + mFilename + "*");
}

void MainWindow::onCptDataChanged(QModelIndex index1, QModelIndex index2)
{
    if (index1 == index2) {
        vector<uint> ass;
        vector<double> probs;
        auto node = mScene->getSelectedNode();
        auto nodeIndex = node->getIndex();
        auto numParents = mBayesNet->getNumParents(nodeIndex);
        auto numValues = mBayesNet->getNumNodeValues(nodeIndex);
        for (uint i = 0; i < numParents; ++i) {
            ass.push_back(mModelCpt->getValue(index1.row(), i));
        }
        for (uint i = numParents; i < numParents+numValues; ++i) {
            probs.push_back(mModelCpt->getValue(index1.row(), i));
        }
        ass.push_back(index1.column()-numParents);
        mBayesNet->setNodeCpt(nodeIndex, ass, probs);
        mIsSaved = false;
        setWindowTitle(mTitle + " - " + mFilename + "*");


        //////@ automatically recalculate the posterior probabilities after CPT data change
        /*
        if (mBayesNet->getNumNodes() == 0) {
            QMessageBox msgBox(QMessageBox::Warning, "Error","Your graph is currently empty.\n"
                        "You must add some nodes!", QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
            msgBox.exec();
            return;
        } else if (!mBayesNet->isConnected()) {
            QMessageBox msgBox(QMessageBox::Warning, "Error","Your graph has nodes that are completely disconnected from the other nodes.\n"
                        "You must connect them somehow!", QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
            msgBox.exec();
            return;
        } else {
            mListPpts.clear();
            if (mBayesNet->isModified())
                mBayesNet->setJointDistribution();

            /// get PPT based on current evidence
            auto numNodes = mBayesNet->getNumNodes();
    //        vector<uint> evidences(numNodes, NONE_NODE_INDEX);
    //        for (uint i = 0; i < numNodes; ++i)
    //        {
    //            if (mScene->getNodeAt(i)->getIsEvidence()) {
    //                evidences[i] = mScene->getNodeAt(i)->getEvidenceValue();
    //            }
    //        }
    //        for (uint i = 0; i < numNodes; ++i)
    //        {
    //            auto ppt = mBayesNet->getPosteriorProbabilities(i, evidences);
    //            mListPpts.push_back(ppt);
    //        }

            ///@
            vector<uint> eIndice;
            vector<int> eVals;
            for (uint i = 0; i < numNodes; ++i)
            {
                if (mScene->getNodeAt(i)->getIsEvidence()) {
                    eIndice.push_back(i);
                    eVals.push_back( mScene->getNodeAt(i)->getEvidenceValue() );
                }
            }
            mBayesNet->updateMul(eIndice, eVals);
            mListPpts = mBayesNet->getBeliefs();
            ///@

            auto selectNode = mScene->getSelectedNode();
            if (selectNode)
                loadSelectedPpt(selectNode->getIndex());
        }
        */
        /////////
    }
}

void MainWindow::onNodePositionChanged()
{
    mIsSaved = false;
    setWindowTitle(mTitle + " - " + mFilename + "*");
}

void MainWindow::on_txtLabel_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    auto node = mScene->getSelectedNode();
    auto label = ui->txtLabel->text();
    if (node && label != node->getLabel()) {
        label.replace(" ", "");
        node->setLabel(label);
        mIsSaved = false;
        setWindowTitle(mTitle + " - " + mFilename + "*");
    }
}

void MainWindow::on_chbIsEvidence_toggled(bool checked)
{
    Q_UNUSED(checked);
    auto isEvidence = ui->chbIsEvidence->isChecked();
    ui->txtEvidenceValue->setEnabled(isEvidence);
    auto node = mScene->getSelectedNode();
    if (node && isEvidence != node->getIsEvidence()) {
        node->setIsEvidence(isEvidence);
        if (isEvidence) {
            auto eValue = ui->txtEvidenceValue->value();
            node->setEvidenceValue(eValue);
        }
        mIsSaved = false;
        setWindowTitle(mTitle + " - " + mFilename + "*");
    }
}

void MainWindow::on_txtNumValues_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    auto numValues = ui->txtNumValues->value();
    ui->txtEvidenceValue->setMaximum(numValues-1);
    auto node = mScene->getSelectedNode();
    if (node) {
        auto nodeIndex = node->getIndex();
        if (mBayesNet->getNumNodeValues(nodeIndex) == numValues) {
            return;
        }
        mBayesNet->setNodeSize(nodeIndex, numValues);
        on_txtEvidenceValue_valueChanged(0);
        loadSelectedCpt(nodeIndex);
        mIsSaved = false;
        setWindowTitle(mTitle + " - " + mFilename + "*");
    }
}

void MainWindow::on_txtEvidenceValue_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    auto node = mScene->getSelectedNode();
    auto value = ui->txtEvidenceValue->value();
    if (node && node->getEvidenceValue() != value) {
        node->setEvidenceValue(value);
        mIsSaved = false;
        setWindowTitle(mTitle + " - " + mFilename + "*");
    }
}

void MainWindow::on_btnRecal_clicked()
{
    if (mBayesNet->getNumNodes() == 0) {
        QMessageBox msgBox(QMessageBox::Warning, "Error","Your graph is currently empty.\n"
                    "You must add some nodes!", QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
        msgBox.exec();
        return;
    } else if (!mBayesNet->isConnected()) {
        QMessageBox msgBox(QMessageBox::Warning, "Error","Your graph has nodes that are completely disconnected from the other nodes.\n"
                    "You must connect them somehow!", QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
        msgBox.exec();
        return;
    } else {
        mListPpts.clear();
        if (mBayesNet->isModified())
            mBayesNet->setJointDistribution();

        /// get PPT based on current evidence
        auto numNodes = mBayesNet->getNumNodes();
        /*
        ///
//        vector<uint> evidences(numNodes, NONE_NODE_INDEX);
//        for (uint i = 0; i < numNodes; ++i)
//        {
//            if (mScene->getNodeAt(i)->getIsEvidence()) {
//                evidences[i] = mScene->getNodeAt(i)->getEvidenceValue();
//            }
//        }
//        for (uint i = 0; i < numNodes; ++i)
//        {
//            auto ppt = mBayesNet->getPosteriorProbabilities(i, evidences);
//            mListPpts.push_back(ppt);
//        }
        ///
*/


        ///@
        vector<uint> eIndice;
        vector<int> eVals;
        for (uint i = 0; i < numNodes; ++i)
        {
            if (mScene->getNodeAt(i)->getIsEvidence()) {
                eIndice.push_back(i);
                eVals.push_back( mScene->getNodeAt(i)->getEvidenceValue() );
            }
        }
        mBayesNet->updateMul(eIndice, eVals);
        mListPpts = mBayesNet->getBeliefs();
        ///@

        auto selectNode = mScene->getSelectedNode();
        if (selectNode)
            loadSelectedPpt(selectNode->getIndex());
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (mFilename.isEmpty())
    {
        QString filename = QFileDialog::getSaveFileName(this, "Save Bayesian Network", mCurrPath, "Bayesian Network Files (*.bayesnet)");
        if (filename.isEmpty())
            return;
//        if (!filename.endsWith(".bayesnet"))
//            filename += ".bayesnet";

        auto rs = mBayesNet->toString();
        rs += "\ndelimiter\n\n";
        rs += mScene->toString().toStdString();
        Utility::writeString(filename.toStdString(), rs);

        auto splits = filename.split("/");
        this->mFilename = splits[splits.size()-1];
        setWindowTitle(mTitle + " - " + this->mFilename);
        mIsSaved = true;
    }
    else
    {
        auto rs = mBayesNet->toString();
        rs += "\ndelimiter\n\n";
        rs += mScene->toString().toStdString();
        Utility::writeString((mCurrPath+mFilename).toStdString(), rs);
        setWindowTitle(mTitle + " - " + mFilename);
        mIsSaved = true;
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open Bayesian Network", mCurrPath, "Bayesian Network Files (*.bayesnet)");
    if (filename.isEmpty())
        return;
    if (mBayesNet) {
        delete mBayesNet;
        mBayesNet = nullptr;
    }
    mBayesNet = BayesNet::loadFromTextFile(filename.toStdString());

    /// read all content of file
    QFile inFile(filename);
    QString text = "";
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Open file error!";
        return;
    }
    QTextStream infile(&inFile);
    text = infile.readAll();
    inFile.close();
    auto listStr = text.split("delimiter");
    mScene->loadFromText(&listStr[1]);
    ///

    onSceneSelectionChanged();
    auto splits = filename.split("/");
    this->mFilename = splits[splits.size()-1];
    setWindowTitle("Bayesian Network Tools - " + this->mFilename);
    mIsSaved = true;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit(0);
}

void MainWindow::on_actionSave_as_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save as Bayesian Network", mCurrPath, "Bayesian Network Files (*.bayesnet)");
    if (filename.isEmpty())
        return;

//    if (!filename.endsWith(".bayesnet"))
//        filename += ".bayesnet";
    auto rs = mBayesNet->toString();
    rs += "\ndelimiter\n\n";
    rs += mScene->toString().toStdString();
    Utility::writeString(filename.toStdString(), rs);
    auto splits = filename.split("/");
    this->mFilename = splits[splits.size()-1];
    setWindowTitle("Bayesian Network Tools - " + mFilename);
    mIsSaved = true;
}

void MainWindow::on_action_New_triggered()
{
    if (mBayesNet)
        delete mBayesNet;
    mBayesNet = new BayesNet();
    mScene->clearNetwork();
    mFilename = "";
    onSceneSelectionChanged();
    mIsSaved = false;
    setWindowTitle(mTitle + " - " + mFilename + "*");
}

void MainWindow::on_action_About_triggered()
{
    QMessageBox msgBox(QMessageBox::Information, "About", "\nBayesian Network Tool \n"
                       "Support GUI for creating and calculating on Bayesian Network.\n"
                       "Version 1.0 Copyright 2015 @TrungVV.\n"
                       "Version 2.0 Copyright 2016 @NguyenND",
                       QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
    msgBox.exec();
}

void MainWindow::clearInfo()
{
    ui->txtIndex->setText("");
    ui->txtLabel->setText("");
    ui->txtNumValues->setValue(2);
    ui->txtEvidenceValue->setValue(0);
    ui->chbIsEvidence->setChecked(false);
    mModelCpt->clear();
}

void MainWindow::loadSelectedCpt(uint nodeIndex)
{
    if (nodeIndex < 0 || nodeIndex >= mBayesNet->getNumNodes())
        return;

    auto numNodeValues  = mBayesNet->getNumNodeValues(nodeIndex);
    auto numCptValues   = mBayesNet->getNumCptValues(nodeIndex);
    auto numParents     = mBayesNet->getNumParents(nodeIndex);
    auto parents        = mBayesNet->getParentIndice(nodeIndex);
    uint nrows          = numCptValues/numNodeValues;
    uint ncols          = numParents+numNodeValues;
    uint col = 0;
    vector<uint> parentSizes;

    /// init modelCpt
    mModelCpt->initValues(nrows, ncols, numParents);
    ///

    /// fill header
    foreach (auto pIndex, parents) {
        mModelCpt->setHeader(col++, "Node " + QString::number(pIndex));
        parentSizes.push_back(mBayesNet->getNumNodeValues(pIndex));
    }
    for (uint i = 0; i < numNodeValues; ++i) {
        if (numNodeValues < 5)
            mModelCpt->setHeader(col+i, "P(node="+QString::number(i)+")");
        else
            mModelCpt->setHeader(col+i, QString::number(i));
    }
    ///

    /// fill table values
    uint cptIndex = 0;
    for (uint r = 0; r < nrows; ++r) {
        auto assignment = Utility::numberToVector(parentSizes, r);
        for (col = 0; col < numParents; ++col) {
            mModelCpt->setValue(r, col, assignment[col]);
        }
        for (; col < ncols; ++col) {
            mModelCpt->setValue(r, col, mBayesNet->getNodeCpt(nodeIndex, cptIndex++));
        }
    }
    ///
}

void MainWindow::loadSelectedPpt(uint nodeIndex)
{
    auto numNodes = mBayesNet->getNumNodes();
    if (nodeIndex < 0 || nodeIndex >= numNodes || mListPpts.size() <= nodeIndex)
        return;

    auto ppt = mListPpts[nodeIndex];
    QVector<double> data;
    QStringList labels;
    auto numNodeValues = mBayesNet->getNumNodeValues(nodeIndex);
    for (uint i = 0; i < numNodeValues; ++i) {
        auto value = ppt[i];
        data.push_back(value);
        labels.push_back("P("+QString::number(i)+")="+QString::number(value));
    }
    plotBar(mScene->getNodeAt(nodeIndex)->getLabel(), data);
}

void MainWindow::plotBar(const QString & nodeLabel, const QVector<double> &y)
{
    ((QCPPlotTitle*)(ui->plot->plotLayout()->element(0,0)))->setText(nodeLabel);

    QVector<double> x;
    QVector<QString> labels;
    for (int i = 0; i < y.size(); ++i)
    {
        x.push_back(i+0.5);
        auto s = QString::number(y[i], 'f', 2);
        if (y.size() < 5)
            labels.push_back("P("+QString::number(i)+")="+s);
        else
            labels.push_back(s);
    }

    ui->plot->xAxis->setTickVector(x);
    ui->plot->xAxis->setTickVectorLabels(labels);
    ui->plot->xAxis->setRange(0, y.size());

    auto bar = (QCPBars*)ui->plot->plottable(0);
    bar->setData(x, y);
    ui->plot->replot();
}

void MainWindow::on_action_Toggle_Scene_toggled(bool arg1)
{
    ui->graphicsView->setVisible(arg1);
}

void MainWindow::on_action_Quick_predict_triggered()
{
    mQuickPredictDialog->focusText();
    auto rs = mQuickPredictDialog->exec();
    if (rs == QDialog::Accepted) {
        auto values = mQuickPredictDialog->getValues();
        if (values.size() < mBayesNet->getNumNodes()) {
            QMessageBox msgBox(QMessageBox::Warning, "Error","Missing value!\n"
                        "You must provide unobserved node as -1 and predict node as -2", QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
            msgBox.exec();
            on_action_Quick_predict_triggered();
        } else {
            auto index = values.indexOf(-2);
            if (index < 0) {
                QMessageBox msgBox(QMessageBox::Warning, "Error","Unspecified predict value!\n"
                            "You must provide unobserved node as -1 and predict node as -2", QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
                msgBox.exec();
                on_action_Quick_predict_triggered();
                return;
            }
            values[index] = -1;
//            vector<uint> uValues;
//            for (const auto & v : values)
//            {
//                uValues.push_back((uint)v);
//            }
//            auto p = mBayesNet->predictValues(index, uValues);
            ///@
            vector<uint> eIndice;
            vector<int> eVals;
            for(int i=0; i<values.size(); ++i)
            {
                if (values.at(i) > -1) {
                    eIndice.push_back(i);
                    eVals.push_back(values[i]);
                }
            }
            mBayesNet->updateMul(eIndice, eVals);
            auto p = mBayesNet->getBeliefs().at(index);
            ///@

            QString str;
            double threat = 0;
            for (uint i = 0; i < p.size(); ++i)
            {
                threat += i*p[i];
                str += QString::number(p[i]);
                str += " ; ";
            }
            str += "\nAggregated label: ";
            str += QString::number(threat);
            str += "\n";
            QMessageBox msgBox(QMessageBox::Question, "Info",
                               "Predicted values: " + str + "\nContinue?"
                               , QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog | Qt::FramelessWindowHint);
            auto isNext = msgBox.exec();
            if (isNext == QMessageBox::Yes)
                on_action_Quick_predict_triggered();
        }
    }
}

void MainWindow::initSplitter()
{
    mSplitter = new QSplitter();
    ui->centralWidget->layout()->addWidget(mSplitter);
    mSplitter->addWidget(ui->graphicsView);
    mSplitter->addWidget(ui->groupBox);
}

void MainWindow::on_actionTest_accuracy_triggered()
{
    QString testFile = QFileDialog::getOpenFileName(this, "Open Test Data Sample", mCurrPath, "Bayesian Network Sample (*.data)");
    if (testFile.isEmpty()) {
        return;
    }
    bool ok;
    QString strInput = QInputDialog::getText(this, "Input Dialog", "Index of tested node:", QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    uint index = (uint) strInput.toInt();

    auto samples = Utility::readTsvUintData(testFile.toStdString());
    vector<vector<double>> values;
    uint counter = 0;
    for (uint i=0; i<samples.size(); ++i) {
//        double p = mBayesNet->predictValue(index, samples[i]);
        ///@
        vector<uint> eIndice;
        vector<int> eVals;
        for(uint j=0; j<samples[i].size(); ++j)
        {
            if(j!=index && samples.at(i).at(j) > -1)
            {
                eIndice.push_back(index);
                eVals.push_back(samples.at(i).at(j));
            }
        }
        mBayesNet->updateMul(eIndice, eVals);
        auto beliefs = mBayesNet->getBeliefs().at(index);
        double p = 0;
        for(uint j=0; j<beliefs.size(); ++j)
        {
            p += beliefs[j] * j;
        }
        ///@

        qDebug() << "predict " << i << ": " << p;
        uint predictValue = std::round(p);
        samples[i].push_back(predictValue);
        if (samples[i][index] == predictValue)
            ++counter;
        vector<double> v;
        for (const auto & s : samples[i])
        {
            v.push_back(s);
        }
        v.push_back(p);
        values.push_back(v);
    }
    QMessageBox msgBox(QMessageBox::Question, "Info",
                       "Test accuracy: " + QString::number(counter/(double)samples.size()) + "\nDo you want to save to file .predict ?"
                       , QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog | Qt::FramelessWindowHint);
    if (msgBox.exec() == QMessageBox::Yes) {
//        Utility::writeTsvUintData((testFile+".predict").toStdString(), samples);
        Utility::writeTsvData((testFile+".predict").toStdString(), values);
    }
}

/**
 * @brief MainWindow::on_actionLearn_Discard_Cpt_triggered
 * learn new
 */
void MainWindow::on_actionLearn_New_triggered()
{
    bool ok = true;
    QString strInput = QInputDialog::getText(this, "Input Dialog", "Prior: ", QLineEdit::Normal, QString(), &ok);
    auto prior = strInput.toDouble(&ok);
    if (!ok) {
        return;
    }
    QMessageBox msgBoxUseCpt(QMessageBox::Question, "Question",
                       "Do you want to use current CPT ?"
                       , QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog | Qt::FramelessWindowHint);
    ok = msgBoxUseCpt.exec() == QMessageBox::Yes;
    if (!ok) {
        return;
    }
    strInput = QInputDialog::getText(this, "Input Dialog", "CPT weight: ", QLineEdit::Normal, QString(), &ok);
    auto cptWeight = strInput.toDouble(&ok);
    if (!ok) {
        return;
    }
    QString dataFile = QFileDialog::getOpenFileName(this, "Open Learning Data Sample", mCurrPath, "Bayesian Network Sample (*.label)");
    if (dataFile.isEmpty())
        return;

    LearnBayes learn(mBayesNet, prior, cptWeight);
    learn.learn(dataFile.toStdString());

    QMessageBox msgBox(QMessageBox::Question, "Info",
                       "Learning completed! \nDo you want to save to file .learn ?"
                       , QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog | Qt::FramelessWindowHint);
    if (msgBox.exec() == QMessageBox::Yes) {
        Utility::writeString((mCurrPath+mFilename+".0.learn").toStdString(), learn.toString());
        setWindowTitle(mTitle + " - " + mFilename + "*");
        mIsSaved = false;
    } else {
        setWindowTitle(mTitle + " - " + mFilename + "*");
        mIsSaved = false;
    }
    onSceneSelectionChanged();
}

/**
 * @brief MainWindow::on_actionLearn_Using_Cpt_triggered
 * learn update
 */
void MainWindow::on_actionLearn_Update_triggered()
{
    QString learnFile = QFileDialog::getOpenFileName(this, "Open Learning History", mCurrPath, "Bayesian Network Learning (*.learn)");
    if (learnFile.isEmpty())
        return;
    QString dataFile = QFileDialog::getOpenFileName(this, "Open Learning Data Sample", mCurrPath, "Bayesian Network Sample (*.label)");
    if (dataFile.isEmpty())
        return;

    auto data = Utility::readTsvUintData(dataFile.toStdString());
    LearnBayes learn(mBayesNet, learnFile.toStdString());
    learn.learn(data);

    QMessageBox msgBox(QMessageBox::Question, "Info",
                       "Learning completed! \nDo you want to save to file .learn ?"
                       , QMessageBox::Yes | QMessageBox::No, this, Qt::Dialog | Qt::FramelessWindowHint);
    if (msgBox.exec() == QMessageBox::Yes) {
        QStringList splits = learnFile.split(".");
        QString saveFile = "";
        for (int i = 0; i < splits.size()-2; ++i) {
            saveFile += splits[i];
            saveFile += ".";
        }
        bool ok;
        int version = splits[splits.size()-2].toInt(&ok);
        if (ok) {
            saveFile += QString::number(version+1) + ".";
        } else {
            saveFile += "0.";
        }
        saveFile += "learn";
        auto str = learn.toString();
        Utility::writeString(saveFile.toStdString(), str);
        setWindowTitle(mTitle + " - " + mFilename + "*");
        mIsSaved = true;
    } else {
        setWindowTitle(mTitle + " - " + mFilename + "*");
        mIsSaved = false;
    }
}

void MainWindow::on_actionReset_all_CPTs_triggered()
{
    for (uint i = 0; i < mBayesNet->getNumNodes(); ++i)
    {
        mBayesNet->initNodeCpt(i);      /// uniform init
    }
    mBayesNet->setJointDistribution({});
    mBayesNet->setIsModified(true);
}

void MainWindow::on_actionConvert_value_to_label_triggered()
{
    mConvertData->show();
}

void MainWindow::on_actionSave_to_binary_triggered()
{
    if (mFilename.isEmpty())
    {
        on_actionSave_triggered();
    }
    else
    {
        auto filename = mCurrPath+mFilename+".bin";
        mBayesNet->save(filename.toStdString());
        QMessageBox msgBox(QMessageBox::Information, "Infomation", "\nSave to Binary File \n"
                                                                   "BayesNet is saved in " + filename,
                           QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
        msgBox.exec();
    }
}
