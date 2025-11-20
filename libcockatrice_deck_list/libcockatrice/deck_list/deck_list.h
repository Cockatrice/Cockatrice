/**
 * @file decklist.h
 * @brief Defines the DeckList class and supporting types for managing a full
 *        deck structure including cards, zones, sideboard plans, and
 *        serialization to/from multiple formats. This is a logic class which
 *        does not care about Qt or user facing views.
 *        See @c DeckListModel for the actual Qt Model to be used for views
 */

#ifndef DECKLIST_H
#define DECKLIST_H

#include "deck_list_memento.h"
#include "inner_deck_list_node.h"

#include <QMap>
#include <QVector>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <libcockatrice/protocol/pb/move_card_to_zone.pb.h>
#include <libcockatrice/utility/card_ref.h>

class AbstractDecklistNode;
class DecklistCardNode;
class CardDatabase;
class QIODevice;
class QTextStream;
class InnerDecklistNode;

/**
 * @class SideboardPlan
 * @ingroup Decks
 * @brief Represents a predefined sideboarding strategy for a deck.
 *
 * Sideboard plans store a named list of card movements that should be applied
 * between the mainboard and sideboard for a specific matchup. Each movement
 * is expressed using a `MoveCard_ToZone` protobuf message.
 *
 * ### Responsibilities:
 * - Store the plan name and list of moves.
 * - Support XML serialization/deserialization.
 *
 * ### Typical usage:
 * A deck can contain multiple sideboard plans (e.g., "vs Aggro", "vs Control"),
 * each describing how to transform the main deck into its intended configuration.
 */
class SideboardPlan
{
private:
    QString name;                    ///< Human-readable name of this plan.
    QList<MoveCard_ToZone> moveList; ///< List of move instructions for this plan.

public:
    /**
     * @brief Construct a new SideboardPlan.
     * @param _name The plan name.
     * @param _moveList Initial list of card move instructions.
     */
    explicit SideboardPlan(const QString &_name = QString(),
                           const QList<MoveCard_ToZone> &_moveList = QList<MoveCard_ToZone>());

    /**
     * @brief Read a SideboardPlan from an XML stream.
     * @param xml XML reader positioned at the plan element.
     * @return true if parsing succeeded.
     */
    bool readElement(QXmlStreamReader *xml);

    /**
     * @brief Write this SideboardPlan to XML.
     * @param xml Stream to append the serialized element to.
     */
    void write(QXmlStreamWriter *xml);

    /// @return The plan name.
    [[nodiscard]] QString getName() const
    {
        return name;
    }

    /// @return Const reference to the move list.
    [[nodiscard]] const QList<MoveCard_ToZone> &getMoveList() const
    {
        return moveList;
    }

    /// @brief Replace the move list with a new one.
    void setMoveList(const QList<MoveCard_ToZone> &_moveList);
};

/**
 * @class DeckList
 * @ingroup Decks
 * @brief Represents a complete deck, including metadata, zones, cards,
 *        and sideboard plans.
 *
 * A DeckList is a QObject wrapper around an `InnerDecklistNode` tree,
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
 * - Owns the root `InnerDecklistNode` tree.
 * - Owns `SideboardPlan` instances stored in `sideboardPlans`.
 *
 * ### Signals:
 * - @c deckHashChanged() — emitted when the deck contents change.
 * - @c deckTagsChanged() — emitted when tags are added/removed.
 *
 * ### Example workflow:
 * ```
 * DeckList deck;
 * deck.setName("Mono Red Aggro");
 * deck.addCard("Lightning Bolt", "main", -1);
 * deck.addTag("Aggro");
 * deck.saveToFile_Native(device);
 * ```
 */
class DeckList : public QObject
{
    Q_OBJECT
private:
    QString name;                                  ///< User-defined deck name.
    QString comments;                              ///< Free-form comments or notes.
    CardRef bannerCard;                            ///< Optional representative card for the deck.
    QString lastLoadedTimestamp;                   ///< Timestamp string of last load.
    QStringList tags;                              ///< User-defined tags for deck classification.
    QMap<QString, SideboardPlan *> sideboardPlans; ///< Named sideboard plans.
    InnerDecklistNode *root;                       ///< Root of the deck tree (zones + cards).

    /**
     * @brief Cached deck hash, recalculated lazily.
     * An empty string indicates the cache is invalid.
     */
    mutable QString cachedDeckHash;

    // Helpers for traversing the tree
    static void getCardListHelper(InnerDecklistNode *node, QSet<QString> &result);
    static void getCardRefListHelper(InnerDecklistNode *item, QList<CardRef> &result);
    InnerDecklistNode *getZoneObjFromName(const QString &zoneName);

protected:
    /**
     * @brief Map a card name to its zone.
     * Override in subclasses for format-specific logic.
     * @param cardName Card being placed.
     * @param currentZoneName Zone candidate.
     * @return Zone name to use.
     */
    virtual QString getCardZoneFromName(const QString /*cardName*/, QString currentZoneName)
    {
        return currentZoneName;
    };

    /**
     * @brief Produce the complete display name of a card.
     * Override in subclasses to add set suffixes or annotations.
     * @param cardName Base name.
     * @return Full display name.
     */
    virtual QString getCompleteCardName(const QString &cardName) const
    {
        return cardName;
    };

signals:
    /// Emitted when the deck hash changes.
    void deckHashChanged();
    /// Emitted when the deck tags are modified.
    void deckTagsChanged();

public slots:
    /// @name Metadata setters
    ///@{
    void setName(const QString &_name = QString())
    {
        name = _name;
    }
    void setComments(const QString &_comments = QString())
    {
        comments = _comments;
    }
    void setTags(const QStringList &_tags = QStringList())
    {
        tags = _tags;
        emit deckTagsChanged();
    }
    void addTag(const QString &_tag)
    {
        tags.append(_tag);
        emit deckTagsChanged();
    }
    void clearTags()
    {
        tags.clear();
        emit deckTagsChanged();
    }
    void setBannerCard(const CardRef &_bannerCard = {})
    {
        bannerCard = _bannerCard;
    }
    void setLastLoadedTimestamp(const QString &_lastLoadedTimestamp = QString())
    {
        lastLoadedTimestamp = _lastLoadedTimestamp;
    }
    ///@}

public:
    /// @brief Construct an empty deck.
    explicit DeckList();
    /// @brief Delete copy constructor.
    DeckList(const DeckList &) = delete;
    DeckList &operator=(const DeckList &) = delete;
    /// @brief Construct from a serialized native-format string.
    explicit DeckList(const QString &nativeString);
    ~DeckList() override;

    /// @name Metadata getters
    ///@{
    QString getName() const
    {
        return name;
    }
    QString getComments() const
    {
        return comments;
    }
    QStringList getTags() const
    {
        return tags;
    }
    CardRef getBannerCard() const
    {
        return bannerCard;
    }
    QString getLastLoadedTimestamp() const
    {
        return lastLoadedTimestamp;
    }
    ///@}

    bool isBlankDeck() const
    {
        return name.isEmpty() && comments.isEmpty() && getCardList().isEmpty();
    }

    /// @name Sideboard plans
    ///@{
    QList<MoveCard_ToZone> getCurrentSideboardPlan();
    void setCurrentSideboardPlan(const QList<MoveCard_ToZone> &plan);
    const QMap<QString, SideboardPlan *> &getSideboardPlans() const
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
        return root->isEmpty() && name.isEmpty() && comments.isEmpty() && sideboardPlans.isEmpty();
    }
    QStringList getCardList() const;
    QList<CardRef> getCardRefList() const;
    QList<DecklistCardNode *> getCardNodes(const QStringList &restrictToZones = QStringList()) const;
    int getSideboardSize() const;
    InnerDecklistNode *getRoot() const
    {
        return root;
    }
    DecklistCardNode *addCard(const QString &cardName,
                              const QString &zoneName,
                              int position,
                              const QString &cardSetName = QString(),
                              const QString &cardSetCollectorNumber = QString(),
                              const QString &cardProviderId = QString());
    bool deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode = nullptr);
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
