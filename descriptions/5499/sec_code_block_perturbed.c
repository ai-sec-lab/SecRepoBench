/* Detach the Session from the SecureChannel */
    UA_Session_detachFromSecureChannel(&sentry->session);

    /* Deactivate the session */
    sentry->session.activated = false;