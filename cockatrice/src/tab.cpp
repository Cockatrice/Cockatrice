#include "tab.h"
#include "cardinfowidget.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
Tab::Tab(TabSupervisor *_tabSupervisor, QWidget *parent)
    : QWidget(parent), tabSupervisor(_tabSupervisor), contentsChanged(false), infoPopup(0)
{
}

void Tab::showCardInfoPopup(const QPoint &pos, const QString &cardName)
{
    infoPopup = new CardInfoWidget(CardInfoWidget::ModePopUp, cardName, 0, Qt::Widget | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    infoPopup->setAttribute(Qt::WA_TransparentForMouseEvents);
    QRect screenRect = qApp->desktop()->screenGeometry(this);
    infoPopup->move(
        qMax(screenRect.left(), qMin(pos.x() - infoPopup->width() / 2, screenRect.left() + screenRect.width() - infoPopup->width())),
        qMax(screenRect.top(), qMin(pos.y() - infoPopup->height() / 2, screenRect.top() + screenRect.height() - infoPopup->height()))
    );
    infoPopup->show();
}

void Tab::deleteCardInfoPopup(const QString &cardName)
{
    if (infoPopup) {
        if ((infoPopup->getCardName() == cardName) || (cardName == "_")) {
            infoPopup->deleteLater();
            infoPopup = 0;
        }
    }
}
