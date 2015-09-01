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

#include "convertbayesdata.h"
#include "ui_convertbayesdata.h"

#include <QFileDialog>
#include <QMessageBox>

#include "utility.h"
#include "tsvmodel.h"
#include "thresholdmodel.h"

ConvertBayesData::ConvertBayesData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConvertBayesData)
{
    ui->setupUi(this);
    setupPosition();

    mModelValue = new TsvModel(mHeaders);
    ui->tblValue->setModel(mModelValue);
    ui->tblValue->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblValue->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->tblValue->horizontalHeader()->setDefaultSectionSize(100);
    mModelLabel = new TsvModel(mHeaders);
    ui->tblLabel->setModel(mModelLabel);
    ui->tblLabel->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblLabel->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    mModelThreshold = new ThresholdModel(mHeaders.size());
    ui->tblThreshold->setModel(mModelThreshold);
    ui->tblThreshold->setSelectionMode(QAbstractItemView::SingleSelection);
//    ui->tblThreshold->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tblThreshold->horizontalHeader()->resizeSection(0, 50);
    ui->tblThreshold->horizontalHeader()->resizeSection(1, 450);

    ui->btnSaveLabel->setEnabled(false);
}

ConvertBayesData::~ConvertBayesData()
{
    delete ui;
}

void ConvertBayesData::on_btnValue_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select Value File", QDir::currentPath() + "/data/", "Bayes Value Files (*.value)");
    if (filename.isEmpty())
        return;

    ui->txtValueFile->setText(filename);
    setupTblValue();
    ui->txtLabelFile->setText(filename.mid(0, filename.length()-5) + "label");
}

void ConvertBayesData::on_btnLabel_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Select Label File", QDir::currentPath() + "/data/", "Bayes Label Files (*.label)");
    if (filename.isEmpty())
        return;

    ui->txtLabelFile->setText(filename);
}

void ConvertBayesData::setupTblValue()
{
    auto orgValues = Utility::readTsvData(ui->txtValueFile->text().toStdString());
    if (ui->chbColumn->isChecked() && !ui->txtColumnOrder->text().isEmpty())
    {
        QStringList list = ui->txtColumnOrder->text().split(QRegExp("[;,]"));
        if (mHeaders.empty())
        {
            for (int i=0; i<list.size(); ++i)
            {
                mHeaders.append(QString::number(i));
            }
        }

        vector<vector<double>> values;
        bool ok;
        for (const auto & row : orgValues)
        {
            vector<double> newRow;
            for (const QString & s : list)
            {
                int column = s.toInt(&ok);
                if (!ok)
                {
                    QMessageBox msgBox(QMessageBox::Information, "Error", "\nInvalid Column Order \n"
                                        "Your specified column order is invalid",
                                       QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
                    msgBox.exec();
                    return;
                }
                if (column < 0 || column > row.size())
                    newRow.push_back(-1);
                else
                    newRow.push_back(row[column]);
            }
            values.push_back(newRow);
        }

        mModelValue->setValue(values);
    }
    else if (!orgValues.empty())
    {
        if (mHeaders.empty())
        {
            for (int i=0; i<orgValues[0].size(); ++i)
            {
                mHeaders.append(QString::number(i));
            }
//            for (int i=0; i<orgValues.size(); ++i)
//            {
//                auto threat = orgValues[i][8];
//                if (threat < 0.25)
//                    orgValues[i][9] = 0;
//                else if (threat < 0.5)
//                    orgValues[i][9] = 1;
//                else if (threat < 0.75)
//                    orgValues[i][9] = 2;
//                else if (threat < 1)
//                    orgValues[i][9] = 3;
//            }
        }
        mModelValue->setValue(orgValues);
    }
    mModelValue->setHeaders(mHeaders);
    setupTblThreshold();
}

void ConvertBayesData::setupPosition()
{
    move(200, 50);
}

void ConvertBayesData::setupTblThreshold()
{
    mModelThreshold->setRowCount(mHeaders.size());
}

void ConvertBayesData::on_btnConvert_clicked()
{
    if (ui->txtLabelFile->text().isEmpty())
    {
        QMessageBox msgBox(QMessageBox::Information, "Error", "\nMissing filename \n"
                            "You must select output label file",
                           QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
        msgBox.exec();
        return;
    }

    auto thresholds = mModelThreshold->getThresholds();
    if (thresholds.empty())
    {
        QMessageBox msgBox(QMessageBox::Information, "Error", "\nInvalid thresholds \n"
                            "You must specify valid thresholds",
                           QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
        msgBox.exec();
        return;
    }

    auto values = mModelValue->getValue();
    vector<vector<double>> labels;
    for (const auto & row : values)
    {
        vector<double> lRow;
        for (int i = 0; i < row.size(); ++i)
        {
            if (thresholds[i].empty())
            {
                /// discrete
                lRow.push_back(row[i]);
            }
            else if (thresholds[i].size() > 1)
            {
                int j;
                if (thresholds[i][0] < thresholds[i][1])
                {
                    /// positive increasement
                    for (j = 1; j < thresholds[i].size(); ++j)
                    {
                        if (thresholds[i][j] >= row[i])
                        {
                            break;
                        }
                    }
//                    lRow.push_back(thresholds[i].size()-j);
                    lRow.push_back(j-1);
                }
                else
                {
                    /// negative increasement
                    for (j = 0; j < thresholds[i].size(); ++j)
                    {
                        if (thresholds[i][j] <= row[i])
                        {
                            break;
                        }
                    }
                    lRow.push_back(j);
                }
            }
            else
            {
                lRow.push_back(-1);
            }
        }
        labels.push_back(lRow);
    }
    mModelLabel->setHeaders(mHeaders);
    mModelLabel->setValue(labels);
    ui->btnSaveLabel->setEnabled(true);
}

void ConvertBayesData::on_chbColumn_toggled(bool checked)
{
    Q_UNUSED(checked);
    ui->txtColumnOrder->setEnabled(ui->chbColumn->isChecked());
    on_txtColumnOrder_editingFinished();
}

void ConvertBayesData::on_txtColumnOrder_editingFinished()
{
    mHeaders.clear();
    setupTblValue();
}

void ConvertBayesData::on_btnSaveValue_clicked()
{
    auto filename = (ui->txtValueFile->text()+".save");
    Utility::writeTsvData(filename.toStdString(), mModelValue->getValue());
    QMessageBox msgBox(QMessageBox::Information, "Infomation", "\nSave Column Order to File \n"
                                                               "Data in Value table is saved in " + filename,
                       QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
    msgBox.exec();
}

void ConvertBayesData::on_btnSaveLabel_clicked()
{
    auto filename = ui->txtLabelFile->text();
    Utility::writeTsvData(filename.toStdString(), mModelLabel->getValue());
    QMessageBox msgBox(QMessageBox::Information, "Infomation", "\nSave Output Label to File \n"
                                                               "Data in Label table is saved in " + filename,
                       QMessageBox::Ok, this, Qt::Dialog | Qt::FramelessWindowHint);
    msgBox.exec();
}
