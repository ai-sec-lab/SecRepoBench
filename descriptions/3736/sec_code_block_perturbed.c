// check if entry already in list:
                        UA_Boolean existing = false;
                        for (size_t j=0; j<foundServersSize; j++) {
                            if (UA_String_equal(&foundServerFilteredPointer[j]->serverUri, &request->serverUris[i])) {
                                existing = true;
                                break;
                            }
                        }
                        if (!existing)
                            foundServerFilteredPointer[foundServersSize++] = &current->registeredServer;