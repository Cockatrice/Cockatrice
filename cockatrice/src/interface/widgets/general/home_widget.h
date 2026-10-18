/**
 * @file home_widget.h
 * @ingroup Core
 * @ingroup Widgets
 */
//! \todo Document this file.

#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H
#include "libcockatrice/deck_list/deck_list.h"

#include <QColor>
#include <QPair>
#include <QPixmap>
#include <QWidget>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <optional>
#include <qtmetamacros.h>

class QGridLayout;
class QLabel;
class CardInfoPictureArtCropWidget;
class ExactCard;
class HomeStyledButton;
class QGroupBox;
class QTimer;
class TabSupervisor;

class HomeWidget : public QWidget
{

    Q_OBJECT

public:
    HomeWidget(QWidget *parent, TabSupervisor *tabSupervisor);
    void updateRandomCard();
    static QPair<QColor, QColor> extractDominantColors(const QPixmap &pixmap);

public slots:
    void retranslateUi();
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
    HomeStyledButton *visualDeckEditorButton = nullptr;
    HomeStyledButton *visualDeckStorageButton = nullptr;
    HomeStyledButton *visualDatabaseDisplayButton = nullptr;
    HomeStyledButton *edhrecButton = nullptr;
    HomeStyledButton *archidektButton = nullptr;
    HomeStyledButton *replayButton = nullptr;
    HomeStyledButton *exitButton = nullptr;
    std::optional<ClientStatus> connectionStatus;

    void setRandomCard(ExactCard &newCard);
    void loadBackgroundSourceDeck();
    QPair<QColor, QColor> determineButtonColor() const;
    void updateLogoOverlay();
};

#endif // HOME_WIDGET_H
