#ifndef COCKATRICE_ABSTRACT_SETTINGS_PAGE_H
#define COCKATRICE_ABSTRACT_SETTINGS_PAGE_H

#include <QList>
#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

#define WIKI_CUSTOM_PIC_URL "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Picture-Download-URLs"
#define WIKI_CUSTOM_SHORTCUTS "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Keyboard-Shortcuts"
#define WIKI_TRANSLATION_FAQ "https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ"

struct SettingsSearchEntry;

/**
 * @brief A navigable section of a settings page (one group box)
 */
struct SettingsPageSection
{
    QString title;                    ///< Translated group title
    QWidget *sectionWidget = nullptr; ///< Group box to scroll to and highlight
};

class AbstractSettingsPage : public QWidget
{
    Q_OBJECT
public:
    virtual void retranslateUi() = 0;
    virtual QList<SettingsSearchEntry> getSearchEntries();
    /**
     * @brief Sections of this page for sub-page navigation, in display order
     *
     * Scans the page's direct-child group boxes; pages with a single group
     * report no sections so the nav stays flat for them.
     */
    QList<SettingsPageSection> getSections();

protected:
    static QList<SettingsSearchEntry> autoDetectSearchEntries(QWidget *page, int pageIndex);
};

#endif // COCKATRICE_ABSTRACT_SETTINGS_PAGE_H
