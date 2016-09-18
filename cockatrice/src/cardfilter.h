#ifndef CARDFILTER_H
#define CARDFILTER_H

#include <QString>

class CardFilter {
public:
    enum Type {
        TypeAnd = 0,
        TypeOr,
        TypeAndNot,
        TypeOrNot,
        TypeEnd
    };

    /* if you add an atribute here you also need to
     * add its string representation in attrName */
    enum Attr {
        AttrName = 0,
        AttrType,
        AttrColor,
        AttrText,
        AttrSet,
        AttrManaCost,
        AttrCmc,
        AttrRarity,
        AttrPow,
        AttrTough,
        AttrEnd
    };

private:
    QString trm;
    enum Type t;
    enum Attr a;

public:
    CardFilter(QString term, Type type, Attr attr) : trm(term), t(type), a(attr) {};

    Type type() const { return t; }
    const QString &term() const { return trm; }
    Attr attr() const { return a; }

    static const char *typeName(Type t);
    static const char *attrName(Attr a);
};

#endif
