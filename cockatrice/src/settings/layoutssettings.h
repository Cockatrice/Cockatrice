#ifndef LAYOUTSSETTINGS_H
#define LAYOUTSSETTINGS_H

#include "settingsmanager.h"
#include <QSize>

class LayoutsSettings : public SettingsManager
{
    Q_OBJECT
    friend class SettingsCache;
public:    

    void setDeckEditorLayoutState(const QByteArray &value);
    void setDeckEditorGeometry(const QByteArray &value);
    void setDeckEditorCardSize(const QSize &value);
    void setDeckEditorDeckSize(const QSize &value);
    void setDeckEditorFilterSize(const QSize &value);
    void setDeckEditorDbHeaderState(const QByteArray &value);

    void setGamePlayAreaGeometry(const QByteArray &value);
    void setGamePlayAreaState(const QByteArray &value);
    void setGameCardInfoSize(const QSize &value);
    void setGameMessageLayoutSize(const QSize &value);
    void setGamePlayerListSize(const QSize &value);

    void setReplayPlayAreaGeometry(const QByteArray &value);
    void setReplayPlayAreaState(const QByteArray &value);
    void setReplayCardInfoSize(const QSize &value);
    void setReplayMessageLayoutSize(const QSize &value);
    void setReplayPlayerListSize(const QSize &value);
    void setReplayReplaySize(const QSize &value);

    const QByteArray getDeckEditorLayoutState();
    const QByteArray getDeckEditorGeometry();
    const QSize getDeckEditorCardSize();
    const QSize getDeckEditorDeckSize();
    const QSize getDeckEditorFilterSize();
    const QByteArray getDeckEditorDbHeaderState();

    const QByteArray getGamePlayAreaLayoutState();
    const QByteArray getGamePlayAreaGeometry();
    const QSize getGameCardInfoSize();
    const QSize getGameMessageLayoutSize();
    const QSize getGamePlayerListSize();

    const QByteArray getReplayPlayAreaLayoutState();
    const QByteArray getReplayPlayAreaGeometry();
    const QSize getReplayCardInfoSize();
    const QSize getReplayMessageLayoutSize();
    const QSize getReplayPlayerListSize();
    const QSize getReplayReplaySize();
signals:

public slots:

private:
    LayoutsSettings(QString settingPath,QObject *parent = 0);
    LayoutsSettings( const LayoutsSettings& /*other*/ );
    LayoutsSettings( LayoutsSettings& /*other*/ );
    LayoutsSettings( volatile const LayoutsSettings& /*other*/ );
    LayoutsSettings( volatile LayoutsSettings& /*other*/ );
};

#endif // LAYOUTSSETTINGS_H
