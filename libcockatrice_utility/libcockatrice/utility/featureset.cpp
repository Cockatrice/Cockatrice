#include "featureset.h"

#include <QDebug>
#include <QMap>

FeatureSet::FeatureSet()
{
}

QMap<QString, bool> FeatureSet::getDefaultFeatureList()
{
    initalizeFeatureList(featureList);
    return featureList;
}

void FeatureSet::initalizeFeatureList(QMap<QString, bool> &_featureList)
{
    // default features [name], [is required to connect]
    _featureList.insert("client_id", false);
    _featureList.insert("client_ver", false);
    _featureList.insert("feature_set", false);
    _featureList.insert("user_ban_history", false);
    _featureList.insert("room_chat_history", false);
    _featureList.insert("client_warnings", false);
    _featureList.insert("mod_log_lookup", false);
    _featureList.insert("idle_client", false);
    _featureList.insert("forgot_password", false);
    _featureList.insert("websocket", false);
    // featureList.insert("hashed_password_login", false);
    // These are temp to force users onto a newer client
    _featureList.insert("2.7.0_min_version", false);
    _featureList.insert("2.8.0_min_version", false);
}

void FeatureSet::enableRequiredFeature(QMap<QString, bool> &_featureList, const QString &featureName)
{
    if (_featureList.contains(featureName))
        _featureList.insert(featureName, true);
}

void FeatureSet::disableRequiredFeature(QMap<QString, bool> &_featureList, const QString &featureName)
{
    if (_featureList.contains(featureName))
        _featureList.insert(featureName, false);
}

QMap<QString, bool>
FeatureSet::addFeature(QMap<QString, bool> &_featureList, const QString &featureName, bool isFeatureRequired)
{
    _featureList.insert(featureName, isFeatureRequired);
    return _featureList;
}

QMap<QString, bool> FeatureSet::identifyMissingFeatures(const QMap<QString, bool> &suppliedFeatures,
                                                        QMap<QString, bool> requiredFeatures)
{
    QMap<QString, bool> missingList;
    QMap<QString, bool>::iterator i;
    for (i = requiredFeatures.begin(); i != requiredFeatures.end(); ++i) {
        if (!suppliedFeatures.contains(i.key())) {
            missingList.insert(i.key(), i.value());
        }
    }
    return missingList;
}

bool FeatureSet::isRequiredFeaturesMissing(const QMap<QString, bool> &suppliedFeatures,
                                           QMap<QString, bool> requiredFeatures)
{
    QMap<QString, bool>::iterator i;
    for (i = requiredFeatures.begin(); i != requiredFeatures.end(); ++i) {
        if (i.value() && suppliedFeatures.contains(i.key())) {
            return true;
        }
    }
    return false;
}
