#ifndef SEARCH_SYNTAX_HELP_H
#define SEARCH_SYNTAX_HELP_H

#include <QLineEdit>
#include <QLoggingCategory>
#include <QTextBrowser>

inline Q_LOGGING_CATEGORY(SyntaxHelpLog, "syntax_help");

QTextBrowser *createSearchSyntaxHelpWindow(QLineEdit *lineEdit);

QTextBrowser *createDeckSearchSyntaxHelpWindow(QLineEdit *lineEdit);

#endif // SEARCH_SYNTAX_HELP_H
