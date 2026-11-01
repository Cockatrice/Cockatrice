/**
 * @file table_activation.h
 * @brief Row activation for tables that must not navigate on a single click.
 */

#ifndef TABLE_ACTIVATION_H
#define TABLE_ACTIVATION_H

#include <functional>

class QModelIndex;
class QObject;
class QTableWidget;

/**
 * @brief Call @p handler on double-click or Enter, never on a single click.
 *
 * QAbstractItemView::activated fires on a single click on platforms where
 * SH_ItemView_ActivateItemOnSingleClick is set, so navigation actions must not
 * use it. This connects doubleClicked and installs Return/Enter shortcuts that
 * forward the current index.
 *
 * @param table   The table whose rows activate.
 * @param context Object that owns the connections; the handler is disconnected
 *                with it.
 * @param handler Receives the activated row's index, which may be invalid.
 */
void connectTableActivation(QTableWidget *table, QObject *context,
                            const std::function<void(const QModelIndex &)> &handler);

#endif // TABLE_ACTIVATION_H
