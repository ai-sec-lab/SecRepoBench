static UA_StatusCode
removeSession(UA_SessionManager *sm, session_list_entry *sentry) {
    /* Detach the Session from the SecureChannel */
    UA_Session_detachFromSecureChannel(&sentry->session);

    /* Deactivate the session */
    sentry->session.activated = false;

    /* Add a delayed callback to remove the session when the currently
     * scheduled jobs have completed */
    UA_StatusCode retval = UA_Server_delayedCallback(sm->server, removeSessionCallback, sentry);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_WARNING_SESSION(sm->server->config.logger, &sentry->session,
                       "Could not remove session with error code %s",
                       UA_StatusCode_name(retval));
        return retval; /* Try again next time */
    }

    /* Detach the session from the session manager and make the capacity
     * available */
    LIST_REMOVE(sentry, pointers);
    UA_atomic_add(&sm->currentSessionCount, (UA_UInt32)-1);
    return UA_STATUSCODE_GOOD;
}