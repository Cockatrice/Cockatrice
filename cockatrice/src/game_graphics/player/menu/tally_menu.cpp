#include "tally_menu.h"

#include "../../../client/settings/cache_settings.h"

#include <QActionGroup>

TallyMenu::TallyMenu()
{
    actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);

    aTallyNone = createTallyAction(TallyType::None);
    aTallySubtypes = createTallyAction(TallyType::Subtypes);

    addAction(aTallyNone);
    addSeparator();
    addAction(aTallySubtypes);

    retranslateUi();
}

QAction *TallyMenu::createTallyAction(TallyType tallyType)
{
    TallyType currentType = Tally::intToType(SettingsCache::instance().getTallyType());

    QAction *action = new QAction(this);
    action->setCheckable(true);
    action->setChecked(tallyType == currentType);

    connect(action, &QAction::triggered, &SettingsCache::instance(),
            [tallyType] { SettingsCache::instance().setTallyType(static_cast<int>(tallyType)); });

    actionGroup->addAction(action);

    return action;
}

void TallyMenu::setShortcutsActive()
{
    // no-op because we haven't decided if we're adding shortcuts for tally types
}

void TallyMenu::setShortcutsInactive()
{
    // no-op because we haven't decided if we're adding shortcuts for tally types
}

void TallyMenu::retranslateUi()
{
    setTitle(tr("Tally"));

    aTallyNone->setText(tr("None"));
    aTallySubtypes->setText(tr("Subtypes"));
}
