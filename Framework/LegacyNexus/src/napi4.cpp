/*---------------------------------------------------------------------------
  NeXus - Neutron & X-ray Common Data Format

  Application Program Interface (HDF4) Routines

  Copyright (C) 1997-2006 Mark Koennecke, Przemek Klosowski

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  For further information, see <http://www.nexusformat.org>

  $Id$

----------------------------------------------------------------------------*/

#include "MantidLegacyNexus/napiconfig.h"

#ifdef WITH_HDF4

#include <assert.h>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// clang-format off
#include "MantidLegacyNexus/napi.h"
#include "MantidLegacyNexus/napi_internal.h"
#include "MantidLegacyNexus/napi4.h"
// clang-format on

extern void *NXpData;

typedef struct __LgcyNexusFile {
  struct iStack {
    int32 *iRefDir;
    int32 *iTagDir;
    int32 iVref;
    int32 __iStack_pad;
    int iNDir;
    int iCurDir;
  } iStack[NXMAXSTACK];
  struct iStack iAtt;
  int32 iVID;
  int32 iSID;
  int32 iCurrentVG;
  int32 iCurrentSDS;
  int iNXID;
  int iStackPtr;
  char iAccess[2];
} LgcyNexusFile, *pLgcyNexusFile;

/*-------------------------------------------------------------------*/

std::map<NXnumtype, int> const nxToHDF4Map{
    {NXnumtype::CHAR, DFNT_CHAR8},    {NXnumtype::INT8, DFNT_INT8},       {NXnumtype::UINT8, DFNT_UINT8},
    {NXnumtype::INT16, DFNT_INT16},   {NXnumtype::UINT16, DFNT_UINT16},   {NXnumtype::INT32, DFNT_INT32},
    {NXnumtype::UINT32, DFNT_UINT32}, {NXnumtype::FLOAT32, DFNT_FLOAT32}, {NXnumtype::FLOAT64, DFNT_FLOAT64}};

static int nxToHDF4Type(NXnumtype type) {
  auto const iter = nxToHDF4Map.find(type);
  if (iter != nxToHDF4Map.cend()) {
    return iter->second;
  } else {
    NXReportError("ERROR: nxToHDF4Type: unknown type");
    return -1;
  }
}

/*-------------------------------------------------------------------*/

static pLgcyNexusFile NXIassert(NXhandle fid) {
  pLgcyNexusFile pRes;

  assert(fid != NULL);
  pRes = static_cast<pLgcyNexusFile>(fid);
  assert(pRes->iNXID == NXSIGNATURE);
  return pRes;
}
/*----------------------------------------------------------------------*/
static int findNapiClass(pLgcyNexusFile pFile, int groupRef, NXname nxclass) {
  NXname classText;
  int32 tags[2], attID, linkID, groupID;

  groupID = Vattach(pFile->iVID, groupRef, "r");
  Vgetclass(groupID, classText);
  if (strcmp(classText, "NAPIlink") != 0) {
    /* normal group */
    strcpy(nxclass, classText);
    Vdetach(groupID);
  } else {
    /* code for linked renamed groups */
    attID = Vfindattr(groupID, "NAPIlink");
    if (attID >= 0) {
      Vgetattr(groupID, attID, tags);
      linkID = Vattach(pFile->iVID, tags[1], "r");
      NXname linkClass;
      Vgetclass(linkID, linkClass);
      Vdetach(groupID);
      Vdetach(linkID);
      strcpy(nxclass, linkClass);
      return tags[1];
    }
  }
  return groupRef;
}
/* --------------------------------------------------------------------- */

static int32 NXIFindVgroup(pLgcyNexusFile pFile, CONSTCHAR *name, CONSTCHAR *nxclass) {
  int const NX_EOD = static_cast<int>(NXstatus::NX_EOD);
  int32 iNew, iRef, iTag;
  int iN, i;
  int32 *pArray = NULL;
  NXname pText;

  assert(pFile != NULL);

  if (pFile->iCurrentVG == 0) { /* root level */
    /* get the number and ID's of all lone Vgroups in the file */
    iN = Vlone(pFile->iVID, NULL, 0);
    if (iN == 0) {
      return NX_EOD;
    }
    pArray = static_cast<int32 *>(malloc(static_cast<size_t>(iN) * sizeof(int32)));
    if (!pArray) {
      NXReportError("ERROR: out of memory in NXIFindVgroup");
      return NX_EOD;
    }
    Vlone(pFile->iVID, pArray, iN);

    /* loop and check */
    for (i = 0; i < iN; i++) {
      iNew = Vattach(pFile->iVID, pArray[i], "r");
      Vgetname(iNew, pText);
      Vdetach(iNew);
      if (strcmp(pText, name) == 0) {
        pArray[i] = findNapiClass(pFile, pArray[i], pText);
        if (strcmp(pText, nxclass) == 0) {
          /* found ! */
          iNew = pArray[i];
          free(pArray);
          return iNew;
        }
      }
    }
    /* nothing found */
    free(pArray);
    return NX_EOD;
  } else { /* case in Vgroup */
    iN = Vntagrefs(pFile->iCurrentVG);
    for (i = 0; i < iN; i++) {
      Vgettagref(pFile->iCurrentVG, i, &iTag, &iRef);
      if (iTag == DFTAG_VG) {
        iNew = Vattach(pFile->iVID, iRef, "r");
        Vgetname(iNew, pText);
        Vdetach(iNew);
        if (strcmp(pText, name) == 0) {
          iRef = findNapiClass(pFile, iRef, pText);
          if (strcmp(pText, nxclass) == 0) {
            return iRef;
          }
        }
      }
    } /* end for */
  } /* end else */
  /* not found */
  return NX_EOD;
}

/*----------------------------------------------------------------------*/

static int32 NXIFindSDS(NXhandle fid, CONSTCHAR *name) {
  int const NX_EOD = static_cast<int>(NXstatus::NX_EOD);
  pLgcyNexusFile self;
  int32 iNew, iRet, iTag, iRef;
  int32 i, iN, iA, iD1, iD2;
  NXname pNam;
  int32 iDim[H4_MAX_VAR_DIMS];

  self = NXIassert(fid);

  /* root level search */
  if (self->iCurrentVG == 0) {
    i = SDfileinfo(self->iSID, &iN, &iA);
    if (i < 0) {
      NXReportError("ERROR: failure to read file information");
      return NX_EOD;
    }
    for (i = 0; i < iN; i++) {
      iNew = SDselect(self->iSID, i);
      SDgetinfo(iNew, pNam, &iA, iDim, &iD1, &iD2);
      if (strcmp(pNam, name) == 0) {
        iRet = SDidtoref(iNew);
        SDendaccess(iNew);
        return iRet;
      } else {
        SDendaccess(iNew);
      }
    }
    /* not found */
    return NX_EOD;
  }
  /* end root level */
  else { /* search in a Vgroup */
    iN = Vntagrefs(self->iCurrentVG);
    for (i = 0; i < iN; i++) {
      Vgettagref(self->iCurrentVG, i, &iTag, &iRef);
      /* we are now writing using DFTAG_NDG, but need others for backward compatability */
      if ((iTag == DFTAG_SDG) || (iTag == DFTAG_NDG) || (iTag == DFTAG_SDS)) {
        iNew = SDreftoindex(self->iSID, iRef);
        iNew = SDselect(self->iSID, iNew);
        SDgetinfo(iNew, pNam, &iA, iDim, &iD1, &iD2);
        if (strcmp(pNam, name) == 0) {
          SDendaccess(iNew);
          return iRef;
        }
        SDendaccess(iNew);
      }
    } /* end for */
  } /* end Vgroup */
  /* we get here, only if nothing found */
  return NX_EOD;
}

/*----------------------------------------------------------------------*/

static NXstatus NXIInitDir(pLgcyNexusFile self) {
  int32 iTag, iRef;
  int iStackPtr;

  /*
   * Note: the +1 to various malloc() operations is to avoid a
   *  malloc(0), which is an error on some operating systems
   */
  iStackPtr = self->iStackPtr;
  if (self->iCurrentVG == 0 && self->iStack[iStackPtr].iRefDir == NULL) { /* root level */
    /* get the number and ID's of all lone Vgroups in the file */
    self->iStack[iStackPtr].iNDir = Vlone(self->iVID, NULL, 0);
    self->iStack[iStackPtr].iRefDir =
        static_cast<int32 *>(malloc(static_cast<size_t>(self->iStack[iStackPtr].iNDir) * sizeof(int32) + 1));
    if (!self->iStack[iStackPtr].iRefDir) {
      NXReportError("ERROR: out of memory in NXIInitDir");
      return NXstatus::NX_EOD;
    }
    Vlone(self->iVID, self->iStack[self->iStackPtr].iRefDir, self->iStack[self->iStackPtr].iNDir);
  } else {
    /* Vgroup level */
    self->iStack[iStackPtr].iNDir = Vntagrefs(self->iCurrentVG);
    self->iStack[iStackPtr].iRefDir =
        static_cast<int32 *>(malloc(static_cast<size_t>(self->iStack[iStackPtr].iNDir) * sizeof(int32) + 1));
    self->iStack[iStackPtr].iTagDir =
        static_cast<int32 *>(malloc(static_cast<size_t>(self->iStack[iStackPtr].iNDir) * sizeof(int32) + 1));
    if ((!self->iStack[iStackPtr].iRefDir) || (!self->iStack[iStackPtr].iTagDir)) {
      NXReportError("ERROR: out of memory in NXIInitDir");
      return NXstatus::NX_EOD;
    }
    for (int i = 0; i < self->iStack[self->iStackPtr].iNDir; i++) {
      Vgettagref(self->iCurrentVG, i, &iTag, &iRef);
      self->iStack[iStackPtr].iRefDir[i] = iRef;
      self->iStack[iStackPtr].iTagDir[i] = iTag;
    }
  }
  self->iStack[iStackPtr].iCurDir = 0;
  return NXstatus::NX_OK;
}

/*----------------------------------------------------------------------*/

static void NXIKillDir(pLgcyNexusFile self) {
  if (self->iStack[self->iStackPtr].iRefDir) {
    free(self->iStack[self->iStackPtr].iRefDir);
    self->iStack[self->iStackPtr].iRefDir = NULL;
  }
  if (self->iStack[self->iStackPtr].iTagDir) {
    free(self->iStack[self->iStackPtr].iTagDir);
    self->iStack[self->iStackPtr].iTagDir = NULL;
  }
  self->iStack[self->iStackPtr].iCurDir = 0;
  self->iStack[self->iStackPtr].iNDir = 0;
}

/*-------------------------------------------------------------------------*/

static NXstatus NXIInitAttDir(pLgcyNexusFile pFile) {
  int iRet;
  int32 iData, iAtt, iRank, iType;
  int32 iDim[H4_MAX_VAR_DIMS];

  pFile->iAtt.iCurDir = 0;
  if (pFile->iCurrentSDS != 0) { /* SDS level */
    NXname pNam;
    iRet = SDgetinfo(pFile->iCurrentSDS, pNam, &iRank, iDim, &iType, &iAtt);
  } else {
    if (pFile->iCurrentVG == 0) {
      /* global level */
      iRet = SDfileinfo(pFile->iSID, &iData, &iAtt);
    } else {
      /* group attribute */
      iRet = Vnattrs(pFile->iCurrentVG);
      iAtt = iRet;
    }
  }
  if (iRet < 0) {
    NXReportError("ERROR: HDF cannot read attribute numbers");
    pFile->iAtt.iNDir = 0;
    return NXstatus::NX_ERROR;
  }
  pFile->iAtt.iNDir = iAtt;
  return NXstatus::NX_OK;
}

/* --------------------------------------------------------------------- */

static void NXIKillAttDir(pLgcyNexusFile self) {
  if (self->iAtt.iRefDir) {
    free(self->iAtt.iRefDir);
    self->iAtt.iRefDir = NULL;
  }
  if (self->iAtt.iTagDir) {
    free(self->iAtt.iTagDir);
    self->iAtt.iTagDir = NULL;
  }
  self->iAtt.iCurDir = 0;
  self->iAtt.iNDir = 0;
}
/*------------------------------------------------------------------*/
static void NXIbuildPath(pLgcyNexusFile pFile, char *buffer, int bufLen) {
  int i;
  int32 groupID, iA, iD1, iD2, iDim[H4_MAX_VAR_DIMS];
  NXname pText;

  buffer[0] = '\0';
  for (i = 1; i <= pFile->iStackPtr; i++) {
    strncat(buffer, "/", bufLen - strlen(buffer));
    groupID = Vattach(pFile->iVID, pFile->iStack[i].iVref, "r");
    if (groupID != -1) {
      if (Vgetname(groupID, pText) != -1) {
        strncat(buffer, pText, bufLen - strlen(buffer));
      } else {
        NXReportError("ERROR: NXIbuildPath cannot get vgroup name");
      }
      Vdetach(groupID);
    } else {
      NXReportError("ERROR: NXIbuildPath cannot attach to vgroup");
    }
  }
  if (pFile->iCurrentSDS != 0) {
    if (SDgetinfo(pFile->iCurrentSDS, pText, &iA, iDim, &iD1, &iD2) != -1) {
      strncat(buffer, "/", bufLen - strlen(buffer));
      strncat(buffer, pText, bufLen - strlen(buffer));
    } else {
      NXReportError("ERROR: NXIbuildPath cannot read SDS");
    }
  }
}
/* ----------------------------------------------------------------------

                        Definition of NeXus API

 ---------------------------------------------------------------------*/

NXstatus NX4open(CONSTCHAR *filename, NXaccess am, NXhandle *pHandle) {
  pLgcyNexusFile pNew = NULL;
  char pBuffer[512];
  char *time_puffer = NULL;
  uint32 lmajor, lminor, lrelease;
  int32 am1 = 0;

  *pHandle = NULL;

  /* mask off any options for now */
  am = (NXaccess)(am & NXACCMASK_REMOVEFLAGS);
  /* map Nexus NXaccess types to HDF4 types */
  if (am == NXACC_CREATE) {
    am1 = DFACC_CREATE;
  } else if (am == NXACC_CREATE4) {
    am1 = DFACC_CREATE;
  } else if (am == NXACC_READ) {
    am1 = DFACC_READ;
  } else if (am == NXACC_RDWR) {
    am1 = DFACC_RDWR;
  }
  /* get memory */
  pNew = static_cast<pLgcyNexusFile>(malloc(sizeof(LgcyNexusFile)));
  if (!pNew) {
    NXReportError("ERROR: no memory to create File datastructure");
    return NXstatus::NX_ERROR;
  }
  memset(pNew, 0, sizeof(LgcyNexusFile));

  /* start SDS interface */
  pNew->iSID = SDstart(filename, am1);
  if (pNew->iSID <= 0) {
    sprintf(pBuffer, "ERROR: cannot open file_b: %s", filename);
    NXReportError(pBuffer);
    free(pNew);
    return NXstatus::NX_ERROR;
  }
  /*
   * need to create global attributes         file_name file_time NeXus_version
   * at some point for new files
   */
  if (am == NXACC_CREATE || am == NXACC_CREATE4) {
    /* set the NeXus_version attribute*/
    if (SDsetattr(pNew->iSID, "NeXus_version", DFNT_CHAR8, static_cast<int32>(strlen(NEXUS_VERSION)), NEXUS_VERSION) <
        0) {
      NXReportError("ERROR: HDF failed to store NeXus_version attribute ");
      free(pNew);
      return NXstatus::NX_ERROR;
    }

    char HDF_VERSION[64];
    /* set the HDF4 version attribute */
    Hgetlibversion(&lmajor, &lminor, &lrelease, HDF_VERSION);
    if (SDsetattr(pNew->iSID, "HDF_version", DFNT_CHAR8, static_cast<int32>(strlen(HDF_VERSION)), HDF_VERSION) < 0) {
      NXReportError("ERROR: HDF failed to store HDF_version attribute ");
      free(pNew);
      return NXstatus::NX_ERROR;
    }

    /* set the filename attribute */
    if (SDsetattr(pNew->iSID, "file_name", DFNT_CHAR8, static_cast<int32>(strlen(filename)),
                  static_cast<const char *>(filename)) < 0) {
      NXReportError("ERROR: HDF failed to store file_name attribute ");
      free(pNew);
      return NXstatus::NX_ERROR;
    }

    /* set the file_time attribute */
    time_puffer = NXIformatNeXusTime();
    if (time_puffer != NULL) {
      if (SDsetattr(pNew->iSID, "file_time", DFNT_CHAR8, static_cast<int32>(strlen(time_puffer)), time_puffer) < 0) {
        NXReportError("ERROR: HDF failed to store file_time attribute ");
        free(pNew);
        free(time_puffer);
        return NXstatus::NX_ERROR;
      }
      free(time_puffer);
    }

    if (SDsetattr(pNew->iSID, "NX_class", DFNT_CHAR8, 7, "NXroot") < 0) {
      NXReportError("ERROR: HDF failed to store NX_class attribute ");
      free(pNew);
      return NXstatus::NX_ERROR;
    }
  }

  /*
   * Otherwise we try to create the file two times which makes HDF
   * Throw up on us.
   */
  // cppcheck-suppress duplicateCondition
  if (am == NXACC_CREATE || am == NXACC_CREATE4) {
    am = NXACC_RDWR;
    am1 = DFACC_RDWR;
  }

  /* Set Vgroup access mode */
  if (am == NXACC_READ) {
    strcpy(pNew->iAccess, "r");
  } else {
    strcpy(pNew->iAccess, "w");
  }

  /* start Vgroup API */

  pNew->iVID = Hopen(filename, am1, 100);
  if (pNew->iVID <= 0) {
    sprintf(pBuffer, "ERROR: cannot open file_c: %s", filename);
    NXReportError(pBuffer);
    free(pNew);
    return NXstatus::NX_ERROR;
  }
  Vstart(pNew->iVID);
  pNew->iNXID = NXSIGNATURE;
  pNew->iStack[0].iVref = 0; /* root! */

  *pHandle = static_cast<NXhandle>(pNew);
  return NXstatus::NX_OK;
}

/*-----------------------------------------------------------------------*/

NXstatus NX4close(NXhandle *fid) {
  pLgcyNexusFile pFile = NULL;
  int iRet;

  pFile = NXIassert(*fid);
  iRet = 0;
  /* close links into vGroups or SDS */
  if (pFile->iCurrentVG != 0) {
    Vdetach(pFile->iCurrentVG);
  }
  if (pFile->iCurrentSDS != 0) {
    iRet = SDendaccess(pFile->iCurrentSDS);
  }
  if (iRet < 0) {
    NXReportError("ERROR: ending access to SDS");
  }
  /* close the SDS and Vgroup API's */
  Vend(pFile->iVID);
  iRet = SDend(pFile->iSID);
  if (iRet < 0) {
    NXReportError("ERROR: HDF cannot close SDS interface");
  }
  iRet = Hclose(pFile->iVID);
  if (iRet < 0) {
    NXReportError("ERROR: HDF cannot close HDF file");
  }
  /* release memory */
  NXIKillDir(pFile);
  free(pFile);
  *fid = NULL;
  return NXstatus::NX_OK;
}

/*------------------------------------------------------------------------*/
NXstatus NX4opengroup(NXhandle fid, CONSTCHAR *name, CONSTCHAR *nxclass) {
  pLgcyNexusFile pFile;
  int32 iRef;

  pFile = NXIassert(fid);

  iRef = NXIFindVgroup(pFile, static_cast<const char *>(name), nxclass);
  if (iRef < 0) {
    char pBuffer[256];
    sprintf(pBuffer, "ERROR: Vgroup \"%s\", class \"%s\" NOT found", name, nxclass);
    NXReportError(pBuffer);
    return NXstatus::NX_ERROR;
  }
  /* are we at root level ? */
  if (pFile->iCurrentVG == 0) {
    pFile->iCurrentVG = Vattach(pFile->iVID, iRef, pFile->iAccess);
    pFile->iStackPtr++;
    pFile->iStack[pFile->iStackPtr].iVref = iRef;
  } else {
    Vdetach(pFile->iCurrentVG);
    pFile->iStackPtr++;
    pFile->iStack[pFile->iStackPtr].iVref = iRef;
    pFile->iCurrentVG = Vattach(pFile->iVID, pFile->iStack[pFile->iStackPtr].iVref, pFile->iAccess);
  }
  NXIKillDir(pFile);
  return NXstatus::NX_OK;
}
/* ------------------------------------------------------------------- */

NXstatus NX4closegroup(NXhandle fid) {
  pLgcyNexusFile pFile;

  pFile = NXIassert(fid);

  /* first catch the trivial case: we are at root and cannot get
     deeper into a negative directory hierarchy (anti-directory)
   */
  if (pFile->iCurrentVG == 0) {
    NXIKillDir(pFile);
    return NXstatus::NX_OK;
  } else { /* Sighhh. Some work to do */
    /* close the current VG and decrement stack */
    Vdetach(pFile->iCurrentVG);
    NXIKillDir(pFile);
    pFile->iStackPtr--;
    if (pFile->iStackPtr <= 0) { /* we hit root */
      pFile->iStackPtr = 0;
      pFile->iCurrentVG = 0;
    } else {
      /* attach to the lower Vgroup */
      pFile->iCurrentVG = Vattach(pFile->iVID, pFile->iStack[pFile->iStackPtr].iVref, pFile->iAccess);
    }
  }
  return NXstatus::NX_OK;
}

NXstatus NX4opendata(NXhandle fid, CONSTCHAR *name) {
  pLgcyNexusFile pFile;
  int32 iNew, attID, tags[2];
  pFile = NXIassert(fid);

  /* First find the reference number of the SDS */
  iNew = NXIFindSDS(fid, name);
  if (iNew < 0) {
    char pBuffer[256];
    sprintf(pBuffer, "ERROR: SDS \"%s\" not found at this level", name);
    NXReportError(pBuffer);
    return NXstatus::NX_ERROR;
  }
  /* Be nice: properly close the old open SDS silently if there is
   * still an SDS open.
   */
  if (pFile->iCurrentSDS) {
    const int iRet = SDendaccess(pFile->iCurrentSDS);
    if (iRet < 0) {
      NXReportError("ERROR: HDF cannot end access to SDS");
    }
  }
  /* clear pending attribute directories first */
  NXIKillAttDir(pFile);

  /* open the SDS, thereby watching for linked SDS under a different name */
  iNew = SDreftoindex(pFile->iSID, iNew);
  pFile->iCurrentSDS = SDselect(pFile->iSID, iNew);
  attID = SDfindattr(pFile->iCurrentSDS, "NAPIlink");
  if (attID >= 0) {
    SDreadattr(pFile->iCurrentSDS, attID, tags);
    SDendaccess(pFile->iCurrentSDS);
    iNew = SDreftoindex(pFile->iSID, tags[1]);
    pFile->iCurrentSDS = SDselect(pFile->iSID, iNew);
  }

  if (pFile->iCurrentSDS < 0) {
    NXReportError("ERROR: HDF error opening SDS");
    pFile->iCurrentSDS = 0;
    return NXstatus::NX_ERROR;
  }
  return NXstatus::NX_OK;
}

/* ----------------------------------------------------------------- */

NXstatus NX4closedata(NXhandle fid) {
  pLgcyNexusFile pFile;
  pFile = NXIassert(fid);

  if (pFile->iCurrentSDS != 0) {
    int iRet = SDendaccess(pFile->iCurrentSDS);
    pFile->iCurrentSDS = 0;
    if (iRet < 0) {
      NXReportError("ERROR: HDF cannot end access to SDS");
      return NXstatus::NX_ERROR;
    }
  } else {
    NXReportError("ERROR: no SDS open --> nothing to do");
    return NXstatus::NX_ERROR;
  }
  NXIKillAttDir(pFile); /* for attribute data */
  return NXstatus::NX_OK;
}

/* ------------------------------------------------------------------- */

NXstatus NX4getdataID(NXhandle fid, NXlink *sRes) {
  pLgcyNexusFile pFile;
  int datalen;
  NXnumtype type = NXnumtype::CHAR;

  pFile = NXIassert(fid);

  if (pFile->iCurrentSDS == 0) {
    sRes->iTag = static_cast<int>(NXstatus::NX_ERROR);
    return NXstatus::NX_ERROR;
  } else {
    sRes->iTag = DFTAG_NDG;
    sRes->iRef = SDidtoref(pFile->iCurrentSDS);
    NXMDisableErrorReporting();
    datalen = 1024;
    memset(&sRes->targetPath, 0, 1024);
    if (NX4getattr(fid, "target", &sRes->targetPath, &datalen, &type) != NXstatus::NX_OK) {
      NXIbuildPath(pFile, sRes->targetPath, 1024);
    }
    NXMEnableErrorReporting();
    return NXstatus::NX_OK;
  }
  sRes->iTag = static_cast<int>(NXstatus::NX_ERROR);
  return NXstatus::NX_ERROR; /* not reached */
}

/*-------------------------------------------------------------------------*/

NXstatus NX4getnextentry(NXhandle fid, NXname name, NXname nxclass, NXnumtype *datatype) {
  pLgcyNexusFile pFile;
  int iStackPtr, iCurDir;
  int32 iTemp, iD1, iD2, iA;
  int32 iDim[H4_MAX_VAR_DIMS];

  pFile = NXIassert(fid);

  iStackPtr = pFile->iStackPtr;
  iCurDir = pFile->iStack[pFile->iStackPtr].iCurDir;

  /* first case to check for: no directory entry */
  if (pFile->iStack[pFile->iStackPtr].iRefDir == NULL) {
    if (NXIInitDir(pFile) == NXstatus::NX_EOD) {
      NXReportError("ERROR: no memory to store directory info");
      return NXstatus::NX_EOD;
    }
  }

  /* Next case: end of directory */
  if (iCurDir >= pFile->iStack[pFile->iStackPtr].iNDir) {
    NXIKillDir(pFile);
    return NXstatus::NX_EOD;
  }

  /* Next case: we have data! supply it and increment counter */
  if (pFile->iCurrentVG == 0) { /* root level */
    iTemp = Vattach(pFile->iVID, pFile->iStack[iStackPtr].iRefDir[iCurDir], "r");
    if (iTemp < 0) {
      NXReportError("ERROR: HDF cannot attach to Vgroup");
      return NXstatus::NX_ERROR;
    }
    Vgetname(iTemp, name);
    Vdetach(iTemp);
    findNapiClass(pFile, pFile->iStack[pFile->iStackPtr].iRefDir[iCurDir], nxclass);
    *datatype = static_cast<NXnumtype>(DFTAG_VG);
    pFile->iStack[pFile->iStackPtr].iCurDir++;
    return NXstatus::NX_OK;
  } else {                                                       /* in Vgroup */
    if (pFile->iStack[iStackPtr].iTagDir[iCurDir] == DFTAG_VG) { /* Vgroup */
      iTemp = Vattach(pFile->iVID, pFile->iStack[iStackPtr].iRefDir[iCurDir], "r");
      if (iTemp < 0) {
        NXReportError("ERROR: HDF cannot attach to Vgroup");
        return NXstatus::NX_ERROR;
      }
      Vgetname(iTemp, name);
      Vdetach(iTemp);
      findNapiClass(pFile, pFile->iStack[pFile->iStackPtr].iRefDir[iCurDir], nxclass);
      *datatype = static_cast<NXnumtype>(DFTAG_VG);
      pFile->iStack[pFile->iStackPtr].iCurDir++;
      Vdetach(iTemp);
      return NXstatus::NX_OK;
      /* we are now writing using DFTAG_NDG, but need others for backward compatability */
    } else if ((pFile->iStack[iStackPtr].iTagDir[iCurDir] == DFTAG_SDG) ||
               (pFile->iStack[iStackPtr].iTagDir[iCurDir] == DFTAG_NDG) ||
               (pFile->iStack[iStackPtr].iTagDir[iCurDir] == DFTAG_SDS)) {
      iTemp = SDreftoindex(pFile->iSID, pFile->iStack[iStackPtr].iRefDir[iCurDir]);
      iTemp = SDselect(pFile->iSID, iTemp);
      SDgetinfo(iTemp, name, &iA, iDim, &iD1, &iD2);
      strcpy(nxclass, "SDS");
      *datatype = static_cast<NXnumtype>(iD1);
      SDendaccess(iTemp);
      pFile->iStack[pFile->iStackPtr].iCurDir++;
      return NXstatus::NX_OK;
    } else { /* unidentified */
      strcpy(name, "UNKNOWN");
      strcpy(nxclass, "UNKNOWN");
      *datatype = static_cast<NXnumtype>(pFile->iStack[iStackPtr].iTagDir[iCurDir]);
      pFile->iStack[pFile->iStackPtr].iCurDir++;
      return NXstatus::NX_OK;
    }
  }
  return NXstatus::NX_ERROR; /* not reached */
}

/*-------------------------------------------------------------------------*/

NXstatus NX4getdata(NXhandle fid, void *data) {
  pLgcyNexusFile pFile;
  int32 iStart[H4_MAX_VAR_DIMS], iSize[H4_MAX_VAR_DIMS];
  NXname pBuffer;
  int32 iRank, iAtt, iType;

  pFile = NXIassert(fid);

  /* check if there is an SDS open */
  if (pFile->iCurrentSDS == 0) {
    NXReportError("ERROR: no SDS open");
    return NXstatus::NX_ERROR;
  }
  /* first read dimension information */
  memset(iStart, 0, H4_MAX_VAR_DIMS * sizeof(int32));
  SDgetinfo(pFile->iCurrentSDS, pBuffer, &iRank, iSize, &iType, &iAtt);
  /* actually read */
  SDreaddata(pFile->iCurrentSDS, iStart, NULL, iSize, data);
  return NXstatus::NX_OK;
}

/*-------------------------------------------------------------------------*/

NXstatus NX4getinfo64(NXhandle fid, int *rank, int64_t dimension[], NXnumtype *iType) {
  pLgcyNexusFile pFile;
  NXname pBuffer;
  int32 iAtt, myDim[H4_MAX_VAR_DIMS], i, iRank, mType;

  pFile = NXIassert(fid);

  /* check if there is an SDS open */
  if (pFile->iCurrentSDS == 0) {
    NXReportError("ERROR: no SDS open");
    return NXstatus::NX_ERROR;
  }
  /* read information */
  SDgetinfo(pFile->iCurrentSDS, pBuffer, &iRank, myDim, &mType, &iAtt);

  /* conversion to proper ints for the platform */
  *iType = static_cast<NXnumtype>(mType);
  *rank = (int)iRank;
  for (i = 0; i < iRank; i++) {
    dimension[i] = (int)myDim[i];
  }
  return NXstatus::NX_OK;
}

/*-------------------------------------------------------------------------*/

NXstatus NX4getnextattr(NXhandle fileid, NXname pName, int *iLength, NXnumtype *iType) {
  pLgcyNexusFile pFile;
  NXstatus iRet;
  int32 iPType, iCount, count;

  pFile = NXIassert(fileid);

  /* first check if we have to start a new attribute search */
  if (pFile->iAtt.iNDir == 0) {
    iRet = NXIInitAttDir(pFile);
    if (iRet == NXstatus::NX_ERROR) {
      return NXstatus::NX_ERROR;
    }
  }
  /* are we done ? */
  if (pFile->iAtt.iCurDir >= pFile->iAtt.iNDir) {
    NXIKillAttDir(pFile);
    return NXstatus::NX_EOD;
  }
  /* well, there must be data to copy */
  if (pFile->iCurrentSDS == 0) {
    if (pFile->iCurrentVG == 0) {
      /* global attribute */
      iRet = static_cast<NXstatus>(SDattrinfo(pFile->iSID, pFile->iAtt.iCurDir, pName, &iPType, &iCount));
    } else {
      /* group attribute */
      iRet = static_cast<NXstatus>(Vattrinfo(pFile->iCurrentVG, pFile->iAtt.iCurDir, pName, &iPType, &iCount, &count));
    }
  } else {
    iRet = static_cast<NXstatus>(SDattrinfo(pFile->iCurrentSDS, pFile->iAtt.iCurDir, pName, &iPType, &iCount));
  }
  if (iRet == NXstatus::NX_EOD) {
    NXReportError("ERROR: HDF cannot read attribute info");
    return NXstatus::NX_ERROR;
  }
  *iLength = iCount;
  *iType = static_cast<NXnumtype>(iPType);
  pFile->iAtt.iCurDir++;
  return NXstatus::NX_OK;
}

/*-------------------------------------------------------------------------*/

NXstatus NX4getattr(NXhandle fid, const char *name, void *data, int *datalen, NXnumtype *iType) {
  pLgcyNexusFile pFile;
  int32 iNew, iType32, count;
  void *pData = NULL;
  int32 iLen, iRet;
  int type;
  char pBuffer[256];
  NXname pNam;

  type = nxToHDF4Type(*iType);
  *datalen = (*datalen) * DFKNTsize(type);
  pFile = NXIassert(fid);

  /* find attribute */
  if (pFile->iCurrentSDS != 0) {
    /* SDS attribute */
    iNew = SDfindattr(pFile->iCurrentSDS, name);
  } else {
    if (pFile->iCurrentVG == 0) {
      /* global attribute */
      iNew = SDfindattr(pFile->iSID, name);
    } else {
      /* group attribute */
      iNew = Vfindattr(pFile->iCurrentVG, name);
    }
  }
  if (iNew < 0) {
    sprintf(pBuffer, "ERROR: attribute \"%s\" not found", name);
    NXReportError(pBuffer);
    return NXstatus::NX_ERROR;
  }
  /* get more info, allocate temporary data space */
  iType32 = (int32)type;
  if (pFile->iCurrentSDS != 0) {
    iRet = SDattrinfo(pFile->iCurrentSDS, iNew, pNam, &iType32, &iLen);
  } else {
    if (pFile->iCurrentVG == 0) {
      iRet = SDattrinfo(pFile->iSID, iNew, pNam, &iType32, &iLen);
    } else {
      iRet = Vattrinfo(pFile->iCurrentVG, iNew, pNam, &iType32, &count, &iLen);
    }
  }
  if (iRet < 0) {
    sprintf(pBuffer, "ERROR: HDF could not read attribute info");
    NXReportError(pBuffer);
    return NXstatus::NX_ERROR;
  }
  *iType = static_cast<NXnumtype>(iType32);
  iLen = iLen * DFKNTsize(type);
  if (*iType == NXnumtype::CHAR) {
    iLen += 1;
  }
  pData = malloc(iLen);
  if (!pData) {
    NXReportError("ERROR: allocating memory in NXgetattr");
    return NXstatus::NX_ERROR;
  }
  memset(pData, 0, iLen);

  /* finally read the data */
  if (pFile->iCurrentSDS != 0) {
    iRet = SDreadattr(pFile->iCurrentSDS, iNew, pData);
  } else {
    if (pFile->iCurrentVG == 0) {
      iRet = SDreadattr(pFile->iSID, iNew, pData);
    } else {
      iRet = Vgetattr(pFile->iCurrentVG, iNew, pData);
    }
  }
  if (iRet < 0) {
    sprintf(pBuffer, "ERROR: HDF could not read attribute data");
    NXReportError(pBuffer);
    return NXstatus::NX_ERROR;
  }
  /* copy data to caller */
  memset(data, 0, *datalen);
  if ((*datalen <= iLen) && (type == DFNT_UINT8 || type == DFNT_CHAR8 || type == DFNT_UCHAR8)) {
    iLen = *datalen - 1; /* this enforces NULL termination regardless of size of datalen */
  }
  memcpy(data, pData, iLen);
  *datalen = iLen / DFKNTsize(type);
  free(pData);
  return NXstatus::NX_OK;
}

/*-------------------------------------------------------------------------*/

NXstatus NX4getgroupID(NXhandle fileid, NXlink *sRes) {
  pLgcyNexusFile pFile;

  pFile = NXIassert(fileid);

  if (pFile->iCurrentVG == 0) {
    sRes->iTag = static_cast<int>(NXstatus::NX_ERROR);
    return NXstatus::NX_ERROR;
  } else {
    sRes->iTag = DFTAG_VG;
    sRes->iRef = VQueryref(pFile->iCurrentVG);
    int datalen = 1024;
    NXnumtype type = NXnumtype::CHAR;
    if (NX4getattr(pFile, "target", &sRes->targetPath, &datalen, &type) != NXstatus::NX_OK) {
      NXIbuildPath(pFile, sRes->targetPath, 1024);
    }
    return NXstatus::NX_OK;
  }
  /* not reached */
  sRes->iTag = static_cast<int>(NXstatus::NX_ERROR);
  return NXstatus::NX_ERROR;
}

/* ------------------------------------------------------------------- */

NXstatus NX4sameID(NXhandle fileid, NXlink const *pFirstID, NXlink const *pSecondID) {
  NXIassert(fileid);
  if ((pFirstID->iTag == pSecondID->iTag) & (pFirstID->iRef == pSecondID->iRef)) {
    return NXstatus::NX_OK;
  } else {
    return NXstatus::NX_ERROR;
  }
}

/*-------------------------------------------------------------------------*/

NXstatus NX4initattrdir(NXhandle fid) {
  pLgcyNexusFile pFile;
  NXstatus iRet;

  pFile = NXIassert(fid);
  NXIKillAttDir(pFile);
  iRet = NXIInitAttDir(pFile);
  if (iRet == NXstatus::NX_ERROR)
    return NXstatus::NX_ERROR;
  return NXstatus::NX_OK;
}

/*-------------------------------------------------------------------------*/

NXstatus NX4initgroupdir(NXhandle fid) {
  pLgcyNexusFile pFile;
  NXstatus iRet;

  pFile = NXIassert(fid);
  NXIKillDir(pFile);
  iRet = NXIInitDir(pFile);
  if (iRet == NXstatus::NX_EOD) {
    NXReportError("NX_ERROR: no memory to store directory info");
    return NXstatus::NX_EOD;
  }
  return NXstatus::NX_OK;
}

/*--------------------------------------------------------------------*/
NXstatus NX4getnextattra(NXhandle handle, NXname pName, int *rank, int dim[], NXnumtype *iType) {
  NXstatus ret = NX4getnextattr(handle, pName, dim, iType);
  if (ret != NXstatus::NX_OK)
    return ret;
  (*rank) = 1;
  if (dim[0] <= 1)
    (*rank) = 0;
  return NXstatus::NX_OK;
}

/*--------------------------------------------------------------------*/
void NX4assignFunctions(LgcyFunction &fHandle) {
  fHandle.nxclose = NX4close;
  fHandle.nxopengroup = NX4opengroup;
  fHandle.nxclosegroup = NX4closegroup;
  fHandle.nxopendata = NX4opendata;
  fHandle.nxclosedata = NX4closedata;
  fHandle.nxgetdataID = NX4getdataID;
  fHandle.nxgetdata = NX4getdata;
  fHandle.nxgetinfo64 = NX4getinfo64;
  fHandle.nxgetnextentry = NX4getnextentry;
  fHandle.nxgetattr = NX4getattr;
  fHandle.nxgetgroupID = NX4getgroupID;
  fHandle.nxinitgroupdir = NX4initgroupdir;
  fHandle.nxinitattrdir = NX4initattrdir;
  fHandle.nxgetnextattra = NX4getnextattra;
}

#endif /*HDF4*/
