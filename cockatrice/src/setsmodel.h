#ifndef SETSMODEL_H
#define SETSMODEL_H

#include <QAbstractTableModel>
#include <QMimeData>
#include <QSet>
#include "carddatabase.h"

class SetsProxyModel;

class SetsMimeData : public QMimeData {
    Q_OBJECT
private:
    int oldRow;
public:
    SetsMimeData(int _oldRow) : oldRow(_oldRow) { }
    int getOldRow() const { return oldRow; }
    QStringList formats() const { return QStringList() << "application/x-cockatricecardset"; }
};

class SetsModel : public QAbstractTableModel {
    Q_OBJECT
    friend class SetsProxyModel;
private:
    static const int NUM_COLS = 7;
    SetList sets;
    QSet<CardSet *> enabledSets;
public:
    enum SetsColumns { SortKeyCol, IsKnownCol, EnabledCol, LongNameCol, ShortNameCol, SetTypeCol, ReleaseDateCol };
    enum Role { SortRole=Qt::UserRole };

    SetsModel(CardDatabase *_db, QObject *parent = 0);
    ~SetsModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return NUM_COLS; }
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    Qt::DropActions supportedDropActions() const;

    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QStringList mimeTypes() const;
    void swapRows(int oldRow, int newRow);
    void toggleRow(int row, bool enable);
    void toggleRow(int row);
    void toggleAll(bool);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void save(CardDatabase *db);
    void restore(CardDatabase *db);
};

#endif
