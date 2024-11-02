#include "tab.h"

#include "../ui/widgets/cards/card_info_display_widget.h"

#include <QApplication>
#include <QDebug>
#include <QScreen>

Tab::Tab(TabSupervisor *_tabSupervisor, QWidget *parent)
    : QMainWindow(parent), tabSupervisor(_tabSupervisor), contentsChanged(false), infoPopup(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

void Tab::showCardInfoPopup(const QPoint &pos, const QString &cardName)
{
    if (infoPopup) {
        infoPopup->deleteLater();
    }
    currentCardName = cardName;
    infoPopup = new CardInfoDisplayWidget(
        cardName, 0, Qt::Widget | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    infoPopup->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto screenRect = qApp->primaryScreen()->geometry();
    infoPopup->move(qMax(screenRect.left(), qMin(pos.x() - infoPopup->width() / 2,
                                                 screenRect.left() + screenRect.width() - infoPopup->width())),
                    qMax(screenRect.top(), qMin(pos.y() - infoPopup->height() / 2,
                                                screenRect.top() + screenRect.height() - infoPopup->height())));
    infoPopup->show();
}

void Tab::deleteCardInfoPopup(const QString &cardName)
{
    if (infoPopup) {
        if ((currentCardName == cardName) || (cardName == "_")) {
            infoPopup->deleteLater();
            infoPopup = 0;
        }
    }
}
