#include "tab.h"

#include "../ui/widgets/cards/card_info_display_widget.h"
#include "./tab_supervisor.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QScreen>

Tab::Tab(TabSupervisor *_tabSupervisor)
    : QMainWindow(_tabSupervisor), tabSupervisor(_tabSupervisor), contentsChanged(false), infoPopup(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

void Tab::showCardInfoPopup(const QPoint &pos, const CardRef &cardRef)
{
    if (infoPopup) {
        infoPopup->deleteLater();
    }
    currentCard = cardRef;
    infoPopup = new CardInfoDisplayWidget(currentCard, nullptr,
                                          Qt::Widget | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint |
                                              Qt::WindowStaysOnTopHint);
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
        if (currentCard.name == cardName || cardName == "_") {
            infoPopup->deleteLater();
            infoPopup = 0;
        }
    }
}

bool Tab::closeRequest()
{
    return close();
}