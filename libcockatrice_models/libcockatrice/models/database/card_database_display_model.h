/**
 * @file card_database_display_model.h
 * @ingroup CardDatabaseModels
 * @brief The CardDatabaseDisplayModel is a QSortFilterProxyModel that allows applying filters and sorting to a
 * CardDatabaseModel.
 */

#ifndef COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H
#define COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H

#include "libcockatrice/card/card_info.h"

#include <QCollator>
#include <QModelIndex>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringView>
#include <QTimer>
#include <libcockatrice/card/card_localization.h>
#include <libcockatrice/filters/filter_string.h>
#include <qtmetamacros.h>

class CardDatabaseModel;
class FilterTree;
class QObject;
template <class Key, class T> class QMap;

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

    enum Role
    {
        /**
         * @brief The CardInfoPtr behind a proxy row.
         *
         * Saves views from looking the card up again through its name, and keeps
         * them from depending on the source model's column layout.
         */
        CardInfoRole = Qt::UserRole + 1
    };

private:
    FilterBool isToken;
    QString cardName, cardText;
    QSet<QString> cardNameSet, cardTypes, cardColors;
    FilterTree *filterTree;
    FilterString *filterString;
    /// The source model, kept typed so CardInfoRole does not cast per row.
    CardDatabaseModel *cardModel;
    int loadedRowCount;
    QTimer dirtyTimer;
    CardSearchLanguage searchLanguage;
    QString searchText;

    /** The translation table that will be used for sanitizeCardName. */
    static QMap<wchar_t, wchar_t> characterTranslation;

    /** Natural, locale-aware comparison shared by every sort in this model. */
    QCollator collator;

public:
    explicit CardDatabaseDisplayModel(QObject *parent = nullptr);
    void setSourceModel(QAbstractItemModel *model) override;
    void setFilterTree(FilterTree *_filterTree);
    void setIsToken(FilterBool _isToken)
    {
        isToken = _isToken;
        dirty();
    }

    void setCardName(const QString &_cardName)
    {
        if (filterString != nullptr) {
            delete filterString;
            filterString = nullptr;
        }
        cardName = sanitizeCardName(_cardName, characterTranslation);
        dirty();
    }
    void setStringFilter(const QString &_src);
    void setCardNameSet(const QSet<QString> &_cardNameSet)
    {
        cardNameSet = _cardNameSet;
        dirty();
    }
    void setSearchLanguage(const CardSearchLanguage &searchLang);

    void dirty()
    {
        dirtyTimer.start(20);
    }
    void clearFilterAll();
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Returns the card behind a proxy row, without the QVariant wrapper of CardInfoRole.
     *
     * The visual grid calls this for every row on each rebuild; CardInfoRole stays the
     * generic way in for consumers that only see the model through data().
     */
    [[nodiscard]] CardInfoPtr cardInfoAt(const QModelIndex &index) const;
    [[nodiscard]] bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
signals:
    void modelDirty();

protected:
    [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    int lessThanNumerically(QStringView left, QStringView right) const;
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    [[nodiscard]] bool rowMatchesCardName(CardInfoPtr info) const;

private slots:
    void filterTreeChanged();
    /** Will translate all undesirable characters in DIRTYNAME according to the TABLE. */
    const QString sanitizeCardName(const QString &dirtyName, const QMap<wchar_t, wchar_t> &table);
};

#endif // COCKATRICE_CARD_DATABASE_DISPLAY_MODEL_H
