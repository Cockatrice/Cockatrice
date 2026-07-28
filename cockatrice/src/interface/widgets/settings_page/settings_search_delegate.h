/**
 * @file settings_search_delegate.h
 * @brief Custom delegate for rendering settings search results
 * @ingroup Dialogs
 */
#ifndef COCKATRICE_SETTINGS_SEARCH_DELEGATE_H
#define COCKATRICE_SETTINGS_SEARCH_DELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief Custom paint delegate for settings search result items
 *
 * Renders each search result with a breadcrumb line ("Page > Group"),
 * the setting label, and a subtle separator. Supports selected/hovered states.
 */
class SettingsSearchDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SettingsSearchDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /** @brief Sets the translated page names for breadcrumb display */
    void setPageNames(const QStringList &names);

private:
    QStringList pageNames; ///< Translated page names indexed by page position
};

#endif // COCKATRICE_SETTINGS_SEARCH_DELEGATE_H
