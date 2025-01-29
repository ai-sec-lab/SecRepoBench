UA_LOG_DEBUG_SESSION(server->config.logger, session,
                         "Processing DeleteSubscriptionsRequest");

    response->responseHeader.serviceResult = 
        UA_Server_processServiceOperations(server, session,
                  (UA_ServiceOperation)Operation_DeleteSubscription,
                  &request->subscriptionIdsSize, &UA_TYPES[UA_TYPES_UINT32],
                  &response->resultsSize, &UA_TYPES[UA_TYPES_STATUSCODE]);

    /* The session has at least one subscription */
    if(LIST_FIRST(&session->serverSubscriptions))
        return;

    /* Send remaining publish responses in a delayed callback if the last
     * subscription was removed */
    UA_Server_delayedCallback(server, (UA_ServerCallback)
                              UA_Subscription_answerPublishRequestsNoSubscription,
                              session);