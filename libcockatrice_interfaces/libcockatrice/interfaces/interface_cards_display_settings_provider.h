#ifndef COCKATRICE_INTERFACE_CARDS_DISPLAY_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_CARDS_DISPLAY_SETTINGS_PROVIDER_H

class ICardsDisplaySettingsProvider
{
public:
    virtual ~ICardsDisplaySettingsProvider() = default;

    [[nodiscard]] virtual bool getDisplayCardNames() const = 0;
    [[nodiscard]] virtual bool getRoundCardCorners() const = 0;
    [[nodiscard]] virtual bool getOverrideAllCardArtWithPersonalPreference() const = 0;
    [[nodiscard]] virtual bool getBumpSetsWithCardsInDeckToTop() const = 0;
    [[nodiscard]] virtual int getPrintingSelectorSortOrder() const = 0;
    [[nodiscard]] virtual int getPrintingSelectorCardSize() const = 0;
    [[nodiscard]] virtual bool getIncludeRebalancedCards() const = 0;
    [[nodiscard]] virtual bool getPrintingSelectorNavigationButtonsVisible() const = 0;
    [[nodiscard]] virtual bool getDeckEditorBannerCardComboBoxVisible() const = 0;
    [[nodiscard]] virtual bool getDeckEditorTagsWidgetVisible() const = 0;
    [[nodiscard]] virtual bool getTapAnimation() const = 0;
    [[nodiscard]] virtual bool getAutoRotateSidewaysLayoutCards() const = 0;
    [[nodiscard]] virtual bool getScaleCards() const = 0;
    [[nodiscard]] virtual int getStackCardOverlapPercent() const = 0;
    [[nodiscard]] virtual int getCardInfoViewMode() const = 0;
    [[nodiscard]] virtual bool getShowShortcuts() const = 0;
    [[nodiscard]] virtual bool getShowGameSelectorFilterToolbar() const = 0;
};

#endif // COCKATRICE_INTERFACE_CARDS_DISPLAY_SETTINGS_PROVIDER_H
