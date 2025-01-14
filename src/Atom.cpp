/*******************************************************************************************************************************
Copyright (c) Xiaoqiang Huang

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************************************/
#include "Atom.h"
#include <string.h>

extern BOOL FLAG_WRITE_HYDROGEN;

int AtomCreate(Atom* pThis)
{
  strcpy(pThis->name, "");
  pThis->xyz.X = -1e3;
  pThis->xyz.Y = -1e3;
  pThis->xyz.Z = -1e3;
  pThis->isXyzValid = FALSE;
  strcpy(pThis->chainName, "");
  pThis->posInChain = -1;
  pThis->isInHBond = FALSE;
  pThis->isBBAtom = FALSE;
  pThis->bfactor = 1000.0;

  return Success;
}


int AtomDestroy(Atom* pThis)
{
  strcpy(pThis->name, "");
  pThis->xyz.X = -1e3;
  pThis->xyz.Y = -1e3;
  pThis->xyz.Z = -1e3;
  pThis->isXyzValid = FALSE;
  return Success;
}


int AtomCopy(Atom* pThis, Atom* pOther)
{
  *pThis = *pOther;
  return Success;
}


char* AtomGetName(Atom* pThis)
{
  return pThis->name;
}


int AtomSetName(Atom* pThis, char* newName)
{
  if (newName == NULL || strlen(newName) > MAX_LEN_ATOM_NAME)
  {
    char errMsg[MAX_LEN_ERR_MSG + 1];
    int result = ValueError;
    sprintf(errMsg, "in file %s line %d, atom name is not specified or is too long", __FILE__, __LINE__);
    TraceError(errMsg, result);
    return result;
  }
  strcpy(pThis->name, newName);

  return Success;
}


char* AtomGetType(Atom* pThis)
{
  return pThis->type;
}


Type_AtomHybridType AtomGetHybridType(Atom* pThis)
{
  return pThis->hybridType;
}


char* AtomGetHbHorA(Atom* pThis)
{
  return pThis->hbHorA;
}


char* AtomGetHbDorB(Atom* pThis)
{
  return pThis->hbDorB;
}


char* AtomGetHbB2(Atom* pThis)
{
  return pThis->hbB2;
}


BOOL AtomIsHydrogen(Atom* pThis)
{
  if (pThis->name[0] == 'H') return TRUE;
  else return FALSE;
}


int AtomSetParamsByStringArray(Atom* pThis, StringArray* pParams)
{
  if (StringArrayGetCount(pParams) != 15)
  {
    return ValueError;
  }

  char* atomName = StringArrayGet(pParams, 1);
  char* atomType = StringArrayGet(pParams, 2);
  char* isBB = StringArrayGet(pParams, 3);
  char* polar = StringArrayGet(pParams, 4);
  double  epsilon = atof(StringArrayGet(pParams, 5));
  double  rmin = atof(StringArrayGet(pParams, 6));
  double  charge = atof(StringArrayGet(pParams, 7));
  char* hbHorA = StringArrayGet(pParams, 8);
  char* hbDorB = StringArrayGet(pParams, 9);
  char* hbB2 = StringArrayGet(pParams, 10);
  char* hybrid = StringArrayGet(pParams, 11);
  double  EEF1DGfree = atof(StringArrayGet(pParams, 12));
  double  EEF1volume = atof(StringArrayGet(pParams, 13));
  double  EEF1lambda = atof(StringArrayGet(pParams, 14));

  if (strlen(atomName) > MAX_LEN_ATOM_NAME ||
    strlen(atomType) > MAX_LEN_ATOM_TYPE ||
    strlen(hbHorA) > MAX_LEN_ATOM_DONOR ||
    strlen(hbDorB) > MAX_LEN_ATOM_ACCEPTOR)
  {
    return ValueError;
  }

  pThis->vdw_epsilon = epsilon;
  pThis->vdw_radius = rmin;
  pThis->charge = charge;
  pThis->EEF1_freeDG = EEF1DGfree;
  pThis->EEF1_volume = EEF1volume;
  pThis->EEF1_lamda_ = EEF1lambda;

  AtomSetName(pThis, atomName);
  strcpy(pThis->type, atomType);

  if (strcmp(hbHorA, "H") == 0)
  {
    pThis->isHBatomH = TRUE;
    pThis->isHBatomA = FALSE;
  }
  else if (strcmp(hbHorA, "A") == 0)
  {
    pThis->isHBatomH = FALSE;
    pThis->isHBatomA = TRUE;
  }
  else if (strcmp(hbHorA, "D") == 0)
  { // this atom can serve as both HB donor and acceptor
    pThis->isHBatomH = TRUE;
    pThis->isHBatomA = TRUE;
  }
  else
  {
    pThis->isHBatomH = FALSE;
    pThis->isHBatomA = FALSE;
  }

  strcpy(pThis->hbHorA, hbHorA);
  strcpy(pThis->hbDorB, hbDorB);
  strcpy(pThis->hbB2, hbB2);

  if (strcmp(hybrid, "SP2") == 0)
  {
    pThis->hybridType = Type_AtomHybridType_SP2;
  }
  else if (strcmp(hybrid, "SP3") == 0)
  {
    pThis->hybridType = Type_AtomHybridType_SP3;
  }
  else if (strcmp(hybrid, "SP") == 0)
  {
    pThis->hybridType = Type_AtomHybridType_SP;
  }
  else
  {
    pThis->hybridType = Type_AtomHybridType_None;
  }

  switch (isBB[0])
  {
  case 'Y':
    pThis->isBBAtom = TRUE;
    break;
  case 'N':
    pThis->isBBAtom = FALSE;
    break;
  default:
    return ValueError;
  }

  if (strcmp(polar, "P") == 0)
  {
    pThis->polarity = Type_AtomPolarity_P;
  }
  else if (strcmp(polar, "C") == 0)
  {
    pThis->polarity = Type_AtomPolarity_C;
  }
  else if (strcmp(polar, "NP1") == 0)
  {
    pThis->polarity = Type_AtomPolarity_NPAliphatic;
  }
  else if (strcmp(polar, "NP2") == 0)
  {
    pThis->polarity = Type_AtomPolarity_NPAromatic;
  }
  //for debug
  //AtomShowParams(pAtom);

  return Success;
}


int AtomShowParams(Atom* pThis)
{
  char polarity[32];
  switch (pThis->polarity)
  {
  case Type_AtomPolarity_P:
    strcpy(polarity, "P  ");
    break;
  case Type_AtomPolarity_C:
    strcpy(polarity, "C  ");
    break;
  case Type_AtomPolarity_NPAliphatic:
    strcpy(polarity, "NP1");
    break;
  case Type_AtomPolarity_NPAromatic:
    strcpy(polarity, "NP2");
    break;
  default:
    strcpy(polarity, "polarity type Unknown");
    break;
  }

  printf("%5s %5s %c %3s %8.4f %8.4f %8.4f %5s %5s %5s %d\n",
    pThis->name, pThis->type,
    pThis->isBBAtom ? 'Y' : 'N',
    polarity, pThis->vdw_epsilon, pThis->vdw_radius, pThis->charge,
    pThis->hbHorA, pThis->hbDorB, pThis->hbB2, pThis->hybridType);
  return Success;
}


char* AtomGetChainName(Atom* pThis)
{
  return pThis->chainName;
}


int AtomSetChainName(Atom* pThis, char* newChainName)
{
  if (strlen(newChainName) > MAX_LEN_CHAIN_NAME)
  {
    return ValueError;
  }
  else
  {
    strcpy(pThis->chainName, newChainName);
    return Success;
  }
}


int AtomGetPosInChain(Atom* pThis)
{
  return pThis->posInChain;
}


int AtomSetPosInChain(Atom* pThis, int pos)
{
  pThis->posInChain = pos;
  return Success;
}


int AtomShowInPDBFormat(Atom* pAtom, char* header, char* resiName, char* chainName, int atomIndex, int resiIndex, FILE* pFile)
{
  // type, serial, name, altLoc, resName, chainID, resSeq, iCode, X, Y, Z
  // 0,  6,    12, 16,   17,    21,    22,   26,  30, 38, 46
  // 6,  5,    4,  1,    4,     1,     4,    1,   8, 8, 8
  if (FLAG_WRITE_HYDROGEN == FALSE && AtomIsHydrogen(pAtom))
  {
    return Success;
  }
  if (!pAtom->isXyzValid)
  {
    return Warning;
  }
  if (!pFile)
  {
    pFile = stdout;
  }
  char atomName[MAX_LEN_ATOM_NAME + 1];
  strcpy(atomName, AtomGetName(pAtom));
  if (strlen(atomName) >= 4)
  { //it should be a hydrogen atom
    char tempName[MAX_LEN_ATOM_NAME + 1];
    tempName[0] = atomName[3];
    tempName[1] = atomName[0];
    tempName[2] = atomName[1];
    tempName[3] = atomName[2];
    tempName[4] = '\0';
    strcpy(atomName, tempName);
    fprintf(pFile, "%-6.6s%5d %-4.4s %-3.3s %1.1s%4d    %8.3f%8.3f%8.3f  %d                    %c\n",
      header, atomIndex, atomName, resiName, chainName, resiIndex, pAtom->xyz.X, pAtom->xyz.Y, pAtom->xyz.Z, pAtom->isXyzValid, pAtom->type[0]);
  }
  else if ((!strcmp(AtomGetType(pAtom), "_FE") || !strcmp(AtomGetType(pAtom), "_Fe"))
    || (!strcmp(AtomGetType(pAtom), "_CO") || !strcmp(AtomGetType(pAtom), "_Co"))
    || (!strcmp(AtomGetType(pAtom), "_NI") || !strcmp(AtomGetType(pAtom), "_Ni"))
    || (!strcmp(AtomGetType(pAtom), "_CU") || !strcmp(AtomGetType(pAtom), "_Cu"))
    || (!strcmp(AtomGetType(pAtom), "_MG") || !strcmp(AtomGetType(pAtom), "_Mg"))
    || (!strcmp(AtomGetType(pAtom), "_CL") || !strcmp(AtomGetType(pAtom), "_Cl"))
    || (!strcmp(AtomGetType(pAtom), "_BR") || !strcmp(AtomGetType(pAtom), "_Br"))
    )
  {
    fprintf(pFile, "%-6.6s%5d %-4.4s %-3.3s %1.1s%4d    %8.3f%8.3f%8.3f  %d                   %-2s\n",
      header, atomIndex, atomName, resiName, chainName, resiIndex, pAtom->xyz.X, pAtom->xyz.Y, pAtom->xyz.Z, pAtom->isXyzValid, pAtom->type + 1);
  }
  else
  {
    if (strcmp(resiName, "ILE") == 0 && strcmp(atomName, "CD") == 0)
    {
      strcpy(atomName, "CD1");
    }
    fprintf(pFile, "%-6.6s%5d  %-3.3s %-3.3s %1.1s%4d    %8.3f%8.3f%8.3f  %d                    %c\n",
      header, atomIndex, atomName, resiName, chainName, resiIndex, pAtom->xyz.X, pAtom->xyz.Y, pAtom->xyz.Z, pAtom->isXyzValid, pAtom->type[0]);
  }

  return Success;
}


int AtomShowAtomParameter(Atom* pThis)
{
  char* name = AtomGetName(pThis);
  char* type = AtomGetType(pThis);
  char* hbHorA = AtomGetHbHorA(pThis);
  char* hbDorB = AtomGetHbDorB(pThis);
  char* hbB2 = AtomGetHbB2(pThis);
  Type_AtomPolarity polarity = pThis->polarity;
  Type_AtomHybridType hybridType = AtomGetHybridType(pThis);
  char* chainName = AtomGetChainName(pThis);
  int posInChain = AtomGetPosInChain(pThis);
  XYZ  xyz = pThis->xyz;
  BOOL isXyzValid = pThis->isXyzValid;
  BOOL isBBAtom = pThis->isBBAtom;
  BOOL isInHBond = pThis->isInHBond;
  BOOL isHBatomH = pThis->isHBatomH;
  BOOL isHBatomA = pThis->isHBatomA;
  // CHARMM parameters
  double CHARMM_epsilon = pThis->vdw_epsilon;
  double CHARMM_radius = pThis->vdw_radius;
  double CHARMM_charge = pThis->charge;

  // parameters used in LK model
  double EEF1_volume = pThis->EEF1_volume;
  double EEF1_lamda_ = pThis->EEF1_lamda_;
  double EEF1_freeDG = pThis->EEF1_freeDG;

  printf("%4s %4s %4s %4s %4s %d %d %1s %4d %8.3f %8.3f %8.3f %d %d %d %d %d ",
    name, type, hbHorA, hbDorB, hbB2, polarity, hybridType, chainName, posInChain,
    xyz.X, xyz.Y, xyz.Z, isXyzValid, isBBAtom, isInHBond, isHBatomH, isHBatomA);
  printf("%5.2f %5.2f %5.2f ", CHARMM_epsilon, CHARMM_radius, CHARMM_charge);
  printf("%6.2f %6.2f %6.2f \n", EEF1_volume, EEF1_lamda_, EEF1_freeDG);

  return Success;
}


int AtomArrayCreate(AtomArray* pThis)
{
  pThis->atoms = NULL;
  pThis->atomNum = 0;
  return Success;
}


int AtomArrayDestroy(AtomArray* pThis)
{
  for (int i = 0;i < pThis->atomNum;i++)
  {
    AtomDestroy(&pThis->atoms[i]);
  }
  free(pThis->atoms);
  pThis->atoms = NULL;
  pThis->atomNum = 0;
  return Success;
}


int AtomArrayCopy(AtomArray* pThis, AtomArray* pOther)
{
  AtomArrayDestroy(pThis);
  AtomArrayCreate(pThis);
  pThis->atomNum = pOther->atomNum;
  pThis->atoms = (Atom*)malloc(sizeof(Atom) * pThis->atomNum);
  for (int i = 0;i < pThis->atomNum;i++)
  {
    AtomCreate(&pThis->atoms[i]);
    AtomCopy(&pThis->atoms[i], &pOther->atoms[i]);
  }
  return Success;
}


int AtomArrayGetCount(AtomArray* pThis)
{
  return pThis->atomNum;
}


Atom* AtomArrayGet(AtomArray* pThis, int index)
{
  if (index < 0 || index >= pThis->atomNum)
  {
    return NULL;
  }
  return pThis->atoms + index;
}


Atom* AtomArrayGetByName(AtomArray* pThis, char* atomName)
{
  int index = -1;
  int result;
  result = AtomArrayFind(pThis, atomName, &index);
  if (FAILED(result))
  {
    return NULL;
  }
  else
  {
    return AtomArrayGet(pThis, index);
  }
}


int AtomArrayFind(AtomArray* pThis, char* atomName, int* pIndex)
{
  for (int i = 0;i < pThis->atomNum;i++)
  {
    if (strcmp(AtomGetName(&pThis->atoms[i]), atomName) == 0)
    {
      *pIndex = i;
      return Success;
    }
  }
  return DataNotExistError;
}


int AtomArrayInsert(AtomArray* pThis, int index, Atom* pNewAtom)
{
  if (index < 0 || index > pThis->atomNum)
  {
    return IndexError;
  }
  int newCount = pThis->atomNum + 1;
  pThis->atoms = (Atom*)realloc(pThis->atoms, sizeof(Atom) * newCount);
  pThis->atomNum = newCount;

  AtomCreate(&pThis->atoms[newCount - 1]);
  for (int i = newCount - 1; i > index; i--)
  {
    AtomCopy(&pThis->atoms[i], &pThis->atoms[i - 1]);
  }
  return AtomCopy(&pThis->atoms[index], pNewAtom);
}


int AtomArrayRemove(AtomArray* pThis, int index)
{
  if (index < 0 || index >= pThis->atomNum)
  {
    return IndexError;
  }

  for (int i = index; i < pThis->atomNum - 1; i++)
  {
    AtomCopy(&pThis->atoms[i], &pThis->atoms[i + 1]);
  }
  AtomDestroy(&pThis->atoms[pThis->atomNum - 1]);
  (pThis->atomNum)--;
  return Success;
}


int AtomArrayRemoveByName(AtomArray* pThis, char* atomName)
{
  int index = -1;
  int result = AtomArrayFind(pThis, atomName, &index);
  if (FAILED(result))
  {
    return result;
  }
  else
  {
    return AtomArrayRemove(pThis, index);
  }
}


int AtomArrayAppend(AtomArray* pThis, Atom* pNewAtom)
{
  return AtomArrayInsert(pThis, AtomArrayGetCount(pThis), pNewAtom);
}


double AtomArrayCalcTotalCharge(AtomArray* pThis)
{
  double totalCharge = 0.0;
  for (int i = 0; i < AtomArrayGetCount(pThis); i++)
  {
    totalCharge += pThis->atoms[i].charge;
  }
  return totalCharge;
}


double AtomArrayCalcMinDistance(AtomArray* pThis, AtomArray* pOther)
{
  double minDist = 1e8;
  for (int i = 0; i < AtomArrayGetCount(pThis); i++)
  {
    if (AtomIsHydrogen(&pThis->atoms[i]))
    {
      continue;
    }
    for (int j = 0; j < AtomArrayGetCount(pOther); j++)
    {
      if (AtomIsHydrogen(&pOther->atoms[j]))
      {
        continue;
      }
      double dist = XYZDistance(&pThis->atoms[i].xyz, &pOther->atoms[j].xyz);
      if (dist < minDist)
      {
        minDist = dist;
      }
    }
  }
  return minDist;
}


BOOL AtomArrayAllAtomXYZAreValid(AtomArray* pThis)
{
  for (int i = 0;i < pThis->atomNum;i++)
  {
    if (pThis->atoms[i].isXyzValid == FALSE)
    {
      return FALSE;
    }
  }
  return TRUE;
}


int AtomArrayShowInPDBFormat(AtomArray* pThis, char* header, char* resiName, char* chainName, int atomIndex, int resiIndex, FILE* pFile)
{
  for (int i = 0;i < AtomArrayGetCount(pThis); i++)
  {
    AtomShowInPDBFormat(&pThis->atoms[i], header, resiName, chainName, atomIndex + i, resiIndex, pFile);
  }
  return Success;
}


int AtomCopyParameter(Atom* pThis, Atom* pOther)
{
  strcpy(pThis->type, pOther->type);
  pThis->vdw_epsilon = pOther->vdw_epsilon;
  pThis->vdw_radius = pOther->vdw_radius;
  pThis->charge = pOther->charge;
  pThis->EEF1_atType = pOther->EEF1_atType;
  pThis->EEF1_volume = pOther->EEF1_volume;
  pThis->EEF1_lamda_ = pOther->EEF1_lamda_;
  pThis->EEF1_freeDG = pOther->EEF1_freeDG;
  pThis->polarity = pOther->polarity;
  pThis->hybridType = pOther->hybridType;

  return Success;
}


