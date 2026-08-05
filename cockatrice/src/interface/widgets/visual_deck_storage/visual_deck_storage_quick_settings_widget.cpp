#include "visual_deck_storage_quick_settings_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "visual_deck_storage_widget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

VisualDeckStorageQuickSettingsWidget::VisualDeckStorageQuickSettingsWidget(QWidget *parent)
    : SettingsButtonWidget(parent)
{
    // show folders checkbox
    showFoldersCheckBox = new QCheckBox(this);
    showFoldersCheckBox->setChecked(SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowFolders());
    connect(showFoldersCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &VisualDeckStorageQuickSettingsWidget::showFoldersChanged);
    connect(showFoldersCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowFolders);

    // show tag filter widget checkbox
    showTagFilterCheckBox = new QCheckBox(this);
    showTagFilterCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagFilter());
    connect(showTagFilterCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &VisualDeckStorageQuickSettingsWidget::showTagFilterChanged);
    connect(showTagFilterCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowTagFilter);

    // show color identity on DeckPreviewWidget checkbox
    showColorIdentityCheckBox = new QCheckBox(this);
    showColorIdentityCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowColorIdentity());
    connect(showColorIdentityCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &VisualDeckStorageQuickSettingsWidget::showColorIdentityChanged);
    connect(showColorIdentityCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowColorIdentity);

    // show tags on DeckPreviewWidget checkbox
    showTagsOnDeckPreviewsCheckBox = new QCheckBox(this);
    showTagsOnDeckPreviewsCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagsOnDeckPreviews());
    connect(showTagsOnDeckPreviewsCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &VisualDeckStorageQuickSettingsWidget::showTagsOnDeckPreviewsChanged);
    connect(showTagsOnDeckPreviewsCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowTagsOnDeckPreviews);

    // show banner card selector checkbox
    showBannerCardComboBoxCheckBox = new QCheckBox(this);
    showBannerCardComboBoxCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowBannerCardComboBox());
    connect(showBannerCardComboBoxCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &VisualDeckStorageQuickSettingsWidget::showBannerCardComboBoxChanged);
    connect(showBannerCardComboBoxCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowBannerCardComboBox);

    // draw unused color identities checkbox
    drawUnusedColorIdentitiesCheckBox = new QCheckBox(this);
    drawUnusedColorIdentitiesCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageDrawUnusedColorIdentities());
    connect(drawUnusedColorIdentitiesCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &VisualDeckStorageQuickSettingsWidget::drawUnusedColorIdentitiesChanged);
    connect(drawUnusedColorIdentitiesCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageDrawUnusedColorIdentities);

    // color identity opacity selector
    auto unusedColorIdentityOpacityWidget = new QWidget(this);

    unusedColorIdentitiesOpacityLabel = new QLabel(unusedColorIdentityOpacityWidget);
    unusedColorIdentitiesOpacitySpinBox = new QSpinBox(unusedColorIdentityOpacityWidget);

    unusedColorIdentitiesOpacitySpinBox->setMinimum(0);
    unusedColorIdentitiesOpacitySpinBox->setMaximum(100);
    unusedColorIdentitiesOpacitySpinBox->setValue(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageUnusedColorIdentitiesOpacity());
    connect(unusedColorIdentitiesOpacitySpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &VisualDeckStorageQuickSettingsWidget::unusedColorIdentitiesOpacityChanged);
    connect(unusedColorIdentitiesOpacitySpinBox, qOverload<int>(&QSpinBox::valueChanged),
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageUnusedColorIdentitiesOpacity);

    unusedColorIdentitiesOpacityLabel->setBuddy(unusedColorIdentitiesOpacitySpinBox);

    auto unusedColorIdentityOpacityLayout = new QHBoxLayout(unusedColorIdentityOpacityWidget);
    unusedColorIdentityOpacityLayout->setContentsMargins(11, 0, 11, 0);
    unusedColorIdentityOpacityLayout->addWidget(unusedColorIdentitiesOpacityLabel);
    unusedColorIdentityOpacityLayout->addWidget(unusedColorIdentitiesOpacitySpinBox);

    // tooltip selector
    auto deckPreviewTooltipWidget = new QWidget(this);

    deckPreviewTooltipLabel = new QLabel(deckPreviewTooltipWidget);
    deckPreviewTooltipComboBox = new QComboBox(deckPreviewTooltipWidget);
    deckPreviewTooltipComboBox->setFocusPolicy(Qt::StrongFocus);
    deckPreviewTooltipComboBox->addItem("", TooltipType::None);
    deckPreviewTooltipComboBox->addItem("", TooltipType::Filepath);

    deckPreviewTooltipComboBox->setCurrentIndex(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageTooltipType());
    connect(deckPreviewTooltipComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this] { emit deckPreviewTooltipChanged(getDeckPreviewTooltip()); });
    connect(deckPreviewTooltipComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageTooltipType);

    auto deckPreviewTooltipLayout = new QHBoxLayout(deckPreviewTooltipWidget);
    deckPreviewTooltipLayout->setContentsMargins(11, 0, 11, 0);
    deckPreviewTooltipLayout->addWidget(deckPreviewTooltipLabel);
    deckPreviewTooltipLayout->addWidget(deckPreviewTooltipComboBox);

    // card size slider
    cardSizeWidget =
        new CardSizeWidget(this, nullptr, SettingsCache::instance().visualDeckStorage().getVisualDeckStorageCardSize());
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, this,
            &VisualDeckStorageQuickSettingsWidget::cardSizeChanged);
    connect(cardSizeWidget, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageCardSize);

    // putting everything together
    this->addSettingsWidget(showFoldersCheckBox);
    this->addSettingsWidget(showTagFilterCheckBox);
    this->addSettingsWidget(showColorIdentityCheckBox);
    this->addSettingsWidget(showTagsOnDeckPreviewsCheckBox);
    this->addSettingsWidget(showBannerCardComboBoxCheckBox);
    this->addSettingsWidget(drawUnusedColorIdentitiesCheckBox);
    this->addSettingsWidget(unusedColorIdentityOpacityWidget);
    this->addSettingsWidget(deckPreviewTooltipWidget);
    this->addSettingsWidget(cardSizeWidget);

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this,
            &VisualDeckStorageQuickSettingsWidget::retranslateUi);
    retranslateUi();
}

void VisualDeckStorageQuickSettingsWidget::retranslateUi()
{
    showFoldersCheckBox->setText(tr("Show Folders"));
    showTagFilterCheckBox->setText(tr("Show Tag Filter"));
    showColorIdentityCheckBox->setText(tr("Show Color Identity"));
    showTagsOnDeckPreviewsCheckBox->setText(tr("Show Tags On Deck Previews"));
    showBannerCardComboBoxCheckBox->setText(tr("Show Banner Card Selection Option"));
    drawUnusedColorIdentitiesCheckBox->setText(tr("Draw unused Color Identities"));
    unusedColorIdentitiesOpacityLabel->setText(tr("Unused Color Identities Opacity"));
    unusedColorIdentitiesOpacitySpinBox->setSuffix("%");

    deckPreviewTooltipLabel->setText(tr("Deck tooltip:"));
    deckPreviewTooltipComboBox->setItemText(0, tr("None"));
    deckPreviewTooltipComboBox->setItemText(1, tr("Filepath"));
}

bool VisualDeckStorageQuickSettingsWidget::getShowFolders() const
{
    return showFoldersCheckBox->isChecked();
}

bool VisualDeckStorageQuickSettingsWidget::getDrawUnusedColorIdentities() const
{
    return drawUnusedColorIdentitiesCheckBox->isChecked();
}

bool VisualDeckStorageQuickSettingsWidget::getShowColorIdentity() const
{
    return showColorIdentityCheckBox->isChecked();
}

bool VisualDeckStorageQuickSettingsWidget::getShowBannerCardComboBox() const
{
    return showBannerCardComboBoxCheckBox->isChecked();
}

bool VisualDeckStorageQuickSettingsWidget::getShowTagFilter() const
{
    return showTagFilterCheckBox->isChecked();
}

bool VisualDeckStorageQuickSettingsWidget::getShowTagsOnDeckPreviews() const
{
    return showTagsOnDeckPreviewsCheckBox->isChecked();
}

int VisualDeckStorageQuickSettingsWidget::getUnusedColorIdentitiesOpacity() const
{
    return unusedColorIdentitiesOpacitySpinBox->value();
}

VisualDeckStorageQuickSettingsWidget::TooltipType VisualDeckStorageQuickSettingsWidget::getDeckPreviewTooltip() const
{
    return deckPreviewTooltipComboBox->currentData().value<TooltipType>();
}

int VisualDeckStorageQuickSettingsWidget::getCardSize() const
{
    return cardSizeWidget->getSlider()->value();
}