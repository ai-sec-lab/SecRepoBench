if(session->header.channel && session->header.channel != secureChannel) {
        UA_LOG_INFO_SESSION(server->config.logger, session,
                            "ActivateSession: Detach from old channel");
        UA_Session_detachFromSecureChannel(session);
        session->activated = false;
    }

    if (session->activated) {
        UA_LOG_INFO_SESSION(server->config.logger, session,
                            "ActivateSession: SecureChannel %i wants "
                                    "to activate, but the session is already activated",
                            secureChannel->securityToken.channelId);
        response->responseHeader.serviceResult =
                UA_STATUSCODE_BADSESSIONIDINVALID;
        return;

    }
    /* Attach to the SecureChannel and activate */
    UA_Session_attachToSecureChannel(session, secureChannel);
    session->activated = true;
    UA_Session_updateLifetime(session);
    UA_LOG_INFO_SESSION(server->config.logger, session,
                        "ActivateSession: Session activated");