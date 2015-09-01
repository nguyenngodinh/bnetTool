#include "dialogquickpredict.h"
#include "ui_dialogquickpredict.h"

DialogQuickPredict::DialogQuickPredict(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogQuickPredict)
{
    ui->setupUi(this);
    ui->lineEdit->setFocus();
}

DialogQuickPredict::~DialogQuickPredict()
{
    delete ui;
}

void DialogQuickPredict::focusText()
{
    ui->lineEdit->setFocus();
}

QList<int> DialogQuickPredict::getValues()
{
    QString text = ui->lineEdit->text();
    QStringList texts;
    if (text.contains(";"))
        texts = text.split(";");
    else if (text.contains(","))
        texts = text.split(",");
    else
        texts = text.split(" ");
    QList<int> values;
    for (QString text : texts)
    {
        if (!text.isEmpty()) {
            values.push_back(text.toInt());
        }
    }
    return values;
}

void DialogQuickPredict::setValues(QList<int> values)
{
    QString text;
    for (uint i = 0; i < values.size(); ++i) {
        text += QString::number(values[i]);
        text += ";";
    }
    ui->lineEdit->setText(text);
}
