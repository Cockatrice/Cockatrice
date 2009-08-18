#include "phasestoolbar.h"
#include <QAction>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QPen>

PhaseButton::PhaseButton(QIcon icon)
	: QPushButton(icon, QString())
{

}

void PhaseButton::update()
{
	QPushButton::update();
}

void PhaseButton::paintEvent(QPaintEvent *event)
{
	QPushButton::paintEvent(event);
	if (isChecked()) {
		QPainter painter(this);
		int height = size().height();
		int width = size().width();

		QPen pen;
		pen.setWidth(3);
		pen.setColor(QColor::fromRgb(180, 0, 0));
		painter.setPen(pen);

//		painter.setPen(QColor(0, 0, 0, 0));
		QRadialGradient grad(QPointF(0.5, 0.5), 0.5);
		grad.setCoordinateMode(QGradient::ObjectBoundingMode);
		grad.setColorAt(0, QColor(180, 0, 0, 0));
		grad.setColorAt(0.8, QColor(180, 0, 0, 0));
		grad.setColorAt(1, QColor(180, 0, 0, 255));
		painter.setBrush(QBrush(grad));
//		painter.drawEllipse(QRect(0, 0, width, height));

		painter.drawRect(3, 3, width - 7, height - 7);

	}
}

void PhaseButton::setPhaseText(const QString &_phaseText)
{
	phaseText = _phaseText;
	setToolTip(phaseText);
}

PhasesToolbar::PhasesToolbar(QWidget *parent)
	: QFrame(parent)
{
	PhaseButton *untapButton = new PhaseButton(QIcon(":/resources/icon_phase_untap.svg"));
	PhaseButton *upkeepButton = new PhaseButton(QIcon(":/resources/icon_phase_upkeep.svg"));
	PhaseButton *drawButton = new PhaseButton(QIcon(":/resources/icon_phase_draw.svg"));
	PhaseButton *main1Button = new PhaseButton(QIcon(":/resources/icon_phase_main1.svg"));
	PhaseButton *combatStartButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_start.svg"));
	PhaseButton *combatAttackersButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_attackers.svg"));
	PhaseButton *combatBlockersButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_blockers.svg"));
	PhaseButton *combatDamageButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_damage.svg"));
	PhaseButton *combatEndButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_end.svg"));
	PhaseButton *main2Button = new PhaseButton(QIcon(":/resources/icon_phase_main2.svg"));
	PhaseButton *cleanupButton = new PhaseButton(QIcon(":/resources/icon_phase_cleanup.svg"));
	
	buttonList << untapButton << upkeepButton << drawButton << main1Button << combatStartButton
		<< combatAttackersButton << combatBlockersButton << combatDamageButton << combatEndButton
		<< main2Button << cleanupButton;
	
	for (int i = 0; i < buttonList.size(); ++i) {
		buttonList[i]->setCheckable(true);
		buttonList[i]->setIconSize(QSize(40, 40));
	}
	
	QPushButton *nextTurnButton = new QPushButton(QIcon(":/resources/icon_nextturn.svg"), QString());
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
	
	retranslateUi();
	setLayout(layout);
}

void PhasesToolbar::retranslateUi()
{
	buttonList[0]->setPhaseText(tr("Untap step"));
	buttonList[1]->setPhaseText(tr("Upkeep step"));
	buttonList[2]->setPhaseText(tr("Draw step"));
	buttonList[3]->setPhaseText(tr("First main phase"));
	buttonList[4]->setPhaseText(tr("Beginning of combat step"));
	buttonList[5]->setPhaseText(tr("Declare attackers step"));
	buttonList[6]->setPhaseText(tr("Declare blockers step"));
	buttonList[7]->setPhaseText(tr("Combat damage step"));
	buttonList[8]->setPhaseText(tr("End of combat step"));
	buttonList[9]->setPhaseText(tr("Second main phase"));
	buttonList[10]->setPhaseText(tr("End of turn step"));
}
