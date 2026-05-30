#include "sound_settings_page.h"

#include "../../../client/settings/cache_settings.h"
#include "../client/sound_engine.h"

#include <QGridLayout>

SoundSettingsPage::SoundSettingsPage()
{
    soundEnabledCheckBox.setChecked(SettingsCache::instance().getSoundEnabled());
    connect(&soundEnabledCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance(),
            &SettingsCache::setSoundEnabled);

    QString themeName = SettingsCache::instance().getSoundThemeName();

    QStringList themeDirs = soundEngine->getAvailableThemes().keys();
    for (int i = 0; i < themeDirs.size(); i++) {
        themeBox.addItem(themeDirs[i]);
        if (themeDirs[i] == themeName) {
            themeBox.setCurrentIndex(i);
        }
    }

    connect(&themeBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundSettingsPage::themeBoxChanged);
    connect(&soundTestButton, &QPushButton::clicked, soundEngine, &SoundEngine::testSound);

    masterVolumeSlider = new QSlider(Qt::Horizontal);
    masterVolumeSlider->setMinimum(0);
    masterVolumeSlider->setMaximum(100);
    masterVolumeSlider->setValue(SettingsCache::instance().getMasterVolume());
    masterVolumeSlider->setToolTip(QString::number(SettingsCache::instance().getMasterVolume()));
    connect(&SettingsCache::instance(), &SettingsCache::masterVolumeChanged, this,
            &SoundSettingsPage::masterVolumeChanged);
    connect(masterVolumeSlider, &QSlider::sliderReleased, soundEngine, &SoundEngine::testSound);
    connect(masterVolumeSlider, &QSlider::valueChanged, &SettingsCache::instance(), &SettingsCache::setMasterVolume);

    masterVolumeSpinBox = new QSpinBox();
    masterVolumeSpinBox->setMinimum(0);
    masterVolumeSpinBox->setMaximum(100);
    masterVolumeSpinBox->setValue(SettingsCache::instance().getMasterVolume());
    connect(masterVolumeSlider, &QSlider::valueChanged, masterVolumeSpinBox, &QSpinBox::setValue);
    connect(masterVolumeSpinBox, qOverload<int>(&QSpinBox::valueChanged), masterVolumeSlider, &QSlider::setValue);

    auto *soundGrid = new QGridLayout;
    soundGrid->addWidget(&soundEnabledCheckBox, 0, 0, 1, 3);
    soundGrid->addWidget(&masterVolumeLabel, 1, 0);
    soundGrid->addWidget(masterVolumeSlider, 1, 1);
    soundGrid->addWidget(masterVolumeSpinBox, 1, 2);
    soundGrid->addWidget(&themeLabel, 2, 0);
    soundGrid->addWidget(&themeBox, 2, 1);
    soundGrid->addWidget(&soundTestButton, 3, 1);

    soundGroupBox = new QGroupBox;
    soundGroupBox->setLayout(soundGrid);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(soundGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(&SettingsCache::instance(), &SettingsCache::langChanged, this, &SoundSettingsPage::retranslateUi);
    retranslateUi();
}

void SoundSettingsPage::themeBoxChanged(int index)
{
    QStringList themeDirs = soundEngine->getAvailableThemes().keys();
    if (index >= 0 && index < themeDirs.count()) {
        SettingsCache::instance().setSoundThemeName(themeDirs.at(index));
    }
}

void SoundSettingsPage::masterVolumeChanged(int value)
{
    masterVolumeSlider->setToolTip(QString::number(value));
}

void SoundSettingsPage::retranslateUi()
{
    soundEnabledCheckBox.setText(tr("Enable &sounds"));
    themeLabel.setText(tr("Current sounds theme:"));
    soundTestButton.setText(tr("Test system sound engine"));
    soundGroupBox->setTitle(tr("Sound settings"));
    masterVolumeLabel.setText(tr("Master volume"));
}