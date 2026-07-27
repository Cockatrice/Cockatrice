#include "cards_display_settings.h"

CardsDisplaySettings::CardsDisplaySettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "cards_display.ini", "cards", QString(), parent)
{
}

bool CardsDisplaySettings::getDisplayCardNames() const
{
    return getValue("displaycardnames", QString(), QString(), true).toBool();
}

bool CardsDisplaySettings::getRoundCardCorners() const
{
    return getValue("roundcardcorners", QString(), QString(), true).toBool();
}

bool CardsDisplaySettings::getOverrideAllCardArtWithPersonalPreference() const
{
    return getValue("overrideallcardartwithpersonalpreference", QString(), QString(), false).toBool();
}

bool CardsDisplaySettings::getBumpSetsWithCardsInDeckToTop() const
{
    return getValue("bumpsetswithcardsindecktotop", QString(), QString(), true).toBool();
}

int CardsDisplaySettings::getPrintingSelectorSortOrder() const
{
    return getValue("printingselectorsortorder", QString(), QString(), 1).toInt();
}

int CardsDisplaySettings::getPrintingSelectorCardSize() const
{
    return getValue("printingselectorcardsize", QString(), QString(), 100).toInt();
}

bool CardsDisplaySettings::getIncludeRebalancedCards() const
{
    return getValue("includerebalancedcards", QString(), QString(), true).toBool();
}

bool CardsDisplaySettings::getPrintingSelectorNavigationButtonsVisible() const
{
    return getValue("printingselectornavigationbuttonsvisible", QString(), QString(), true).toBool();
}

bool CardsDisplaySettings::getDeckEditorBannerCardComboBoxVisible() const
{
    return getValue("deckeditorbannercardcomboboxvisible", "interface", QString(), true).toBool();
}

bool CardsDisplaySettings::getDeckEditorTagsWidgetVisible() const
{
    return getValue("deckeditortagswidgetvisible", "interface", QString(), true).toBool();
}

bool CardsDisplaySettings::getTapAnimation() const
{
    return getValue("tapanimation", QString(), QString(), true).toBool();
}

bool CardsDisplaySettings::getAutoRotateSidewaysLayoutCards() const
{
    return getValue("autorotatesidewayslayoutcards", QString(), QString(), true).toBool();
}

bool CardsDisplaySettings::getScaleCards() const
{
    return getValue("scaleCards", QString(), QString(), true).toBool();
}

int CardsDisplaySettings::getStackCardOverlapPercent() const
{
    return getValue("verticalCardOverlapPercent", QString(), QString(), 33).toInt();
}

int CardsDisplaySettings::getCardInfoViewMode() const
{
    return getValue("cardinfoviewmode", QString(), QString(), 0).toInt();
}

bool CardsDisplaySettings::getShowShortcuts() const
{
    return getValue("showshortcuts", "menu", QString(), true).toBool();
}

bool CardsDisplaySettings::getShowGameSelectorFilterToolbar() const
{
    return getValue("showgameselectorfiltertoolbar", "menu", QString(), true).toBool();
}

void CardsDisplaySettings::setDisplayCardNames(bool _displayCardNames)
{
    setValue(_displayCardNames, "displaycardnames");
    emit displayCardNamesChanged();
}

void CardsDisplaySettings::setRoundCardCorners(bool _roundCardCorners)
{
    if (_roundCardCorners == getRoundCardCorners()) {
        return;
    }
    setValue(_roundCardCorners, "roundcardcorners");
    emit roundCardCornersChanged(_roundCardCorners);
}

void CardsDisplaySettings::setOverrideAllCardArtWithPersonalPreference(bool _overrideAllCardArt)
{
    setValue(_overrideAllCardArt, "overrideallcardartwithpersonalpreference");
    emit overrideAllCardArtWithPersonalPreferenceChanged(_overrideAllCardArt);
}

void CardsDisplaySettings::setBumpSetsWithCardsInDeckToTop(bool _bumpSetsWithCardsInDeckToTop)
{
    setValue(_bumpSetsWithCardsInDeckToTop, "bumpsetswithcardsindecktotop");
    emit bumpSetsWithCardsInDeckToTopChanged();
}

void CardsDisplaySettings::setPrintingSelectorSortOrder(int _printingSelectorSortOrder)
{
    setValue(_printingSelectorSortOrder, "printingselectorsortorder");
    emit printingSelectorSortOrderChanged();
}

void CardsDisplaySettings::setPrintingSelectorCardSize(int _printingSelectorCardSize)
{
    setValue(_printingSelectorCardSize, "printingselectorcardsize");
    emit printingSelectorCardSizeChanged();
}

void CardsDisplaySettings::setIncludeRebalancedCards(bool _includeRebalancedCards)
{
    if (_includeRebalancedCards == getIncludeRebalancedCards()) {
        return;
    }
    setValue(_includeRebalancedCards, "includerebalancedcards");
    emit includeRebalancedCardsChanged(_includeRebalancedCards);
}

void CardsDisplaySettings::setPrintingSelectorNavigationButtonsVisible(bool _navigationButtonsVisible)
{
    setValue(_navigationButtonsVisible, "printingselectornavigationbuttonsvisible");
    emit printingSelectorNavigationButtonsVisibleChanged();
}

void CardsDisplaySettings::setDeckEditorBannerCardComboBoxVisible(bool _deckEditorBannerCardComboBoxVisible)
{
    setValue(_deckEditorBannerCardComboBoxVisible, "deckeditorbannercardcomboboxvisible", "interface");
    emit deckEditorBannerCardComboBoxVisibleChanged(_deckEditorBannerCardComboBoxVisible);
}

void CardsDisplaySettings::setDeckEditorTagsWidgetVisible(bool _deckEditorTagsWidgetVisible)
{
    setValue(_deckEditorTagsWidgetVisible, "deckeditortagswidgetvisible", "interface");
    emit deckEditorTagsWidgetVisibleChanged(_deckEditorTagsWidgetVisible);
}

void CardsDisplaySettings::setTapAnimation(bool _tapAnimation)
{
    setValue(_tapAnimation, "tapanimation");
}

void CardsDisplaySettings::setAutoRotateSidewaysLayoutCards(bool _autoRotateSidewaysLayoutCards)
{
    setValue(_autoRotateSidewaysLayoutCards, "autorotatesidewayslayoutcards");
}

void CardsDisplaySettings::setCardScaling(bool _scaleCards)
{
    setValue(_scaleCards, "scaleCards");
}

void CardsDisplaySettings::setStackCardOverlapPercent(int _verticalCardOverlapPercent)
{
    setValue(_verticalCardOverlapPercent, "verticalCardOverlapPercent");
}

void CardsDisplaySettings::setCardInfoViewMode(int _viewMode)
{
    setValue(_viewMode, "cardinfoviewmode");
}

void CardsDisplaySettings::setShowShortcuts(bool _showShortcuts)
{
    setValue(_showShortcuts, "showshortcuts", "menu");
}

void CardsDisplaySettings::setShowGameSelectorFilterToolbar(bool _showGameSelectorFilterToolbar)
{
    setValue(_showGameSelectorFilterToolbar, "showgameselectorfiltertoolbar", "menu");
    emit showGameSelectorFilterToolbarChanged(_showGameSelectorFilterToolbar);
}
