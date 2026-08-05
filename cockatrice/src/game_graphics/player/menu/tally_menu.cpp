#include "tally_menu.h"

#include "../../../client/settings/cache_settings.h"

#include <QActionGroup>

TallyMenu::TallyMenu()
{
    actionGroup = new QActionGroup(this);
    actionGroup->setExclusive(true);

    aTallyNone = createTallyAction(TallyType::None);
    aTallySubtypes = createTallyAction(TallyType::Subtypes);
    aTallyTotalPower = createTallyAction(TallyType::TotalPower);

    addAction(aTallyNone);
    addSeparator();
    addAction(aTallySubtypes);
    addAction(aTallyTotalPower);

    retranslateUi();
}

QAction *TallyMenu::createTallyAction(TallyType tallyType)
{
    TallyType currentType = Tally::intToType(SettingsCache::instance().interface().getTallyType());

    QAction *action = new QAction(this);
    action->setCheckable(true);
    action->setChecked(tallyType == currentType);

    connect(action, &QAction::triggered, &SettingsCache::instance().interface(),
            [tallyType] { SettingsCache::instance().interface().setTallyType(static_cast<int>(tallyType)); });

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
    aTallyTotalPower->setText(tr("Total Power"));
}
