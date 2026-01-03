/**
 * @file deck_list.h
 * @brief Defines the DeckList class, which manages a full
 *        deck structure including cards, zones, sideboard plans, and
 *        serialization to/from multiple formats. This is a logic class which
 *        does not care about Qt or user facing views.
 *        See @c DeckListModel for the actual Qt Model to be used for views
 */

#ifndef DECKLIST_H
#define DECKLIST_H

#include "deck_list_memento.h"
#include "deck_list_node_tree.h"
#include "sideboard_plan.h"
#include "tree/inner_deck_list_node.h"

#include <QMap>
#include <QVector>
#include <QtCore/QXmlStreamReader>
#include <libcockatrice/utility/card_ref.h>

class AbstractDecklistNode;
class DecklistCardNode;
class CardDatabase;
class QIODevice;
class QTextStream;
class InnerDecklistNode;

/**
 * @class DeckList
 * @ingroup Decks
 * @brief Represents a complete deck, including metadata, zones, cards,
 *        and sideboard plans.
 *
 * A DeckList is a wrapper around an `InnerDecklistNode` tree,
 * enriched with metadata like deck name, comments, tags, banner card,
 * and multiple sideboard plans.
 *
 * ### Core responsibilities:
 * - Store and manage the root node tree (zones → groups → cards).
 * - Provide deck-level metadata (name, comments, tags, banner).
 * - Support multiple sideboard plans (meta-game strategies).
 * - Provide import/export in multiple formats:
 *   - Cockatrice native XML format.
 *   - Plain-text list format.
 * - Provide hashing for deck identity (deck hash).
 *
 * ### Ownership:
 * - Owns the `DecklistNodeTree`.
 * - Owns `SideboardPlan` instances stored in `sideboardPlans`.
 *
 * ### Example workflow:
 * ```
 * DeckList deck;
 * deck.setName("Mono Red Aggro");
 * deck.addCard("Lightning Bolt", "main");
 * deck.addTag("Aggro");
 * deck.saveToFile_Native(device);
 * ```
 */
class DeckList
{
public:
    struct Metadata
    {
        QString name;                ///< User-defined deck name.
        QString comments;            ///< Free-form comments or notes.
        QString gameFormat;          ///< The name of the game format this deck contains legal cards for
        CardRef bannerCard;          ///< Optional representative card for the deck.
        QStringList tags;            ///< User-defined tags for deck classification.
        QString lastLoadedTimestamp; ///< Timestamp string of last load.

        /**
         * @brief Checks if all values (except for lastLoadedTimestamp) in the metadata is empty.
         */
        bool isEmpty() const;
    };

private:
    Metadata metadata;                           ///< Deck metadata that is stored in the deck file
    QMap<QString, SideboardPlan> sideboardPlans; ///< Named sideboard plans.
    DecklistNodeTree tree;                       ///< The deck tree (zones + cards).

    /**
     * @brief Cached deck hash, recalculated lazily.
     * An empty string indicates the cache is invalid.
     */
    mutable QString cachedDeckHash;

public:
    /// @name Metadata setters
    ///@{
    void setName(const QString &_name = QString())
    {
        metadata.name = _name;
    }
    void setComments(const QString &_comments = QString())
    {
        metadata.comments = _comments;
    }
    void setTags(const QStringList &_tags = QStringList())
    {
        metadata.tags = _tags;
    }
    void addTag(const QString &_tag)
    {
        metadata.tags.append(_tag);
    }
    void clearTags()
    {
        metadata.tags.clear();
    }
    void setBannerCard(const CardRef &_bannerCard = {})
    {
        metadata.bannerCard = _bannerCard;
    }
    void setLastLoadedTimestamp(const QString &_lastLoadedTimestamp = QString())
    {
        metadata.lastLoadedTimestamp = _lastLoadedTimestamp;
    }
    void setGameFormat(const QString &_gameFormat = QString())
    {
        metadata.gameFormat = _gameFormat;
    }
    ///@}

    /// @brief Construct an empty deck.
    explicit DeckList();
    /// @brief Construct from a serialized native-format string.
    explicit DeckList(const QString &nativeString);
    /// @brief Construct from components
    DeckList(const Metadata &metadata,
             const DecklistNodeTree &tree,
             const QMap<QString, SideboardPlan> &sideboardPlans = {});

    /**
     * @brief Gets a pointer to the underlying node tree.
     * Note: DO NOT call this method unless the object needs to have access to the underlying model.
     * For now, only the DeckListModel should be calling this.
     */
    DecklistNodeTree *getTree()
    {
        return &tree;
    }

    /// @name Metadata getters
    /// The individual metadata getters still exist for backwards compatibility.
    ///@{
    //! \todo Figure out when we can remove them.
    const Metadata &getMetadata() const
    {
        return metadata;
    }
    QString getName() const
    {
        return metadata.name;
    }
    QString getComments() const
    {
        return metadata.comments;
    }
    QStringList getTags() const
    {
        return metadata.tags;
    }
    CardRef getBannerCard() const
    {
        return metadata.bannerCard;
    }
    QString getLastLoadedTimestamp() const
    {
        return metadata.lastLoadedTimestamp;
    }
    QString getGameFormat() const
    {
        return metadata.gameFormat;
    }
    ///@}

    bool isBlankDeck() const
    {
        return metadata.isEmpty() && getCardList().isEmpty();
    }

    /// @name Sideboard plans
    ///@{
    QList<MoveCard_ToZone> getCurrentSideboardPlan() const;
    void setCurrentSideboardPlan(const QList<MoveCard_ToZone> &plan);
    const QMap<QString, SideboardPlan> &getSideboardPlans() const
    {
        return sideboardPlans;
    }
    ///@}

    /// @name Serialization (XML)
    ///@{
    bool readElement(QXmlStreamReader *xml);
    void write(QXmlStreamWriter *xml) const;
    bool loadFromXml(QXmlStreamReader *xml);
    bool loadFromString_Native(const QString &nativeString);
    QString writeToString_Native() const;
    bool loadFromFile_Native(QIODevice *device);
    bool saveToFile_Native(QIODevice *device);
    ///@}

    /// @name Serialization (Plain text)
    ///@{
    bool loadFromStream_Plain(QTextStream &stream, bool preserveMetadata);
    bool loadFromFile_Plain(QIODevice *device);
    bool saveToStream_Plain(QTextStream &stream, bool prefixSideboardCards, bool slashTappedOutSplitCards);
    bool saveToFile_Plain(QIODevice *device, bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false);
    QString writeToString_Plain(bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false);
    ///@}

    /// @name Deck manipulation
    ///@{
    void cleanList(bool preserveMetadata = false);
    bool isEmpty() const
    {
        return tree.isEmpty() && metadata.isEmpty() && sideboardPlans.isEmpty();
    }
    QStringList getCardList() const;
    QList<CardRef> getCardRefList() const;
    QList<const DecklistCardNode *> getCardNodes(const QSet<QString> &restrictToZones = {}) const;
    QList<const InnerDecklistNode *> getZoneNodes() const;
    int getSideboardSize() const;

    DecklistCardNode *addCard(const QString &cardName,
                              const QString &zoneName,
                              int position = -1,
                              const QString &cardSetName = QString(),
                              const QString &cardSetCollectorNumber = QString(),
                              const QString &cardProviderId = QString(),
                              const bool formatLegal = true);
    ///@}

    /// @name Deck identity
    ///@{
    QString getDeckHash() const;
    void refreshDeckHash();
    ///@}

    /**
     * @brief Apply a function to every card in the deck tree.
     *
     * @param func Function taking (zone node, card node).
     */
    void forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func) const;
    DeckListMemento createMemento(const QString &reason) const;
    void restoreMemento(const DeckListMemento &m);
};

#endif
