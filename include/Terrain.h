/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: Terrain.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#ifndef __Terrain__
#define __Terrain__

#include <GenericXmlStreaming.h>

// TODO: Terrain
//#include <Terrain\Terrain.h>

using namespace Generic;

namespace Demeter 
{ 
	class Terrain; 
	class Vector; 
};

namespace Juice {

	class Application;

	namespace Render
	{
		class Scene;
	};

	class Terrain : public XML::Serializable
	{
		std::string m_strElevationImage;
		std::string m_strTexture;
		std::string m_strDetailTexture;
		real m_rVertexSpacing;
		real m_rElevationScale;
		int m_iMaxNumTriangles;

		Demeter::Terrain *m_pTerrain;

	public:

		class Vertex
		{
		public:
			real x, y, z;
		};

		class Triangle
		{
		public:
			int v1, v2, v3;
		};

	protected:

		void vBuildArrays ();

		Terrain::Vertex *m_pVertexArray;
		Terrain::Triangle *m_pTriangleArray;
		int m_iVertexCount;
		int m_iTriangleCount;

	public:

		Terrain ();
		~Terrain ();

		void vInitialize ();

		void vSetParameters (
			const char *szElevationImage, 
			const char *szTexture, 
			const char *szDetailTexture, 
			real rVertexSpacing, 
			real rElevationScale, 
			int iMaxNumTriangles);

		Juice::TerrainLoadError eLoad ();

		bool fLoaded ();

		void vRender ();

		void vSerialize (XML::Stream &Stream);

		real rGetElevation (real rX, real rY);

		void vGetDimensions (real &rX, real &rY);

		void vGetDimensions (int &iX, int &iY);

		//const Demeter::Vector *pGetVertexArray ();
		void vGetTriangles (Terrain::Vertex **ppVertexArray, int *pVertices,
			Terrain::Triangle **ppTriangleArray, int *pTriangles)
		{
			if (m_pTriangleArray && m_pVertexArray && pVertices && pTriangles)
			{
				*ppVertexArray = m_pVertexArray;
				*ppTriangleArray = m_pTriangleArray;
				*pVertices = m_iVertexCount;
				*pTriangles = m_iTriangleCount;

				return;
			}
		}

		real rCastShadow (const VectorXYZ &vecObject, const VectorXYZ &vecLightSource, VectorXYZ &vecShadowPoint);

		void vGetNormal (real rX, real rY, VectorXYZ &vecNormal);
	};

};
#endif

// Terrain.h ends here ->