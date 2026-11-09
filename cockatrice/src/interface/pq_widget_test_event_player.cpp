#include "pq_widget_test_event_player.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QPixmap>
#include <QTextStream>
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
