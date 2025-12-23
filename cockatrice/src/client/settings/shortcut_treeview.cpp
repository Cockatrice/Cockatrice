#include "shortcut_treeview.h"

#include "cache_settings.h"
#include "shortcuts_settings.h"

#include <QHeaderView>

ShortcutFilterProxyModel::ShortcutFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

/**
 * Appends the parent and source row together before doing the regex match.
 */
bool ShortcutFilterProxyModel::filterAcceptsRow(const int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex nameIndex = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);
    QModelIndex parentIndex = sourceModel()->index(sourceParent.row(), filterKeyColumn(), sourceParent.parent());

    QString name = sourceModel()->data(nameIndex).toString();
    QString parentName = sourceModel()->data(parentIndex).toString();

    QString searchedString = parentName + " " + name;

    return searchedString.contains(filterRegularExpression());
}

ShortcutTreeView::ShortcutTreeView(QWidget *parent) : QTreeView(parent)
{
    // model
    shortcutsModel = new QStandardItemModel(this);
    shortcutsModel->setColumnCount(3);
    populateShortcutsModel();

    // filter proxy
    proxyModel = new ShortcutFilterProxyModel(this);
    proxyModel->setRecursiveFilteringEnabled(true);
    proxyModel->setSourceModel(shortcutsModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0);

    QTreeView::setModel(proxyModel);

    // treeview
    hideColumn(2);

    setUniformRowHeights(true);
    setAlternatingRowColors(true);

    setSortingEnabled(true);
    proxyModel->sort(0, Qt::AscendingOrder);

    header()->setSectionResizeMode(QHeaderView::Interactive);
    header()->setSortIndicator(0, Qt::AscendingOrder);
    setSelectionMode(SingleSelection);
    setSelectionBehavior(SelectRows);

    expandAll();

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &ShortcutTreeView::refreshShortcuts);
}

void ShortcutTreeView::populateShortcutsModel()
{
    QHash<QString, QStandardItem *> parentItems;
    QStandardItem *curParent = nullptr;
    for (const auto &key : SettingsCache::instance().shortcuts().getAllShortcutKeys()) {
        QString name = SettingsCache::instance().shortcuts().getShortcut(key).getName();
        QString group = SettingsCache::instance().shortcuts().getShortcut(key).getGroupName();
        QString shortcut = SettingsCache::instance().shortcuts().getShortcutString(key);

        if (parentItems.contains(group)) {
            curParent = parentItems.value(group);
        } else {
            curParent = new QStandardItem(group);
            static QFont font = curParent->font();
            font.setBold(true);
            curParent->setFont(font);
            parentItems.insert(group, curParent);
        }

        QList<QStandardItem *> list = {};
        list << new QStandardItem(name) << new QStandardItem(shortcut) << new QStandardItem(key);
        curParent->appendRow(list);
    }

    for (const auto &parent : parentItems) {
        shortcutsModel->appendRow(parent);
    }
}

void ShortcutTreeView::retranslateUi()
{
    shortcutsModel->setHeaderData(0, Qt::Horizontal, tr("Action"));
    shortcutsModel->setHeaderData(1, Qt::Horizontal, tr("Shortcut"));
    refreshShortcuts();
}

/**
 * Loops over the model and reloads all rows
 */
static void loopOverModel(QAbstractItemModel *model, const QModelIndex &parent = QModelIndex())
{
    for (int r = 0; r < model->rowCount(parent); ++r) {
        const auto friendlyNameIndex = model->index(r, 0, parent);

        if (model->hasChildren(friendlyNameIndex)) {
            const auto childIndex = model->index(0, 2, friendlyNameIndex);
            const auto key = model->data(childIndex).toString();
            const auto shortcutGroupName = SettingsCache::instance().shortcuts().getShortcut(key).getGroupName();
            model->setData(friendlyNameIndex, shortcutGroupName);

            loopOverModel(model, friendlyNameIndex);
        } else {
            const auto shortcutSequenceIndex = model->index(r, 1, parent);
            const auto keyIndex = model->index(r, 2, parent);
            const auto key = model->data(keyIndex).toString();

            const auto shortcutKey = SettingsCache::instance().shortcuts().getShortcut(key).getName();
            const auto shortcutSequence = SettingsCache::instance().shortcuts().getShortcutString(key);
            model->setData(friendlyNameIndex, shortcutKey);
            model->setData(shortcutSequenceIndex, shortcutSequence);
        }
    }
}

void ShortcutTreeView::refreshShortcuts()
{
    loopOverModel(shortcutsModel);
}

void ShortcutTreeView::currentChanged(const QModelIndex &current, const QModelIndex & /* previous */)
{
    QTreeView::scrollTo(current, QAbstractItemView::EnsureVisible);
    if (current.parent().isValid()) {
        auto shortcutName = model()->data(model()->index(current.row(), 2, current.parent())).toString();
        emit currentItemChanged(shortcutName);
    } else {
        // emit empty string if the selection is a category header
        emit currentItemChanged("");
    }
}

/**
 * The search string is split by word.
 * A String is a match as long as it contains all the words in the search string in order
 */
void ShortcutTreeView::updateSearchString(const QString &searchString)
{
    QStringList searchWords = searchString.split(" ", Qt::SkipEmptyParts);

    auto escapeRegex = [](const QString &s) { return QRegularExpression::escape(s); };
    std::transform(searchWords.begin(), searchWords.end(), searchWords.begin(), escapeRegex);

    auto regex = QRegularExpression(searchWords.join(".*"), QRegularExpression::CaseInsensitiveOption);

    proxyModel->setFilterRegularExpression(regex);
    expandAll();
}
