#ifndef COCKATRICE_OVERRIDE_PRINTING_WARN_H
#define COCKATRICE_OVERRIDE_PRINTING_WARN_H
#include <QMessageBox>

namespace OverridePrintingWarning
{

/**
 * @brief Pops up the warning message for the changing the override printing setting.
 * If the user clicks accept, then this also handles changing the setting and resetting the card image cache.
 *
 * @param parent The parent widget
 * @param enable Whether the user is trying to enable or disable the setting
 * @return Whether the user clicked accept. All other ways of closing the window returns false.
 */
bool execMessageBox(QWidget *parent, bool enable);

} // namespace OverridePrintingWarning

#endif // COCKATRICE_OVERRIDE_PRINTING_WARN_H
