#ifndef CARDFILTER_H
#define CARDFILTER_H

#include <QObject>
#include <QString>

class CardFilter : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        TypeAnd = 0,
        TypeOr,
        TypeAndNot,
        TypeOrNot,
        TypeEnd
    };

    /* if you add an atribute here you also need to
     * add its string representation in attrName */
    enum Attr
    {
        AttrCmc = 0,
        AttrColor,
        AttrLoyalty,
        AttrManaCost,
        AttrName,
        AttrPow,
        AttrRarity,
        AttrSet,
        AttrText,
        AttrTough,
        AttrType,
        AttrEnd
    };

private:
    QString trm;
    enum Type t;
    enum Attr a;

public:
    CardFilter(QString term, Type type, Attr attr) : trm(term), t(type), a(attr){};

    Type type() const
    {
        return t;
    }
    const QString &term() const
    {
        return trm;
    }
    Attr attr() const
    {
        return a;
    }

    static const QString typeName(Type t);
    static const QString attrName(Attr a);
};

#endif
