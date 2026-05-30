#ifndef COCKATRICE_ABSTRACT_SETTINGS_PAGE_H
#define COCKATRICE_ABSTRACT_SETTINGS_PAGE_H

#include <QWidget>

#define WIKI_CUSTOM_PIC_URL "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Picture-Download-URLs"
#define WIKI_CUSTOM_SHORTCUTS "https://github.com/Cockatrice/Cockatrice/wiki/Custom-Keyboard-Shortcuts"
#define WIKI_TRANSLATION_FAQ "https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ"

class AbstractSettingsPage : public QWidget
{
public:
    virtual void retranslateUi() = 0;
};

#endif // COCKATRICE_ABSTRACT_SETTINGS_PAGE_H
