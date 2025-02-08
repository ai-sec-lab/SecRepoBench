void
Service_ActivateSession(UA_Server *server, UA_SecureChannel *secureChannel,
                        UA_Session *session, const UA_ActivateSessionRequest *request,
                        UA_ActivateSessionResponse *response) {
    if(session->validTill < UA_DateTime_nowMonotonic()) {
        UA_LOG_INFO_SESSION(server->config.logger, session,
                            "ActivateSession: SecureChannel %i wants "
                            "to activate, but the session has timed out",
                            secureChannel->securityToken.channelId);
        response->responseHeader.serviceResult =
            UA_STATUSCODE_BADSESSIONIDINVALID;
        return;
    }

    checkSignature(server, secureChannel, session, request, response);
    if(response->responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        return;

    /* Callback into userland access control */
    response->responseHeader.serviceResult =
        server->config.accessControl.activateSession(&session->sessionId,
                                                     &request->userIdentityToken,
                                                     &session->sessionHandle);
    if(response->responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        return;

    /* Detach the old SecureChannel */
    // <MASK>
}