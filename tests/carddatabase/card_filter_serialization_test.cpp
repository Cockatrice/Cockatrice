#include <QCoreApplication>
#include <QJsonDocument>
#include <QTranslator>
#include <gtest/gtest.h>
#include <libcockatrice/filters/filter_card.h>
#include <memory>

namespace
{
// Translate every filter label so missing catalog translations cannot conceal
// an accidental dependency on the current UI language.
class FilterTranslator : public QTranslator
{
public:
    QString languagePrefix;

    bool isEmpty() const override
    {
        return false;
    }

    QString translate(const char *context, const char *source, const char *, int) const override
    {
        return QByteArray(context) == "CardFilter" ? languagePrefix + QString::fromUtf8(source) : QString();
    }
};

class CardFilterSerialization : public testing::Test
{
protected:
    FilterTranslator translator;

    void SetUp() override
    {
        translator.languagePrefix = "first-language:";
        ASSERT_TRUE(QCoreApplication::installTranslator(&translator));
    }

    void TearDown() override
    {
        QCoreApplication::removeTranslator(&translator);
    }
};

TEST_F(CardFilterSerialization, AllFiltersSurviveLanguageChangeAndJsonRoundTrip)
{
    for (int type = 0; type < CardFilter::TypeEnd; ++type) {
        for (int attr = 0; attr < CardFilter::AttrEnd; ++attr) {
            translator.languagePrefix = "first-language:";
            const QString term = QString::fromUtf8("Éowyn / tagged \"test\"\n>=3");
            const CardFilter original(term, static_cast<CardFilter::Type>(type), static_cast<CardFilter::Attr>(attr));
            const QJsonObject saved = original.toJson();
            ASSERT_TRUE(saved["type"].toString().startsWith(translator.languagePrefix));
            ASSERT_TRUE(saved["attr"].toString().startsWith(translator.languagePrefix));
            const QByteArray bytes = QJsonDocument(saved).toJson();

            translator.languagePrefix = "second-language:";
            std::unique_ptr<CardFilter> restored(CardFilter::fromJson(QJsonDocument::fromJson(bytes).object()));
            ASSERT_NE(restored, nullptr) << "type=" << type << ", attr=" << attr;
            EXPECT_EQ(restored->type(), original.type());
            EXPECT_EQ(restored->attr(), original.attr());
            EXPECT_EQ(restored->term(), term);
            EXPECT_EQ(restored->toJson()["typeId"], saved["typeId"]);
            EXPECT_EQ(restored->toJson()["attrId"], saved["attrId"]);
        }
    }
}

TEST_F(CardFilterSerialization, LegacyCurrentLanguageStillLoads)
{
    for (int type = 0; type < CardFilter::TypeEnd; ++type) {
        for (int attr = 0; attr < CardFilter::AttrEnd; ++attr) {
            const CardFilter original("", static_cast<CardFilter::Type>(type), static_cast<CardFilter::Attr>(attr));
            QJsonObject legacy = original.toJson();
            legacy.remove("typeId");
            legacy.remove("attrId");
            std::unique_ptr<CardFilter> restored(CardFilter::fromJson(legacy));
            ASSERT_NE(restored, nullptr);
            EXPECT_EQ(restored->type(), original.type());
            EXPECT_EQ(restored->attr(), original.attr());
            EXPECT_EQ(restored->term(), QString());
        }
    }
}

TEST_F(CardFilterSerialization, LegacyEnglishLoadsWhileUiIsTranslated)
{
    const QJsonObject legacy{{"term", "U"}, {"type", "AND NOT"}, {"attr", "Color Identity"}};
    std::unique_ptr<CardFilter> restored(CardFilter::fromJson(legacy));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->type(), CardFilter::TypeAndNot);
    EXPECT_EQ(restored->attr(), CardFilter::AttrColorIdentity);
    EXPECT_EQ(restored->term(), "U");
}

TEST_F(CardFilterSerialization, StableIdsTakePrecedenceOverDisplayLabels)
{
    // A translated or edited label cannot change the saved filter's identity.
    QJsonObject saved{{"term", "favorite"}, {"typeId", "or-not"}, {"attrId", "tag"}, {"type", "AND"}, {"attr", "Name"}};
    std::unique_ptr<CardFilter> restored(CardFilter::fromJson(saved));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->type(), CardFilter::TypeOrNot);
    EXPECT_EQ(restored->attr(), CardFilter::AttrTag);
    saved.remove("type");
    saved.remove("attr");
    restored.reset(CardFilter::fromJson(saved));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->type(), CardFilter::TypeOrNot);
    EXPECT_EQ(restored->attr(), CardFilter::AttrTag);
}

TEST_F(CardFilterSerialization, InvalidIdsNeverFallBackToOtherwiseValidLabels)
{
    const CardFilter original("test", CardFilter::TypeAnd, CardFilter::AttrName);
    const QJsonObject good = original.toJson();
    for (const QString &key : {QString("typeId"), QString("attrId")}) {
        for (const QJsonValue &invalid :
             {QJsonValue(), QJsonValue(2), QJsonValue(true), QJsonValue("unknown"), QJsonValue("")}) {
            QJsonObject bad = good;
            bad[key] = invalid;
            std::unique_ptr<CardFilter> restored(CardFilter::fromJson(bad));
            EXPECT_EQ(restored, nullptr);
        }
        QJsonObject missing = good;
        missing.remove(key);
        std::unique_ptr<CardFilter> restored(CardFilter::fromJson(missing));
        EXPECT_EQ(restored, nullptr);
    }
}

TEST_F(CardFilterSerialization, UnknownLegacyLabelsAndInvalidTermsAreRejected)
{
    QJsonObject legacy{{"term", "example"}, {"type", "AND"}, {"attr", "unknown-language-name"}};
    std::unique_ptr<CardFilter> restored(CardFilter::fromJson(legacy));
    EXPECT_EQ(restored, nullptr);
    legacy["attr"] = "Name";
    legacy["type"] = "unknown-operator";
    restored.reset(CardFilter::fromJson(legacy));
    EXPECT_EQ(restored, nullptr);
    legacy["type"] = "AND";
    legacy["term"] = 42;
    restored.reset(CardFilter::fromJson(legacy));
    EXPECT_EQ(restored, nullptr);
    legacy.remove("term");
    restored.reset(CardFilter::fromJson(legacy));
    EXPECT_EQ(restored, nullptr);
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
