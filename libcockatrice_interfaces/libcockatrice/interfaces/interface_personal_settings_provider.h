#ifndef COCKATRICE_INTERFACE_PERSONAL_SETTINGS_PROVIDER_H
#define COCKATRICE_INTERFACE_PERSONAL_SETTINGS_PROVIDER_H

#include <QDate>
#include <QList>
#include <QString>

class IPersonalSettingsProvider
{
public:
    virtual ~IPersonalSettingsProvider() = default;

    [[nodiscard]] virtual QString getLang() const = 0;
    [[nodiscard]] virtual QString getClientID() = 0;
    [[nodiscard]] virtual QString getClientVersion() = 0;
    [[nodiscard]] virtual int getKeepAlive() const = 0;
    [[nodiscard]] virtual int getTimeOut() const = 0;
    [[nodiscard]] virtual bool getPicDownload() const = 0;
    [[nodiscard]] virtual bool getShowStatusBar() const = 0;
    [[nodiscard]] virtual int getMaxFontSize() const = 0;
    [[nodiscard]] virtual QString getHighlightWords() const = 0;
    [[nodiscard]] virtual QString getHomeTabBackgroundSource() const = 0;
    [[nodiscard]] virtual int getHomeTabBackgroundShuffleFrequency() const = 0;
    [[nodiscard]] virtual bool getHomeTabDisplayCardName() const = 0;
    [[nodiscard]] virtual bool getShowTipsOnStartup() const = 0;
    [[nodiscard]] virtual QList<int> getSeenTips() const = 0;
    [[nodiscard]] virtual bool getDownloadSpoilersStatus() const = 0;
};

#endif // COCKATRICE_INTERFACE_PERSONAL_SETTINGS_PROVIDER_H
