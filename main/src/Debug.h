#ifndef DEBUG_H
#define DEBUG_H

/*
  Build Configuration
    DEBUG : 1 = enable serial debug (all modules)
            0 = disable serial debug
*/

#define DEBUG 1

#if (DEBUG != 0) && (DEBUG != 1)
  #error "DEBUG must be 1 or 0"
#endif

#endif
