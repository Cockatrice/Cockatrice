#ifndef CARDFRAME_H
#define CARDFRAME_H

#include "../../../../game/cards/card_database.h"

#include <QPushButton>
#include <QTabWidget>

class AbstractCardItem;
class CardInfoPictureWidget;
class CardInfoTextWidget;
class QVBoxLayout;
class QSplitter;

/**
 * This widget contains the CardInfoPictureWidget and the "view transformation" button.
 *
 * Simply hiding the button will still leave a small gap at the bottom of the CardInfoPictureWidget.
 * This widget deletes the button when it's not relevant in order to not leave any spaces.
 */
class CardPictureAndButtonWidget : public QWidget
{
    Q_OBJECT

    CardInfoPictureWidget *pic;
    QPushButton *viewTransformationButton;

    QPushButton *createViewTransformationButton();
    void setTransformButtonVisibility(bool visible);

public:
    explicit CardPictureAndButtonWidget(QWidget *parent = nullptr);
    void retranslateUi();

public slots:
    void setCard(const CardInfoPtr &card);

signals:
    /**
     * Forwarded from CardInfoPictureWidget::cardChanged
     */
    void cardChanged(CardInfoPtr card);
    void transformationButtonClicked();
};

class CardInfoFrameWidget : public QTabWidget
{
    Q_OBJECT
private:
    CardInfoPtr info;
    CardPictureAndButtonWidget *pic;
    CardInfoTextWidget *text;
    bool cardTextOnly;
    QWidget *tab1, *tab2, *tab3;
    QVBoxLayout *tab1Layout, *tab2Layout, *tab3Layout;
    QSplitter *splitter;

public:
    enum ViewMode
    {
        ImageOnlyView,
        TextOnlyView,
        ImageAndTextView
    };

    explicit CardInfoFrameWidget(const QString &cardName = QString(), QWidget *parent = nullptr);
    CardInfoPtr getInfo()
    {
        return info;
    }
    void retranslateUi();

public slots:
    void setCard(CardInfoPtr card);
    void setCard(const QString &cardName);
    void setCard(const QString &cardName, const QString &providerId);
    void setCard(AbstractCardItem *card);
    void viewTransformation();
    void clearCard();
    void setViewMode(int mode);
};

#endif
