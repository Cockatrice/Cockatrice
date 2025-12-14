#ifndef CARDS_DISPLAY_SETTINGS_H
#define CARDS_DISPLAY_SETTINGS_H

#include "settings_manager.h"

#include <libcockatrice/interfaces/interface_cards_display_settings_provider.h>

enum deckEditorCommanderSpellbookIntegrationEnabledIndex
{
    deckEditorCommanderSpellbookIntegrationEnabledIndexDisabled,
    deckEditorCommanderSpellbookIntegrationEnabledIndexEnabled,
    deckEditorCommanderSpellbookIntegrationEnabledIndexAutomatic,
    deckEditorCommanderSpellbookIntegrationEnabledIndexUnprompted,
};

class CardsDisplaySettings : public SettingsManager, public ICardsDisplaySettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    [[nodiscard]] bool getDisplayCardNames() const override;
    [[nodiscard]] bool getRoundCardCorners() const override;
    [[nodiscard]] bool getOverrideAllCardArtWithPersonalPreference() const override;
    [[nodiscard]] bool getBumpSetsWithCardsInDeckToTop() const override;
    [[nodiscard]] int getPrintingSelectorSortOrder() const override;
    [[nodiscard]] int getPrintingSelectorCardSize() const override;
    [[nodiscard]] bool getIncludeRebalancedCards() const override;
    [[nodiscard]] bool getPrintingSelectorNavigationButtonsVisible() const override;
    [[nodiscard]] bool getDeckEditorBannerCardComboBoxVisible() const override;
    [[nodiscard]] bool getDeckEditorTagsWidgetVisible() const override;
    [[nodiscard]] bool getTapAnimation() const override;
    [[nodiscard]] bool getAutoRotateSidewaysLayoutCards() const override;
    [[nodiscard]] bool getScaleCards() const override;
    [[nodiscard]] int getStackCardOverlapPercent() const override;
    [[nodiscard]] int getCardInfoViewMode() const override;
    [[nodiscard]] bool getShowShortcuts() const override;
    [[nodiscard]] bool getShowGameSelectorFilterToolbar() const override;

    void setDisplayCardNames(bool _displayCardNames);
    void setRoundCardCorners(bool _roundCardCorners);
    void setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArt);
    void setBumpSetsWithCardsInDeckToTop(bool _bumpSetsWithCardsInDeckToTop);
    void setPrintingSelectorSortOrder(int _printingSelectorSortOrder);
    void setPrintingSelectorCardSize(int _printingSelectorCardSize);
    void setIncludeRebalancedCards(bool _includeRebalancedCards);
    void setPrintingSelectorNavigationButtonsVisible(bool _navigationButtonsVisible);
    void setDeckEditorBannerCardComboBoxVisible(bool _deckEditorBannerCardComboBoxVisible);
    void setDeckEditorTagsWidgetVisible(bool _deckEditorTagsWidgetVisible);
    [[nodiscard]] int getDeckEditorCommanderSpellbookIntegrationEnabled() const;
    [[nodiscard]] bool getDeckEditorCommanderSpellbookIntegrationUseOfficialBracketNames() const;
    void setDeckEditorCommanderSpellbookIntegrationEnabled(int _deckEditorCommanderSpellbookIntegrationEnabled);
    void setDeckEditorCommanderSpellbookIntegrationUseOfficialBracketNames(
        bool _deckEditorCommanderSpellbookIntegrationUseOfficialBracketNames);
    void setTapAnimation(bool _tapAnimation);
    void setAutoRotateSidewaysLayoutCards(bool _autoRotateSidewaysLayoutCards);
    void setCardScaling(bool _scaleCards);
    void setStackCardOverlapPercent(int _verticalCardOverlapPercent);
    void setCardInfoViewMode(int _viewMode);
    void setShowShortcuts(bool _showShortcuts);
    void setShowGameSelectorFilterToolbar(bool _showGameSelectorFilterToolbar);

signals:
    void displayCardNamesChanged();
    void roundCardCornersChanged(bool roundCardCorners);
    void overrideAllCardArtWithPersonalPreferenceChanged(bool _overrideAllCardArtWithPersonalPreference);
    void bumpSetsWithCardsInDeckToTopChanged();
    void printingSelectorSortOrderChanged();
    void printingSelectorCardSizeChanged();
    void includeRebalancedCardsChanged(bool _includeRebalancedCards);
    void printingSelectorNavigationButtonsVisibleChanged();
    void deckEditorBannerCardComboBoxVisibleChanged(bool _visible);
    void deckEditorTagsWidgetVisibleChanged(bool _visible);
    void deckEditorCommanderSpellbookIntegrationEnabledChanged(int _enabled);
    void deckEditorCommanderSpellbookIntegrationUseOfficialBracketNamesChanged(bool _useOfficialBracketNames);
    void showGameSelectorFilterToolbarChanged(bool state);

private:
    explicit CardsDisplaySettings(const QString &settingPath, QObject *parent = nullptr);
    CardsDisplaySettings(const CardsDisplaySettings & /*other*/);
};

#endif // CARDS_DISPLAY_SETTINGS_H
