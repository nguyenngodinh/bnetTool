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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <QMainWindow>
#include <QModelIndex>
#include <QSplitter>

#include "dialogquickpredict.h"
#include "convertbayesdata.h"

class BnGraphicsScene;
class CptModel;
class BayesNet;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onSceneSelectionChanged();
    void onNodeAdded();
    void onNodeDeleted(uint index);
    void onEdgeAdded(uint parentIndex, uint childIndex);
    void onEdgeDeleted(uint parentIndex, uint childIndex);
    void onCptDataChanged(QModelIndex index1, QModelIndex index2);
    void onNodePositionChanged();

private slots:
    void on_txtLabel_textChanged(const QString &arg1);
    void on_chbIsEvidence_toggled(bool checked);
    void on_txtNumValues_valueChanged(int arg1);
    void on_txtEvidenceValue_valueChanged(int arg1);
    void on_btnRecal_clicked();
    void on_actionSave_triggered();
    void on_actionOpen_triggered();
    void on_actionQuit_triggered();
    void on_actionSave_as_triggered();
    void on_action_New_triggered();
    void on_action_About_triggered();
    void on_action_Toggle_Scene_toggled(bool arg1);
    void on_action_Quick_predict_triggered();
    void on_actionTest_accuracy_triggered();
    void on_actionLearn_New_triggered();
    void on_actionLearn_Update_triggered();
    void on_actionReset_all_CPTs_triggered();
    void on_actionConvert_value_to_label_triggered();
    void on_actionSave_to_binary_triggered();

private:
    void initSplitter();
    void clearInfo();
    void loadSelectedCpt(uint nodeIndex);
    void loadSelectedPpt(uint nodeIndex);
    void plotBar(const QString &nodeLabel, const QVector<double> & y);

    Ui::MainWindow *ui;

    QSplitter *mSplitter;
    BnGraphicsScene *mScene;
    CptModel *mModelCpt;
    BayesNet *mBayesNet;
    DialogQuickPredict* mQuickPredictDialog;
    ConvertBayesData* mConvertData;

    std::vector<std::vector<double>> mListPpts;
    QString mFilename, mCurrPath, mTitle;
    bool mIsSaved;
};

#endif // MAINWINDOW_H
