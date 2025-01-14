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

#ifndef RESIDUE_H
#define RESIDUE_H

#include "AtomParamsSet.h"
#include "ResidueTopology.h"

// enum types associated with Residue
typedef enum _Type_ResiduePolarity
{
  Type_ResiduePolarity_Charged,
  Type_ResiduePolarity_Polar,
  Type_ResiduePolarity_NonPolar
}Type_ResiduePolarity;

typedef enum _Type_ResiduePosition
{
  Type_ResiduePosition_Buried,
  Type_ResiduePosition_Intermediate,
  Type_ResiduePosition_Exposed
}Type_ResiduePosition;

typedef enum _Type_ResidueIsTerminal
{
  Type_ResIsNter,
  Type_ResIsCter,
  Type_ResIsNotTer
} Type_ResidueIsTerminal;

typedef enum _Type_ResidueDesignType
{
  Type_DesType_Fixed,
  Type_DesType_Mutable,
  Type_DesType_Repackable,
  Type_DesType_SmallMol,
  Type_DesType_Catalytic,
  Type_DesType_NatRot,
}Type_ResidueDesignType;

typedef struct _Residue
{
  StringArray patches;
  AtomArray atoms;
  BondSet bonds;
  char name[MAX_LEN_RES_NAME + 1];
  char chainName[MAX_LEN_CHAIN_NAME + 1];
  int posInChain;
  int nCbIn10A;
  Type_ResidueIsTerminal terminalType;
  Type_ResidueDesignType desType;
  double internalEnergy;
  double backboneEnergy;
  double phipsi[2];
  DoubleArray Xs;//side-chain torsions
  double dunbrack;
  double aapp;
  double rama;
  BOOL isSCIntact;
  BOOL isBBIntact;
  char designAATypes[30];
} Residue;

int ResidueCreate(Residue* pThis);
int ResidueDestroy(Residue* pThis);
int ResidueCopy(Residue* pThis, Residue* pOther);
char* ResidueGetName(Residue* pThis);
int ResidueSetName(Residue* pThis, char* newName);
char* ResidueGetChainName(Residue* pThis);
int ResidueSetChainName(Residue* pThis, char* newChainName);
int ResidueGetPosInChain(Residue* pThis);
int ResidueSetPosInChain(Residue* pThis, int newPosInChain);
int ResidueGetDesignType(Residue* pThis);
int ResidueSetDesignType(Residue* pThis, Type_ResidueDesignType newFlag);
double ResidueGetDunbrack(Residue* pThis);
int ResidueSetDunbrack(Residue* pThis, double dun);
double ResidueGetCharge(Residue* pThis);
int ResidueGetPolarity(Residue* pThis, Type_ResiduePolarity* pPolarity);
int ResidueGetAtomCount(Residue* pThis);
Atom* ResidueGetAtom(Residue* pThis, int index);
Atom* ResidueGetAtomByName(Residue* pThis, char* atomName);
int ResidueFindAtom(Residue* pThis, char* atomName, int* pIndex);
int ResidueGetAtomXYZ(Residue* pThis, char* atomName, XYZ* pXYZ);
AtomArray* ResidueGetAllAtoms(Residue* pThis);
int ResidueInsertAtom(Residue* pThis, int newIndex, Atom* pNewAtom);
int ResidueAddAtom(Residue* pThis, Atom* pNewAtom);
int ResidueDeleteAtom(Residue* pThis, char* atomName);

int PDBReaderCheckHisState(FileReader* pPDBFileReader, int* state);
int ResidueReadXYZFromPDB(Residue* pThis, FileReader* pPDBFileReader);
int ResidueAddAtomsFromAtomParams(Residue* pThis, AtomParamsSet* pAtomParams);


BondSet* ResidueGetBonds(Residue* pThis);
int ResidueAddBondsFromResiTopos(Residue* pThis, ResiTopoSet* pResiTopoCollection);
int ResidueShowInPDBFormat(Residue* pThis, char* header, char* chainName, int atomIndex, int resiIndex, FILE* pFile);

int ResiduePatch(Residue* pThis, char* patchName, AtomParamsSet* pAtomParam, ResiTopoSet* pTopos);
int ResiduePatchCTER(Residue* pThis, char* patchName, AtomParamsSet* pAtomParam, ResiTopoSet* pTopos);
int ResiduePatchNTERorCTER(Residue* pThis, char* NTERorCTER, AtomParamsSet* pAtomParam, ResiTopoSet* pTopos);
StringArray* ResidueGetPatchingHistory(Residue* pThis);
int ResidueCalcAtomXYZ(Residue* pThis, ResiTopoSet* pResiTopos, Residue* pPrevResi, Residue* pNextResi, char* atomName, XYZ* pDestXYZ);
int ResidueCalcAllAtomXYZ(Residue* pThis, ResiTopoSet* pResiTopos, Residue* pPrevResi, Residue* pNextResi);
int ResidueCalcAllBackboneAtomXYZ(Residue* pThis, ResiTopoSet* pResiTopos, Residue* pPrevResi, Residue* pNextResi);
int ResidueCalcAllSidechainAtomXYZ(Residue* pThis, ResiTopoSet* pResiTopos);
int ResidueShowAtomParameter(Residue* pThis);
int ResidueShowBondInformation(Residue* pThis);
double ResidueAndResidueSidechainRMSD(Residue* pThis, Residue* pOther);
BOOL LigandResidueNameConflictWithAminoAcid(char* ligname);
int ResidueCheckAtomCoordinateValidity(Residue* pThis);
BOOL ResidueIsSymmetricalCheck(Residue* pResidue);
int SymmetricalResidueGenerate(Residue* pSymmetrical, Residue* pResidue);
BOOL ResidueAndResidueInSameType(Residue* pThis, Residue* pOther);
BOOL ResidueAndResidueAllTorsionsAreSimilar(Residue* pThis, Residue* pOther, double cutoff);
BOOL ResidueAndResidueCheckTorsionSimilarity(Residue* pThis, Residue* pOther, double cutoff, IntArray* pSimArray);
double ResidueGetAverageBfactor(Residue* pThis);
BOOL IsResidueHistidine(char* resiName);

#endif //RESIDUE_H
