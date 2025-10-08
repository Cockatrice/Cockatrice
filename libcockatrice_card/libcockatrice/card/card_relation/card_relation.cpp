#include "card_relation.h"

#include "card_relation_type.h"

CardRelation::CardRelation(const QString &_name,
                           CardRelationType _attachType,
                           bool _isCreateAllExclusion,
                           bool _isVariableCount,
                           int _defaultCount,
                           bool _isPersistent)
    : name(_name), attachType(_attachType), isCreateAllExclusion(_isCreateAllExclusion),
      isVariableCount(_isVariableCount), defaultCount(_defaultCount), isPersistent(_isPersistent)
{
}