#ifndef COCKATRICE_DECK_ANALYTICS_WIDGET_BASE_H
#define COCKATRICE_DECK_ANALYTICS_WIDGET_BASE_H

#include "../general/display/banner_widget.h"

#include <QJsonObject>
#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class DeckListStatisticsAnalyzer;
class QDialog;
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;

class AbstractAnalyticsPanelWidget : public QWidget
{
    Q_OBJECT
public slots:
    virtual void updateDisplay() = 0;
    // Widgets must return a config dialog
    virtual QDialog *createConfigDialog(QWidget *parent) = 0;

public:
    explicit AbstractAnalyticsPanelWidget(QWidget *parent, DeckListStatisticsAnalyzer *analyzer);

    void setDisplayTitle(const QString &title)
    {
        displayTitle = title;
        if (bannerWidget) {
            bannerWidget->setText(displayTitle);
        }
    }

    QString displayTitleText() const
    {
        return displayTitle;
    }

    virtual QJsonObject saveConfig() const
    {
        return {};
    }
    virtual void loadConfig(const QJsonObject &)
    {
    }

    // Unified helper to run config dialog and update widget
    bool applyConfigFromDialog();

    // Dialog → JSON must be supplied by each subclass
    virtual QJsonObject extractConfigFromDialog(QDialog *dlg) const = 0;

protected:
    DeckListStatisticsAnalyzer *analyzer;
    QVBoxLayout *layout;
    QWidget *bannerAndSettingsContainer;
    QHBoxLayout *bannerAndSettingsLayout;
    QString displayTitle;
    BannerWidget *bannerWidget;
    QPushButton *configureButton;
};

#endif // COCKATRICE_DECK_ANALYTICS_WIDGET_BASE_H
