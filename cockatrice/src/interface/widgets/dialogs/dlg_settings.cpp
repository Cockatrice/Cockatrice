/**
 * @file dlg_settings.cpp
 * @brief Implementation of the main settings dialog (thin frame around SettingsView)
 * @ingroup Dialogs
 */
#include "dlg_settings.h"

#include "../../../client/settings/cache_settings.h"
#include "../main.h"
#include "../settings_page/abstract_settings_page.h"
#include "libcockatrice/card/database/card_database.h"
#include "libcockatrice/card/database/card_database_loader.h"
#include "libcockatrice/card/database/card_database_manager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMessageLogger>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/personal_settings.h>

static constexpr int MIN_DIALOG_WIDTH = 750;
static constexpr int MIN_DIALOG_HEIGHT = 700;
static constexpr int MAX_DIALOG_WIDTH = 1280;
static constexpr double DIALOG_SIZE_RATIO = 0.9;

/**
 * @brief Calculates the initial dialog size from the host window or screen
 * @param hostSize Size of the hosting main window, or empty when parentless
 * @param availableSize Available screen area the dialog must fit into
 * @param minimumSize Minimum dialog size already applied via setMinimumSize
 * @return Target size at ~90% of the host with small margins, clamped to fit
 */
static QSize calculateDialogSize(const QSize &hostSize, const QSize &availableSize, const QSize &minimumSize)
{
    QSize target = !hostSize.isEmpty() ? QSize(qRound(hostSize.width() * DIALOG_SIZE_RATIO),
                                               qRound(hostSize.height() * DIALOG_SIZE_RATIO))
                                       : QSize(qRound(availableSize.width() * DIALOG_SIZE_RATIO),
                                               qRound(availableSize.height() * DIALOG_SIZE_RATIO));
    // Evenly-spaced columns leave unnatural dead space on very wide windows, so clamp the width.
    target.setWidth(qMin(target.width(), MAX_DIALOG_WIDTH));
    return target.expandedTo(minimumSize).boundedTo(availableSize);
}

DlgSettings::DlgSettings(QWidget *parent) : QDialog(parent)
{
    // Fill the main window with small margins so multi-group pages (e.g. Deck Editor) are not buried in a scroll.
    // 90% of the host window leaves ~5% margins on each side; falls back to 90% of the screen when parentless.
    QWidget *hostWindow = parent ? parent->window() : nullptr;
    QScreen *screen = nullptr;
    QSize hostSize;
    if (hostWindow) {
        hostSize = hostWindow->size();
        screen = QGuiApplication::screenAt(hostWindow->geometry().center());
    }
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect available = screen ? screen->availableGeometry() : QRect(0, 0, 1280, 800);
    const QSize minimum(qMin(MIN_DIALOG_WIDTH, available.width()), qMin(MIN_DIALOG_HEIGHT, available.height()));
    setMinimumSize(minimum);

    resize(calculateDialogSize(hostSize, available.size(), minimum));
    setSizeGripEnabled(true);

    setObjectName("settingsDialog");

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this, &DlgSettings::updateLanguage);

    settingsView = new SettingsView(this);

    // Bottom buttons
    auto *buttonBox = new QHBoxLayout;
    buttonBox->addStretch();
    okButton = new QPushButton;
    okButton->setDefault(true);
    okButton->setObjectName("settingsOkButton");
    connect(okButton, &QPushButton::clicked, this, &DlgSettings::close);
    buttonBox->addWidget(okButton);

    // Main layout
    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(settingsView);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(buttonBox);
    setLayout(mainLayout);

    connect(&SettingsCache::instance().personal(), &PersonalSettings::langChanged, this, &DlgSettings::retranslateUi);
    retranslateUi();
}

void DlgSettings::setTab(int index)
{
    settingsView->setTab(index);
}

AbstractSettingsPage *DlgSettings::page(SettingsPage which) const
{
    return settingsView->page(which);
}

void DlgSettings::updateLanguage()
{
    qApp->removeTranslator(translator); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    installNewTranslator();
}

void DlgSettings::closeEvent(QCloseEvent *event)
{
    bool showLoadError = true;
    QString loadErrorMessage = tr("Unknown Error loading card database");
    LoadStatus loadStatus = CardDatabaseManager::getInstance()->getLoadStatus();
    qCInfo(DlgSettingsLog) << "Card Database load status: " << loadStatus;
    switch (loadStatus) {
        case Ok:
            showLoadError = false;
            break;
        case Invalid:
            loadErrorMessage = tr("Your card database is invalid.\n\n"
                                  "Cockatrice may not function correctly with an invalid database\n\n"
                                  "You may need to rerun Oracle to update your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case VersionTooOld:
            loadErrorMessage = tr("Your card database version is too old.\n\n"
                                  "This can cause problems loading card information or images\n\n"
                                  "Usually this can be fixed by rerunning Oracle to to update your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case NotLoaded:
            loadErrorMessage = tr("Your card database did not finish loading\n\n"
                                  "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues with your "
                                  "cards.xml attached\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case FileError:
            loadErrorMessage = tr("File Error loading your card database.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        case NoCards:
            loadErrorMessage = tr("Your card database was loaded but contains no cards.\n\n"
                                  "Would you like to change your database location setting?");
            break;
        default:
            loadErrorMessage = tr("Unknown card database load status\n\n"
                                  "Please file a ticket at https://github.com/Cockatrice/Cockatrice/issues\n\n"
                                  "Would you like to change your database location setting?");

            break;
    }

    if (showLoadError) {
        if (QMessageBox::critical(this, tr("Error"), loadErrorMessage, QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }

    if (!QDir(SettingsCache::instance().paths().getDeckPath()).exists() ||
        SettingsCache::instance().paths().getDeckPath().isEmpty()) {
        if (QMessageBox::critical(
                this, tr("Error"),
                tr("The path to your deck directory is invalid. Would you like to go back and set the correct path?"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }

    if (!QDir(SettingsCache::instance().paths().getPicsPath()).exists() ||
        SettingsCache::instance().paths().getPicsPath().isEmpty()) {
        if (QMessageBox::critical(this, tr("Error"),
                                  tr("The path to your card pictures directory is invalid. Would you like to go back "
                                     "and set the correct path?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void DlgSettings::retranslateUi()
{
    setWindowTitle(tr("Settings"));
    okButton->setText(tr("OK"));
}
