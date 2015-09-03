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

    const QByteArray getDeckEditorLayoutState();
    const QByteArray getDeckEditorGeometry();
    const QSize getDeckEditorCardSize();
    const QSize getDeckEditorDeckSize();
    const QSize getDeckEditorFilterSize();
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
