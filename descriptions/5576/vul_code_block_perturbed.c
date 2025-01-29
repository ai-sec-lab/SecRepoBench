if(session->header.channel && session->header.channel != secureChannel) {
        UA_LOG_INFO_SESSION(server->config.logger, session,
                            "ActivateSession: Detach from old channel");
        UA_Session_detachFromSecureChannel(session);
    }

    /* Attach to the SecureChannel and activate */
    UA_Session_attachToSecureChannel(session, secureChannel);
    session->activated = true;
    UA_Session_updateLifetime(session);
    UA_LOG_INFO_SESSION(server->config.logger, session,
                        "ActivateSession: Session activated");