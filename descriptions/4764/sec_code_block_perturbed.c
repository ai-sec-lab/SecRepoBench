UA_Connection *connection = messageContext->channel->connection;
    connection->releaseSendBuffer(connection, &messageContext->messageBuffer);