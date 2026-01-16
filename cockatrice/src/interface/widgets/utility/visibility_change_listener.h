#ifndef COCKATRICE_VISIBILITY_LISTENER_H
#define COCKATRICE_VISIBILITY_LISTENER_H

#include <QObject>

/**
 * @brief This filter listens to the visibility changes of a target widget, emitting signals whenever the visibility of
 * that widget changes.
 */
class VisibilityChangeListener : public QObject
{
    Q_OBJECT

    QWidget *targetWidget;

public:
    /**
     * Creates a new instance of this class, watching the targetWidget.
     * This class automatically installs itself as an eventFilter to the targetWidget.
     *
     * @param targetWidget The widget to watch. Sets that widget as this object's parent.
     */
    explicit VisibilityChangeListener(QWidget *targetWidget);

    bool eventFilter(QObject *o, QEvent *e) override;

signals:
    /**
     * Emitted whenever the target widget's visibility changes
     * @param visible The widget's new visibility
     */
    void visibilityChanged(bool visible);
};

#endif // COCKATRICE_VISIBILITY_LISTENER_H
