#ifndef SMTPEXPORTS_H
#define SMTPEXPORTS_H

#ifdef SMTP_BUILD
#define SMTP_EXPORT Q_DECL_EXPORT
#else
#define SMTP_EXPORT Q_DECL_IMPORT
#endif

#endif // SMTPEXPORTS_H
