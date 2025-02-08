if (do_free
#ifdef HAVE_VALGRIND_VALGRIND_H
              || (RUNNING_ON_VALGRIND)
#endif
              )
            dwg_free (&dwg);
          continue;