#ifndef FEATURE_INTRO_BADGE_H
#define FEATURE_INTRO_BADGE_H

#include <QToolButton>

/**
 * @file feature_intro_badge.h
 * @brief The "New" marker that makes a feature's settings group discoverable.
 *
 * Shown while the user has not answered the feature's introduction yet — which
 * is exactly the case the whole system exists for: opt-out features are on, so
 * nothing else would ever point at the group that controls them. Hidden once
 * the question is answered, dismissed, or applies to nobody (baseline/skip).
 *
 * A flat QToolButton rather than a label: it is reachable from the keyboard in
 * the settings tab order, and activating it opens the feature's entry in the
 * What's New dialog.
 */
class FeatureIntroBadge : public QToolButton
{
    Q_OBJECT

public:
    explicit FeatureIntroBadge(const QString &featureId, QWidget *parent = nullptr);

    /// Re-reads the stored decision; call after the introduction is answered.
    void refresh();

    [[nodiscard]] QString featureId() const
    {
        return id;
    }

private:
    QString id;
};

#endif // FEATURE_INTRO_BADGE_H
