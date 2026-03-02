#include "override_printing_warning.h"

#include "../../card_picture_loader/card_picture_loader.h"
#include "../../client/settings/cache_settings.h"

bool OverridePrintingWarning::execMessageBox(QWidget *parent, bool enable)
{
    QString message;
    if (enable) {
        message =
            QObject::tr("Enabling this feature will disable the use of the Printing Selector.\n\n"
                        "You will not be able to manage printing preferences on a per-deck basis, "
                        "or see printings other people have selected for their decks.\n\n"
                        "You will have to use the Set Manager, available through Card Database -> Manage Sets.\n\n"
                        "Are you sure you would like to enable this feature?");
    } else {
        message = QObject::tr(
            "Disabling this feature will enable the Printing Selector.\n\n"
            "You can now choose printings on a per-deck basis in the Deck Editor and configure which printing "
            "gets added to a deck by default by pinning it in the Printing Selector.\n\n"
            "You can also use the Set Manager to adjust custom sort order for printings in the Printing Selector"
            " (other sort orders like alphabetical or release date are available).\n\n"
            "Are you sure you would like to disable this feature?");
    }

    QMessageBox::StandardButton result =
        QMessageBox::question(parent, QObject::tr("Confirm Change"), message, QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        SettingsCache::instance().setOverrideAllCardArtWithPersonalPreference(static_cast<Qt::CheckState>(enable));
        // Caches are now invalid.
        CardPictureLoader::clearPixmapCache();
        CardPictureLoader::clearNetworkCache();
        return true;
    }

    return false;
}