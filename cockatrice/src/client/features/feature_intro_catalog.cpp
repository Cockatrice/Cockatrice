#include "feature_intro.h"

const QVector<FeatureIntro> &featureIntroCatalog()
{
    static const QVector<FeatureIntro> catalog = {};
    return catalog;
}

const FeatureIntro *findFeatureIntro(const QString &id)
{
    for (const FeatureIntro &intro : featureIntroCatalog()) {
        if (intro.id == id) {
            return &intro;
        }
    }
    return nullptr;
}
