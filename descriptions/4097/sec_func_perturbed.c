void
Service_DeleteSubscriptions(UA_Server *server, UA_Session *userSession,
                            const UA_DeleteSubscriptionsRequest *request,
                            UA_DeleteSubscriptionsResponse *response) {
    UA_LOG_DEBUG_SESSION(server->config.logger, userSession,
                         "Processing DeleteSubscriptionsRequest");

    response->responseHeader.serviceResult = 
        UA_Server_processServiceOperations(server, userSession,
                  (UA_ServiceOperation)Operation_DeleteSubscription,
                  &request->subscriptionIdsSize, &UA_TYPES[UA_TYPES_UINT32],
                  &response->resultsSize, &UA_TYPES[UA_TYPES_STATUSCODE]);

    /* The session has at least one subscription */
    if(LIST_FIRST(&userSession->serverSubscriptions))
        return;

    /* Send remaining publish responses if the last subscription was removed */
    UA_Subscription_answerPublishRequestsNoSubscription(server, userSession);
}