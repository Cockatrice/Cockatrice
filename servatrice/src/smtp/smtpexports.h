#ifndef SMTPEXPORTS_H
#define SMTPEXPORTS_H

/*
#ifdef SMTP_BUILD
#define SMTP_EXPORT Q_DECL_EXPORT
#else
#define SMTP_EXPORT Q_DECL_IMPORT
#endif
*/

// Servatrice compiles this statically in, so there's no need to declare exports
#define SMTP_EXPORT

#endif // SMTPEXPORTS_H
