/**
 * @file home_widget.h
 * @ingroup Core
 * @ingroup Widgets
 * @brief TODO: Document this.
 */

#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H

#include <QWidget>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/network/client/abstract/client_status.h>

class TabSupervisor;
class QGroupBox;
class QGridLayout;
class CardInfoPictureArtCropWidget;
class DeckList;
class HomeStyledButton;
class ExactCard;
class HomeWidget : public QWidget
{

    Q_OBJECT

public:
    HomeWidget(QWidget *parent, TabSupervisor *tabSupervisor);
    void updateRandomCard();
    QPair<QColor, QColor> extractDominantColors(const QPixmap &pixmap);

public slots:
    void paintEvent(QPaintEvent *event) override;
    void initializeBackgroundFromSource();
    void onBackgroundShuffleFrequencyChanged();
    void updateBackgroundProperties();
    void updateButtonsToBackgroundColor();
    QGroupBox *createButtons();
    void updateConnectButton(const ClientStatus status);

private:
    QGridLayout *layout;
    QTimer *cardChangeTimer;
    TabSupervisor *tabSupervisor;
    QPixmap background;
    CardInfoPictureArtCropWidget *backgroundSourceCard = nullptr;
    DeckList backgroundSourceDeck;
    QPixmap overlay;
    QPair<QColor, QColor> gradientColors;
    HomeStyledButton *connectButton;

    void setRandomCard(ExactCard &newCard);
    void loadBackgroundSourceDeck();
};

#endif // HOME_WIDGET_H
