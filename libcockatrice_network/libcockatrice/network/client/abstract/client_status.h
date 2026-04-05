#ifndef COCKATRICE_CLIENT_STATUS_H
#define COCKATRICE_CLIENT_STATUS_H

enum ClientStatus
{
    StatusDisconnected,
    StatusDisconnecting,
    StatusConnecting,
    StatusRegistering,
    StatusActivating,
    StatusLoggingIn,
    StatusLoggedIn,
    StatusRequestingForgotPassword,
    StatusSubmitForgotPasswordReset,
    StatusSubmitForgotPasswordChallenge,
    StatusGettingPasswordSalt,
};

#endif // COCKATRICE_CLIENT_STATUS_H
