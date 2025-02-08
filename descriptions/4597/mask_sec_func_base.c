gtemplate *extpdstemplate(g2int number,g2int *list)
///$$$  SUBPROGRAM DOCUMENTATION BLOCK
//                .      .    .                                       .
// SUBPROGRAM:    extpdstemplate
//   PRGMMR: Gilbert         ORG: W/NP11    DATE: 2000-05-11
//
// ABSTRACT: This subroutine generates the remaining octet map for a
//   given Product Definition Template, if required.  Some Templates can
//   vary depending on data values given in an earlier part of the
//   Template, and it is necessary to know some of the earlier entry
//   values to generate the full octet map of the Template.
//
// PROGRAM HISTORY LOG:
// 2000-05-11  Gilbert
// 2009-01-14  Vuong     Changed structure name template to gtemplate
// 2009-08-05  Vuong     Added Product Definition Template 4.31
// 2010-08-03  Vuong     Added Product Definition Template 4.42 and 4.43
// 2010-12-08  Vuong     Corrected Product Definition Template 4.42 and 4.43
// 2012-02-15  Vuong     Added Templates 4.44,4.45,4.46,4.47,4.48,4.50,
//                       4.51,4.91,4.32 and 4.52
// 2013-08-05  Vuong     Corrected 4.91 and added Templates 4.33,4.34,4.53,4.54
// 2015-10-07  Vuong     Added Templates 4.57, 4.60, 4.61 and
//                       allow a forecast time to be negative
//
// USAGE:    CALL extpdstemplate(number,list)
//   INPUT ARGUMENT LIST:
//     number   - NN, indicating the number of the Product Definition
//                Template 4.NN that is being requested.
//     list()   - The list of values for each entry in the
//                the Product Definition Template 4.NN.
//
//   RETURN VALUE:
//        - Pointer to the returned template struct.
//          Returns NULL pointer, if template not found.
//
// ATTRIBUTES:
//   LANGUAGE: C
//   MACHINE:  IBM SP
//
//$$$
{
           printf("This is a test for CodeGuard+\n");
           // <MASK>
