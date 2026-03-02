/**
 * @file sets_model.h
 * @ingroup CardDatabaseModels
 * @brief TODO: Document this.
 */

#ifndef SETSMODEL_H
#define SETSMODEL_H

#include <QAbstractTableModel>
#include <QMimeData>
#include <QSet>
#include <QSortFilterProxyModel>
#include <libcockatrice/card/database/card_database.h>

class SetsProxyModel;

class SetsMimeData : public QMimeData
{
    Q_OBJECT
private:
    int oldRow;

public:
    SetsMimeData(int _oldRow) : oldRow(_oldRow)
    {
    }
    [[nodiscard]] int getOldRow() const
    {
        return oldRow;
    }
    [[nodiscard]] QStringList formats() const
    {
        return QStringList() << "application/x-cockatricecardset";
    }
};

class SetsModel : public QAbstractTableModel
{
    Q_OBJECT
    friend class SetsProxyModel;

private:
    static const int NUM_COLS = 7;
    CardSetList sets;
    QSet<CardSetPtr> enabledSets;

public:
    enum SetsColumns
    {
        SortKeyCol,
        IsKnownCol,
        EnabledCol,
        LongNameCol,
        ShortNameCol,
        SetTypeCol,
        ReleaseDateCol,
        PriorityCol
    };
    enum Role
    {
        SortRole = Qt::UserRole
    };

    explicit SetsModel(CardDatabase *_db, QObject *parent = nullptr);
    ~SetsModel() override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return NUM_COLS;
    }
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
    [[nodiscard]] Qt::DropActions supportedDropActions() const override;

    [[nodiscard]] QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool
    dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    [[nodiscard]] QStringList mimeTypes() const override;
    void swapRows(int oldRow, int newRow);
    void toggleRow(int row, bool enable);
    void toggleRow(int row);
    void toggleAll(bool);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void save(CardDatabase *db);
    void restore(CardDatabase *db);
    void restoreOriginalOrder();
};

class SetsDisplayModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SetsDisplayModel(QObject *parent = nullptr);

protected:
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void fetchMore(const QModelIndex &index) override;
};

#endif
