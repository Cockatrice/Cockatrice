#include "table_activation.h"

#include <QKeySequence>
#include <QShortcut>
#include <QTableWidget>

namespace
{
void addActivationShortcut(QTableWidget *table,
                           QObject *context,
                           int key,
                           const std::function<void(const QModelIndex &)> &handler)
{
    auto *shortcut = new QShortcut(QKeySequence(key), table);
    // The table's viewport takes focus, so match children as well.
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, &QShortcut::activated, context, [table, handler] {
        handler(table->currentIndex());
    });
}
} // namespace

void connectTableActivation(QTableWidget *table,
                            QObject *context,
                            const std::function<void(const QModelIndex &)> &handler)
{
    QObject::connect(table, &QTableWidget::doubleClicked, context, handler);
    addActivationShortcut(table, context, Qt::Key_Return, handler);
    addActivationShortcut(table, context, Qt::Key_Enter, handler);
}
