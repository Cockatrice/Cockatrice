#ifndef COCKATRICE_ABSTRACT_SETTINGS_PAGE_H
#define COCKATRICE_ABSTRACT_SETTINGS_PAGE_H

#include <QList>
#include <QWidget>

#define WIKI_CUSTOM_PIC_URL "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Picture-Download-URLs"
#define WIKI_CUSTOM_SHORTCUTS "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Keyboard-Shortcuts"
#define WIKI_TRANSLATION_FAQ "https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ"

struct SettingsSearchEntry;

class AbstractSettingsPage : public QWidget
{
    Q_OBJECT
public:
    virtual void retranslateUi() = 0;
    virtual QList<SettingsSearchEntry> getSearchEntries();
    virtual void resetToDefaults();

protected:
    static QList<SettingsSearchEntry> autoDetectSearchEntries(QWidget *page, int pageIndex);
};

#endif // COCKATRICE_ABSTRACT_SETTINGS_PAGE_H
