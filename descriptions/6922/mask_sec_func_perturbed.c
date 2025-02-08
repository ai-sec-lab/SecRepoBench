int reduce(CPL_UNUSED integer *kfildo, integer *jmin, integer *jmax,
	integer *lbit, integer *nov, integer *lx, integer *ndg, integer *ibit,
	 integer *jbit, integer *kbit, integer *novref, integer *ibxx2,
	integer *errorcode)
{
    /* Initialized data */

    static const integer ifeed = 12;

    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    integer newboxtp = 0, j, l, m = 0, jj = 0, lxn = 0, left = 0;
    real pimp = 0;
    integer move = 0, novl = 0;
    char cfeed[1];
    integer /* nboxj[31], */ lxnkp = 0, iorigb = 0, ibxx2m1 = 0, movmin = 0,
        ntotbt[31], ntotpr = 0, newboxt = 0;
    integer *newbox, *newboxp;


/*        NOVEMBER 2001   GLAHN   TDL   GRIB2 */
/*        MARCH    2002   GLAHN   COMMENT IER = 715 */
/*        MARCH    2002   GLAHN   MODIFIED TO ACCOMMODATE LX=1 ON ENTRY */

/*        PURPOSE */
/*            DETERMINES WHETHER THE NUMBER OF GROUPS SHOULD BE */
/*            INCREASED IN ORDER TO REDUCE THE SIZE OF THE LARGE */
/*            GROUPS, AND TO MAKE THAT ADJUSTMENT.  BY REDUCING THE */
/*            SIZE OF THE LARGE GROUPS, LESS BITS MAY BE NECESSARY */
/*            FOR PACKING THE GROUP SIZES AND ALL THE INFORMATION */
/*            ABOUT THE GROUPS. */

/*            THE REFERENCE FOR NOV( ) WAS REMOVED IN THE CALLING */
/*            ROUTINE SO THAT KBIT COULD BE DETERMINED.  THIS */
/*            FURNISHES A STARTING POINT FOR THE ITERATIONS IN REDUCE. */
/*            HOWEVER, THE REFERENCE MUST BE CONSIDERED. */

/*        DATA SET USE */
/*           KFILDO - UNIT NUMBER FOR OUTPUT (PRINT) FILE. (OUTPUT) */

/*        VARIABLES IN CALL SEQUENCE */
/*              KFILDO = UNIT NUMBER FOR OUTPUT (PRINT) FILE.  (INPUT) */
/*             JMIN(J) = THE MINIMUM OF EACH GROUP (J=1,LX).  IT IS */
/*                       POSSIBLE AFTER SPLITTING THE GROUPS, JMIN( ) */
/*                       WILL NOT BE THE MINIMUM OF THE NEW GROUP. */
/*                       THIS DOESN'T MATTER; JMIN( ) IS REALLY THE */
/*                       GROUP REFERENCE AND DOESN'T HAVE TO BE THE */
/*                       SMALLEST VALUE.  (INPUT/OUTPUT) */
/*             JMAX(J) = THE MAXIMUM OF EACH GROUP (J=1,LX). */
/*                       (INPUT/OUTPUT) */
/*             LBIT(J) = THE NUMBER OF BITS NECESSARY TO PACK EACH GROUP */
/*                       (J=1,LX).  (INPUT/OUTPUT) */
/*              NOV(J) = THE NUMBER OF VALUES IN EACH GROUP (J=1,LX). */
/*                       (INPUT/OUTPUT) */
/*                  LX = THE NUMBER OF GROUPS.  THIS WILL BE INCREASED */
/*                       IF GROUPS ARE SPLIT.  (INPUT/OUTPUT) */
/*                 NDG = THE DIMENSION OF JMIN( ), JMAX( ), LBIT( ), AND */
/*                       NOV( ).  (INPUT) */
/*                IBIT = THE NUMBER OF BITS NECESSARY TO PACK THE JMIN(J) */
/*                       VALUES, J=1,LX.  (INPUT) */
/*                JBIT = THE NUMBER OF BITS NECESSARY TO PACK THE LBIT(J) */
/*                       VALUES, J=1,LX.  (INPUT) */
/*                KBIT = THE NUMBER OF BITS NECESSARY TO PACK THE NOV(J) */
/*                       VALUES, J=1,LX.  IF THE GROUPS ARE SPLIT, KBIT */
/*                       IS REDUCED.  (INPUT/OUTPUT) */
/*              NOVREF = REFERENCE VALUE FOR NOV( ).  (INPUT) */
/*            IBXX2(J) = 2**J (J=0,30).  (INPUT) */
/*                 IER = ERROR RETURN.  (OUTPUT) */
/*                         0 = GOOD RETURN. */
/*                       714 = PROBLEM IN ALGORITHM.  REDUCE ABORTED. */
/*                       715 = NGP NOT LARGE ENOUGH.  REDUCE ABORTED. */
/*           NTOTBT(J) = THE TOTAL BITS USED FOR THE PACKING BITS J */
/*                       (J=1,30).  (INTERNAL) */
/*            NBOXJ(J) = NEW BOXES NEEDED FOR THE PACKING BITS J */
/*                       (J=1,30).  (INTERNAL) */
/*           NEWBOX(L) = NUMBER OF NEW BOXES (GROUPS) FOR EACH ORIGINAL */
/*                       GROUP (L=1,LX) FOR THE CURRENT J.  (AUTOMATIC) */
/*                       (INTERNAL) */
/*          NEWBOXP(L) = SAME AS NEWBOX( ) BUT FOR THE PREVIOUS J. */
/*                       THIS ELIMINATES RECOMPUTATION.  (AUTOMATIC) */
/*                       (INTERNAL) */
/*               CFEED = CONTAINS THE CHARACTER REPRESENTATION */
/*                       OF A PRINTER FORM FEED.  (CHARACTER) (INTERNAL) */
/*               IFEED = CONTAINS THE INTEGER VALUE OF A PRINTER */
/*                       FORM FEED.  (INTERNAL) */
/*              IORIGB = THE ORIGINAL NUMBER OF BITS NECESSARY */
/*                       FOR THE GROUP VALUES.  (INTERNAL) */
/*        1         2         3         4         5         6         7 X */

/*        NON SYSTEM SUBROUTINES CALLED */
/*           NONE */


    // <MASK>
}