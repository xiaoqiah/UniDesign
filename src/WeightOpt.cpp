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

#include "WeightOpt.h"
#include "ErrorTracker.h"
#include "Utility.h"
#include "EnergyFunction.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>

extern BOOL FLAG_MONOMER;
extern BOOL FLAG_PPI;
extern BOOL FLAG_PROT_LIG;

extern double WEIGHTS[MAX_ENERGY_TERM];

#define PNATAA_USING_ROSETTA_LOSS_FUNCTION
//#define PNATAA_USING_EVOEF_LOSS_FUNCTION
//#define PNATAA_USING_EVOEF_LOSS_FUNCTION2
//#define PNATROT_USING_EVOEF_LOSS_FUNCTION


//this function used to check whether or not a value is NaN
int isNumber(double x)
{
  return (x == x);
}

int isFiniteNumber(double d)
{
  return (d <= DBL_MAX && d >= -DBL_MAX);
}


int EnergyTermRotCreate(EnergyTermRot* pTerm)
{
  for (int i = 0;i < 2;i++)
  {
    pTerm->rotCount[i] = 0;
    pTerm->energy[i] = NULL;
  }
  return Success;
}


int EnergyTermRotDestroy(EnergyTermRot* pTerm)
{
  for (int i = 0;i < 2;i++)
  {
    for (int j = 0;j < pTerm->rotCount[i];j++)
    {
      free(pTerm->energy[i][j]);
    }
    free(pTerm->energy[i]);
    pTerm->rotCount[i] = 0;
  }
  return Success;
}

int EnergyTermsRotCreate(EnergyTermsRot* pTerms)
{
  pTerms->posCount = 0;
  pTerms->terms = NULL;
  return Success;
}

int EnergyTermsRotDestroy(EnergyTermsRot* pTerms)
{
  if (pTerms->terms != NULL)
  {
    for (int i = 0; i < pTerms->posCount;i++)
    {
      EnergyTermRotDestroy(&pTerms->terms[i]);
    }
  }
  pTerms->terms = NULL;
  pTerms->posCount = 0;
  return Success;
}



int EnergyTermsRotRead(EnergyTermsRot* pTerms, char* pdblist)
{
  FileReader fr;
  FileReaderCreate(&fr, pdblist);
  char line[MAX_LEN_ONE_LINE_CONTENT + 1];
  while (!FAILED(FileReaderGetNextLine(&fr, line)))
  {
    char pdbid[20];
    ExtractFirstStringFromSourceString(pdbid, line);
    char energyfile[MAX_LEN_ONE_LINE_CONTENT + 1];
    sprintf(energyfile, "%s_rotenergy.txt", pdbid);
    FileReader fr2;
    FileReaderCreate(&fr2, energyfile);
    char line2[MAX_LEN_ONE_LINE_CONTENT + 1];
    StringArray strings;
    StringArrayCreate(&strings);
    int iniPos = -1;
    while (!FAILED(FileReaderGetNextLine(&fr2, line2)))
    {
      StringArraySplitString(&strings, line2, ' ');
      int resPos = atoi(StringArrayGet(&strings, 1));
      if (resPos != iniPos)
      {//new residue position
        pTerms->posCount++;
        pTerms->terms = (EnergyTermRot*)realloc(pTerms->terms, sizeof(EnergyTermRot) * pTerms->posCount);
        EnergyTermRotCreate(&pTerms->terms[pTerms->posCount - 1]);
        iniPos = resPos;
      }
      EnergyTermRot* pTermRot = &pTerms->terms[pTerms->posCount - 1];
      if (strcmp("XAL", StringArrayGet(&strings, 2)) == 0)
      {
        pTermRot->rotCount[0]++;
        pTermRot->energy[0] = (double**)realloc(pTermRot->energy[0], sizeof(double*) * pTermRot->rotCount[0]);
        pTermRot->energy[0][pTermRot->rotCount[0] - 1] = (double*)malloc(sizeof(double) * NUM_ENERGY_TERM);
        for (int i = 0; i < NUM_ENERGY_TERM; i++)
        {
          pTermRot->energy[0][pTermRot->rotCount[0] - 1][i] = atof(StringArrayGet(&strings, i + 3));
          //for the crystal conformation, set the energy term to be zero if it is too large
          if (FLAG_PROT_LIG)
          {
            if (i >= NUM_ENERGY_MON + NUM_ENERGY_PPI && i < NUM_ENERGY_TERM && pTermRot->energy[0][pTermRot->rotCount[0] - 1][i]>10.0)
            {
              pTermRot->energy[0][pTermRot->rotCount[0] - 1][i] = 0.0;
            }
          }
        }
      }
      else
      {
        pTermRot->rotCount[1]++;
        pTermRot->energy[1] = (double**)realloc(pTermRot->energy[1], sizeof(double*) * pTermRot->rotCount[1]);
        pTermRot->energy[1][pTermRot->rotCount[1] - 1] = (double*)malloc(sizeof(double) * NUM_ENERGY_TERM);
        for (int i = 0; i < NUM_ENERGY_TERM; i++)
        {
          pTermRot->energy[1][pTermRot->rotCount[1] - 1][i] = atof(StringArrayGet(&strings, i + 3));
        }
      }
    }
    FileReaderDestroy(&fr2);
    StringArrayDestroy(&strings);
  }
  FileReaderDestroy(&fr);

  return Success;
}



double GradientNorm(double* grad, int xcount)
{
  double sum = 0;
  for (int i = 0;i < xcount;i++)
  {
    sum += grad[i] * grad[i];
  }
  return sqrt(sum);
}


int GradientUnit(double* grad, int xcount)
{
  double sum = 0;
  for (int i = 0;i < xcount;i++)
  {
    sum += grad[i] * grad[i];
  }
  if (sum < 1e-5)
  {
    char errMsg[MAX_LEN_ERR_MSG + 1];
    sprintf(errMsg, "in file %s line %d, the normalization of a gradient is close to zero", __FILE__, __LINE__);
    TraceError(errMsg, ZeroDivisonError);
    return ZeroDivisonError;
  }
  sum = sqrt(sum);
  for (int i = 0; i < xcount; i++)
  {
    grad[i] /= sum;
  }
  return Success;
}


double GradientMax(double* grad, int xcount)
{
  double max = 0;
  for (int i = 0;i < xcount;i++)
  {
    if (max < fabs(grad[i])) max = fabs(grad[i]);
  }
  return Success;
}


int Xupdate(double* x, double* grad, int xcount, double alpha)
{
  for (int i = 0; i < xcount; i++)
  {
    x[i] += -1.0 * grad[i] * alpha;
    if (i >= NUM_REFERENCE && i < xcount)
    {
      if (x[i] < 0) x[i] = 0.0;
    }
  }
  return Success;
}

int Xshow(double* x, int xcount)
{
  for (int j = 0;j < xcount;j++)
  {
    printf(" %6.2f", x[j]);
  }
  printf("\n");
  return Success;
}


int PNATROT_LossFunction(EnergyTermsRot* pTerms, double* x, int xcount, double* loss)
{
  double score = 0;
  for (int i = 0; i < pTerms->posCount; i++)
  {
    double wtsum = 0, mutsum = 0;
    EnergyTermRot* pTermRot = &pTerms->terms[i];
    for (int j = 0; j < pTermRot->rotCount[0]; j++)
    {
      double wtsumJ = 0;
      for (int k = 0;k < xcount;k++)
      {
        wtsumJ += pTermRot->energy[0][j][k] * x[k];
      }
      wtsum += exp(-1.0 * wtsumJ);
    }
    for (int j = 0; j < pTermRot->rotCount[1]; j++)
    {
      double mutsumJ = 0;
      for (int k = 0;k < xcount;k++)
      {
        mutsumJ += pTermRot->energy[1][j][k] * x[k];
      }
      mutsum += exp(-1.0 * mutsumJ);
    }
    score -= log(wtsum / (wtsum + mutsum));
  }
  *loss = score;
  return Success;
}




int PNATROT_BacktrackLineSearch(EnergyTermsRot* pTerms, double lossk, double* xk, double* gradk, double scalar, int xcnt)
{
  double ak = 1.0;
  double losskn = 0.0;
  double* xkn = (double*)malloc(sizeof(double) * xcnt);
  do
  {
    memcpy(xkn, xk, sizeof(double) * xcnt);
    Xupdate(xkn, gradk, xcnt, ak * scalar);
    //printf("wgt:");
    //Xshow(xkn);
    PNATROT_LossFunction(pTerms, xkn, xcnt, &losskn); //get losskn
    if (losskn >= lossk)
    {
      ak *= 0.1;
    }
    else break;
  } while (TRUE);
  memcpy(xk, xkn, sizeof(double) * xcnt);
  free(xkn);
  return Success;
}




int PNATROT_CalcGradientTwoSide(EnergyTermsRot* pTerms, double loss, double* x, double* grad, int xcount)
{
  double lossf = 0, lossb = 0;

  memset(grad, 0, sizeof(double) * xcount);
  if (FLAG_MONOMER)
  {
    for (int i = 0;i < NUM_ENERGY_MON;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      x[i] = tmp - DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossb);
      grad[i] = (lossf - lossb) / (2.0 * DELTA_X);
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PPI)
  {
    for (int i = NUM_ENERGY_MON;i < NUM_ENERGY_MON + NUM_ENERGY_PPI;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      x[i] = tmp - DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossb);
      grad[i] = (lossf - lossb) / (2.0 * DELTA_X);
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PROT_LIG)
  {
    for (int i = NUM_ENERGY_MON + NUM_ENERGY_PPI;i < xcount;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      x[i] = tmp - DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossb);
      grad[i] = (lossf - lossb) / (2.0 * DELTA_X);
      x[i] = tmp;//restore
    }
  }

  return Success;
}

int PNATROT_CalcGradientForward(EnergyTermsRot* pTerms, double loss, double* x, double* grad, int xcount)
{
  double lossf = 0;

  memset(grad, 0, sizeof(double) * xcount);
  if (FLAG_MONOMER)
  {
    for (int i = 0;i < NUM_ENERGY_MON;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (lossf - loss) / DELTA_X;
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PPI)
  {
    for (int i = NUM_ENERGY_MON;i < NUM_ENERGY_MON + NUM_ENERGY_PPI;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (lossf - loss) / DELTA_X;
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PROT_LIG)
  {
    for (int i = NUM_ENERGY_MON + NUM_ENERGY_PPI;i < xcount;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (lossf - loss) / DELTA_X;
      x[i] = tmp;//restore
    }
  }

  return Success;
}

int PNATROT_CalcGradientBackword(EnergyTermsRot* pTerms, double loss, double* x, double* grad, int xcount)
{
  double lossf = 0;

  memset(grad, 0, sizeof(double) * xcount);
  if (FLAG_MONOMER)
  {
    for (int i = 0;i < NUM_ENERGY_MON;i++)
    {
      double tmp = x[i];
      x[i] = tmp - DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (loss - lossf) / DELTA_X;
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PPI)
  {
    for (int i = NUM_ENERGY_MON;i < NUM_ENERGY_MON + NUM_ENERGY_PPI;i++)
    {
      double tmp = x[i];
      x[i] = tmp - DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (loss - lossf) / DELTA_X;
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PROT_LIG)
  {
    for (int i = NUM_ENERGY_MON + NUM_ENERGY_PPI;i < xcount;i++)
    {
      double tmp = x[i];
      x[i] = tmp - DELTA_X;
      PNATROT_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (loss - lossf) / DELTA_X;
      x[i] = tmp;//restore
    }
  }

  return Success;
}



int PNATROT_WeightOptByGradientDescent(char* pdblistfile)
{
  srand((unsigned int)time(NULL));
  EnergyTermsRot terms;
  EnergyTermsRotCreate(&terms);
  EnergyTermsRotRead(&terms, pdblistfile);

  int xcount = NUM_ENERGY_TERM;
  double x[NUM_ENERGY_TERM];
  double grad[NUM_ENERGY_TERM];
  //for(int i=0;i<20;i++) x[i]=0;
  //for(int i=20;i<NUM_ENERGY_TERM;i++) x[i]=(double)rand()/RAND_MAX;
  for (int i = 0;i < 20;i++) x[i] = 1;
  for (int i = 20;i < NUM_ENERGY_TERM;i++) x[i] = 1;


  int count = 1;
  double gradnorm = 0;
  double loss = 0;
  PNATROT_LossFunction(&terms, x, xcount, &loss);
  while (count <= MAX_STEP)
  {
    printf("step %d:\n", count);
    printf("wgt:");
    Xshow(x, xcount);
    double lossold = loss;
    PNATROT_CalcGradientForward(&terms, loss, x, grad, xcount);
    gradnorm = GradientNorm(grad, xcount);
    printf("gradnorm: %12.6f, loss: %12.6f\n", gradnorm, loss);
    //GradientUnit(grad);
    if (gradnorm < 1e-3) break;
    PNATROT_BacktrackLineSearch(&terms, loss, x, grad, 10.0 / gradnorm, xcount);
    PNATROT_LossFunction(&terms, x, xcount, &loss);
    if (fabs(loss - lossold) < EPSILON) break;
    count++;
  };
  printf("final weights:\n");
  printf("wgt:");
  Xshow(x, xcount);

  return Success;
}


int EnergyTermAllCreate(EnergyTermAA* pTerm)
{
  for (int i = 0;i < TYPE_TWENTYTWO;i++)
  {
    pTerm->rotCount[i] = 0;
    pTerm->energy[i] = NULL;
  }
  return Success;
}


int EnergyTermAllDestroy(EnergyTermAA* pTerm)
{
  for (int i = 0;i < TYPE_TWENTYTWO;i++)
  {
    for (int j = 0;j < pTerm->rotCount[i];j++)
    {
      free(pTerm->energy[i][j]);
    }
    free(pTerm->energy[i]);
    pTerm->rotCount[i] = 0;
  }
  return Success;
}

int EnergyTermsAACreate(EnergyTermsAA* pTerms)
{
  pTerms->posCount = 0;
  pTerms->terms = NULL;
  return Success;
}

int EnergyTermsAADestroy(EnergyTermsAA* pTerms)
{
  if (pTerms->terms != NULL)
  {
    for (int i = 0; i < pTerms->posCount;i++)
    {
      EnergyTermAllDestroy(&pTerms->terms[i]);
    }
  }
  pTerms->terms = NULL;
  pTerms->posCount = 0;
  return Success;
}


int EnergyTermsAARead(EnergyTermsAA* pTerms, char* pdblist)
{
  int result = Success;
  char errMsg[MAX_LEN_ERR_MSG + 1];
  FileReader fr;
  if (FAILED(FileReaderCreate(&fr, pdblist)))
  {
    sprintf(errMsg, "in file %s line %d, failed to read pdblist %s", __FILE__, __LINE__, pdblist);
    result = IOError;
    TraceError(errMsg, result);
    return result;
  }
  char line[MAX_LEN_ONE_LINE_CONTENT + 1];
  while (!FAILED(FileReaderGetNextLine(&fr, line)))
  {
    char pdbid[20];
    ExtractFirstStringFromSourceString(pdbid, line);
    char energyfile[MAX_LEN_ONE_LINE_CONTENT + 1];
    sprintf(energyfile, "%s_rotenergy.txt", pdbid);
    FileReader fr2;
    if (FAILED(FileReaderCreate(&fr2, energyfile)))
    {
      sprintf(errMsg, "in file %s line %d, failed to read rotamer energy file %s", __FILE__, __LINE__, energyfile);
      result = IOError;
      TraceError(errMsg, result);
      return result;
    }
    char line2[MAX_LEN_ONE_LINE_CONTENT + 1];
    int iniPos = -1;
    while (!FAILED(FileReaderGetNextLine(&fr2, line2)))
    {
      StringArray strings;
      StringArrayCreate(&strings);
      StringArraySplitString(&strings, line2, ' ');
      int resPos = atoi(StringArrayGet(&strings, 1));
      if (resPos != iniPos)
      {//new residue position
        pTerms->posCount++;
        pTerms->terms = (EnergyTermAA*)realloc(pTerms->terms, sizeof(EnergyTermAA) * pTerms->posCount);
        EnergyTermAllCreate(&pTerms->terms[pTerms->posCount - 1]);
        iniPos = resPos;
      }
      EnergyTermAA* pTermAll = &pTerms->terms[pTerms->posCount - 1];
      if (strcmp("XAL", StringArrayGet(&strings, 2)) == 0)
      {
        pTermAll->rotCount[TYPE_TWENTYTWO - 2]++;
        pTermAll->energy[TYPE_TWENTYTWO - 2] = (double**)realloc(pTermAll->energy[TYPE_TWENTYTWO - 2], sizeof(double*) * pTermAll->rotCount[TYPE_TWENTYTWO - 2]);
        pTermAll->energy[TYPE_TWENTYTWO - 2][pTermAll->rotCount[TYPE_TWENTYTWO - 2] - 1] = (double*)malloc(sizeof(double) * NUM_ENERGY_TERM);
        for (int i = 0; i < NUM_ENERGY_TERM; i++)
        {
          pTermAll->energy[TYPE_TWENTYTWO - 2][pTermAll->rotCount[TYPE_TWENTYTWO - 2] - 1][i] = atof(StringArrayGet(&strings, i + 3));
          //for the crystal conformation, set the energy term to be zero if it is too large
          if (FLAG_PROT_LIG)
          {
            if (i >= NUM_ENERGY_MON + NUM_ENERGY_PPI && i < NUM_ENERGY_TERM && pTermAll->energy[TYPE_TWENTYTWO - 2][pTermAll->rotCount[TYPE_TWENTYTWO - 2] - 1][i]>10.0)
            {
              pTermAll->energy[TYPE_TWENTYTWO - 2][pTermAll->rotCount[TYPE_TWENTYTWO - 2] - 1][i] = 0.0;
            }
          }
        }
      }
      else if (strcmp("NAT", StringArrayGet(&strings, 2)) == 0)
      {
        pTermAll->rotCount[TYPE_TWENTYTWO - 1]++;
        pTermAll->energy[TYPE_TWENTYTWO - 1] = (double**)realloc(pTermAll->energy[TYPE_TWENTYTWO - 1], sizeof(double*) * pTermAll->rotCount[TYPE_TWENTYTWO - 1]);
        pTermAll->energy[TYPE_TWENTYTWO - 1][pTermAll->rotCount[TYPE_TWENTYTWO - 1] - 1] = (double*)malloc(sizeof(double) * NUM_ENERGY_TERM);
        for (int i = 0; i < NUM_ENERGY_TERM; i++)
        {
          pTermAll->energy[TYPE_TWENTYTWO - 1][pTermAll->rotCount[TYPE_TWENTYTWO - 1] - 1][i] = atof(StringArrayGet(&strings, i + 3));
          //for the crystal conformation, set the energy term to be zero if it is too large
          if (FLAG_PROT_LIG)
          {
            if (i >= NUM_ENERGY_MON + NUM_ENERGY_PPI && i < NUM_ENERGY_TERM && pTermAll->energy[TYPE_TWENTYTWO - 1][pTermAll->rotCount[TYPE_TWENTYTWO - 1] - 1][i]>10.0)
            {
              pTermAll->energy[TYPE_TWENTYTWO - 1][pTermAll->rotCount[TYPE_TWENTYTWO - 1] - 1][i] = 0.0;
            }
          }
        }
      }
      else
      {
        int aaindex = AA3GetIndex(StringArrayGet(&strings, 2));
        pTermAll->rotCount[aaindex]++;
        pTermAll->energy[aaindex] = (double**)realloc(pTermAll->energy[aaindex], sizeof(double*) * pTermAll->rotCount[aaindex]);
        pTermAll->energy[aaindex][pTermAll->rotCount[aaindex] - 1] = (double*)malloc(sizeof(double) * NUM_ENERGY_TERM);
        //the intra-chain and inter-chain VDW repulsive energy should be sufficiently small to be meanlingful
        if (atof(StringArrayGet(&strings, 32 + 3)) < 20.0 && atof(StringArrayGet(&strings, 55 + 3)) < 20.0)
        {
          for (int i = 0; i < NUM_ENERGY_TERM; i++)
          {
            pTermAll->energy[aaindex][pTermAll->rotCount[aaindex] - 1][i] = atof(StringArrayGet(&strings, i + 3));
          }
        }
      }
      //printf("%s\n",line2);
      StringArrayDestroy(&strings);
    }
    FileReaderDestroy(&fr2);
  }
  FileReaderDestroy(&fr);

  return Success;
}


int PNATAA_LossFunction(EnergyTermsAA* pTerms, double* x, int xcount, double* loss)
{
#ifdef PNATAA_USING_ROSETTA_LOSS_FUNCTION
  printf("optimize weights using the rosetta-like loss function\n");
  long double lossscore = 0;
  for (int i = 0; i < pTerms->posCount; i++)
  {
    EnergyTermAA* pTermAll = &pTerms->terms[i];
    double natbest = 100;
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 2]; j++)
    {
      double natscore = 0;
      for (int k = 0;k < xcount;k++)
      {
        natscore += pTermAll->energy[TYPE_TWENTYTWO - 2][j][k] * x[k];
      }
      if (natscore < natbest) natbest = natscore;
    }
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 1]; j++)
    {
      double score = 0;
      for (int k = 0;k < xcount;k++)
      {
        score += pTermAll->energy[TYPE_TWENTYTWO - 1][j][k] * x[k];
      }
      if (score < natbest) natbest = score;
    }
    long double partition = 0;
    for (int s = 0;s < TYPE_TWENTYTWO - 2;s++)
    {
      if (pTermAll->rotCount[s] <= 0) continue;
      double rotbest = 100;
      for (int j = 0; j < pTermAll->rotCount[s]; j++)
      {
        double rotscore = 0;
        for (int k = 0;k < xcount;k++)
        {
          rotscore += pTermAll->energy[s][j][k] * x[k];
        }
        if (rotscore < rotbest) rotbest = rotscore;
      }
      partition += exp(natbest - rotbest);
    }
    lossscore += log(1 + partition);
  }
  *loss = lossscore;
#endif

#ifdef PNATAA_USING_EVOEF_LOSS_FUNCTION
  double lossscore = 0;
  for (int i = 0; i < pTerms->posCount; i++)
  {
    EnergyTermAA* pTermAll = &pTerms->terms[i];
    double nominator = 0;
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 2]; j++)
    {
      double score = 0;
      for (int k = 0;k < xcount;k++)
      {
        score += pTermAll->energy[TYPE_TWENTYTWO - 2][j][k] * x[k];
      }
      nominator += exp(-1 * score);
    }
    double denominator = 0;
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 1]; j++)
    {
      double score = 0;
      for (int k = 0;k < xcount;k++)
      {
        score += pTermAll->energy[TYPE_TWENTYTWO - 1][j][k] * x[k];
      }
      denominator += exp(-1 * score);
    }
    for (int s = 0;s < TYPE_TWENTYTWO - 2;s++)
    {
      for (int j = 0; j < pTermAll->rotCount[s]; j++)
      {
        double score = 0;
        for (int k = 0;k < xcount;k++)
        {
          score += pTermAll->energy[s][j][k] * x[k];
        }
        denominator += exp(-1 * score);
      }
    }
    lossscore -= log(nominator / (nominator + denominator));
  }
  *loss = lossscore;
#endif


#ifdef PNATAA_USING_EVOEF_LOSS_FUNCTION2
  double lossscore = 0;
  for (int i = 0; i < pTerms->posCount; i++)
  {
    EnergyTermAA* pTermAll = &pTerms->terms[i];
    double nominator = 0;
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 2]; j++)
    {
      double score = 0;
      for (int k = 0;k < xcount;k++)
      {
        score += pTermAll->energy[TYPE_TWENTYTWO - 2][j][k] * x[k];
      }
      nominator += exp(-1 * score);
    }
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 1]; j++)
    {
      double score = 0;
      for (int k = 0;k < xcount;k++)
      {
        score += pTermAll->energy[TYPE_TWENTYTWO - 1][j][k] * x[k];
      }
      nominator += exp(-1 * score);
    }
    double denominator = 0;
    for (int s = 0;s < TYPE_TWENTYTWO - 2;s++)
    {
      for (int j = 0; j < pTermAll->rotCount[s]; j++)
      {
        double score = 0;
        for (int k = 0;k < xcount;k++)
        {
          score += pTermAll->energy[s][j][k] * x[k];
        }
        denominator += exp(-1 * score);
      }
    }
    lossscore -= log(nominator / (nominator + denominator));
  }
  *loss = lossscore;
#endif



#ifdef PNATROT_USING_EVOEF_LOSS_FUNCTION
  double lossscore = 0;
  for (int i = 0; i < pTerms->posCount; i++)
  {
    EnergyTermAA* pTermAll = &pTerms->terms[i];
    double nominator = 0;
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 2]; j++)
    {
      double score = 0;
      for (int k = 0;k < xcount;k++)
      {
        score += pTermAll->energy[TYPE_TWENTYTWO - 2][j][k] * x[k];
      }
      nominator += exp(-1 * score);
    }
    double denominator = 0;
    for (int j = 0; j < pTermAll->rotCount[TYPE_TWENTYTWO - 1]; j++)
    {
      double score = 0;
      for (int k = 0;k < xcount;k++)
      {
        score += pTermAll->energy[TYPE_TWENTYTWO - 1][j][k] * x[k];
      }
      denominator += exp(-1 * score);
    }
    for (int s = 0;s < TYPE_TWENTYTWO - 2;s++)
    {
      for (int j = 0; j < pTermAll->rotCount[s]; j++)
      {
        double score = 0;
        for (int k = 0;k < xcount;k++)
        {
          score += pTermAll->energy[s][j][k] * x[k];
        }
        denominator += exp(-1 * score);
      }
    }
    lossscore -= log(nominator / (nominator + denominator));
  }
  *loss = lossscore;
#endif


  return Success;
}

int PNATAA_BacktrackLineSearch(EnergyTermsAA* pTerms, double lossk, double* xk, double* gradk, double scalar, int xcnt)
{
  double ak = 1.0;
  double losskn = 0.0;
  double* xkn = (double*)malloc(sizeof(double) * xcnt);
  do
  {
    memcpy(xkn, xk, sizeof(double) * xcnt);
    Xupdate(xkn, gradk, xcnt, ak * scalar);
    //printf("Wgt:");
    //Xshow(xkn);
    PNATAA_LossFunction(pTerms, xkn, xcnt, &losskn);   //get losskn
    if (losskn >= lossk)
    {
      ak *= 0.1;
    }
    else break;
  } while (TRUE);
  memcpy(xk, xkn, sizeof(double) * xcnt);
  free(xkn);
  return Success;
}



int PNATAA_CalcGradientForward(EnergyTermsAA* pTerms, double loss, double* x, double* grad, int xcount)
{
  double lossf = 0;

  memset(grad, 0, sizeof(double) * xcount);
  if (FLAG_MONOMER)
  {
    for (int i = 0;i < NUM_ENERGY_MON;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATAA_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (lossf - loss) / DELTA_X;
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PPI)
  {
    for (int i = NUM_ENERGY_MON;i < NUM_ENERGY_MON + NUM_ENERGY_PPI;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATAA_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (lossf - loss) / DELTA_X;
      x[i] = tmp;//restore
    }
  }
  else if (FLAG_PROT_LIG)
  {
    for (int i = NUM_ENERGY_MON + NUM_ENERGY_PPI;i < xcount;i++)
    {
      double tmp = x[i];
      x[i] = tmp + DELTA_X;
      PNATAA_LossFunction(pTerms, x, xcount, &lossf);
      grad[i] = (lossf - loss) / DELTA_X;
      x[i] = tmp;//restore
    }
  }

  return Success;
}


int PNATAA_WeightOptByGradientDescent(char* pdblist)
{
  srand((unsigned int)time(NULL));
  EnergyTermsAA terms;
  EnergyTermsAACreate(&terms);
  EnergyTermsAARead(&terms, pdblist);

  int xcount = NUM_ENERGY_TERM;
  double x[NUM_ENERGY_TERM];
  double grad[NUM_ENERGY_TERM];
  //the 20 reference energies are initialized to be zero
  //for(int i=0;i<20;i++) x[i]=0;
  //for(int i=20;i<NUM_ENERGY_TERM;i++) x[i]=(double)rand()/RAND_MAX;
  //for(int i=0;i<20;i++) x[i]=0;
  //for(int i=20;i<NUM_ENERGY_TERM;i++) x[i]=1;
  //set initial values from weights read from file
  x[0] = WEIGHTS[1];
  x[1] = WEIGHTS[2];
  x[2] = WEIGHTS[3];
  x[3] = WEIGHTS[4];
  x[4] = WEIGHTS[5];
  x[5] = WEIGHTS[6];
  x[6] = WEIGHTS[7];
  x[7] = WEIGHTS[8];
  x[8] = WEIGHTS[9];
  x[9] = WEIGHTS[10];
  x[10] = WEIGHTS[11];
  x[11] = WEIGHTS[12];
  x[12] = WEIGHTS[13];
  x[13] = WEIGHTS[14];
  x[14] = WEIGHTS[15];
  x[15] = WEIGHTS[16];
  x[16] = WEIGHTS[17];
  x[17] = WEIGHTS[18];
  x[18] = WEIGHTS[19];
  x[19] = WEIGHTS[20];

  x[20] = WEIGHTS[21];
  x[21] = WEIGHTS[22];
  x[22] = WEIGHTS[23];
  x[23] = WEIGHTS[24];
  x[24] = WEIGHTS[25];
  x[25] = WEIGHTS[26];
  x[26] = WEIGHTS[27];
  x[27] = WEIGHTS[28];
  x[28] = WEIGHTS[91];
  x[29] = WEIGHTS[92];
  x[30] = WEIGHTS[93];

  x[31] = WEIGHTS[31];
  x[32] = WEIGHTS[32];
  x[33] = WEIGHTS[33];
  x[34] = WEIGHTS[34];
  x[35] = WEIGHTS[35];
  x[36] = WEIGHTS[36];
  x[37] = WEIGHTS[41];
  x[38] = WEIGHTS[42];
  x[39] = WEIGHTS[43];
  x[40] = WEIGHTS[44];
  x[41] = WEIGHTS[45];
  x[42] = WEIGHTS[46];
  x[43] = WEIGHTS[47];
  x[44] = WEIGHTS[48];
  x[45] = WEIGHTS[49];

  x[46] = WEIGHTS[51];
  x[47] = WEIGHTS[52];
  x[48] = WEIGHTS[53];
  x[49] = WEIGHTS[54];
  x[50] = WEIGHTS[55];
  x[51] = WEIGHTS[56];
  x[52] = WEIGHTS[61];
  x[53] = WEIGHTS[62];
  x[54] = WEIGHTS[63];
  x[55] = WEIGHTS[64];
  x[56] = WEIGHTS[65];
  x[57] = WEIGHTS[66];
  x[58] = WEIGHTS[67];
  x[59] = WEIGHTS[68];
  x[60] = WEIGHTS[69];

  x[61] = WEIGHTS[71];
  x[62] = WEIGHTS[72];
  x[63] = WEIGHTS[73];
  x[64] = WEIGHTS[74];
  x[65] = WEIGHTS[75];
  x[66] = WEIGHTS[84];
  x[67] = WEIGHTS[85];
  x[68] = WEIGHTS[86];
  x[69] = WEIGHTS[87];
  x[70] = WEIGHTS[88];
  x[71] = WEIGHTS[89];

  int count = 1;
  double gradnorm = 0;
  double loss = 0;
  PNATAA_LossFunction(&terms, x, xcount, &loss);
  while (count <= MAX_STEP)
  {
    printf("Step %d:\n", count);
    printf("Wgt:");
    Xshow(x, xcount);
    double lossold = loss;
    PNATAA_CalcGradientForward(&terms, loss, x, grad, xcount);
    gradnorm = GradientNorm(grad, xcount);
    printf("GradNorm: %12.6f, Loss: %12.6f\n", gradnorm, loss);
    //GradientUnit(grad);
    if (gradnorm < 1e-3) break;
    PNATAA_BacktrackLineSearch(&terms, loss, x, grad, 10.0 / gradnorm, xcount);
    PNATAA_LossFunction(&terms, x, xcount, &loss);
    if (fabs(loss - lossold) < EPSILON) break;
    count++;
  };
  printf("Final weights:\n");
  printf("Wgt:");
  Xshow(x, xcount);

  EnergyTermsAADestroy(&terms);
  return Success;
}
