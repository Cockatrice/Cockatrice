/**
 * @file layouts_settings.h
 * @ingroup CoreSettings
 * @brief TODO: Document this.
 */

#ifndef LAYOUTSSETTINGS_H
#define LAYOUTSSETTINGS_H

#include "settings_manager.h"

#include <QSize>

class LayoutsSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    void setMainWindowGeometry(const QByteArray &value);

    void setDeckEditorLayoutState(const QByteArray &value);
    void setDeckEditorGeometry(const QByteArray &value);
    void setDeckEditorWidgetSize(const QString &widgetName, const QSize &value);

    void setDeckEditorDbHeaderState(const QByteArray &value);
    void setSetsDialogHeaderState(const QByteArray &value);
    void setSetsDialogGeometry(const QByteArray &value);
    void setTokenDialogGeometry(const QByteArray &value);

    void setGamePlayAreaGeometry(const QByteArray &value);
    void setGamePlayAreaState(const QByteArray &value);
    void setGamePlayAreaWidgetSize(const QString &widgetName, const QSize &value);

    void setReplayPlayAreaGeometry(const QByteArray &value);
    void setReplayPlayAreaState(const QByteArray &value);
    void setReplayPlayAreaWidgetSize(const QString &widgetName, const QSize &value);

    QByteArray getMainWindowGeometry();

    QByteArray getDeckEditorLayoutState();
    QByteArray getDeckEditorGeometry();
    QSize getDeckEditorWidgetSize(const QString &widgetName, const QSize &defaultValue = {});

    QByteArray getDeckEditorDbHeaderState();
    QByteArray getSetsDialogHeaderState();
    QByteArray getSetsDialogGeometry();
    QByteArray getTokenDialogGeometry();

    QByteArray getGamePlayAreaLayoutState();
    QByteArray getGamePlayAreaGeometry();
    QSize getGamePlayAreaWidgetSize(const QString &widgetName, const QSize &defaultValue = {});

    QByteArray getReplayPlayAreaLayoutState();
    QByteArray getReplayPlayAreaGeometry();
    QSize getReplayPlayAreaWidgetSize(const QString &widgetName, const QSize &defaultValue = {});
signals:

public slots:

private:
    explicit LayoutsSettings(const QString &settingPath, QObject *parent = nullptr);
    LayoutsSettings(const LayoutsSettings & /*other*/);
};

#endif // LAYOUTSSETTINGS_H
