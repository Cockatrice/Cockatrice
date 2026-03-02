/**
 * @file card_database_display_model.h
 * @ingroup CardDatabaseModels
 * @brief The CardDatabaseDisplayModel is a QSortFilterProxyModel that allows applying filters and sorting to a
 * CardDatabaseModel.
 */

#ifndef COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H
#define COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H

#include <QSortFilterProxyModel>
#include <QTimer>
#include <libcockatrice/filters/filter_string.h>

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
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
signals:
    void modelDirty();

protected:
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    static int lessThanNumerically(const QString &left, const QString &right);
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    [[nodiscard]] bool rowMatchesCardName(CardInfoPtr info) const;

private slots:
    void filterTreeChanged();
    /** Will translate all undesirable characters in DIRTYNAME according to the TABLE. */
    const QString sanitizeCardName(const QString &dirtyName, const QMap<wchar_t, wchar_t> &table);
};

#endif // COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H
