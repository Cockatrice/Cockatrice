#ifndef PQ_WIDGET_TEST_EVENT_PLAYER_H
#define PQ_WIDGET_TEST_EVENT_PLAYER_H

#include "pqWidgetEventPlayer.h"

class QAbstractItemView;
class QModelIndex;

/**
 * Widget-level commands for the UI tests.
 *
 * pqCockatriceEventPlayer handles cards and zones inside the game scene. This
 * player covers the rest of the client: capturing a widget as an image,
 * dumping the paths QtTesting addresses widgets by, sizing the window, and
 * driving item views, whose selection QtTesting's own `currentChanged` command
 * cannot handle.
 *
 * Commands:
 *   screenshot "<path>"
 *       grabs the addressed widget and saves it as an image. Parent directories
 *       are created, and $NAME / ${NAME} in the path expand from the
 *       environment, so the runner decides where captures land
 *   dumpTree "<path>"
 *       writes the addressed widget's subtree with the paths QtTesting
 *       addresses it by, for writing a test
 *   resize "WIDTHxHEIGHT"
 *       resizes the addressed widget's top-level window
 *   selectRow "row[/row...][,column]"
 *       selects that row of an item view, the whole row, without a mouse event
 *   clickRow "row[/row...][,column]"
 *       left-clicks the centre of that cell, so the view's own handlers run
 *   doubleClickRow "row[/row...][,column]"
 *       double-clicks it, for rows that act on double click
 *
 * A tree addresses a row by the path of rows leading to it: "2/0" is the first
 * child of the third top-level row. A click opens a collapsed path first.
 * Rows are model rows of the addressed view, so a filtered or sorted model
 * counts the rows it shows.
 */
class pqWidgetTestEventPlayer : public pqWidgetEventPlayer
{
    Q_OBJECT
    typedef pqWidgetEventPlayer Superclass;

public:
    explicit pqWidgetTestEventPlayer(QObject *parent = nullptr);

    using Superclass::playEvent;

    bool playEvent(QObject *object, const QString &command, const QString &arguments, bool &error) override;

private:
    bool playScreenshot(QObject *object, const QString &arguments);
    bool playDumpTree(QObject *object, const QString &arguments);
    bool playResize(QObject *object, const QString &arguments);
    bool playSelectRow(QObject *object, const QString &arguments);
    bool playClickRow(QObject *object, const QString &arguments, bool doubleClick);

    /** @brief The item view the event is addressed at, following a viewport to its view. */
    static QAbstractItemView *itemViewFor(QObject *object);
    /** @brief Resolves "row[,column]" to a valid model index of @p view. */
    static bool resolveCell(QAbstractItemView *view, const QString &arguments, QModelIndex &index);
    /** @brief Sends the mouse events a real click on @p index produces. */
    static bool clickCell(QAbstractItemView *view, const QModelIndex &index, bool doubleClick);
    /** @brief Expands $NAME / ${NAME} and creates the file's parent directory. */
    static bool resolveOutputPath(const QString &command, const QString &path, QString &resolved);
};

#endif // PQ_WIDGET_TEST_EVENT_PLAYER_H
