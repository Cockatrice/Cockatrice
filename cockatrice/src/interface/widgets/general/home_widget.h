/**
 * @file home_widget.h
 * @ingroup Core
 * @ingroup Widgets
 */
//! \todo Document this file.

#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H
#include "../../../interface/widgets/tabs/tab_supervisor.h"
#include "../cards/card_info_picture_art_crop_widget.h"
#include "home_styled_button.h"

#include <QGridLayout>
#include <QWidget>
#include <libcockatrice/network/client/abstract/abstract_client.h>

class QGridLayout;
class QLabel;

class HomeWidget : public QWidget
{

    Q_OBJECT

public:
    HomeWidget(QWidget *parent, TabSupervisor *tabSupervisor);
    void updateRandomCard();
    static QPair<QColor, QColor> extractDominantColors(const QPixmap &pixmap);

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
    QLabel *logoLabel = nullptr;
    QPair<QColor, QColor> gradientColors;
    HomeStyledButton *connectButton;

    void setRandomCard(ExactCard &newCard);
    void loadBackgroundSourceDeck();
    QPair<QColor, QColor> determineButtonColor() const;
    void updateLogoOverlay();
};

#endif // HOME_WIDGET_H
