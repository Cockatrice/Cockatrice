#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H
#include "../../../server/abstract_client.h"
#include "../../../tabs/tab_supervisor.h"
#include "../cards/card_info_picture_art_crop_widget.h"
#include "home_styled_button.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QWidget>

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
    DeckLoader *backgroundSourceDeck;
    QPixmap overlay;
    QPair<QColor, QColor> gradientColors;
    HomeStyledButton *connectButton;
};

#endif // HOME_WIDGET_H
