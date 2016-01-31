#include "layoutssettings.h"

LayoutsSettings::LayoutsSettings(QString settingPath, QObject *parent)
    : SettingsManager(settingPath+"layouts.ini", parent)
{
}

const QByteArray LayoutsSettings::getDeckEditorLayoutState()
{
    return getValue("layouts/deckEditor_state").toByteArray();
}

void LayoutsSettings::setDeckEditorLayoutState(const QByteArray &value)
{
    setValue(value,"layouts/deckEditor_state");
}

const QByteArray LayoutsSettings::getDeckEditorGeometry()
{
    return getValue("layouts/deckEditor_geometry").toByteArray();
}

void LayoutsSettings::setDeckEditorGeometry(const QByteArray &value)
{
    setValue(value,"layouts/deckEditor_geometry");
}

const QSize LayoutsSettings::getDeckEditorCardSize()
{
    QVariant previous = getValue("layouts/deckEditor_CardSize");
    return previous == QVariant() ? QSize(250,500) : previous.toSize();
}

void LayoutsSettings::setDeckEditorCardSize(const QSize &value)
{
    setValue(value,"layouts/deckEditor_CardSize");
}

const QSize LayoutsSettings::getDeckEditorDeckSize()
{
    QVariant previous = getValue("layouts/deckEditor_DeckSize");
    return previous == QVariant() ? QSize(250,360) : previous.toSize();
}

void LayoutsSettings::setDeckEditorDeckSize(const QSize &value)
{
    setValue(value,"layouts/deckEditor_DeckSize");
}

const QSize LayoutsSettings::getDeckEditorFilterSize()
{
    QVariant previous = getValue("layouts/deckEditor_FilterSize");
    return previous == QVariant() ? QSize(250,250) : previous.toSize();
}

void LayoutsSettings::setDeckEditorFilterSize(const QSize &value)
{
    setValue(value,"layouts/deckEditor_FilterSize");
}

void LayoutsSettings::setGamePlayAreaGeometry(const QByteArray &value)
{
    setValue(value,"layouts/gameplayarea_geometry");
}

void LayoutsSettings::setGamePlayAreaState(const QByteArray &value)
{
    setValue(value,"layouts/gameplayarea_state");
}

const QByteArray LayoutsSettings::getGamePlayAreaLayoutState()
{
    return getValue("layouts/gameplayarea_state").toByteArray();
}

const QByteArray LayoutsSettings::getGamePlayAreaGeometry()
{
    return getValue("layouts/gameplayarea_geometry").toByteArray();
}

void LayoutsSettings::setReplayPlayAreaGeometry(const QByteArray &value)
{
    setValue(value,"layouts/replayplayarea_geometry");
}

void LayoutsSettings::setReplayPlayAreaState(const QByteArray &value)
{
    setValue(value,"layouts/replayplayarea_state");
}

const QByteArray LayoutsSettings::getReplayPlayAreaLayoutState()
{
    return getValue("layouts/replayplayarea_state").toByteArray();
}

const QByteArray LayoutsSettings::getReplayPlayAreaGeometry()
{
    return getValue("layouts/replayplayarea_geometry").toByteArray();
}
