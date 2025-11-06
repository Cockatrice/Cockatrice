#ifndef COCKATRICE_INETWORKSETTINGSPROVIDER_H
#define COCKATRICE_INETWORKSETTINGSPROVIDER_H
#include <QString>

class INetworkSettingsProvider
{
public:
    virtual ~INetworkSettingsProvider() = default;

    virtual QString getClientID() = 0;

    [[nodiscard]] virtual int getTimeOut() const = 0;
    [[nodiscard]] virtual int getKeepAlive() const = 0;
    [[nodiscard]] virtual bool getNotifyAboutUpdates() const = 0;

    virtual void setKnownMissingFeatures(const QString &_knownMissingFeatures) = 0;
    virtual QString getKnownMissingFeatures() = 0;
};

#endif // COCKATRICE_INETWORKSETTINGSPROVIDER_H
