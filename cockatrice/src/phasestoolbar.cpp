#include "phasestoolbar.h"
#include <QAction>
#include <QPushButton>
#include <QVBoxLayout>

PhasesToolbar::PhasesToolbar(QWidget *parent)
	: QFrame(parent)
{
	QPushButton *untapButton = new QPushButton(QIcon(":/resources/icon_phase_untap.svg"), QString());
	QPushButton *upkeepButton = new QPushButton(QIcon(":/resources/icon_phase_upkeep.svg"), QString());
	QPushButton *drawButton = new QPushButton(QIcon(":/resources/icon_phase_draw.svg"), QString());
	QPushButton *main1Button = new QPushButton(QIcon(":/resources/icon_phase_main1.svg"), QString());
	QPushButton *combatStartButton = new QPushButton(QIcon(":/resources/icon_phase_combat_start.svg"), QString());
	QPushButton *combatAttackersButton = new QPushButton(QIcon(":/resources/icon_phase_combat_attackers.svg"), QString());
	QPushButton *combatBlockersButton = new QPushButton(QIcon(":/resources/icon_phase_combat_blockers.svg"), QString());
	QPushButton *combatDamageButton = new QPushButton(QIcon(":/resources/icon_phase_combat_damage.svg"), QString());
	QPushButton *combatEndButton = new QPushButton(QIcon(":/resources/icon_phase_combat_end.svg"), QString());
	QPushButton *main2Button = new QPushButton(QIcon(":/resources/icon_phase_main2.svg"), QString());
	QPushButton *cleanupButton = new QPushButton(QIcon(":/resources/icon_phase_cleanup.svg"), QString());
	
	buttonList << untapButton << upkeepButton << drawButton << main1Button << combatStartButton
		<< combatAttackersButton << combatBlockersButton << combatDamageButton << combatEndButton
		<< main2Button << cleanupButton;
		
	for (int i = 0; i < buttonList.size(); ++i) {
		buttonList[i]->setCheckable(true);
		buttonList[i]->setIconSize(QSize(45, 45));
	}
	
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(0);
	
	layout->addStretch(1);
	layout->addWidget(untapButton);
	layout->addWidget(upkeepButton);
	layout->addWidget(drawButton);
	layout->addSpacing(15);
	layout->addWidget(main1Button);
	layout->addSpacing(15);
	layout->addWidget(combatStartButton);
	layout->addWidget(combatAttackersButton);
	layout->addWidget(combatBlockersButton);
	layout->addWidget(combatDamageButton);
	layout->addWidget(combatEndButton);
	layout->addSpacing(15);
	layout->addWidget(main2Button);
	layout->addSpacing(15);
	layout->addWidget(cleanupButton);
	layout->addStretch(1);
	
	setLayout(layout);
}
