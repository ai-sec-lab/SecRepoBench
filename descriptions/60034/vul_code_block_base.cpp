// given the while conditions that continue jumps to, this situation
            // never ends
            if (!scpd && !onlycpdrule && simplifiedcpd) {
                HUNSPELL_WARNING(stderr, "break infinite loop\n");
                break;
            }