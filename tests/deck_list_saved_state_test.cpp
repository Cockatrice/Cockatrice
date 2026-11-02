#include <gtest/gtest.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/deck_list_history_manager.h>
#include <libcockatrice/deck_list/deck_list_node_tree.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

namespace
{
void rename(DeckListHistoryManager &history, DeckList &deck, const QString &name)
{
    history.save(deck.createMemento("Rename"));
    deck.setName(name);
}
}

TEST(DeckListSavedState, UndoAndRedoCrossSavedState)
{
    DeckList deck;
    DeckListHistoryManager history;
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    rename(history, deck, "Edited");
    EXPECT_TRUE(history.isModified(deck));
    history.undo(&deck);
    EXPECT_FALSE(history.isModified(deck));
    history.redo(&deck);
    EXPECT_TRUE(history.isModified(deck));
}

TEST(DeckListSavedState, SaveInMiddleOfHistoryRetainsUndoAndRedo)
{
    DeckList deck;
    DeckListHistoryManager history;
    rename(history, deck, "First");
    rename(history, deck, "Second");
    history.undo(&deck);
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    ASSERT_TRUE(history.canUndo());
    ASSERT_TRUE(history.canRedo());
    history.redo(&deck);
    EXPECT_TRUE(history.isModified(deck));
    history.undo(&deck);
    EXPECT_FALSE(history.isModified(deck));
    history.undo(&deck);
    EXPECT_TRUE(history.isModified(deck));
}

TEST(DeckListSavedState, NewBranchCannotReuseAnUnrelatedCleanHistoryIndex)
{
    DeckList deck;
    DeckListHistoryManager history;
    rename(history, deck, "Saved branch");
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    history.undo(&deck);
    rename(history, deck, "Other branch");
    EXPECT_FALSE(history.canRedo());
    EXPECT_TRUE(history.isModified(deck));
    rename(history, deck, "Saved branch");
    EXPECT_FALSE(history.isModified(deck));
}

TEST(DeckListSavedState, PendingSaveRecordsUploadedSnapshotInsteadOfNewerEdits)
{
    DeckList deck;
    DeckListHistoryManager history;
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    rename(history, deck, "Uploaded");
    const auto upload = history.createSavePoint(deck);
    rename(history, deck, "Newer unsaved edit");
    ASSERT_TRUE(history.markSaved(upload));
    EXPECT_TRUE(history.isModified(deck));
    history.undo(&deck);
    EXPECT_EQ(deck.getName(), "Uploaded");
    EXPECT_FALSE(history.isModified(deck));
    history.undo(&deck);
    EXPECT_TRUE(history.isModified(deck));
}

TEST(DeckListSavedState, SaveCompletionAfterUndoRecognizesExactSavedContent)
{
    DeckList deck;
    DeckListHistoryManager history;
    rename(history, deck, "Uploaded");
    const auto upload = history.createSavePoint(deck);
    rename(history, deck, "Temporary edit");
    history.undo(&deck);
    ASSERT_TRUE(history.markSaved(upload));
    EXPECT_FALSE(history.isModified(deck));
    EXPECT_TRUE(history.canRedo());
}

TEST(DeckListSavedState, ReplacementRejectsOldSaveEvenWithIdenticalContents)
{
    DeckList deck;
    DeckListHistoryManager history;
    deck.setName("Same contents");
    const auto oldUpload = history.createSavePoint(deck);
    history.clear();
    EXPECT_FALSE(history.markSaved(oldUpload));
    EXPECT_TRUE(history.isModified(deck));
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    EXPECT_FALSE(history.isModified(deck));
    EXPECT_FALSE(history.markSaved(oldUpload));
    EXPECT_FALSE(history.isModified(deck));
}

TEST(DeckListSavedState, ExplicitUnsavedImportStaysDirtyThroughUndo)
{
    DeckList deck;
    DeckListHistoryManager history;
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    history.invalidateSavedState();
    rename(history, deck, "Temporary edit");
    history.undo(&deck);
    EXPECT_TRUE(history.isModified(deck));
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    EXPECT_FALSE(history.isModified(deck));
}

TEST(DeckListSavedState, MetadataAndPrintingsArePartOfSavedState)
{
    DeckList deck;
    DeckListHistoryManager history;
    auto *card = deck.getTree()->addCard("Island", 4, "main", -1, "ABC", "1", "printing-a");
    const auto original = history.createSavePoint(deck);
    ASSERT_TRUE(history.markSaved(original));
    const auto hash = deck.getDeckHash();
    deck.setComments("Keep an opening hand with two lands");
    EXPECT_EQ(deck.getDeckHash(), hash);
    EXPECT_TRUE(history.isModified(deck));
    deck.setComments("");
    EXPECT_FALSE(history.isModified(deck));
    deck.setTags({"Control"});
    EXPECT_TRUE(history.isModified(deck));
    deck.setTags({});
    card->setCardProviderId("printing-b");
    EXPECT_EQ(deck.getDeckHash(), hash);
    EXPECT_TRUE(history.isModified(deck));
    card->setCardProviderId("printing-a");
    EXPECT_FALSE(history.isModified(deck));
    ASSERT_NE(deck.getTree()->addCustomZone("main", "Lands"), nullptr);
    EXPECT_EQ(deck.getDeckHash(), hash);
    EXPECT_TRUE(history.isModified(deck));
}

TEST(DeckListSavedState, FullMetadataRoundTripsThroughHistory)
{
    DeckList deck;
    DeckListHistoryManager history;
    deck.setName("Control");
    deck.setComments("Notes with <markup> & symbols");
    deck.setTags({"Blue", "Favorite"});
    deck.setBannerCard({"Island", "banner-printing"});
    deck.setPlaymat({{"Island", "playmat-printing"}, {0.1, 0.2, 0.3, 1.25}});
    deck.getTree()->addCard("Island", 4, "main", -1, "ABC", "1", "printing-a");
    ASSERT_NE(deck.getTree()->addCustomZone("main", "Lands"), nullptr);
    ASSERT_TRUE(history.markSaved(history.createSavePoint(deck)));
    rename(history, deck, "Changed");
    history.undo(&deck);
    EXPECT_FALSE(history.isModified(deck));
    history.redo(&deck);
    EXPECT_TRUE(history.isModified(deck));
}
