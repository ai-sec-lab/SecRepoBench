if (genericIndex == -1) {
                return false;
            }
            *outReturnType = fReturnType.coercibleTypes()[genericIndex];