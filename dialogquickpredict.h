#ifndef DIALOGQUICKPREDICT_H
#define DIALOGQUICKPREDICT_H

#include <QDialog>

namespace Ui {
class DialogQuickPredict;
}

class DialogQuickPredict : public QDialog
{
    Q_OBJECT

public:
    explicit DialogQuickPredict(QWidget *parent = 0);
    ~DialogQuickPredict();

    void focusText();
    QList<int> getValues();
    void setValues(QList<int>);

private slots:

private:
    Ui::DialogQuickPredict *ui;
};

#endif // DIALOGQUICKPREDICT_H
