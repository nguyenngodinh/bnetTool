/**
 * author trungvv1
 *
 * date 24 Jul 2015
 * class
 *
 * brief write something about your class
 *
 *
 */

#ifndef THRESHOLDMODEL_H
#define THRESHOLDMODEL_H

#include <vector>
#include <QAbstractTableModel>
#include <QStringList>

using namespace std;

class ThresholdModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ThresholdModel(int n, QObject* parent = 0);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    vector<vector<double>> getThresholds() const;
    void setRowCount(int n);

private:
    int mNRows, mNColumns;
    QStringList mThresholds;
};

#endif // THRESHOLDMODEL_H
