/**
 * @file shortcut_treeview.h
 * @ingroup CoreSettings
 * @brief TODO: Document this.
 */

#ifndef SHORTCUT_TREEVIEW_H
#define SHORTCUT_TREEVIEW_H

#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTreeView>

/**
 * Custom implementation of QSortFilterProxyModel that appends the source and parent strings together when filtering
 */
class ShortcutFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ShortcutFilterProxyModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class ShortcutTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit ShortcutTreeView(QWidget *parent);
    void retranslateUi();

signals:
    void currentItemChanged(const QString &shortcut);

public slots:
    void updateSearchString(const QString &searchString);

private:
    QStandardItemModel *shortcutsModel;
    ShortcutFilterProxyModel *proxyModel;
    void populateShortcutsModel();

private slots:
    void refreshShortcuts();

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
};

#endif // SHORTCUT_TREEVIEW_H
