#include "phasestoolbar.h"
#include <QAction>
//#include <QPushButton>
#include "phasebutton.h"
#include <QVBoxLayout>
#include <QButtonGroup>

PhasesToolbar::PhasesToolbar(QWidget *parent)
	: QFrame(parent)
{
	PhaseButton *untapButton = new PhaseButton(QIcon(":/resources/icon_phase_untap.svg"), QString());
	PhaseButton *upkeepButton = new PhaseButton(QIcon(":/resources/icon_phase_upkeep.svg"), QString());
	PhaseButton *drawButton = new PhaseButton(QIcon(":/resources/icon_phase_draw.svg"), QString());
	PhaseButton *main1Button = new PhaseButton(QIcon(":/resources/icon_phase_main1.svg"), QString());
	PhaseButton *combatStartButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_start.svg"), QString());
	PhaseButton *combatAttackersButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_attackers.svg"), QString());
	PhaseButton *combatBlockersButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_blockers.svg"), QString());
	PhaseButton *combatDamageButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_damage.svg"), QString());
	PhaseButton *combatEndButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_end.svg"), QString());
	PhaseButton *main2Button = new PhaseButton(QIcon(":/resources/icon_phase_main2.svg"), QString());
	PhaseButton *cleanupButton = new PhaseButton(QIcon(":/resources/icon_phase_cleanup.svg"), QString());
	
	buttonList << untapButton << upkeepButton << drawButton << main1Button << combatStartButton
		<< combatAttackersButton << combatBlockersButton << combatDamageButton << combatEndButton
		<< main2Button << cleanupButton;
		
	for (int i = 0; i < buttonList.size(); ++i) {
		buttonList[i]->setCheckable(true);
		buttonList[i]->setIconSize(QSize(40, 40));
	}
	
	PhaseButton *nextTurnButton = new PhaseButton(QIcon(":/resources/icon_nextturn.svg"), QString());
	nextTurnButton->setIconSize(QSize(40, 40));
		
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(0);
	
	QButtonGroup *bg = new QButtonGroup;
	for (int i = 0; i < buttonList.size(); ++i) {
		bg->addButton(buttonList[i]);
	}
	
	
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
	layout->addSpacing(25);
	layout->addWidget(nextTurnButton);
	layout->addStretch(1);
	
	setLayout(layout);
}
