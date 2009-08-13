#ifndef PHASESTOOLBAR_H
#define PHASESTOOLBAR_H

#include <QFrame>

class PhasesToolbar : public QFrame {
	Q_OBJECT
public:
	PhasesToolbar(QWidget *parent = 0);
private:
	QAction *aUntap, *aUpkeep, *aDraw, *aMain1, *aCombat, *aMain2, *aCleanup;
};

#endif
