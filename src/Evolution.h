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

#ifndef EVOLUTION_H
#define EVOLUTION_H

//the first parameter is the path of the current executable program used to compute evolution score
//this path is very important, because it is used to determine the location of feature parameters required by the program
float EvolutionScoreAllFromFile(char* seqfile);
float EvolutionScoreAllFromSeq(char* seq);
float EvolutionScorePrfSSSAFromSeq(char* seq);
float EvolutionScorePrfFromSeq(char* seq);
float EvolutionScorePrfFromFile(char* seqfile);
float EvolutionScoreFromPSSMWithoutAlignment(char* seq);

int SSPred(char* seqfile);
int SAPred(char* seqfile);
int PhiPsiPred(char* seqfile);


#endif
