/**
 * @file game_zone_options_widget.h
 * @ingroup RoomDialogs
 * @brief Reusable widget for Commander format zone options.
 *
 * This widget encapsulates the zone enable/disable checkboxes used in both
 * DlgCreateGame and DlgLocalGameOptions. It provides a simple interface
 * while hiding settings persistence details.
 */

#ifndef GAME_ZONE_OPTIONS_WIDGET_H
#define GAME_ZONE_OPTIONS_WIDGET_H

#include <QWidget>

class QCheckBox;
class QVBoxLayout;

/**
 * @class GameZoneOptionsWidget
 * @brief Widget for configuring Commander format zone options.
 *
 * Provides checkboxes for enabling command zone, companion zone, and
 * background zone. Handles settings persistence internally, so callers
 * don't need to know about SettingsCache keys or default values.
 */
class GameZoneOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameZoneOptionsWidget(QWidget *parent = nullptr);

    [[nodiscard]] bool enableCommandZone() const;
    [[nodiscard]] bool enableCompanionZone() const;
    [[nodiscard]] bool enableBackgroundZone() const;

    void setCommandZoneEnabled(bool enabled);
    void setCompanionZoneEnabled(bool enabled);
    void setBackgroundZoneEnabled(bool enabled);

    void loadFromSettings();
    void saveToSettings();
    void resetToDefaults();

    void setReadOnly(bool readOnly);

    QCheckBox *getCommandZoneCheckBox() const
    {
        return enableCommandZoneCheckBox;
    }
    QCheckBox *getCompanionZoneCheckBox() const
    {
        return enableCompanionZoneCheckBox;
    }
    QCheckBox *getBackgroundZoneCheckBox() const
    {
        return enableBackgroundZoneCheckBox;
    }

private:
    QCheckBox *enableCommandZoneCheckBox;
    QCheckBox *enableCompanionZoneCheckBox;
    QCheckBox *enableBackgroundZoneCheckBox;
};

#endif // GAME_ZONE_OPTIONS_WIDGET_H
