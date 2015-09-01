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

#ifndef TSVMODEL_H
#define TSVMODEL_H

#include <vector>
#include <QAbstractTableModel>
#include <QStringList>

using namespace std;

class TsvModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TsvModel(const QStringList &headers, QObject* parent = 0);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setValue(const vector<vector<double>> & values);
    const vector<vector<double>>& getValue() const;
    void setHeaders(const QStringList & headers);
    void clear();

private:
    int mNumRows, mNumColumns;
    vector<vector<double>> mValues;
    QStringList mHeaders;
};

#endif // TSVMODEL_H
