#include "featureset.h"
#include <QMap>
#include <QDebug>

FeatureSet::FeatureSet()
{

}

QMap<QString, bool> FeatureSet::getDefaultFeatureList() {
    initalizeFeatureList(featureList);
    return featureList;
}

void FeatureSet::initalizeFeatureList(QMap<QString, bool> &featureList) {
    featureList.insert("client_id", false);
    featureList.insert("client_ver", false);
    featureList.insert("feature_set", false);
    featureList.insert("user_ban_history", false);
    featureList.insert("room_chat_history", false);
    featureList.insert("client_warnings", false);
    featureList.insert("mod_log_lookup", false);
}

void FeatureSet::enableRequiredFeature(QMap<QString, bool> &featureList, QString featureName){
    if (featureList.contains(featureName))
        featureList.insert(featureName,true);
}

void FeatureSet::disableRequiredFeature(QMap<QString, bool> &featureList, QString featureName){
    if (featureList.contains(featureName))
        featureList.insert(featureName,false);
}

QMap<QString, bool> FeatureSet::addFeature(QMap<QString, bool> &featureList, QString featureName, bool isFeatureRequired){
    featureList.insert(featureName,isFeatureRequired);
    return featureList;
}

QMap<QString, bool> FeatureSet::identifyMissingFeatures(QMap<QString, bool> suppliedFeatures, QMap<QString, bool> requiredFeatures){
    QMap<QString, bool> missingList;
    QMap<QString, bool>::iterator i;
    for (i = requiredFeatures.begin(); i != requiredFeatures.end(); ++i) {
        if (!suppliedFeatures.contains(i.key())) {
            missingList.insert(i.key(), i.value());
        }
    }
    return missingList;
}

bool FeatureSet::isRequiredFeaturesMissing(QMap<QString, bool> suppliedFeatures, QMap<QString, bool> requiredFeatures) {
    QMap<QString, bool>::iterator i;
    for (i = requiredFeatures.begin(); i != requiredFeatures.end(); ++i) {
        if (i.value() && suppliedFeatures.contains(i.key())) {
                return true;
        }
    }
    return false;
}
