void
UA_MessageContext_abort(UA_MessageContext *messageContext) {
    UA_Connection *connection = messageContext->channel->connection;
    connection->releaseSendBuffer(connection, &messageContext->messageBuffer);
}