#include "path_safety_warning.h"

#include "../../client/settings/cache_settings.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <libcockatrice/settings/paths_settings.h>

class QWidget;

bool PathSafetyWarning::confirmOutsideProgramDir(QWidget *parent, const QString &path)
{
    if (SettingsCache::instance().getIsPortableBuild()) {
        return true;
    }

    if (!PathsSettings::isInsideDir(path, QCoreApplication::applicationDirPath())) {
        return true;
    }

    QMessageBox box(QMessageBox::Warning, QObject::tr("Path inside the program directory"),
                    QObject::tr("'%1' is inside the Cockatrice program directory.\n\n"
                                "Files stored there can be deleted by an update or by uninstalling "
                                "Cockatrice, so this change was not applied. Choose a location outside "
                                "the program directory.")
                        .arg(path),
                    QMessageBox::Ok, parent);
    box.setTextFormat(Qt::PlainText);
    box.exec();

    return false;
}
