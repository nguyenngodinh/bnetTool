/**
 * \author trungvv1
 *
 * \date 4/1/2015
 * \class CptModel
 *
 * \brief write something about your class
 *
 *
 */

#ifndef CPTMODEL_H
#define CPTMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

class CptModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CptModel(QObject* parent = 0);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void initValues(int mNRows, int mNColumns, int mNodeparents);
    void setValue(int row, int column, double value);
    double getValue(int row, int column);
    void setHeader(int column, QString value);
    void clear();

private:
    int mNRows, mNColumns;
    int mNodeparents;           /// number of node's parents
    double **mValues;        /// rows = number of assignment of parents
                            /// cols = nparents + nvalues
    QStringList mHeaders;
};

#endif // CPTMODEL_H
