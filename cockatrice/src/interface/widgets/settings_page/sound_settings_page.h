#ifndef COCKATRICE_SOUND_SETTINGS_PAGE_H
#define COCKATRICE_SOUND_SETTINGS_PAGE_H

#include "abstract_settings_page.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

class SoundSettingsPage : public AbstractSettingsPage
{
    Q_OBJECT
public:
    SoundSettingsPage();
    void retranslateUi() override;

private:
    QLabel themeLabel;
    QComboBox themeBox;
    QGroupBox *soundGroupBox;
    QPushButton soundTestButton;
    QCheckBox soundEnabledCheckBox;
    QLabel masterVolumeLabel;
    QSlider *masterVolumeSlider;
    QSpinBox *masterVolumeSpinBox;

private slots:
    void masterVolumeChanged(int value);
    void themeBoxChanged(int index);
};

#endif // COCKATRICE_SOUND_SETTINGS_PAGE_H
