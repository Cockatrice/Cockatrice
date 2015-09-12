#ifndef FEATURESET_H
#define FEATURESET_H

#include <QMap>
#include <QSet>
#include <QString>

class FeatureSet
{
public:
    FeatureSet();
    QMap<QString, bool> getDefaultFeatureList();
    void initalizeFeatureList(QMap<QString, bool> &featureList);
    void enableRequiredFeature(QMap<QString, bool> &featureList, QString featureName);
    void disableRequiredFeature(QMap<QString, bool> &featureList, QString featureName);
    QMap<QString, bool> addFeature(QMap<QString, bool> &featureList, QString featureName, bool isFeatureRequired);
    QMap<QString, bool> identifyMissingFeatures(QMap<QString, bool> featureListToCheck, QMap<QString, bool> featureListToCompareTo);
    bool isRequiredFeaturesMissing(QMap<QString, bool> featureListToCheck, QMap<QString, bool> featureListToCompareTo);
private:
    QMap<QString, bool> featureList;
};


#endif // FEEATURESET_H
