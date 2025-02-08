UA_Connection *connection = mc->channel->connection;
    connection->releaseSendBuffer(connection, &mc->messageBuffer);