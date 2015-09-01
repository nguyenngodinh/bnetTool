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

#include "tsvmodel.h"

TsvModel::TsvModel(const QStringList & headers, QObject* parent)
    : QAbstractTableModel(parent), mHeaders(headers)
{
    mNumRows = mNumColumns = 0;
}

QVariant TsvModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (index.isValid())
    {
        if (role == Qt::DisplayRole)
        {            
            auto s = QString::number(mValues[row][col], 'f', 2);
            if (s.endsWith(".00"))                  /// round to int
                s = s.mid(0, s.length()-3);
            if (s.contains(".") && s.length() > 6)  /// round to int
                s = s.mid(0, s.length()-3);
            if (s.length() > 6)                     /// 111111 = 111e3
                s = s.mid(0, 3) + "e" + QString::number(s.length()-3);
            return s;
        }
        else if (role == Qt::TextAlignmentRole)
        {
            return Qt::AlignCenter;
        }
    }
    return QVariant();
}

bool TsvModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (value.toString().isEmpty()) {
            return false;
        }
        double newValue = value.toDouble();
        mValues[index.row()][index.column()-1] = newValue;
        return true;
    }
    return false;
}

QVariant TsvModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
        {
            return mHeaders[section];
        }
        else if (orientation == Qt::Vertical)
        {
            return section+1;
        }
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    return QVariant();
}

int TsvModel::rowCount(const QModelIndex &parent) const
{
    return mNumRows;
}

int TsvModel::columnCount(const QModelIndex &parent) const
{
    return mNumColumns;
}

bool TsvModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

bool TsvModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

Qt::ItemFlags TsvModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

void TsvModel::setValue(const vector<vector<double>> & values)
{
    beginResetModel();
    clear();
    mNumRows = values.size();    
    mValues = values;
    endResetModel();
}

const vector<vector<double> > &TsvModel::getValue() const
{
    return mValues;
}

void TsvModel::setHeaders(const QStringList &headers)
{
    beginResetModel();
    mHeaders.clear();
    mHeaders.append(headers);
    mNumColumns = mHeaders.size();
    endResetModel();
}

void TsvModel::clear()
{
    mNumRows = 0;
    mValues.clear();
}

