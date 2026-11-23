/**
 * @file visual_display_model_test.cpp
 * @brief Tests for the flattened model behind the visual card database grid.
 */

#include "card_database_display_model.h"
#include "card_database_model.h"
#include "card_database_visual_display_model.h"
#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/card/printing/printing_info.h"
#include "libcockatrice/card/set/card_set.h"
#include "libcockatrice/interfaces/noop_card_preference_provider.h"
#include "libcockatrice/interfaces/noop_card_set_priority_controller.h"
#include "test_card_database_path_provider.h"

#include "gtest/gtest.h"
#include <QCoreApplication>
#include <QDate>
#include <QEventLoop>
#include <QHash>
#include <QMap>
#include <QString>
#include <QTimer>
#include <memory>

namespace
{
// GTest's main() creates no QCoreApplication, which QTimer needs for its event loop.
const QCoreApplication &application()
{
    static int argc = 1;
    static char executable[] = "visual_display_model_test";
    static char *argv[] = {executable, nullptr};
    static const QCoreApplication app(argc, argv);
    return app;
}

/// Lets the rebuild timer fire, as it would in the running client.
void waitForRebuild()
{
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();
}

class CardDatabaseVisualDisplayModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        (void)application();

        db = std::make_unique<CardDatabase>(nullptr, new NoopCardPreferenceProvider(),
                                            new TestCardDatabasePathProvider(), new NoopCardSetPriorityController());
        db->loadCardDatabases();

        cardModel = std::make_unique<CardDatabaseModel>(db.get(), false);
        displayModel = std::make_unique<CardDatabaseDisplayModel>();
        displayModel->setSourceModel(cardModel.get());
        visualModel = std::make_unique<CardDatabaseVisualDisplayModel>(displayModel.get(), db->query());
        visualModel->refresh();
    }

    /**
     * @brief Builds a card with @p printingCount printings in each named set.
     * @param printingCountPerSet Set short name to the number of printings in that set.
     */
    [[nodiscard]] CardInfoPtr makeCard(const QString &name, const QMap<QString, int> &printingCountPerSet)
    {
        SetToPrintingsMap setsInfo;
        for (auto it = printingCountPerSet.constBegin(); it != printingCountPerSet.constEnd(); ++it) {
            CardSetPtr set = CardSet::newInstance(&setController, it.key(), it.key() + " test set", "expansion",
                                                  QDate(), CardSet::PriorityPrimary);
            for (int printing = 0; printing < it.value(); ++printing) {
                QHash<QString, QString> properties;
                properties["uuid"] = QString("%1-%2-%3").arg(it.key(), name).arg(printing);
                setsInfo[it.key()].append(PrintingInfo(set, LazyPropertiesHash(properties)));
            }
        }

        CardInfo::UiAttributes attributes = {.tableRow = 1};
        return CardInfo::newInstance(name, "", false, {}, {}, {}, setsInfo, attributes);
    }

    NoopCardSetPriorityController setController;
    std::unique_ptr<CardDatabase> db;
    std::unique_ptr<CardDatabaseModel> cardModel;
    std::unique_ptr<CardDatabaseDisplayModel> displayModel;
    std::unique_ptr<CardDatabaseVisualDisplayModel> visualModel;
};

TEST_F(CardDatabaseVisualDisplayModelTest, ExposesTheCardInfoBehindAProxyRow)
{
    ASSERT_GT(displayModel->rowCount(), 0);

    const QVariant info = displayModel->data(displayModel->index(0, CardDatabaseModel::NameColumn),
                                             CardDatabaseDisplayModel::CardInfoRole);
    ASSERT_TRUE(info.isValid());
    EXPECT_FALSE(info.value<CardInfoPtr>().isNull());
}

TEST_F(CardDatabaseVisualDisplayModelTest, FlattensOneRowPerProxyCardWithoutSetFilters)
{
    ASSERT_GT(displayModel->rowCount(), 0);
    EXPECT_EQ(displayModel->rowCount(), visualModel->rowCount());

    for (int row = 0; row < visualModel->rowCount(); ++row) {
        const ExactCard card = visualModel->cardAt(row);
        ASSERT_TRUE(card) << "row " << row;
        EXPECT_EQ(
            displayModel->data(displayModel->index(row, CardDatabaseModel::NameColumn), Qt::DisplayRole).toString(),
            card.getName())
            << "row " << row;
    }
}

TEST_F(CardDatabaseVisualDisplayModelTest, ShowsOneRowPerCardWhenNoSetIsFiltered)
{
    const int rowsBefore = visualModel->rowCount();

    db->addCard(makeCard("Multi", {{"TST", 2}, {"OTH", 1}}));
    visualModel->refresh();

    EXPECT_EQ(rowsBefore + 1, visualModel->rowCount());
    EXPECT_EQ("Multi", visualModel->cardAt(visualModel->rowCount() - 1).getName());
}

TEST_F(CardDatabaseVisualDisplayModelTest, ShowsEveryPrintingOfAFilteredSet)
{
    db->addCard(makeCard("Multi", {{"TST", 2}, {"OTH", 1}}));
    visualModel->refresh();

    visualModel->setSetFilterTerms({"TST"});

    ASSERT_EQ(2, visualModel->rowCount());
    for (int row = 0; row < visualModel->rowCount(); ++row) {
        const ExactCard card = visualModel->cardAt(row);
        ASSERT_TRUE(card) << "row " << row;
        EXPECT_EQ("Multi", card.getName());
        ASSERT_FALSE(card.getPrinting().isEmpty()) << "row " << row;
        EXPECT_EQ("TST", card.getPrinting().getSet()->getShortName()) << "row " << row;
    }
    EXPECT_NE(visualModel->cardAt(0).getPrinting(), visualModel->cardAt(1).getPrinting());
}

TEST_F(CardDatabaseVisualDisplayModelTest, HidesCardsWithoutAPrintingInTheFilteredSets)
{
    visualModel->setSetFilterTerms({"NO SUCH SET"});

    EXPECT_EQ(0, visualModel->rowCount());
}

TEST_F(CardDatabaseVisualDisplayModelTest, RestoresOneRowPerCardWhenTheSetTermsClear)
{
    db->addCard(makeCard("Multi", {{"TST", 2}, {"OTH", 1}}));
    visualModel->refresh();
    const int rowsBefore = visualModel->rowCount();

    visualModel->setSetFilterTerms({"TST"});
    ASSERT_EQ(2, visualModel->rowCount());

    visualModel->setSetFilterTerms({});
    EXPECT_EQ(rowsBefore, visualModel->rowCount());
}

TEST_F(CardDatabaseVisualDisplayModelTest, RebuildsByItselfWhenTheProxyGainsARow)
{
    const int rowsBefore = visualModel->rowCount();

    db->addCard(makeCard("Late", {{"TST", 1}}));
    waitForRebuild();

    EXPECT_EQ(rowsBefore + 1, visualModel->rowCount());
}

TEST_F(CardDatabaseVisualDisplayModelTest, ReportsTheCardNameForDisplayAndAccessibleText)
{
    ASSERT_GT(visualModel->rowCount(), 0);
    const QString expected = visualModel->cardAt(0).getName();

    EXPECT_EQ(expected, visualModel->data(visualModel->index(0), Qt::DisplayRole).toString());
    EXPECT_EQ(expected, visualModel->data(visualModel->index(0), Qt::AccessibleTextRole).toString());
    EXPECT_FALSE(visualModel->data(QModelIndex(), Qt::DisplayRole).isValid());
}
} // namespace
