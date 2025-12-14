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
    [[nodiscard]] bool getTapAnimation() const override;
    [[nodiscard]] bool getAutoRotateSidewaysLayoutCards() const override;
    [[nodiscard]] bool getScaleCards() const override;
    [[nodiscard]] int getStackCardOverlapPercent() const override;
    [[nodiscard]] int getCardInfoViewMode() const override;
    [[nodiscard]] int getVisualDeckStorageCardSize() const override;
    [[nodiscard]] int getVisualDatabaseDisplayCardSize() const override;
    [[nodiscard]] int getVisualDeckEditorCardSize() const override;
    [[nodiscard]] int getEDHRecCardSize() const override;
    [[nodiscard]] int getArchidektPreviewSize() const override;
    [[nodiscard]] int getSampleHandSize() const override;

    [[nodiscard]] int getDeckEditorCommanderSpellbookIntegrationEnabled() const;
    [[nodiscard]] bool getDeckEditorCommanderSpellbookIntegrationUseOfficialBracketNames() const;
    void setDeckEditorCommanderSpellbookIntegrationEnabled(int _deckEditorCommanderSpellbookIntegrationEnabled);
    void setDeckEditorCommanderSpellbookIntegrationUseOfficialBracketNames(
        bool _deckEditorCommanderSpellbookIntegrationUseOfficialBracketNames);

    void setDisplayCardNames(bool _displayCardNames);
    void setRoundCardCorners(bool _roundCardCorners);
    void setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArt);
    void setBumpSetsWithCardsInDeckToTop(bool _bumpSetsWithCardsInDeckToTop);
    void setPrintingSelectorSortOrder(int _printingSelectorSortOrder);
    void setPrintingSelectorCardSize(int _printingSelectorCardSize);
    void setIncludeRebalancedCards(bool _includeRebalancedCards);
    void setPrintingSelectorNavigationButtonsVisible(bool _navigationButtonsVisible);
    void setTapAnimation(bool _tapAnimation);
    void setAutoRotateSidewaysLayoutCards(bool _autoRotateSidewaysLayoutCards);
    void setCardScaling(bool _scaleCards);
    void setStackCardOverlapPercent(int _verticalCardOverlapPercent);
    void setCardInfoViewMode(int _viewMode);
    void setVisualDeckStorageCardSize(int _cardSize);
    void setVisualDatabaseDisplayCardSize(int _cardSize);
    void setVisualDeckEditorCardSize(int _cardSize);
    void setEDHRecCardSize(int _edhrecCardSize);
    void setArchidektPreviewCardSize(int _archidektPreviewCardSize);
    void setSampleHandSize(int _sampleHandSize);

signals:
    void displayCardNamesChanged();
    void roundCardCornersChanged(bool roundCardCorners);
    void overrideAllCardArtWithPersonalPreferenceChanged(bool _overrideAllCardArtWithPersonalPreference);
    void bumpSetsWithCardsInDeckToTopChanged();
    void printingSelectorSortOrderChanged();
    void printingSelectorCardSizeChanged();
    void includeRebalancedCardsChanged(bool _includeRebalancedCards);
    void printingSelectorNavigationButtonsVisibleChanged();
    void visualDeckStorageCardSizeChanged();
    void visualDatabaseDisplayCardSizeChanged();
    void visualDeckEditorCardSizeChanged();
    void edhRecCardSizeChanged();
    void archidektPreviewSizeChanged();
    void sampleHandSizeChanged(int amount);
    void deckEditorCommanderSpellbookIntegrationEnabledChanged(int _enabled);
    void deckEditorCommanderSpellbookIntegrationUseOfficialBracketNamesChanged(bool _useOfficialBracketNames);

public:
    explicit CardsDisplaySettings(const QString &settingPath, QObject *parent = nullptr);

private:
    CardsDisplaySettings(const CardsDisplaySettings & /*other*/);
};

#endif // CARDS_DISPLAY_SETTINGS_H
