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

const QByteArray LayoutsSettings::getDeckEditorLayoutState()
{
    return getValue(STATE_PROP, GROUP_DECK_EDITOR).toByteArray();
}

void LayoutsSettings::setDeckEditorLayoutState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_DECK_EDITOR);
}

const QByteArray LayoutsSettings::getDeckEditorGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_DECK_EDITOR).toByteArray();
}

void LayoutsSettings::setDeckEditorGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_DECK_EDITOR);
}

QSize LayoutsSettings::getDeckEditorCardDatabaseSize()
{
    QVariant previous = getValue("cardDatabase", GROUP_DECK_EDITOR, SIZE_PROP);
    return previous == QVariant() ? QSize(500, 500) : previous.toSize();
}

void LayoutsSettings::setDeckEditorCardDatabaseSize(const QSize &value)
{
    setValue(value, "cardDatabase", GROUP_DECK_EDITOR, SIZE_PROP);
}

QSize LayoutsSettings::getDeckEditorCardSize()
{
    QVariant previous = getValue("card", GROUP_DECK_EDITOR, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 500) : previous.toSize();
}

void LayoutsSettings::setDeckEditorCardSize(const QSize &value)
{
    setValue(value, "card", GROUP_DECK_EDITOR, SIZE_PROP);
}

QSize LayoutsSettings::getDeckEditorDeckSize()
{
    QVariant previous = getValue("deck", GROUP_DECK_EDITOR, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 360) : previous.toSize();
}

void LayoutsSettings::setDeckEditorDeckSize(const QSize &value)
{
    setValue(value, "deck", GROUP_DECK_EDITOR, SIZE_PROP);
}

QSize LayoutsSettings::getDeckEditorPrintingSelectorSize()
{
    QVariant previous = getValue("printingSelector", GROUP_DECK_EDITOR, SIZE_PROP);
    return previous == QVariant() ? QSize(525, 250) : previous.toSize();
}

void LayoutsSettings::setDeckEditorPrintingSelectorSize(const QSize &value)
{
    setValue(value, "printingSelector", GROUP_DECK_EDITOR, SIZE_PROP);
}

QSize LayoutsSettings::getDeckEditorFilterSize()
{
    QVariant previous = getValue("filter", GROUP_DECK_EDITOR, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 250) : previous.toSize();
}

void LayoutsSettings::setDeckEditorFilterSize(const QSize &value)
{
    setValue(value, "filter", GROUP_DECK_EDITOR, SIZE_PROP);
}

const QByteArray LayoutsSettings::getDeckEditorDbHeaderState()
{
    return getValue(STATE_PROP, GROUP_DECK_EDITOR_DB, "header").toByteArray();
}

void LayoutsSettings::setDeckEditorDbHeaderState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_DECK_EDITOR_DB, "header");
}

const QByteArray LayoutsSettings::getSetsDialogHeaderState()
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

const QByteArray LayoutsSettings::getGamePlayAreaLayoutState()
{
    return getValue(STATE_PROP, GROUP_GAME_PLAY_AREA).toByteArray();
}

const QByteArray LayoutsSettings::getGamePlayAreaGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_GAME_PLAY_AREA).toByteArray();
}

const QSize LayoutsSettings::getGameCardInfoSize()
{
    QVariant previous = getValue("cardInfo", GROUP_GAME_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 360) : previous.toSize();
}

void LayoutsSettings::setGameCardInfoSize(const QSize &value)
{
    setValue(value, "cardInfo", GROUP_GAME_PLAY_AREA, SIZE_PROP);
}

const QSize LayoutsSettings::getGameMessageLayoutSize()
{
    QVariant previous = getValue("messageLayout", GROUP_GAME_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 250) : previous.toSize();
}

void LayoutsSettings::setGameMessageLayoutSize(const QSize &value)
{
    setValue(value, "messageLayout", GROUP_GAME_PLAY_AREA, SIZE_PROP);
}

const QSize LayoutsSettings::getGamePlayerListSize()
{
    QVariant previous = getValue("playerList", GROUP_GAME_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 50) : previous.toSize();
}

void LayoutsSettings::setGamePlayerListSize(const QSize &value)
{
    setValue(value, "playerList", GROUP_GAME_PLAY_AREA, SIZE_PROP);
}

void LayoutsSettings::setReplayPlayAreaGeometry(const QByteArray &value)
{
    setValue(value, GEOMETRY_PROP, GROUP_REPLAY_PLAY_AREA);
}

void LayoutsSettings::setReplayPlayAreaState(const QByteArray &value)
{
    setValue(value, STATE_PROP, GROUP_REPLAY_PLAY_AREA);
}

const QByteArray LayoutsSettings::getReplayPlayAreaLayoutState()
{
    return getValue(STATE_PROP, GROUP_REPLAY_PLAY_AREA).toByteArray();
}

const QByteArray LayoutsSettings::getReplayPlayAreaGeometry()
{
    return getValue(GEOMETRY_PROP, GROUP_REPLAY_PLAY_AREA).toByteArray();
}

const QSize LayoutsSettings::getReplayCardInfoSize()
{
    QVariant previous = getValue("cardInfo", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 360) : previous.toSize();
}

void LayoutsSettings::setReplayCardInfoSize(const QSize &value)
{
    setValue(value, "cardInfo", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
}

const QSize LayoutsSettings::getReplayMessageLayoutSize()
{
    QVariant previous = getValue("messageLayout", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 200) : previous.toSize();
}

void LayoutsSettings::setReplayMessageLayoutSize(const QSize &value)
{
    setValue(value, "messageLayout", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
}

const QSize LayoutsSettings::getReplayPlayerListSize()
{
    QVariant previous = getValue("playerList", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? QSize(250, 50) : previous.toSize();
}

void LayoutsSettings::setReplayPlayerListSize(const QSize &value)
{
    setValue(value, "playerList", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
}

const QSize LayoutsSettings::getReplayReplaySize()
{
    QVariant previous = getValue("replay", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
    return previous == QVariant() ? QSize(900, 100) : previous.toSize();
}

void LayoutsSettings::setReplayReplaySize(const QSize &value)
{
    setValue(value, "replay", GROUP_REPLAY_PLAY_AREA, SIZE_PROP);
}
