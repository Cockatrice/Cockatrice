/**
 * @file reversed_completer_model.h
 * @ingroup UtilityWidgets
 */
//! \todo Document this file.

#ifndef REVERSED_COMPLETER_MODEL_H
#define REVERSED_COMPLETER_MODEL_H

#include <QAbstractProxyModel>

class QAbstractItemModel;

/**
 * @brief A completer model that can present its rows bottom-to-top.
 *
 * The original row order is kept intact in the source model (row 0 is always the
 * closest match). When enabled, the proxy maps the source rows in reverse so the
 * popup shows the closest match in the row nearest to the text edit. Any change
 * in the source model is forwarded as a full reset, which is all QCompleter
 * needs to rebuild its completion list.
 */
class ReversedCompleterModel : public QAbstractProxyModel
{
public:
    using QAbstractProxyModel::QAbstractProxyModel;

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    void setEnabled(bool enabled);

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    int sourceRowCount() const;
    void invalidate();

    bool isEnabled = false;
};

#endif // REVERSED_COMPLETER_MODEL_H
