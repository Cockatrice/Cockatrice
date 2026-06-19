#include "tally_menu.h"

#include "../../client/settings/cache_settings.h"

TallyMenu::TallyMenu()
{
    aTallyNone = createTallyAction(TallyType::None);
    aTallySubtype = createTallyAction(TallyType::Subtypes);

    addAction(aTallyNone);
    addSeparator();
    addAction(aTallySubtype);

    retranslateUi();
}

QAction *TallyMenu::createTallyAction(TallyType tallyType)
{
    TallyType currentType = SettingsCache::instance().getTallyType();

    QAction *action = new QAction(this);
    action->setCheckable(true);
    action->setChecked(tallyType == currentType);

    connect(action, &QAction::triggered, &SettingsCache::instance(),
            [tallyType] { SettingsCache::instance().setTallyType(tallyType); });
    connect(&SettingsCache::instance(), &SettingsCache::tallyTypeChanged, action,
            [action, tallyType](TallyType type) { action->setChecked(type == tallyType); });

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
    aTallySubtype->setText(tr("Subtype"));
}
