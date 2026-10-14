#ifndef CARDS_DISPLAY_SETTINGS_H
#define CARDS_DISPLAY_SETTINGS_H

#include "settings_manager.h"

#include <QString>
#include <libcockatrice/interfaces/interface_cards_display_settings_provider.h>
#include <qtmetamacros.h>

class QObject;

class CardsDisplaySettings : public SettingsManager, public ICardsDisplaySettingsProvider
{
    Q_OBJECT
    friend class SettingsCache;

public:
    /// What can summon the hover zoom or panel. Stored as int; values are the
    /// serialization contract, so never reorder or renumber.
    enum HoverPanelTrigger
    {
        HoverPanelTriggerHover = 0,
        HoverPanelTriggerRightClick = 1,
        HoverPanelTriggerWheel = 2,
        HoverPanelTriggerShortcut = 3
    };

    [[nodiscard]] bool getDisplayCardNames() const override;
    [[nodiscard]] bool getRoundCardCorners() const override;
    [[nodiscard]] bool getOverrideAllCardArtWithPersonalPreference() const override;
    [[nodiscard]] bool getBumpSetsWithCardsInDeckToTop() const override;
    [[nodiscard]] int getPrintingSelectorSortOrder() const override;
    [[nodiscard]] int getPrintingSelectorCardSize() const override;
    [[nodiscard]] bool getPrintingSelectorNavigationButtonsVisible() const override;
    [[nodiscard]] bool getTapAnimation() const override;
    [[nodiscard]] bool getArrowDrawAnimation() const override;
    [[nodiscard]] bool getAutoRotateSidewaysLayoutCards() const override;
    [[nodiscard]] bool getScaleCards() const override;
    /// What summons the in-game card hover panel (a HoverPanelTrigger value).
    [[nodiscard]] int getHoverPanelTrigger() const override;
    /// What zooms the hovered card (a HoverPanelTrigger value). Zoom and panel triggers are independent.
    [[nodiscard]] int getHoverZoomTrigger() const override;
    /// When true, the trigger first only zooms; repeating it shows the panel.
    [[nodiscard]] bool getHoverPanelDoubleConfirm() const override;
    /// Hover dwell before the zoom kicks in (ms). Only used with the hover trigger.
    [[nodiscard]] int getHoverZoomDelayMs() const override;
    /// Wait after the zoom before the tab panel shows (ms). Hover trigger, one-step only.
    [[nodiscard]] int getHoverPanelDelayMs() const override;
    [[nodiscard]] int getStackCardOverlapPercent() const override;
    [[nodiscard]] int getCardInfoViewMode() const override;
    [[nodiscard]] int getVisualDeckStorageCardSize() const override;
    [[nodiscard]] int getVisualDatabaseDisplayCardSize() const override;
    [[nodiscard]] int getVisualDeckEditorCardSize() const override;
    [[nodiscard]] bool getVisualDeckEditorShowCardCounts() const override;
    [[nodiscard]] int getEDHRecCardSize() const override;
    [[nodiscard]] int getArchidektPreviewSize() const override;
    [[nodiscard]] int getSampleHandSize() const override;
    [[nodiscard]] QString getCardLang() const override;
    [[nodiscard]] int getCardSearchLanguage() const override;

    void setDisplayCardNames(bool _displayCardNames);
    void setRoundCardCorners(bool _roundCardCorners);
    void setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArt);
    void setBumpSetsWithCardsInDeckToTop(bool _bumpSetsWithCardsInDeckToTop);
    void setPrintingSelectorSortOrder(int _printingSelectorSortOrder);
    void setPrintingSelectorCardSize(int _printingSelectorCardSize);
    void setPrintingSelectorNavigationButtonsVisible(bool _navigationButtonsVisible);
    void setTapAnimation(bool _tapAnimation);
    void setArrowDrawAnimation(bool _arrowDrawAnimation);
    void setAutoRotateSidewaysLayoutCards(bool _autoRotateSidewaysLayoutCards);
    void setCardScaling(bool _scaleCards);
    void setHoverPanelTrigger(int _trigger);
    void setHoverZoomTrigger(int _trigger);
    void setHoverPanelDoubleConfirm(bool _doubleConfirm);
    void setHoverZoomDelayMs(int _ms);
    void setHoverPanelDelayMs(int _ms);
    void setStackCardOverlapPercent(int _verticalCardOverlapPercent);
    void setCardInfoViewMode(int _viewMode);
    void setVisualDeckStorageCardSize(int _cardSize);
    void setVisualDatabaseDisplayCardSize(int _cardSize);
    void setVisualDeckEditorCardSize(int _cardSize);
    void setVisualDeckEditorShowCardCounts(bool _showCardCounts);
    void setEDHRecCardSize(int _edhrecCardSize);
    void setArchidektPreviewCardSize(int _archidektPreviewCardSize);
    void setSampleHandSize(int _sampleHandSize);
    void setCardLang(const QString &_cardLang);
    void setCardSearchLanguage(int _cardSearchLanguage);

signals:
    void displayCardNamesChanged();
    void roundCardCornersChanged(bool roundCardCorners);
    void overrideAllCardArtWithPersonalPreferenceChanged(bool _overrideAllCardArtWithPersonalPreference);
    void bumpSetsWithCardsInDeckToTopChanged();
    void printingSelectorSortOrderChanged();
    void printingSelectorCardSizeChanged();
    void printingSelectorNavigationButtonsVisibleChanged();
    void visualDeckStorageCardSizeChanged();
    void visualDatabaseDisplayCardSizeChanged();
    void visualDeckEditorCardSizeChanged();
    void visualDeckEditorShowCardCountsChanged(bool showCardCounts);
    void edhRecCardSizeChanged();
    void archidektPreviewSizeChanged();
    void sampleHandSizeChanged(int amount);
    void cardLangChanged(const QString &lang);
    void cardSearchLanguageChanged(int cardSearchLanguage);
    void hoverPanelTriggerChanged(int trigger);
    void hoverZoomTriggerChanged(int trigger);
    void hoverPanelDoubleConfirmChanged(bool doubleConfirm);
    void hoverZoomDelayMsChanged(int ms);
    void hoverPanelDelayMsChanged(int ms);

public:
    explicit CardsDisplaySettings(const QString &settingPath, QObject *parent = nullptr);

private:
    CardsDisplaySettings(const CardsDisplaySettings & /*other*/);
};

#endif // CARDS_DISPLAY_SETTINGS_H
