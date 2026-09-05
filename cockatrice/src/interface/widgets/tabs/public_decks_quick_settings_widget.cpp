#include "public_decks_quick_settings_widget.h"

#include "../../../client/settings/cache_settings.h"
#include "../cards/card_size_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

PublicDecksQuickSettingsWidget::PublicDecksQuickSettingsWidget(QWidget *parent) : SettingsButtonWidget(parent)
{
    // show color identity on preview tiles checkbox
    showColorIdentityCheckBox = new QCheckBox(this);
    showColorIdentityCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowColorIdentity());
    connect(showColorIdentityCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &PublicDecksQuickSettingsWidget::showColorIdentityChanged);
    connect(showColorIdentityCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowColorIdentity);

    // show tags on preview tiles checkbox
    showTagsOnDeckPreviewsCheckBox = new QCheckBox(this);
    showTagsOnDeckPreviewsCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagsOnDeckPreviews());
    connect(showTagsOnDeckPreviewsCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &PublicDecksQuickSettingsWidget::showTagsOnDeckPreviewsChanged);
    connect(showTagsOnDeckPreviewsCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowTagsOnDeckPreviews);

    // show the last modified / upload time on preview tiles checkbox
    showUploadTimeCheckBox = new QCheckBox(this);
    showUploadTimeCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowUploadTime());
    connect(showUploadTimeCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &PublicDecksQuickSettingsWidget::showUploadTimeChanged);
    connect(showUploadTimeCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowUploadTime);

    // show tag filter box checkbox
    showTagFilterCheckBox = new QCheckBox(this);
    showTagFilterCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageShowTagFilter());
    connect(showTagFilterCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &PublicDecksQuickSettingsWidget::showTagFilterChanged);
    connect(showTagFilterCheckBox, &QCheckBox::QT_STATE_CHANGED, &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageShowTagFilter);

    // draw unused color identities checkbox
    drawUnusedColorIdentitiesCheckBox = new QCheckBox(this);
    drawUnusedColorIdentitiesCheckBox->setChecked(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageDrawUnusedColorIdentities());
    connect(drawUnusedColorIdentitiesCheckBox, &QCheckBox::QT_STATE_CHANGED, this,
            &PublicDecksQuickSettingsWidget::drawUnusedColorIdentitiesChanged);
    connect(drawUnusedColorIdentitiesCheckBox, &QCheckBox::QT_STATE_CHANGED,
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageDrawUnusedColorIdentities);

    // unused color identities opacity selector
    auto unusedColorIdentityOpacityWidget = new QWidget(this);

    unusedColorIdentitiesOpacityLabel = new QLabel(unusedColorIdentityOpacityWidget);
    unusedColorIdentitiesOpacitySpinBox = new QSpinBox(unusedColorIdentityOpacityWidget);

    unusedColorIdentitiesOpacitySpinBox->setMinimum(0);
    unusedColorIdentitiesOpacitySpinBox->setMaximum(100);
    unusedColorIdentitiesOpacitySpinBox->setValue(
        SettingsCache::instance().visualDeckStorage().getVisualDeckStorageUnusedColorIdentitiesOpacity());
    connect(unusedColorIdentitiesOpacitySpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
            &PublicDecksQuickSettingsWidget::unusedColorIdentitiesOpacityChanged);
    connect(unusedColorIdentitiesOpacitySpinBox, qOverload<int>(&QSpinBox::valueChanged),
            &SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::setVisualDeckStorageUnusedColorIdentitiesOpacity);

    unusedColorIdentitiesOpacityLabel->setBuddy(unusedColorIdentitiesOpacitySpinBox);

    auto unusedColorIdentityOpacityLayout = new QHBoxLayout(unusedColorIdentityOpacityWidget);
    unusedColorIdentityOpacityLayout->setContentsMargins(11, 0, 11, 0);
    unusedColorIdentityOpacityLayout->addWidget(unusedColorIdentitiesOpacityLabel);
    unusedColorIdentityOpacityLayout->addWidget(unusedColorIdentitiesOpacitySpinBox);

    // card size slider (kept at the bottom, like the Visual Deck Storage)
    cardSizeWidget =
        new CardSizeWidget(this, nullptr, SettingsCache::instance().cardsDisplay().getVisualDeckStorageCardSize());
    connect(cardSizeWidget->getSlider(), &QSlider::valueChanged, this,
            &PublicDecksQuickSettingsWidget::cardSizeChanged);
    connect(cardSizeWidget, &CardSizeWidget::cardSizeSettingUpdated, &SettingsCache::instance().cardsDisplay(),
            &CardsDisplaySettings::setVisualDeckStorageCardSize);

    this->addSettingsWidget(showColorIdentityCheckBox);
    this->addSettingsWidget(showTagsOnDeckPreviewsCheckBox);
    this->addSettingsWidget(showUploadTimeCheckBox);
    this->addSettingsWidget(showTagFilterCheckBox);
    this->addSettingsWidget(drawUnusedColorIdentitiesCheckBox);
    this->addSettingsWidget(unusedColorIdentityOpacityWidget);
    this->addSettingsWidget(cardSizeWidget);

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this,
            &PublicDecksQuickSettingsWidget::retranslateUi);
    retranslateUi();
}

void PublicDecksQuickSettingsWidget::retranslateUi()
{
    showColorIdentityCheckBox->setText(tr("Show Color Identity"));
    showTagsOnDeckPreviewsCheckBox->setText(tr("Show Tags On Deck Previews"));
    showUploadTimeCheckBox->setText(tr("Show Upload Time"));
    showTagFilterCheckBox->setText(tr("Show Tag Filter"));
    drawUnusedColorIdentitiesCheckBox->setText(tr("Draw unused Color Identities"));
    unusedColorIdentitiesOpacityLabel->setText(tr("Unused Color Identities Opacity"));
    unusedColorIdentitiesOpacitySpinBox->setSuffix("%");
}

bool PublicDecksQuickSettingsWidget::getDrawUnusedColorIdentities() const
{
    return drawUnusedColorIdentitiesCheckBox->isChecked();
}

bool PublicDecksQuickSettingsWidget::getShowColorIdentity() const
{
    return showColorIdentityCheckBox->isChecked();
}

bool PublicDecksQuickSettingsWidget::getShowTagFilter() const
{
    return showTagFilterCheckBox->isChecked();
}

bool PublicDecksQuickSettingsWidget::getShowTagsOnDeckPreviews() const
{
    return showTagsOnDeckPreviewsCheckBox->isChecked();
}

bool PublicDecksQuickSettingsWidget::getShowUploadTime() const
{
    return showUploadTimeCheckBox->isChecked();
}

int PublicDecksQuickSettingsWidget::getUnusedColorIdentitiesOpacity() const
{
    return unusedColorIdentitiesOpacitySpinBox->value();
}

CardSizeWidget *PublicDecksQuickSettingsWidget::getCardSizeWidget() const
{
    return cardSizeWidget;
}