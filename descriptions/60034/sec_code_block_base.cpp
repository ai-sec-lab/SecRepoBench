bool would_continue = !onlycpdrule && simplifiedcpd;
            if (!scpd && would_continue) {
                // given the while conditions that continue jumps to, this situation
                // never ends
                HUNSPELL_WARNING(stderr, "break infinite loop\n");
                break;
            }

            if (scpd > 0 && would_continue) {
                // under these conditions we loop again, but the assumption above
                // appears to be that cmin and cmax are the original values they
                // had in the outside loop
                cmin = oldcmin;
                cmax = oldcmax;
            }