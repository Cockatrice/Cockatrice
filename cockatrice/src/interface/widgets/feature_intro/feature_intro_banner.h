#ifndef FEATURE_INTRO_BANNER_H
#define FEATURE_INTRO_BANNER_H

#include "../../../client/features/feature_intro.h"
#include "../../../client/features/feature_intro_actions.h"

#include <QFrame>

class QHBoxLayout;
class QLabel;
class QToolButton;

/**
 * @file feature_intro_banner.h
 * @brief Non-modal, in-place invitation shown where the feature lives.
 *
 * A banner rather than a dialog: the user is about to use the feature, so the
 * explanation belongs next to it, can be read while playing, and never blocks
 * the board. Closing it with X is final; the Later button and Escape only skip
 * this session, and the service's cooldown decides when to try again.
 */
class FeatureIntroBanner : public QFrame
{
    Q_OBJECT

public:
    explicit FeatureIntroBanner(const FeatureIntro &intro, QWidget *parent = nullptr);

    /// Replaces the buttons with what the user now has, plus its undo.
    void showOutcome(FeatureIntroAction undo);

signals:
    void accepted();
    void declined();
    void laterRequested();
    void settingsRequested();
    void dismissed();
    /// The user undid the switch; the banner has no further purpose.
    void undone();

private:
    void showInvitation();

    FeatureIntro intro;
    QHBoxLayout *actionLayout = nullptr;
    /// The X in the invitation's title row; the outcome row retires it, because
    /// "dismissed for the revision" must not overwrite a decision already made.
    QToolButton *titleCloseButton = nullptr;
};

#endif // FEATURE_INTRO_BANNER_H
