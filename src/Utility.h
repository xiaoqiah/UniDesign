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

#ifndef UTILITY_H
#define UTILITY_H

#include "ErrorTracker.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

#define MAX_LEN_ATOM_NAME         5
#define MAX_LEN_ATOM_TYPE         5
#define MAX_LEN_ATOM_HYBRID       5
#define MAX_LEN_ATOM_DONOR        5
#define MAX_LEN_ATOM_ACCEPTOR     5
#define MAX_LEN_RES_NAME          5
#define MAX_LEN_CHAIN_NAME        3
#define MAX_LEN_STRUCTURE_NAME    5

#define MAX_LEN_ONE_LINE_CONTENT  1024
#define MAX_LEN_FILE_NAME         100

#define COMMENT_LINE_SYMBOL1         '!'
#define COMMENT_LINE_SYMBOL2         '#'

typedef struct _StringArray
{
  char** strings;
  int stringCount;
  int capacity;
} StringArray;

int StringArrayCreate(StringArray* pThis);
int StringArrayDestroy(StringArray* pThis);
int StringArrayCopy(StringArray* pThis, StringArray* pOther);
int StringArrayGetCount(StringArray* pThis);
int StringArraySet(StringArray* pThis, int index, char* srcString);
char* StringArrayGet(StringArray* pThis, int index);
int StringArrayFind(StringArray* pThis, char* srcString, int* pos);
int StringArrayInsert(StringArray* pThis, int pos, char* srcString);
int StringArrayRemove(StringArray* pThis, int pos);
int StringArrayAppend(StringArray* pThis, char* srcString);
int StringArraySplitString(StringArray* pThis, char* srcStr, char splitter);
int StringArrayShow(StringArray* pThis);


typedef enum _Type_CoordinateFile
{
  Type_CoordinateFile_PDB,
  Type_CoordinateFile_MOL2,
  Type_CoordinateFile_Unrecognized
} Type_CoordinateFile;

typedef struct _FileReader
{
  StringArray lines;
  int position;
} FileReader;

int FileReaderCreate(FileReader* pThis, char* path);
int FileReaderDestroy(FileReader* pThis);
int FileReaderGetLineCount(FileReader* pThis);
int FileReaderGetLine(FileReader* pThis, int index, char* dest);
int FileReaderGetCurrentPos(FileReader* pThis);
int FileReaderSetCurrentPos(FileReader* pThis, int index);
int FileReaderGetNextLine(FileReader* pThis, char* dest);
BOOL FileReaderEndOfFile(FileReader* pThis);
Type_CoordinateFile FileReaderRecognizeCoordinateFileType(FileReader* pThis);

typedef struct _IntArray
{
  int* content;
  int length;
  int capacity;
} IntArray;

int IntArrayCreate(IntArray* pThis, int length);
int IntArrayDestroy(IntArray* pThis);
int IntArrayCopy(IntArray* pThis, IntArray* pOther);
int IntArrayResize(IntArray* pThis, int newLength);
int IntArrayGetLength(IntArray* pThis);
int IntArrayGet(IntArray* pThis, int index);
int IntArraySet(IntArray* pThis, int index, int newValue);
int* IntArrayGetAll(IntArray* pThis);
int IntArraySetAll(IntArray* pThis, int* pNewContent);
int IntArrayInsert(IntArray* pThis, int index, int newValue);
int IntArrayRemove(IntArray* pThis, int index);
int IntArrayAppend(IntArray* pThis, int newValue);
int IntArrayShow(IntArray* pThis);
int IntArrayFind(IntArray* pThis, int num);

typedef struct _DoubleArray
{
  double* content;
  int length;
  int capacity;
} DoubleArray;

int DoubleArrayCreate(DoubleArray* pThis, int length);
int DoubleArrayDestroy(DoubleArray* pThis);
int DoubleArrayCopy(DoubleArray* pThis, DoubleArray* pOther);
int DoubleArrayResize(DoubleArray* pThis, int newLength);
int DoubleArrayGetLength(DoubleArray* pThis);
double DoubleArrayGet(DoubleArray* pThis, int index);
int DoubleArraySet(DoubleArray* pThis, int index, double newValue);
double* DoubleArrayGetAll(DoubleArray* pThis);
int DoubleArraySetAll(DoubleArray* pThis, double* pNewContent);
int DoubleArrayInsert(DoubleArray* pThis, int index, double newValue);
int DoubleArrayRemove(DoubleArray* pThis, int index);
int DoubleArrayAppend(DoubleArray* pThis, double newValue);
double DoubleArrayInnerProduct(DoubleArray* pThis, DoubleArray* pOther);
int DoubleArrayScale(DoubleArray* pThis, double scale);
double DoubleArrayNorm(DoubleArray* pThis);
int  DoubleArrayMinus(DoubleArray* pThis, DoubleArray* pOther);
int DoubleArrayShow(DoubleArray* pThis);


int ExtractTargetStringFromSourceString(char* dest, char* src, int start, int length);
int ExtractFirstStringFromSourceString(char* dest, char* src);

int Model(int i, FILE* pFile);
int EndModel(FILE* pFile);
int AddChainTER(FILE* pFile);

int ShowProgress(int width, double percentage);
int SpentTimeShow(time_t ts, time_t te);

typedef enum _Type_Residue
{
  Type_Residue_Ala,
  Type_Residue_Cys,
}Type_Residue;


int AA1ToAA3(char name1, char name3[MAX_LEN_RES_NAME]);
char AA3ToAA1(char name3[MAX_LEN_RES_NAME]);
BOOL IsAAPolar(char* resiName);
int AA1GetIndex(char name1);
int AA3GetIndex(char name3[MAX_LEN_RES_NAME]);
#endif // UTILITY_H
