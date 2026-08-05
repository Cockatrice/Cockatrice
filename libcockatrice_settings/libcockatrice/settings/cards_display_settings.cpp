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
    return getValue("sortOrder", "cards", "printingSelector", 1).toInt();
}

int CardsDisplaySettings::getPrintingSelectorCardSize() const
{
    return getValue("printingSelector", "cards", "cardSize", 100).toInt();
}

bool CardsDisplaySettings::getIncludeRebalancedCards() const
{
    return getValue("includerebalancedcards", QString(), QString(), true).toBool();
}

bool CardsDisplaySettings::getPrintingSelectorNavigationButtonsVisible() const
{
    return getValue("navigationButtonsVisible", "cards", "printingSelector", true).toBool();
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

int CardsDisplaySettings::getVisualDeckStorageCardSize() const
{
    return getValue("visualDeckStorage", "cards", "cardSize", 100).toInt();
}

int CardsDisplaySettings::getVisualDatabaseDisplayCardSize() const
{
    return getValue("visualDatabaseDisplay", "cards", "cardSize", 100).toInt();
}

int CardsDisplaySettings::getVisualDeckEditorCardSize() const
{
    return getValue("visualDeckEditor", "cards", "cardSize", 100).toInt();
}

int CardsDisplaySettings::getEDHRecCardSize() const
{
    return getValue("edhrec", "cards", "cardSize", 100).toInt();
}

int CardsDisplaySettings::getArchidektPreviewSize() const
{
    return getValue("archidektPreview", "cards", "cardSize", 100).toInt();
}

int CardsDisplaySettings::getSampleHandSize() const
{
    return getValue("sampleHandSize", "cards", "cardSize", 7).toInt();
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
    setValue(_printingSelectorSortOrder, "sortOrder", "cards", "printingSelector");
    emit printingSelectorSortOrderChanged();
}

void CardsDisplaySettings::setPrintingSelectorCardSize(int _printingSelectorCardSize)
{
    setValue(_printingSelectorCardSize, "printingSelector", "cards", "cardSize");
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
    setValue(_navigationButtonsVisible, "navigationButtonsVisible", "cards", "printingSelector");
    emit printingSelectorNavigationButtonsVisibleChanged();
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

void CardsDisplaySettings::setVisualDeckStorageCardSize(int _cardSize)
{
    setValue(_cardSize, "visualDeckStorage", "cards", "cardSize");
    emit visualDeckStorageCardSizeChanged();
}

void CardsDisplaySettings::setVisualDatabaseDisplayCardSize(int _cardSize)
{
    setValue(_cardSize, "visualDatabaseDisplay", "cards", "cardSize");
    emit visualDatabaseDisplayCardSizeChanged();
}

void CardsDisplaySettings::setVisualDeckEditorCardSize(int _cardSize)
{
    setValue(_cardSize, "visualDeckEditor", "cards", "cardSize");
    emit visualDeckEditorCardSizeChanged();
}

void CardsDisplaySettings::setEDHRecCardSize(int _edhrecCardSize)
{
    setValue(_edhrecCardSize, "edhrec", "cards", "cardSize");
    emit edhRecCardSizeChanged();
}

void CardsDisplaySettings::setArchidektPreviewCardSize(int _archidektPreviewCardSize)
{
    setValue(_archidektPreviewCardSize, "archidektPreview", "cards", "cardSize");
    emit archidektPreviewSizeChanged();
}

void CardsDisplaySettings::setSampleHandSize(int _sampleHandSize)
{
    setValue(_sampleHandSize, "sampleHandSize", "cards", "cardSize");
    emit sampleHandSizeChanged(_sampleHandSize);
}
