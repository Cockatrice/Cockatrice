#include "pq_widget_test_event_player.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QLoggingCategory>
#include <QMouseEvent>
#include <QPixmap>
#include <QTextStream>
#include <QTreeView>
#include <QWidget>
#include <pqObjectNaming.h>

Q_LOGGING_CATEGORY(PQ_WIDGET_TEST_EVENT_PLAYER, "cockatrice.guiTest.widgetEventPlayer")

namespace
{
void dumpSubtree(QObject *object, QTextStream &out, int depth)
{
    if (auto *widget = qobject_cast<QWidget *>(object)) {
        const QString path = pqObjectNaming::GetName(*widget);
        if (!path.isEmpty()) {
            out << QString(depth * 2, ' ') << path << "  [" << widget->metaObject()->className() << "]";
            if (!widget->objectName().isEmpty()) {
                out << "  objectName=" << widget->objectName();
            }
            out << '\n';
        }
    }

    // children() returns a snapshot, so the recursion below cannot invalidate it.
    const QObjectList children = object->children();
    for (QObject *child : children) {
        dumpSubtree(child, out, depth + 1);
    }
}

/**
 * Expands $NAME and ${NAME} from the environment.
 *
 * A test file is static, so this lets the runner send screenshots to a scratch
 * directory instead of hardcoding a path that only exists on one machine. An
 * unset variable is an error, not an empty string: writing to the working
 * directory would look like a pass.
 */
bool expandEnvironment(const QString &command, const QString &path, QString &expanded)
{
    expanded.clear();
    for (int i = 0; i < path.size(); ++i) {
        if (path.at(i) != QLatin1Char('$')) {
            expanded += path.at(i);
            continue;
        }

        int nameStart = i + 1;
        int nameEnd = nameStart;
        if (nameStart < path.size() && path.at(nameStart) == QLatin1Char('{')) {
            nameStart++;
            nameEnd = path.indexOf(QLatin1Char('}'), nameStart);
            if (nameEnd < 0) {
                qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << command << ": unterminated ${ in path" << path;
                return false;
            }
            i = nameEnd;
        } else {
            while (nameEnd < path.size() &&
                   (path.at(nameEnd).isLetterOrNumber() || path.at(nameEnd) == QLatin1Char('_'))) {
                nameEnd++;
            }
            i = nameEnd - 1;
        }

        const QString name = path.mid(nameStart, nameEnd - nameStart);
        if (name.isEmpty()) {
            expanded += QLatin1Char('$');
            continue;
        }

        const QString value = qEnvironmentVariable(name.toUtf8().constData());
        if (value.isEmpty()) {
            qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << command << ": environment variable" << name << "is not set";
            return false;
        }
        expanded += value;
    }
    return true;
}
} // namespace

pqWidgetTestEventPlayer::pqWidgetTestEventPlayer(QObject *parent) : pqWidgetEventPlayer(parent)
{
}

bool pqWidgetTestEventPlayer::playEvent(QObject *object, const QString &command, const QString &arguments, bool &error)
{
    // A command we know that fails is still a handled event: QtTesting logs
    // "Unhandled event" when playEvent returns false, so set `error` and return
    // true, as pqCockatriceEventPlayer does.
    const auto run = [&error](bool ok) {
        if (!ok) {
            error = true;
        }
        return true;
    };

    if (command == "screenshot") {
        return run(playScreenshot(object, arguments));
    }
    if (command == "dumpTree") {
        return run(playDumpTree(object, arguments));
    }
    if (command == "resize") {
        return run(playResize(object, arguments));
    }
    if (command == "selectRow") {
        return run(playSelectRow(object, arguments));
    }
    if (command == "clickRow") {
        return run(playClickRow(object, arguments, false));
    }
    if (command == "doubleClickRow") {
        return run(playClickRow(object, arguments, true));
    }
    return false;
}

bool pqWidgetTestEventPlayer::playScreenshot(QObject *object, const QString &arguments)
{
    auto *widget = qobject_cast<QWidget *>(object);
    if (!widget) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "screenshot: target is not a QWidget";
        return false;
    }

    QString path;
    if (!resolveOutputPath(QStringLiteral("screenshot"), arguments.trimmed(), path)) {
        return false;
    }

    // repaint() is synchronous, even on the offscreen platform, so the grab
    // contains what the previous events drew.
    widget->repaint();
    const QPixmap pixmap = widget->grab();
    if (pixmap.isNull()) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "screenshot: grab returned a null pixmap for" << widget;
        return false;
    }
    if (!pixmap.save(path)) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER)
            << "screenshot: could not save" << path << "- an image format is chosen from the file extension";
        return false;
    }

    qCInfo(PQ_WIDGET_TEST_EVENT_PLAYER) << "screenshot saved to" << path;
    return true;
}

bool pqWidgetTestEventPlayer::playDumpTree(QObject *object, const QString &arguments)
{
    if (!qobject_cast<QWidget *>(object)) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "dumpTree: target is not a QWidget";
        return false;
    }

    QString path;
    if (!resolveOutputPath(QStringLiteral("dumpTree"), arguments.trimmed(), path)) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "dumpTree: could not open" << path << file.errorString();
        return false;
    }

    QTextStream out(&file);
    dumpSubtree(object, out, 0);

    qCInfo(PQ_WIDGET_TEST_EVENT_PLAYER) << "widget tree dumped to" << path;
    return true;
}

bool pqWidgetTestEventPlayer::playResize(QObject *object, const QString &arguments)
{
    auto *widget = qobject_cast<QWidget *>(object);
    if (!widget) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "resize: target is not a QWidget";
        return false;
    }

    const QStringList parts = arguments.split(QLatin1Char('x'));
    if (parts.size() != 2) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "resize: arguments must be 'WIDTHxHEIGHT', got:" << arguments;
        return false;
    }

    bool okWidth = false;
    bool okHeight = false;
    const int width = parts.at(0).trimmed().toInt(&okWidth);
    const int height = parts.at(1).trimmed().toInt(&okHeight);
    if (!okWidth || !okHeight || width <= 0 || height <= 0) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "resize: invalid size" << arguments;
        return false;
    }

    // Resize the top-level window: the layout owns the children.
    widget->window()->resize(width, height);
    QApplication::processEvents();
    return true;
}

bool pqWidgetTestEventPlayer::playSelectRow(QObject *object, const QString &arguments)
{
    QAbstractItemView *view = itemViewFor(object);
    if (!view) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "selectRow: target is not an item view";
        return false;
    }

    QModelIndex index;
    if (!resolveCell(view, arguments, index)) {
        return false;
    }

    view->setCurrentIndex(index);
    // QtTesting's `currentChanged` crashes on a one-element row list, so select
    // through the selection model.
    view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->scrollTo(index);
    return true;
}

bool pqWidgetTestEventPlayer::playClickRow(QObject *object, const QString &arguments, bool doubleClick)
{
    QAbstractItemView *view = itemViewFor(object);
    if (!view) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "clickRow: target is not an item view";
        return false;
    }

    QModelIndex index;
    if (!resolveCell(view, arguments, index)) {
        return false;
    }

    return clickCell(view, index, doubleClick);
}

QAbstractItemView *pqWidgetTestEventPlayer::itemViewFor(QObject *object)
{
    if (auto *view = qobject_cast<QAbstractItemView *>(object)) {
        return view;
    }
    // A recording names the viewport of an item view; the view is its parent.
    return qobject_cast<QAbstractItemView *>(object ? object->parent() : nullptr);
}

bool pqWidgetTestEventPlayer::resolveCell(QAbstractItemView *view, const QString &arguments, QModelIndex &index)
{
    QAbstractItemModel *model = view->model();
    if (!model) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "item view has no model:" << view;
        return false;
    }

    const QStringList parts = arguments.split(QLatin1Char(','));
    if (parts.isEmpty() || parts.size() > 2) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER)
            << "cell must be 'row[/row...]' or 'row[/row...],column', got:" << arguments;
        return false;
    }

    int column = 0;
    if (parts.size() == 2) {
        bool okColumn = false;
        column = parts.at(1).trimmed().toInt(&okColumn);
        if (!okColumn || column < 0) {
            qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "invalid column in" << arguments;
            return false;
        }
    }

    // "2/0" is the first child of the third top-level row. A tree addresses a
    // row by the path of rows leading to it.
    const QStringList rows = parts.at(0).split(QLatin1Char('/'));
    index = QModelIndex();
    for (int depth = 0; depth < rows.size(); ++depth) {
        bool okRow = false;
        const int row = rows.at(depth).trimmed().toInt(&okRow);
        const int columnCount = model->columnCount(index);
        if (!okRow || row < 0 || row >= model->rowCount(index)) {
            qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "invalid row" << rows.at(depth) << "in" << arguments
                                                    << "- the parent has" << model->rowCount(index) << "rows";
            return false;
        }
        if (column >= columnCount) {
            qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER)
                << "invalid column" << column << "in" << arguments << "- the row has" << columnCount << "columns";
            return false;
        }

        index = model->index(row, depth == rows.size() - 1 ? column : 0, index);
        if (!index.isValid()) {
            qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "no model index for" << arguments;
            return false;
        }
    }
    return true;
}

bool pqWidgetTestEventPlayer::clickCell(QAbstractItemView *view, const QModelIndex &index, bool doubleClick)
{
    // A collapsed tree has no rect for a child, so open the path to it first.
    if (auto *tree = qobject_cast<QTreeView *>(view); tree && index.parent().isValid()) {
        tree->expand(index.parent());
    }
    view->scrollTo(index);
    // scrollTo only requests the scroll, so the visual rect is stale until the
    // view has laid out again.
    QApplication::processEvents();

    const QRect rect = view->visualRect(index);
    if (!rect.isValid() || rect.isEmpty()) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << "cell" << index << "is not visible in" << view;
        return false;
    }

    QWidget *viewport = view->viewport();
    const QPoint pos = rect.center();
    const auto send = [viewport, &pos](QEvent::Type type) {
        QMouseEvent event(type, QPointF(pos), QPointF(viewport->mapToGlobal(pos)), Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
        QApplication::sendEvent(viewport, &event);
    };

    // A double click is press/release, then the double-click event and its
    // release, as a recording produces it.
    send(QEvent::MouseButtonPress);
    send(QEvent::MouseButtonRelease);
    if (doubleClick) {
        send(QEvent::MouseButtonDblClick);
        send(QEvent::MouseButtonRelease);
    }
    return true;
}

bool pqWidgetTestEventPlayer::resolveOutputPath(const QString &command, const QString &path, QString &resolved)
{
    if (path.isEmpty()) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << command << ": output path is empty";
        return false;
    }

    QString expanded;
    if (!expandEnvironment(command, path, expanded)) {
        return false;
    }

    const QFileInfo info(expanded);
    const QString directory = info.absolutePath();
    if (!QDir().mkpath(directory)) {
        qCCritical(PQ_WIDGET_TEST_EVENT_PLAYER) << command << ": could not create directory" << directory;
        return false;
    }

    resolved = info.absoluteFilePath();
    return true;
}
