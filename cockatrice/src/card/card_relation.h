#ifndef COCKATRICE_CARD_RELATION_H
#define COCKATRICE_CARD_RELATION_H

#include "card_relation_type.h"

#include <QObject>
#include <QString>

class CardRelation : public QObject
{
    Q_OBJECT

private:
    QString name;
    CardRelationType attachType;
    bool isCreateAllExclusion;
    bool isVariableCount;
    int defaultCount;
    bool isPersistent;

public:
    explicit CardRelation(const QString &_name = QString(),
                          CardRelationType _attachType = CardRelationType::DoesNotAttach,
                          bool _isCreateAllExclusion = false,
                          bool _isVariableCount = false,
                          int _defaultCount = 1,
                          bool _isPersistent = false);

    const QString &getName() const
    {
        return name;
    }
    CardRelationType getAttachType() const
    {
        return attachType;
    }
    bool getDoesAttach() const
    {
        return attachType != CardRelationType::DoesNotAttach;
    }
    bool getDoesTransform() const
    {
        return attachType == CardRelationType::TransformInto;
    }

    QString getAttachTypeAsString() const
    {
        return cardAttachTypeToString(attachType);
    }

    bool getCanCreateAnother() const
    {
        return !getDoesAttach();
    }
    bool getIsCreateAllExclusion() const
    {
        return isCreateAllExclusion;
    }
    bool getIsVariable() const
    {
        return isVariableCount;
    }
    int getDefaultCount() const
    {
        return defaultCount;
    }
    bool getIsPersistent() const
    {
        return isPersistent;
    }
};

#endif // COCKATRICE_CARD_RELATION_H
