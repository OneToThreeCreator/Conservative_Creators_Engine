#include "../utils.h"

#ifndef ACTIONS_RUNACTIONS_H
/* Generation of files like that should really be automated 
 * Also this file will benefit heavily with introduction of newer C standard */
#define ACTIONS_RUNACTIONS_H

#define CCEA__RUNACTIONS_SETVAR1(varname, runActionsType, type1, structDef1) \
*((type1*) ((varname) + sizeof(runActionsType))) = structDef1

#define CCEA__RUNACTIONS_SETVAR2(varname, runActionsType, type1, structDef1, type2, structDef2) \
CCEA__RUNACTIONS_SETVAR1(varname, runActionsType, type1, structDef1); \
*((type2*) ((varname) + sizeof(runActionsType) + sizeof(type1))) = structDef2

#define CCEA__RUNACTIONS_SETVAR3(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3) \
CCEA__RUNACTIONS_SETVAR2(varname, runActionsType, type1, structDef1, type2, structDef2); \
*((type3*) ((varname) + sizeof(runActionsType) + sizeof(type1) + sizeof(type2))) = structDef3

#define CCEA__RUNACTIONS_SETVAR4(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4) \
CCEA__RUNACTIONS_SETVAR3(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3); \
*((type4*) ((varname) + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3))) = structDef4

#define CCEA__RUNACTIONS_SETVAR5(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5) \
CCEA__RUNACTIONS_SETVAR4(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4); \
*((type5*) ((varname) + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4))) = structDef5

#define CCEA__RUNACTIONS_SETVAR6(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6) \
CCEA__RUNACTIONS_SETVAR5(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5); \
*((type6*) ((varname) + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5))) = structDef6

#define CCEA__RUNACTIONS_SETVAR7(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7) \
CCEA__RUNACTIONS_SETVAR6(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6); \
*((type7*) ((varname) + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6))) = structDef7

#define CCEA__RUNACTIONS_SETVAR8(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7, type8, structDef8) \
CCEA__RUNACTIONS_SETVAR7(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7); \
*((type8*) ((varname) + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7))) = structDef8



#define CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, structDef1, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR1(varname, runActionsType, type1, structDef1)

#define CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR2(varname, runActionsType, type1, structDef1, type2, structDef2)

#define CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR3(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3)

#define CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR4(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4)

#define CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR5(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5)

#define CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR6(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6)

#define CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR7(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7)

#define CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7, type8, structDef8, additionalSize) \
*((runActionsType*) (varname)) = runActionsDef; \
 ((runActionsType*) (varname))->totalSize = sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + (additionalSize); \
CCEA__RUNACTIONS_SETVAR8(varname, runActionsType, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7, type8, structDef8)



#define CCEA_RUNACTIONS_CREATE_STATIC1(varname, runActionsType, runActionsDef, type1, structDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, structDef1, 0)

#define CCEA_RUNACTIONS_CREATE_STATIC2(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, 0)

#define CCEA_RUNACTIONS_CREATE_STATIC3(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, 0)

#define CCEA_RUNACTIONS_CREATE_STATIC4(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, 0)

#define CCEA_RUNACTIONS_CREATE_STATIC5(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, 0)

#define CCEA_RUNACTIONS_CREATE_STATIC6(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6) \
cce_void varname[sizeof(sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, 0)

#define CCEA_RUNACTIONS_CREATE_STATIC7(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7) \
cce_void varname[sizeof(sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7, 0)

#define CCEA_RUNACTIONS_CREATE_STATIC8(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7, type8, structDef8) \
cce_void varname[sizeof(sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, structDef1, type2, structDef2, type3, structDef3, type4, structDef4, type5, structDef5, type6, structDef6, type7, structDef7, type8, structDef8, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED1_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + sizeof(nestedType1)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, sizeof(nestedRunActionsType) + sizeof(nestedType1)); \
CCEA__RUNACTIONS_SET1(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED2_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2)); \
CCEA__RUNACTIONS_SET2(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED3_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3)); \
CCEA__RUNACTIONS_SET3(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED4_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4)); \
CCEA__RUNACTIONS_SET4(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED5_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5)); \
CCEA__RUNACTIONS_SET5(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED6_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6)); \
CCEA__RUNACTIONS_SET6(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED7_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7)); \
CCEA__RUNACTIONS_SET7(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, 0)



#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC1(varname, runActionsType, runActionsDef, type1, def1, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET1(varname, runActionsType, runActionsDef, type1, def1, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET2(varname, runActionsType, runActionsDef, type1, def1, type2, def2, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET3(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET4(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET5(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET6(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET7(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

#define CCEA_RUNACTIONS_CREATE_NESTED8_STATIC8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                                              nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, \
                                              nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8) \
cce_void varname[sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8) + sizeof(nestedRunActionsType) + \
                 sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)]; \
CCEA__RUNACTIONS_SET8(varname, runActionsType, runActionsDef, type1, def1, type2, def2, type3, def3, type4, def4, type5, def5, type6, def6, type7, def7, type8, def8, \
                     sizeof(nestedRunActionsType) + sizeof(nestedType1) + sizeof(nestedType2) + sizeof(nestedType3) + sizeof(nestedType4) + sizeof(nestedType5) + sizeof(nestedType6) + sizeof(nestedType7) + sizeof(nestedType8)); \
CCEA__RUNACTIONS_SET8(varname + sizeof(runActionsType) + sizeof(type1) + sizeof(type2) + sizeof(type3) + sizeof(type4) + sizeof(type5) + sizeof(type6) + sizeof(type7) + sizeof(type8), \
                     nestedRunActionsType, nestedRunActionsDef, nestedType1, nestedDef1, nestedType2, nestedDef2, nestedType3, nestedDef3, nestedType4, nestedDef4, nestedType5, nestedDef5, nestedType6, nestedDef6, nestedType7, nestedDef7, nestedType8, nestedDef8, 0)

// Expects POINTERS to actions, not actions themselves
#define CCEA_RUNACTIONS_CREATE_DYNAMIC(runActionID, runActionSize, ...) \
        cce__runActionsCreateDynamic(runActionID, runActionSize, CCEA_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

#endif // ACTIONS_RUNACTIONS_H
