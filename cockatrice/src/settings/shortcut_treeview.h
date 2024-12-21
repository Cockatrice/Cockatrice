#ifndef SHORTCUT_TREEVIEW_H
#define SHORTCUT_TREEVIEW_H

#include <QModelIndex>
#include <QStandardItemModel>
#include <QTreeView>

class QSortFilterProxyModel;
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
    QSortFilterProxyModel *proxyModel;
    void populateShortcutsModel();

private slots:
    void refreshShortcuts();

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
};

#endif // SHORTCUT_TREEVIEW_H
