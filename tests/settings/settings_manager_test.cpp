#include "gtest/gtest.h"
#include <QSettings>
#include <QTemporaryDir>
#include <libcockatrice/settings/settings_manager.h>

namespace
{

class SettingsManagerTest : public ::testing::Test
{
protected:
    QTemporaryDir tempDir;
    QString settingsPath;

    void SetUp() override
    {
        settingsPath = tempDir.path() + "/";
    }

    void writeValue(const QString &file, const QString &key, const QVariant &value)
    {
        QSettings qs(file, QSettings::IniFormat);
        qs.setValue(key, value);
    }
};

TEST_F(SettingsManagerTest, GetValueNoDefaultReturnsInvalid)
{
    SettingsManager sm(settingsPath + "test.ini");
    QVariant val = sm.getValue("nonexistent");
    ASSERT_TRUE(val.isNull());
}

TEST_F(SettingsManagerTest, GetValueWithDefaultReturnsDefaultWhenMissing)
{
    SettingsManager sm(settingsPath + "test.ini");
    QVariant val = sm.getValue("nonexistent", QString(), QString(), 42);
    ASSERT_EQ(val.toInt(), 42);
}

TEST_F(SettingsManagerTest, GetValueWithDefaultReturnsStoredWhenPresent)
{
    writeValue(settingsPath + "test.ini", "mykey", 99);

    SettingsManager sm(settingsPath + "test.ini");
    QVariant val = sm.getValue("mykey", QString(), QString(), 42);
    ASSERT_EQ(val.toInt(), 99);
}

TEST_F(SettingsManagerTest, GetValueWithGroup)
{
    writeValue(settingsPath + "grouped.ini", "group1/greeting", "world");

    SettingsManager sm(settingsPath + "grouped.ini");
    QVariant val = sm.getValue("greeting", "group1");
    ASSERT_EQ(val.toString(), QString("world"));
}

TEST_F(SettingsManagerTest, GetValueWithGroupAndSubGroup)
{
    writeValue(settingsPath + "nested.ini", "outer/inner/depth", 1);

    SettingsManager sm(settingsPath + "nested.ini");
    QVariant val = sm.getValue("depth", "outer", "inner");
    ASSERT_EQ(val.toInt(), 1);
}

TEST_F(SettingsManagerTest, DefaultGroupUsedInGetValue)
{
    writeValue(settingsPath + "defgroup.ini", "defaultGroup/key", "val");

    SettingsManager sm(settingsPath + "defgroup.ini", "defaultGroup");
    QVariant val = sm.getValue("key");
    ASSERT_EQ(val.toString(), QString("val"));
}

TEST_F(SettingsManagerTest, DefaultGroupScopesGetValue)
{
    writeValue(settingsPath + "scoped.ini", "key", "ungrouped");
    writeValue(settingsPath + "scoped.ini", "defaultGroup/key", "grouped");

    SettingsManager sm(settingsPath + "scoped.ini", "defaultGroup");
    QVariant val = sm.getValue("key");
    ASSERT_EQ(val.toString(), QString("grouped"));
}

TEST_F(SettingsManagerTest, DefaultSubGroupUsedInGetValue)
{
    writeValue(settingsPath + "sub.ini", "top/bottom/key", "nested");

    SettingsManager sm(settingsPath + "sub.ini", "top", "bottom");
    QVariant val = sm.getValue("key");
    ASSERT_EQ(val.toString(), QString("nested"));
}

TEST_F(SettingsManagerTest, BatchWriteWritesAllValues)
{
    SettingsManager sm(settingsPath + "batch.ini");
    sm.batchWrite([&](QSettings &qs) {
        qs.setValue("a", 1);
        qs.setValue("b", 2);
        qs.setValue("c", 3);
    });

    SettingsManager sm2(settingsPath + "batch.ini");
    ASSERT_EQ(sm2.getValue("a", QString(), QString(), 0).toInt(), 1);
    ASSERT_EQ(sm2.getValue("b", QString(), QString(), 0).toInt(), 2);
    ASSERT_EQ(sm2.getValue("c", QString(), QString(), 0).toInt(), 3);
}

TEST_F(SettingsManagerTest, BatchWriteWithGroup)
{
    SettingsManager sm(settingsPath + "batch_group.ini");
    sm.batchWrite([&](QSettings &qs) {
        qs.beginGroup("section");
        qs.setValue("x", 10);
        qs.endGroup();
    });

    SettingsManager sm2(settingsPath + "batch_group.ini");
    ASSERT_EQ(sm2.getValue("x", "section", QString(), 0).toInt(), 10);
}

TEST_F(SettingsManagerTest, SyncPersistsToDisk)
{
    QString filePath = settingsPath + "sync_test.ini";
    {
        SettingsManager sm(filePath);
        sm.batchWrite([&](QSettings &qs) { qs.setValue("persist", "yes"); });
    }
    {
        QSettings qs(filePath, QSettings::IniFormat);
        ASSERT_EQ(qs.value("persist").toString(), QString("yes"));
    }
}

TEST_F(SettingsManagerTest, MultipleReadsAfterWrite)
{
    SettingsManager sm(settingsPath + "multi.ini");
    sm.batchWrite([&](QSettings &qs) { qs.setValue("k1", 1); });

    QVariant v1 = sm.getValue("k1", QString(), QString(), 0);
    QVariant v2 = sm.getValue("nonexistent", QString(), QString(), 99);

    ASSERT_EQ(v1.toInt(), 1);
    ASSERT_EQ(v2.toInt(), 99);
}

TEST_F(SettingsManagerTest, ValuesDeletedViaQSettingsAreReadAsDefault)
{
    writeValue(settingsPath + "del.ini", "deleteMe", "present");

    SettingsManager sm(settingsPath + "del.ini");
    ASSERT_EQ(sm.getValue("deleteMe", QString(), QString(), "default").toString(), QString("present"));

    // Delete via QSettings directly (deleteValue is protected)
    {
        QSettings qs(settingsPath + "del.ini", QSettings::IniFormat);
        qs.remove("deleteMe");
    }

    ASSERT_EQ(sm.getValue("deleteMe", QString(), QString(), "default").toString(), QString("default"));
}

} // namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
