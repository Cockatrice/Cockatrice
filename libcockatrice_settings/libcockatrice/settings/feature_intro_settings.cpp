#include "feature_intro_settings.h"

#include <QSettings>
#include <QVariant>

namespace
{
constexpr auto kRootGroup = "featureIntros";
constexpr auto kEntriesGroup = "featureIntros/entries";

QString entryGroup(const QString &id)
{
    return QStringLiteral("featureIntros/entries/") + id;
}

QString decisionToString(FeatureIntroDecision decision)
{
    switch (decision) {
        case FeatureIntroDecision::Accepted:
            return QStringLiteral("accepted");
        case FeatureIntroDecision::Declined:
            return QStringLiteral("declined");
        case FeatureIntroDecision::Dismissed:
            return QStringLiteral("dismissed");
        case FeatureIntroDecision::Baseline:
            return QStringLiteral("baseline");
        case FeatureIntroDecision::Skipped:
            return QStringLiteral("skipped");
        case FeatureIntroDecision::Pending:
        default:
            return QStringLiteral("pending");
    }
}

FeatureIntroDecision decisionFromString(const QString &value)
{
    if (value == QStringLiteral("accepted")) {
        return FeatureIntroDecision::Accepted;
    }
    if (value == QStringLiteral("declined")) {
        return FeatureIntroDecision::Declined;
    }
    if (value == QStringLiteral("dismissed")) {
        return FeatureIntroDecision::Dismissed;
    }
    if (value == QStringLiteral("baseline")) {
        return FeatureIntroDecision::Baseline;
    }
    if (value == QStringLiteral("skipped")) {
        return FeatureIntroDecision::Skipped;
    }
    return FeatureIntroDecision::Pending;
}
} // namespace

FeatureIntroSettings::FeatureIntroSettings(const QString &settingPath, QObject *parent)
    : SettingsManager(settingPath + "featureIntros.ini", QString(), QString(), parent)
{
}

bool FeatureIntroSettings::getGlobalEnabled() const
{
    return getValue(QStringLiteral("globalEnabled"), kRootGroup, QString(), true).toBool();
}

void FeatureIntroSettings::setGlobalEnabled(bool enabled)
{
    setValue(enabled, QStringLiteral("globalEnabled"), kRootGroup);
    emit globalEnabledChanged(enabled);
}

std::optional<FeatureIntroState> FeatureIntroSettings::getState(const QString &id) const
{
    const QString group = entryGroup(id);
    if (!containsValue(QStringLiteral("decision"), group)) {
        return std::nullopt;
    }

    FeatureIntroState state;
    state.decision = decisionFromString(getValue(QStringLiteral("decision"), group).toString());
    state.revision = getValue(QStringLiteral("revision"), group, QString(), 0).toInt();
    state.decidedAt = getValue(QStringLiteral("decidedAt"), group, QString(), qint64(0)).toLongLong();
    state.shownRevision = getValue(QStringLiteral("shownRevision"), group, QString(), 0).toInt();
    return state;
}

void FeatureIntroSettings::setState(const QString &id, const FeatureIntroState &state)
{
    const QString group = entryGroup(id);
    setValue(decisionToString(state.decision), QStringLiteral("decision"), group);
    setValue(state.revision, QStringLiteral("revision"), group);
    setValue(state.decidedAt, QStringLiteral("decidedAt"), group);
    setValue(state.shownRevision, QStringLiteral("shownRevision"), group);
    emit stateChanged(id);
}

void FeatureIntroSettings::setStates(const QList<QPair<QString, FeatureIntroState>> &states)
{
    batchWrite([&states](QSettings &settings) {
        for (const auto &entry : states) {
            settings.beginGroup(entryGroup(entry.first));
            settings.setValue(QStringLiteral("decision"), decisionToString(entry.second.decision));
            settings.setValue(QStringLiteral("revision"), entry.second.revision);
            settings.setValue(QStringLiteral("decidedAt"), entry.second.decidedAt);
            settings.setValue(QStringLiteral("shownRevision"), entry.second.shownRevision);
            settings.endGroup();
        }
    });

    for (const auto &entry : states) {
        emit stateChanged(entry.first);
    }
}

void FeatureIntroSettings::clearState(const QString &id)
{
    const QString group = entryGroup(id);
    deleteValue(QStringLiteral("decision"), group);
    deleteValue(QStringLiteral("revision"), group);
    deleteValue(QStringLiteral("decidedAt"), group);
    deleteValue(QStringLiteral("shownRevision"), group);
    emit stateChanged(id);
}

void FeatureIntroSettings::setShownRevision(const QString &id, int revision)
{
    const QString group = entryGroup(id);
    if (getValue(QStringLiteral("shownRevision"), group, QString(), 0).toInt() == revision) {
        return;
    }
    setValue(revision, QStringLiteral("shownRevision"), group);
    emit stateChanged(id);
}

QStringList FeatureIntroSettings::getKnownIds() const
{
    auto settings = getSettings();
    settings.beginGroup(kEntriesGroup);
    const QStringList ids = settings.childGroups();
    settings.endGroup();
    return ids;
}
