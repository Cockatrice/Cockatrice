#ifndef PQ_WIDGET_TEST_EVENT_PLAYER_H
#define PQ_WIDGET_TEST_EVENT_PLAYER_H

#include "pqWidgetEventPlayer.h"

/**
 * Widget-level commands for the UI tests.
 *
 * pqCockatriceEventPlayer handles cards and zones inside the game scene. This
 * player covers the rest of the client: capturing a widget as an image,
 * dumping the paths QtTesting addresses widgets by, and sizing the window.
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

    /** @brief Expands $NAME / ${NAME} and creates the file's parent directory. */
    static bool resolveOutputPath(const QString &command, const QString &path, QString &resolved);
};

#endif // PQ_WIDGET_TEST_EVENT_PLAYER_H
