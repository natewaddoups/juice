/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenePool.h

Copyright (c) 2003 Nate Waddoups, coding@natew.com

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/
#pragma once

#include <vector>
#include <algorithm>
using namespace std;

#ifdef GATest

typedef double real;
const int null = 0;
#define GenericsExport

#else
#include <GenericTypes.h>
#endif

namespace Generic
{

/****************************************************************************/
/** This interface should provide a way to switch out different GA 
 ** implementations to compare their performance.
*****************************************************************************/

class GenericsExport IGenePool
{
public:
	struct Parameters
	{
		// Number of individuals that will be allowed to reproduce
		int iSurvivors;

		// Percentage of genes that will be 'crossed' into a new gene from a survivor other than the primary parent
		int iCrossoverPercentage;

		// Percentage of genes to mutate
		int iMutationPercentage;

		// Amount of mutation
		real iMutationSize;
	};

	// Create a population of vectors with "iPopulation" members, each vector being "iVectorSize" long
	virtual void Initialize (int iPopulation, int iVectorSize, vector<real> *pBaseVector = null) = 0;

	// Return a pointer to a parameter structure
	virtual Parameters *pGetParameters () = 0;

	// Returns the size of the population (as set by the iPopulation parameter to Initialize())
	virtual int iPopulationSize () = 0;

	// Returns a vector of an individual's genes
	virtual vector<real> &GetIndividual (int i) = 0;
	
	// Set the score for an individual's genes
	virtual void vSetScore (int i, real rScore) = 0;
    
	// Create a new population using a genetic algorithm
	virtual void vRegenerate () = 0;

	// Load population from disk
	virtual bool fLoadFromFile (const char *szPath) = 0;

	// Save population to disk
	virtual void vSaveToFile (const char *szPath) = 0;
};

/****************************************************************************/
/** Genetic algorithm implementation
*****************************************************************************/

class GenericsExport CGenePool : IGenePool
{
private:
	class CIndividual
	{
	public:
		vector<real> m_vecGenes;
		real m_rScore;
	};

	vector<CIndividual> m_vecPool;

	static bool CGenePool::GreaterScore (CGenePool::CIndividual ind1, CGenePool::CIndividual ind2);

	IGenePool::Parameters m_Params;

	int m_iGeneLength;
	
public:
	CGenePool(void);
	~CGenePool(void);

	void Initialize (int iPopulation, int iVectorSize, vector<real> *pBaseVector = null);

	Parameters *pGetParameters () { return &m_Params; }

	int iPopulationSize ();

	vector<real> &GetIndividual (int iInvidual);
	
	void vSetScore (int iIndividual, real rScore);
    
	void vRegenerate ();

	bool fLoadFromFile (const char *szPath);
	void vSaveToFile (const char *szPath);
};

}; // end of Generic namespace
