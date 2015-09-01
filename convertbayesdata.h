/**
 * \author trungvv1
 *
 * \date 24 Jul 2015
 * \class ConvertBayesData
 *
 * \brief write something about your class
 *
 *
 */

#ifndef CONVERTBAYESDATA_H
#define CONVERTBAYESDATA_H

#include <QWidget>
#include <QStringList>

namespace Ui {
class ConvertBayesData;
}

class TsvModel;
class ThresholdModel;

class ConvertBayesData : public QWidget
{
    Q_OBJECT

public:
    explicit ConvertBayesData(QWidget *parent = 0);
    ~ConvertBayesData();

private slots:
    void on_btnValue_clicked();

    void on_btnLabel_clicked();

    void on_btnConvert_clicked();

    void on_chbColumn_toggled(bool checked);

    void on_txtColumnOrder_editingFinished();

    void on_btnSaveValue_clicked();

    void on_btnSaveLabel_clicked();

private:
    void setupTblValue();
    void setupPosition();
    void setupTblThreshold();

    Ui::ConvertBayesData *ui;
    TsvModel *mModelValue, *mModelLabel;
    ThresholdModel *mModelThreshold;
    QStringList mHeaders;
};

#endif // CONVERTBAYESDATA_H
