/**
 * @file layouts_settings.h
 * @ingroup CoreSettings
 */
//! \todo Document this file.

#ifndef LAYOUTSSETTINGS_H
#define LAYOUTSSETTINGS_H

#include "settings_manager.h"

#include <QByteArray>
#include <QString>
#include <qtmetamacros.h>

class QObject;

class LayoutsSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;

public:
    void setMainWindowGeometry(const QByteArray &value);

    void setDeckEditorLayoutState(const QByteArray &value);
    void setDeckEditorGeometry(const QByteArray &value);

    void setVisualDeckEditorLayoutState(const QByteArray &value);
    void setVisualDeckEditorGeometry(const QByteArray &value);

    void setDeckEditorDbHeaderState(const QByteArray &value);
    void setGameSelectorHeaderState(const QByteArray &value, bool roomGames);
    void setRoomSelectorHeaderState(const QByteArray &value);
    // Invalidate older debounced saves in other open views of the same kind.
    void beginGameSelectorHeaderStateChange(bool roomGames);
    void beginRoomSelectorHeaderStateChange();

    void setSetsDialogHeaderState(const QByteArray &value);
    void setSetsDialogGeometry(const QByteArray &value);
    void setTokenDialogGeometry(const QByteArray &value);

    void setGamePlayAreaGeometry(const QByteArray &value);
    void setGamePlayAreaState(const QByteArray &value);

    void setReplayPlayAreaGeometry(const QByteArray &value);
    void setReplayPlayAreaState(const QByteArray &value);

    QByteArray getMainWindowGeometry() const;

    QByteArray getDeckEditorLayoutState() const;
    QByteArray getDeckEditorGeometry() const;

    QByteArray getVisualDeckEditorLayoutState() const;
    QByteArray getVisualDeckEditorGeometry() const;

    QByteArray getDeckEditorDbHeaderState() const;
    QByteArray getGameSelectorHeaderState(bool roomGames) const;
    QByteArray getRoomSelectorHeaderState() const;

    QByteArray getSetsDialogHeaderState() const;
    QByteArray getSetsDialogGeometry() const;
    QByteArray getTokenDialogGeometry() const;

    QByteArray getGamePlayAreaLayoutState() const;
    QByteArray getGamePlayAreaGeometry() const;

    QByteArray getReplayPlayAreaLayoutState() const;
    QByteArray getReplayPlayAreaGeometry() const;
signals:
    void gameSelectorHeaderStateChangeStarted(bool roomGames);
    void roomSelectorHeaderStateChangeStarted();

public slots:

private:
    explicit LayoutsSettings(const QString &settingPath, QObject *parent = nullptr);
    LayoutsSettings(const LayoutsSettings & /*other*/);
};

#endif // LAYOUTSSETTINGS_H
