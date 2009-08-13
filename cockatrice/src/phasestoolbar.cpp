#include "phasestoolbar.h"
#include <QAction>
#include <QPushButton>
#include <QVBoxLayout>

PhasesToolbar::PhasesToolbar(QWidget *parent)
	: QFrame(parent)
{
	QPushButton *untapButton = new QPushButton;
	untapButton->setIcon(QIcon(":/resources/icon_phase_untap.svg"));
	untapButton->setCheckable(true);
	untapButton->setIconSize(QSize(64, 64));
	QPushButton *upkeepButton = new QPushButton;
	upkeepButton->setIcon(QIcon(":/resources/icon_phase_upkeep.svg"));
	upkeepButton->setCheckable(true);
	upkeepButton->setIconSize(QSize(64, 64));
	QPushButton *drawButton = new QPushButton;
	drawButton->setIcon(QIcon(":/resources/icon_phase_draw.svg"));
	drawButton->setCheckable(true);
	drawButton->setIconSize(QSize(64, 64));
	QPushButton *main1Button = new QPushButton;
	main1Button->setIcon(QIcon(":/resources/icon_phase_main1.svg"));
	main1Button->setCheckable(true);
	main1Button->setIconSize(QSize(64, 64));
	QPushButton *combatButton = new QPushButton;
	combatButton->setIcon(QIcon(":/resources/icon_phase_combat.svg"));
	combatButton->setCheckable(true);
	combatButton->setIconSize(QSize(64, 64));
	QPushButton *main2Button = new QPushButton;
	main2Button->setIcon(QIcon(":/resources/icon_phase_main2.svg"));
	main2Button->setCheckable(true);
	main2Button->setIconSize(QSize(64, 64));
	QPushButton *cleanupButton = new QPushButton;
	cleanupButton->setIcon(QIcon(":/resources/icon_phase_cleanup.svg"));
	cleanupButton->setCheckable(true);
	cleanupButton->setIconSize(QSize(64, 64));
	
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addStretch(1);
	layout->addWidget(untapButton);
	layout->addWidget(upkeepButton);
	layout->addWidget(drawButton);
	layout->addWidget(main1Button);
	layout->addWidget(combatButton);
	layout->addWidget(main2Button);
	layout->addWidget(cleanupButton);
	layout->addStretch(1);
	
	setLayout(layout);
}
