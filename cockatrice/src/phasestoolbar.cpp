#include <QAction>
#include <QVBoxLayout>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include "phasestoolbar.h"
#include "protocol_items.h"

PhaseButton::PhaseButton(const QIcon &icon, QAction *_doubleClickAction)
	: QPushButton(icon, QString()), active(false), activeAnimationCounter(0), doubleClickAction(_doubleClickAction), pixmap(50, 50)
{
	setFocusPolicy(Qt::NoFocus);
	setFixedSize(50, 50);
	
	activeAnimationTimer = new QTimer(this);
	connect(activeAnimationTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
	activeAnimationTimer->setSingleShot(false);
	
	updatePixmap(pixmap);
}

void PhaseButton::updatePixmap(QPixmap &pixmap)
{
	pixmap.fill(Qt::transparent);
	
	QPainter painter(&pixmap);
	int height = pixmap.height();
	int width = pixmap.width();

	icon().paint(&painter, 5, 5, width - 10, height - 10);
}

void PhaseButton::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);

	painter.setBrush(QColor(220 * (activeAnimationCounter / 10.0), 220 * (activeAnimationCounter / 10.0), 220 * (activeAnimationCounter / 10.0)));
	painter.setPen(Qt::gray);
	painter.drawRect(1, 1, pixmap.width() - 2, pixmap.height() - 2);

	painter.drawPixmap(0, 0, size().width(), size().height(), pixmap);
	
//	painter.setBrush(QColor(220 * (activeAnimationCounter / 10.0), 220 * (activeAnimationCounter / 10.0), 220 * (activeAnimationCounter / 10.0), 255 * ((10 - activeAnimationCounter) / 15.0)));
	painter.setBrush(QColor(0, 0, 0, 255 * ((10 - activeAnimationCounter) / 15.0)));
	painter.setPen(Qt::gray);
	painter.drawRect(1, 1, pixmap.width() - 2, pixmap.height() - 2);
}

void PhaseButton::setActive(bool _active)
{
	if (active == _active)
		return;
	
	active = _active;
	activeAnimationTimer->start(50);
}

void PhaseButton::updateAnimation()
{
	if (active) {
		if (++activeAnimationCounter >= 10)
			activeAnimationTimer->stop();
	} else {
		if (--activeAnimationCounter <= 0)
			activeAnimationTimer->stop();
	}
	update();
}

void PhaseButton::setPhaseText(const QString &_phaseText)
{
	phaseText = _phaseText;
	setToolTip(phaseText);
}

void PhaseButton::mouseDoubleClickEvent(QMouseEvent */*event*/)
{
	triggerDoubleClickAction();
}

void PhaseButton::triggerDoubleClickAction()
{
	if (doubleClickAction)
		doubleClickAction->trigger();
}

PhasesToolbar::PhasesToolbar(QWidget *parent)
	: QFrame(parent)
{
	setBackgroundRole(QPalette::Shadow);
	setAutoFillBackground(true);
	
	QAction *aUntapAll = new QAction(this);
	connect(aUntapAll, SIGNAL(triggered()), this, SLOT(actUntapAll()));
	QAction *aDrawCard = new QAction(this);
	connect(aDrawCard, SIGNAL(triggered()), this, SLOT(actDrawCard()));
	
	PhaseButton *untapButton = new PhaseButton(QIcon(":/resources/icon_phase_untap.svg"), aUntapAll);
	untapButton->setShortcut(QKeySequence("F5"));
	PhaseButton *upkeepButton = new PhaseButton(QIcon(":/resources/icon_phase_upkeep.svg"));
	PhaseButton *drawButton = new PhaseButton(QIcon(":/resources/icon_phase_draw.svg"), aDrawCard);
	drawButton->setShortcut(QKeySequence("F6"));
	PhaseButton *main1Button = new PhaseButton(QIcon(":/resources/icon_phase_main1.svg"));
	main1Button->setShortcut(QKeySequence("F7"));
	PhaseButton *combatStartButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_start.svg"));
	combatStartButton->setShortcut(QKeySequence("F8"));
	PhaseButton *combatAttackersButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_attackers.svg"));
	PhaseButton *combatBlockersButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_blockers.svg"));
	PhaseButton *combatDamageButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_damage.svg"));
	PhaseButton *combatEndButton = new PhaseButton(QIcon(":/resources/icon_phase_combat_end.svg"));
	PhaseButton *main2Button = new PhaseButton(QIcon(":/resources/icon_phase_main2.svg"));
	main2Button->setShortcut(QKeySequence("F9"));
	PhaseButton *cleanupButton = new PhaseButton(QIcon(":/resources/icon_phase_cleanup.svg"));
	cleanupButton->setShortcut(QKeySequence("F10"));
	
	buttonList << untapButton << upkeepButton << drawButton << main1Button << combatStartButton
		<< combatAttackersButton << combatBlockersButton << combatDamageButton << combatEndButton
		<< main2Button << cleanupButton;
	
	for (int i = 0; i < buttonList.size(); ++i)
		connect(buttonList[i], SIGNAL(clicked()), this, SLOT(phaseButtonClicked()));
	
	QPushButton *nextTurnButton = new QPushButton(QIcon(":/resources/icon_nextturn.svg"), QString());
	nextTurnButton->setFocusPolicy(Qt::NoFocus);
	nextTurnButton->setIconSize(QSize(40, 40));
	nextTurnButton->setFixedSize(50, 50);
	connect(nextTurnButton, SIGNAL(clicked()), this, SLOT(actNextTurn()));
		
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setSpacing(0);
	
	layout->addStretch(1);
	layout->addWidget(untapButton);
	layout->addWidget(upkeepButton);
	layout->addWidget(drawButton);
	layout->addSpacing(10);
	layout->addWidget(main1Button);
	layout->addSpacing(10);
	layout->addWidget(combatStartButton);
	layout->addWidget(combatAttackersButton);
	layout->addWidget(combatBlockersButton);
	layout->addWidget(combatDamageButton);
	layout->addWidget(combatEndButton);
	layout->addSpacing(10);
	layout->addWidget(main2Button);
	layout->addSpacing(10);
	layout->addWidget(cleanupButton);
	layout->addSpacing(20);
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

void PhasesToolbar::setActivePhase(int phase)
{
	if (phase >= buttonList.size())
		return;
	
	for (int i = 0; i < buttonList.size(); ++i)
		buttonList[i]->setActive(i == phase);
}

void PhasesToolbar::phaseButtonClicked()
{
	PhaseButton *button = qobject_cast<PhaseButton *>(sender());
	if (button->getActive())
		button->triggerDoubleClickAction();
	emit sendGameCommand(new Command_SetActivePhase(-1, buttonList.indexOf(button)), -1);
}

void PhasesToolbar::actNextTurn()
{
	emit sendGameCommand(new Command_NextTurn, -1);
}

void PhasesToolbar::actUntapAll()
{
	emit sendGameCommand(new Command_SetCardAttr(-1, "table", -1, "tapped", "0"), -1);
}

void PhasesToolbar::actDrawCard()
{
	emit sendGameCommand(new Command_DrawCards(-1, 1), -1);
}
