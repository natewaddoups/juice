/*****************************************************************************

Project:  Juice
Filename: GATest.cpp

Copyright (c) 2003 Nate Waddoups, coding@natew.com

GATest.cpp is free software.  You may redistribute the contents of this file 
under the terms of the Gnu General Public License, version 2 (GPL2).   This 
license shall be terminated if any part of GPL2 should be overturned in court 
(any jurisdiction).  In that case, contact the author for updated license 
terms.

*****************************************************************************/

#include "stdafx.h"
#include <math.h>
#include "..\..\include\GenePool.h"

int iGeneWidth = 10;
int iPopSize = 1000;

std::vector<real> vecTarget;

real GetScore (std::vector<real> &vec)
{
	real result = 0;

	int iWidth = vec.size ();
	for (int i = 0; i < iWidth; i++)
	{
		real target = vecTarget[i];
		real actual = vec[i];
		real delta = fabs (target - actual);
		result -= delta;
	}

	return result;
}

int _tmain(int argc, _TCHAR* argv[])
{
	Generic::CGenePool pool;

	real bestScore = -1000;
	int iGenerations = 1;

	// Initialize target vector
	vecTarget.resize (iGeneWidth);

	for (int i = 0; i < iGeneWidth; i++)
	{
		real value = rand ();
		value /= RAND_MAX;
		vecTarget[i] = value;
	}

	// Initialize pool
	pool.Initialize (iPopSize, iGeneWidth);

	while (bestScore < 1)
	{
		for (int i = 0; i < iPopSize; i++)
		{
			std::vector<real> &subject = pool.GetIndividual (i);
			real score = GetScore (subject);
			pool.vSetScore (i, score);

			if (score > bestScore)
			{
				bestScore = score;
			}
		}

		printf ("Generation %d, best score %f\r\n", iGenerations, bestScore);

		iGenerations++;

		pool.vRegenerate ();
	}	

	return 0;
}

