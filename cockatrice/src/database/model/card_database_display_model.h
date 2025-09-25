#ifndef COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H
#define COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H

#include "../../filters/filter_string.h"

#include <QList>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTimer>

class FilterTree;
class CardDatabaseDisplayModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum FilterBool
    {
        ShowTrue,
        ShowFalse,
        ShowAll
    };

private:
    FilterBool isToken;
    QString cardName, cardText;
    QSet<QString> cardNameSet, cardTypes, cardColors;
    FilterTree *filterTree;
    FilterString *filterString;
    int loadedRowCount;
    QTimer dirtyTimer;

    /** The translation table that will be used for sanitizeCardName. */
    static QMap<wchar_t, wchar_t> characterTranslation;

public:
    explicit CardDatabaseDisplayModel(QObject *parent = nullptr);
    void setFilterTree(FilterTree *_filterTree);
    void setIsToken(FilterBool _isToken)
    {
        isToken = _isToken;
        emit modelDirty();
        dirty();
    }

    void setCardName(const QString &_cardName)
    {
        if (filterString != nullptr) {
            delete filterString;
            filterString = nullptr;
        }
        cardName = sanitizeCardName(_cardName, characterTranslation);
        emit modelDirty();
        dirty();
    }
    void setStringFilter(const QString &_src)
    {
        delete filterString;
        filterString = new FilterString(_src);
        emit modelDirty();
        dirty();
    }
    void setCardNameSet(const QSet<QString> &_cardNameSet)
    {
        cardNameSet = _cardNameSet;
        emit modelDirty();
        dirty();
    }

    void dirty()
    {
        dirtyTimer.start(20);
    }
    void clearFilterAll();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
signals:
    void modelDirty();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    static int lessThanNumerically(const QString &left, const QString &right);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool rowMatchesCardName(CardInfoPtr info) const;

private slots:
    void filterTreeChanged();
    /** Will translate all undesirable characters in DIRTYNAME according to the TABLE. */
    const QString sanitizeCardName(const QString &dirtyName, const QMap<wchar_t, wchar_t> &table);
};

#endif // COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H
