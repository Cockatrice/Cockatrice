#include "picture_to_load.h"

#include "../../../settings/cache_settings.h"

#include <QCoreApplication>
#include <QDate>
#include <QRegularExpression>
#include <QUrl>
#include <algorithm>
#include <qloggingcategory.h>

Q_LOGGING_CATEGORY(PictureToLoadLog, "picture_loader.picture_to_load")

PictureToLoad::PictureToLoad(CardInfoPtr _card)
    : card(std::move(_card)), urlTemplates(SettingsCache::instance().downloads().getAllURLs())
{
    if (card) {
        for (const auto &cardInfoPerSetList : card->getSets()) {
            for (const auto &set : cardInfoPerSetList) {
                sortedSets << set.getPtr();
            }
        }
        if (sortedSets.empty()) {
            sortedSets << CardSet::newInstance("", "", "", QDate());
        }
        std::sort(sortedSets.begin(), sortedSets.end(), SetDownloadPriorityComparator());

        // If the user hasn't disabled arts other than their personal preference...
        if (!SettingsCache::instance().getOverrideAllCardArtWithPersonalPreference()) {
            // If the pixmapCacheKey corresponds to a specific set, we have to try to load it first.
            for (const auto &cardInfoPerSetList : card->getSets()) {
                for (const auto &set : cardInfoPerSetList) {
                    if (QLatin1String("card_") + card->getName() + QString("_") + QString(set.getProperty("uuid")) ==
                        card->getPixmapCacheKey()) {
                        long long setIndex = sortedSets.indexOf(set.getPtr());
                        CardSetPtr setForCardProviderID = sortedSets.takeAt(setIndex);
                        sortedSets.prepend(setForCardProviderID);
                    }
                }
            }
        }
        // The first time called, nextSet will also populate the Urls for the first set.
        nextSet();
    }
}

void PictureToLoad::populateSetUrls()
{
    /* currentSetUrls is a list, populated each time a new set is requested for a particular card
       and Urls are removed from it as a download is attempted from each one.  Custom Urls for
       a set are given higher priority, so should be placed first in the list. */
    currentSetUrls.clear();

    if (card && currentSet) {
        QString setCustomURL = card->getCustomPicURL(currentSet->getShortName());

        if (!setCustomURL.isEmpty()) {
            currentSetUrls.append(setCustomURL);
        }
    }

    for (const QString &urlTemplate : urlTemplates) {
        QString transformedUrl = transformUrl(urlTemplate);

        if (!transformedUrl.isEmpty()) {
            currentSetUrls.append(transformedUrl);
        }
    }

    /* Call nextUrl to make sure currentUrl is up-to-date
       but we don't need the result here. */
    (void)nextUrl();
}

bool PictureToLoad::nextSet()
{
    if (!sortedSets.isEmpty()) {
        currentSet = sortedSets.takeFirst();
        populateSetUrls();
        return true;
    }
    currentSet = {};
    return false;
}

bool PictureToLoad::nextUrl()
{
    if (!currentSetUrls.isEmpty()) {
        currentUrl = currentSetUrls.takeFirst();
        return true;
    }
    currentUrl = QString();
    return false;
}

QString PictureToLoad::getSetName() const
{
    if (currentSet) {
        return currentSet->getCorrectedShortName();
    } else {
        return QString();
    }
}

static int parse(const QString &urlTemplate,
                 const QString &propType,
                 const QString &cardName,
                 const QString &setName,
                 std::function<QString(const QString &)> getProperty,
                 QMap<QString, QString> &transformMap)
{
    static const QRegularExpression rxFillWith("^(.+)_fill_with_(.+)$");
    static const QRegularExpression rxSubStr("^(.+)_substr_(\\d+)_(\\d+)$");

    const QRegularExpression rxCardProp("!" + propType + ":([^!]+)!");

    auto matches = rxCardProp.globalMatch(urlTemplate);
    while (matches.hasNext()) {
        auto match = matches.next();
        QString templatePropertyName = match.captured(1);
        auto fillMatch = rxFillWith.match(templatePropertyName);
        QString cardPropertyName;
        QString fillWith;
        int subStrPos = 0;
        int subStrLen = -1;
        if (fillMatch.hasMatch()) {
            cardPropertyName = fillMatch.captured(1);
            fillWith = fillMatch.captured(2);
        } else {
            fillWith = QString();
            auto subStrMatch = rxSubStr.match(templatePropertyName);
            if (subStrMatch.hasMatch()) {
                cardPropertyName = subStrMatch.captured(1);
                subStrPos = subStrMatch.captured(2).toInt();
                subStrLen = subStrMatch.captured(3).toInt();
            } else {
                cardPropertyName = templatePropertyName;
            }
        }
        QString propertyValue = getProperty(cardPropertyName);
        if (propertyValue.isEmpty()) {
            qCDebug(PictureToLoadLog).nospace()
                << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Requested " << propType
                << "property (" << cardPropertyName << ") for Url template (" << urlTemplate << ") is not available";
            return 1;
        } else {
            int propLength = propertyValue.length();
            if (subStrLen > 0) {
                if (subStrPos + subStrLen > propLength) {
                    qCDebug(PictureToLoadLog).nospace()
                        << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Requested " << propType
                        << " property (" << cardPropertyName << ") for Url template (" << urlTemplate
                        << ") is smaller than substr specification (" << subStrPos << " + " << subStrLen << " > "
                        << propLength << ")";
                    return 1;
                } else {
                    propertyValue = propertyValue.mid(subStrPos, subStrLen);
                    propLength = subStrLen;
                }
            }

            if (!fillWith.isEmpty()) {
                int fillLength = fillWith.length();
                if (fillLength < propLength) {
                    qCDebug(PictureToLoadLog).nospace()
                        << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Requested " << propType
                        << " property (" << cardPropertyName << ") for Url template (" << urlTemplate
                        << ") is longer than fill specification (" << fillWith << ")";
                    return 1;
                } else {

                    propertyValue = fillWith.left(fillLength - propLength) + propertyValue;
                }
            }

            transformMap["!" + propType + ":" + templatePropertyName + "!"] = propertyValue;
        }
    }
    return 0;
}

QString PictureToLoad::transformUrl(const QString &urlTemplate) const
{
    /* This function takes Url templates and substitutes actual card details
       into the url.  This is used for making Urls with follow a predictable format
       for downloading images.  If information is requested by the template that is
       not populated for this specific card/set combination, an empty string is returned.*/

    CardSetPtr set = getCurrentSet();

    QMap<QString, QString> transformMap = QMap<QString, QString>();
    QString setName = getSetName();

    // name
    QString cardName = card->getName();
    transformMap["!name!"] = cardName;
    transformMap["!name_lower!"] = card->getName().toLower();
    transformMap["!corrected_name!"] = card->getCorrectedName();
    transformMap["!corrected_name_lower!"] = card->getCorrectedName().toLower();

    // card properties
    if (parse(
            urlTemplate, "prop", cardName, setName, [&](const QString &name) { return card->getProperty(name); },
            transformMap)) {
        return QString();
    }

    if (set) {
        transformMap["!setcode!"] = set->getShortName();
        transformMap["!setcode_lower!"] = set->getShortName().toLower();
        transformMap["!setname!"] = set->getLongName();
        transformMap["!setname_lower!"] = set->getLongName().toLower();

        if (parse(
                urlTemplate, "set", cardName, setName,
                [&](const QString &name) { return card->getSetProperty(set->getShortName(), name); }, transformMap)) {
            return QString();
        }
    }

    // language setting
    transformMap["!sflang!"] = QString(QCoreApplication::translate(
        "PictureLoader", "en", "code for scryfall's language property, not available for all languages"));

    QString transformedUrl = urlTemplate;
    for (const QString &prop : transformMap.keys()) {
        if (transformedUrl.contains(prop)) {
            if (!transformMap[prop].isEmpty()) {
                transformedUrl.replace(prop, QUrl::toPercentEncoding(transformMap[prop]));
            } else {
                /* This means the template is requesting information that is not
                 * populated in this card, so it should return an empty string,
                 * indicating an invalid Url.
                 */
                qCDebug(PictureToLoadLog).nospace()
                    << "PictureLoader: [card: " << cardName << " set: " << setName << "]: Requested information ("
                    << prop << ") for Url template (" << urlTemplate << ") is not available";
                return QString();
            }
        }
    }

    return transformedUrl;
}
