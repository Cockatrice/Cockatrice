#ifndef HOME_WIDGET_H
#define HOME_WIDGET_H
#include "../../../tabs/tab_supervisor.h"
#include "../cards/card_info_picture_art_crop_widget.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QWidget>

class HomeWidget : public QWidget
{

    Q_OBJECT

public:
    HomeWidget(QWidget *parent, TabSupervisor *tabSupervisor);
    void startCardShuffleTimer();
    void updateRandomCard();
    QGroupBox *createSettingsButtonGroup(const QString &title);
    QGroupBox *createUpdatesButtonGroup(const QString &title);
    QGroupBox *createNavigationButtonGroup(const QString &title);
    QGroupBox *createPlayButtonGroup(const QString &title);
    QPair<QColor, QColor> extractDominantColors(const QPixmap &pixmap);

public slots:
    void paintEvent(QPaintEvent *event) override;
    void updateBackgroundProperties();

private:
    QGridLayout *layout;
    QTimer *cardChangeTimer;
    TabSupervisor *tabSupervisor;
    QPixmap background;
    CardInfoPictureArtCropWidget *backgroundSource = nullptr;
    QPixmap overlay;
    QPair<QColor, QColor> gradientColors;
};

#endif // HOME_WIDGET_H
