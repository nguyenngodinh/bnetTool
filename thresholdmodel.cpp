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

#include "thresholdmodel.h"

ThresholdModel::ThresholdModel(int n, QObject* parent)
{
    ncolumns = 2;
    setRowCount(n);
}

QVariant ThresholdModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (index.isValid())
    {
        if (role == Qt::DisplayRole) {
            switch (col) {
            case 0:
                return row;
            case 1:
                return mThresholds[row];
            default:
                return 0;
            }
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        }
    }
    return QVariant();
}

bool ThresholdModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (value.toString().isEmpty()) {
            return false;
        }
        auto newValue = value.toString();
        mThresholds[index.row()] = newValue;
        return true;
    }
    return false;
}

QVariant ThresholdModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal)
        {
            switch (section) {
            case 0:
                return "No.";
            case 1:
                return "Thresholds\t\t";
            default:
                return "";
            }
        }
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignLeft;
    }
    return QVariant();
}

int ThresholdModel::rowCount(const QModelIndex &parent) const
{
    return nrows;
}

int ThresholdModel::columnCount(const QModelIndex &parent) const
{
    return ncolumns;
}

bool ThresholdModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

bool ThresholdModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

Qt::ItemFlags ThresholdModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

vector<vector<double>> ThresholdModel::getThresholds() const
{
    vector<vector<double>> rs;
    bool ok;
    for (int i=0; i<mThresholds.size(); ++i)
    {
        vector<double> row;
        if (!mThresholds[i].isEmpty())
        {
            auto split = mThresholds[i].split(QRegExp("[,;]"));
            for (const auto & s : split)
            {
                row.push_back(s.toDouble(&ok));
                if (!ok)
                    return {};
            }
//            std::sort(row.begin(), row.end());
        }
        rs.push_back(row);
    }

    return rs;
}

void ThresholdModel::setRowCount(int n)
{
    beginResetModel();
    nrows = n;
    mThresholds.clear();
    for (int i=0; i<n; ++i)
        mThresholds.append("");

    /// hard-code for threat
    if (n == 10)
    {
        mThresholds.clear();
        mThresholds << "0,150,250,450"
                    << "0,1500,4500,10500"
                    << "" // "0,1"
                    << "" // "1,2,3,4,5,6,7,8"
                    << "" // "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15"
                    << "0,0.75,1.5,3.5"
                    << "0,45,90,215"
                    << "" // "0,1,2"
                    << "0,0.5"
                    << ""
                       ;
    }
    endResetModel();
}
