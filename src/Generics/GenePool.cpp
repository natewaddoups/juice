/*****************************************************************************

Project:  Juice
Module:   Generics.dll
Filename: GenePool.cpp

Copyright (c) 2003 Nate Waddoups, coding@natew.com

Generics.dll and its source code are free software.  Except as noted, you 
may redistribute this code under the terms of the Gnu General Public License,
version 2 (GPL2).   This license shall be terminated if any part of GPL2 
should be overturned in court (any jurisdiction).  In that case, contact the 
author for updated license terms.

*****************************************************************************/
#include "StdAfx.h"
#include <stdlib.h>

#include <GenePool.h>

using namespace Generic;

/****************************************************************************/
/** 
*****************************************************************************/
CGenePool::CGenePool(void)
{
	// Number of individuals that will be allowed to reproduce
	m_Params.iSurvivors = 5;

	// Percentage of genes that will be 'crossed' into a new gene from a survivor other than the primary parent
	m_Params.iCrossoverPercentage = 10;

	// Percentage of genes to mutate
	m_Params.iMutationPercentage = 10;

	// Amount of mutation
	m_Params.iMutationSize = 10;
}


/****************************************************************************/
/** 
*****************************************************************************/
CGenePool::~CGenePool(void)
{
}


/****************************************************************************/
/** Initialize the pool.  If the last parameter is non-null, use it as a basis
 ** for all of the members of the population.
*****************************************************************************/
void CGenePool::Initialize (int iPopulationSize, int iGeneLength, vector<real> *pBaseVector)
{
	m_iGeneLength = iGeneLength;

	m_vecPool.resize (iPopulationSize);

	for (int i = 0; i < iPopulationSize; i++)
	{
		m_vecPool[i].m_rScore = 0;
		m_vecPool[i].m_vecGenes.resize (m_iGeneLength);

		for (int j = 0; j < m_iGeneLength; j++)
		{
			if (pBaseVector)
			{
				real value = (*pBaseVector)[j];
				real tweak = rand ();
				tweak -= RAND_MAX / 2;
				tweak /= RAND_MAX;

				// 10% variation
				tweak *= ((real) m_Params.iMutationPercentage) / 100;

				// This was originally *=, but 0 *= anything == 0
				value += tweak;

				m_vecPool[i].m_vecGenes[j] = value;
			}
			else
			{
				real value = rand ();
				value /= RAND_MAX;
				m_vecPool[i].m_vecGenes[j] = value;
			}
		}
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
int CGenePool::iPopulationSize ()
{
	return m_vecPool.size ();
}


/****************************************************************************/
/** 
*****************************************************************************/
vector<real> &CGenePool::GetIndividual (int iIndividual)
{
	return m_vecPool[iIndividual].m_vecGenes;
}


/****************************************************************************/
/** 
*****************************************************************************/
void CGenePool::vSetScore (int iIndividual, real rScore)
{
	m_vecPool[iIndividual].m_rScore = rScore;
}


/****************************************************************************/
/** 
*****************************************************************************/
bool CGenePool::GreaterScore (CGenePool::CIndividual ind1, CGenePool::CIndividual ind2)
{
	return ind1.m_rScore > ind2.m_rScore;
}


/****************************************************************************/
/** Create a new population, using Darwinian selection and genetic mutation
*****************************************************************************/
void CGenePool::vRegenerate ()
{
	// Move the highest-scored individuals to the 'front' of the array
	std::sort (m_vecPool.begin(), m_vecPool.end(), GreaterScore);

    // Keep those, and generate new individuals for the rest of the array
	const int iMax = m_vecPool.size ();
	for (int i = m_Params.iSurvivors; i < iMax; i++)
	{
		// Create a new individual 
		CIndividual newIndividual;
		newIndividual.m_vecGenes.resize (m_iGeneLength);

		// Initialize the new individual from a 'primary' survivor
		int iPrimary = rand () % m_Params.iSurvivors;
		std::copy (m_vecPool[iPrimary].m_vecGenes.begin (), 
			m_vecPool[iPrimary].m_vecGenes.end (), 
			newIndividual.m_vecGenes.begin ());

		// Decide upon a 'secondary' survivor
		int iSecondary = iPrimary;
		while (iSecondary == iPrimary)
			iSecondary = rand () % m_Params.iSurvivors;

		for (int j = 0; j < m_iGeneLength; j++)
		{
			// Cross genes in from secondary
			if ((rand () % 100) < m_Params.iCrossoverPercentage)
				newIndividual.m_vecGenes[j] = m_vecPool[iSecondary].m_vecGenes[j];
		}

		// Copy this into the main pool
		m_vecPool[i].m_vecGenes = newIndividual.m_vecGenes;
	}

	// Mutate a small percentage of the genes
	for (int i = 0; i < iMax; i++)
	{
		for (int j = 0; j < m_iGeneLength; j++)
		{
			if ((rand () % 100) < m_Params.iMutationPercentage)
			{
				real value = m_vecPool[i].m_vecGenes[j];

				real factor = rand ();
				factor /= (RAND_MAX / 2); // 0 ... 2
				factor -= 1;  // -1 ... 1
				factor *= m_Params.iMutationSize; // -10 ... 10
				factor /= 100; // -0.1 ... 0.1
				value += factor;

				m_vecPool[i].m_vecGenes[j] = value;
			}
		}
	}
}


/****************************************************************************/
/** not yet implemented
*****************************************************************************/
bool CGenePool::fLoadFromFile (const char *szPath)
{
	return false;
}


/****************************************************************************/
/** not yet implemented
*****************************************************************************/
void CGenePool::vSaveToFile (const char *szPath)
{
}
