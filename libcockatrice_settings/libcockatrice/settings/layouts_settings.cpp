#include "layouts_settings.h"

const static QString STATE_PROP = "state";
const static QString GEOMETRY_PROP = "geometry";
const static QString SIZE_PROP = "widgetSize";

const static QString GROUP_MAIN_WINDOW = "mainWindow";
const static QString GROUP_DECK_EDITOR = "deckEditor";
const static QString GROUP_DECK_EDITOR_DB = "deckEditorDb";
const static QString GROUP_SETS_DIALOG = "setsDialog";
const static QString GROUP_TOKEN_DIALOG = "tokenDialog";
const static QString GROUP_GAME_PLAY_AREA = "gamePlayArea";
const static QString GROUP_REPLAY_PLAY_AREA = "replayPlayArea";

LayoutsSettings::LayoutsSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "layouts.ini", QString(), QString(), parent)
{
}

void LayoutsSettings::setMainWindowGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_MAIN_WINDOW);
}

QByteArray LayoutsSettings::getMainWindowGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_MAIN_WINDOW).toByteArray();
}

QByteArray LayoutsSettings::getDeckEditorLayoutState()
{
    return getValue(STATE_PROP, GROUP_DECK_EDITOR).toByteArray();
}

void LayoutsSettings::setDeckEditorLayoutState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_DECK_EDITOR);
}

QByteArray LayoutsSettings::getDeckEditorGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_DECK_EDITOR).toByteArray();
}

void LayoutsSettings::setDeckEditorGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_DECK_EDITOR);
}

void LayoutsSettings::setDeckEditorWidgetSize(const QString &widgetName, const QSize &value)
{
    setValue(value, widgetName, GROUP_DECK_EDITOR, SIZE_PROP);
}

QSize LayoutsSettings::getDeckEditorWidgetSize(const QString &widgetName, const QSize &defaultValue)
{
    QVariant previous = getValue(widgetName, GROUP_DECK_EDITOR, SIZE_PROP);
    return previous == QVariant() ? defaultValue : previous.toSize();
}

QByteArray LayoutsSettings::getDeckEditorDbHeaderState()
{
    return getValue(STATE_PROP, GROUP_DECK_EDITOR_DB, "header").toByteArray();
}

void LayoutsSettings::setDeckEditorDbHeaderState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_DECK_EDITOR_DB, "header");
}

QByteArray LayoutsSettings::getSetsDialogHeaderState()
{
    return getValue(STATE_PROP, GROUP_SETS_DIALOG, "header").toByteArray();
}

void LayoutsSettings::setSetsDialogHeaderState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_SETS_DIALOG, "header");
}

void LayoutsSettings::setSetsDialogGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_SETS_DIALOG);
}

QByteArray LayoutsSettings::getSetsDialogGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_SETS_DIALOG).toByteArray();
}

void LayoutsSettings::setTokenDialogGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_TOKEN_DIALOG);
}

QByteArray LayoutsSettings::getTokenDialogGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_TOKEN_DIALOG).toByteArray();
}

void LayoutsSettings::setGamePlayAreaGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_GAME_PLAY_AREA);
}

void LayoutsSettings::setGamePlayAreaState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_GAME_PLAY_AREA);
}

QByteArray LayoutsSettings::getGamePlayAreaLayoutState()
{
    return getValue(STATE_PROP, GROUP_GAME_PLAY_AREA).toByteArray();
}

QByteArray LayoutsSettings::getGamePlayAreaGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_GAME_PLAY_AREA).toByteArray();
}

void LayoutsSettings::setGamePlayAreaWidgetSize(const QString &widgetName, const QSize &value)
{
    setValue(value, widgetName, GROUP_GAME_PLAY_AREA, SIZE_PROP);
}

QSize LayoutsSettings::getGamePlayAreaWidgetSize(const QString &widgetName, const QSize &defaultValue)
{
    QVariant previous = getValue(widgetName, GROUP_GAME_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? defaultValue : previous.toSize();
}

void LayoutsSettings::setReplayPlayAreaGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_REPLAY_PLAY_AREA);
}

void LayoutsSettings::setReplayPlayAreaState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_REPLAY_PLAY_AREA);
}

QByteArray LayoutsSettings::getReplayPlayAreaLayoutState()
{
    return getValue(STATE_PROP, GROUP_REPLAY_PLAY_AREA).toByteArray();
}

QByteArray LayoutsSettings::getReplayPlayAreaGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_REPLAY_PLAY_AREA).toByteArray();
}

void LayoutsSettings::setReplayPlayAreaWidgetSize(const QString &widgetName, const QSize &value)
{
    setValue(value, widgetName, GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
}

QSize LayoutsSettings::getReplayPlayAreaWidgetSize(const QString &widgetName, const QSize &defaultValue)
{
    QVariant previous = getValue(widgetName, GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? defaultValue : previous.toSize();
}