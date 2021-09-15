/*****************************************************************************

Project:  Juice
Module:   JuiceEngine.dll
Filename: RenderModel.cpp

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuiceEngine.dll and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#define VC_EXTRALEAN
#define OEMRESOURCE
#include <windows.h>
#include <gl/gl.h>
#include <math.h>

#include <JuiceEngine.h>
#include "RenderInternal.h"

using namespace Juice;
using namespace Juice::Render;

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vRender (Juice::Render::Scene *pScene)
{
	vRender_Internal (pScene, RenderMode::rmShadow);
	vRender_Internal (pScene, RenderMode::rmNormal);
}

/****************************************************************************/
/** 
*****************************************************************************/
void Model::vRender_Internal (Juice::Render::Scene *pScene, RenderMode eRenderMode)
{
	glInitNames ();
	vGetError ();

	// Bodies
	std::list<Juice::Body*>::iterator iterBodies;
	for (iterBodies = m_Bodies.begin (); iterBodies != m_Bodies.end (); iterBodies++)
	{
		Body *pBody = (*iterBodies);

		pBody->vRender (pScene, eRenderMode);
	}
	
	if (m_pApplication && !m_pApplication->m_Preferences.m_fRenderJoints)
		return;

	// Revolute joints
	std::list<Juice::RevoluteJoint*>::iterator iterRevoluteJoints;
	for (iterRevoluteJoints = m_RevoluteJoints.begin (); iterRevoluteJoints != m_RevoluteJoints.end (); iterRevoluteJoints++)
	{
		RevoluteJoint *pRevoluteJoint = (*iterRevoluteJoints);

		if ((pScene->pGetApplication()->eGetAppMode () == amDesign) || pRevoluteJoint->m_pDynamicsObject->fInSimulation ())
			pRevoluteJoint->vRender (pScene, eRenderMode);
	}

	// Prismatic joints
	std::list<Juice::PrismaticJoint*>::iterator iterPrismaticJoints;
	for (iterPrismaticJoints = m_PrismaticJoints.begin (); iterPrismaticJoints != m_PrismaticJoints.end (); iterPrismaticJoints++)
	{
		PrismaticJoint *pPrismaticJoint = (*iterPrismaticJoints);

		if ((pScene->pGetApplication()->eGetAppMode () == amDesign) || pPrismaticJoint->m_pDynamicsObject->fInSimulation ())
			pPrismaticJoint->vRender (pScene, eRenderMode);
	}

	// Spherical joints
	std::list<Juice::SphericalJoint*>::iterator iterSphericalJoints;
	for (iterSphericalJoints = m_SphericalJoints.begin (); iterSphericalJoints != m_SphericalJoints.end (); iterSphericalJoints++)
	{
		SphericalJoint *pSphericalJoint = (*iterSphericalJoints);
		
		if ((pScene->pGetApplication()->eGetAppMode () == amDesign) || pSphericalJoint->m_pDynamicsObject->fInSimulation ())
			pSphericalJoint->vRender (pScene, eRenderMode);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Component::vRender (Scene *pScene, Model::RenderMode eRenderMode)
{
	if (Model::rmNormal == eRenderMode)
	{
		// draw the object in the usual way
		Color *pclr = pGetColor ();
		vSetMaterial (*pclr);

		vSetTransform (pScene);

		glPushName ((GLuint) this);

		vDrawMesh (pScene);

		glPopName ();
		glPopMatrix ();
	}

	if (Model::rmShadow == eRenderMode)
	{
		// draw the object's shadow
		glDisable (GL_DEPTH_TEST);

		vSetShadow (pScene->pGetApplication()->m_Preferences.m_clrShadow);

		VectorXYZ vecPosition;
		vGetPosition (vecPosition);

		if (pScene->fPushShadowMatrix (vecPosition))
		{
			vSetTransform (pScene);

			vDrawMesh (pScene);

			glPopMatrix ();
			glPopMatrix ();
		}

		glEnable (GL_DEPTH_TEST);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
Color* Body::pGetColor ()
{
	Color *pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrBeam;

	if (fSelected ())
		pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrSelected;

	return pclr;
}

/****************************************************************************/
/** 
*****************************************************************************/
void Body::vSetTransform (Scene *pScene)
{
	if (!pScene)
		return;

	if (pScene->pGetApplication()->eGetAppMode () == amDesign)
	{
		pScene->vPushMatrix (m_vecPosition, m_vecRotation);
	}
	else
	{
		// This works
		Matrix4x4 mx;
		m_pDynamicsObject->vGetMatrix (mx);
		pScene->vPushMatrix (mx);

		// This also works 
/*		VectorXYZ xyz;
		m_pDynamicsObject->vGetPosition (xyz);

		Quaternion q;
		m_pDynamicsObject->vGetQuaternion (q);

		// But why is this necessary?
		q.w = -q.w;

		Matrix4x4 mx;
		mx.vSetIdentity ();
		mx.vSetRotation (q);
		mx.vSetTranslation (xyz);

		pScene->vPushMatrix (mx);
*/

/*		// Note to self: quaternion to euler conversion doesn't work very well.
		// It gives good results until yaw exceeds 90 degrees, then everything flips over
		VectorXYZ xyz;
		m_pDynamicsObject->vGetPosition (xyz);

		Quaternion q;
		m_pDynamicsObject->vGetQuaternion (q);
		
		VectorYPR ypr;
		q.vToEuler (ypr);
		ypr.vToDegrees ();
		
		pScene->vPushMatrix (xyz, ypr);
*/
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void Body::vDrawMesh (Scene *pScene)
{
	if (!pScene)
		return;

	switch (m_eShape)
	{
	case Shape::sBox:
		pScene->vDrawBox (m_vecBoxDimensions);
		break;

	case Shape::sCylinder:
		pScene->vDrawCylinder (m_rTubeLength, m_rTubeDiameter / 2);
		break;

	case Shape::sCapsule:
		pScene->vDrawCapsule (m_rTubeLength, m_rTubeDiameter / 2, 1);
		break;

	case Shape::sBall:
		pScene->vDrawSphere (m_rBallDiameter / 2);
		break;
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
Color* RevoluteJoint::pGetColor ()
{
	Color *pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrHinge;

	if (fSelected ())
		pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrSelected;

	return pclr;
}

/****************************************************************************/
/** 
*****************************************************************************/
void RevoluteJoint::vSetTransform (Scene *pScene)
{
	if (!pScene)
		return;

	if (pScene->pGetApplication()->eGetAppMode () == amDesign)
	{
		VectorYPR vecRotation = m_vecRotation;
		vecRotation.pitch *= -1;
		pScene->vPushMatrix (m_vecPosition, m_vecRotation);
	}
	else
	{
		Matrix4x4 mx;

		if (m_pBody1 && m_pBody2 && (m_pBody1 != m_pBody2))
			m_pDynamicsObject->vGetMatrix (mx);
		else
			mx.vSetIdentity ();

		pScene->vPushMatrix (mx);
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void RevoluteJoint::vDrawMesh (Scene *pScene)
{
	if (!pScene)
		return;

	pScene->vDrawCapsule (m_rLength, m_rDiameter / 2, 0.1);
}

/****************************************************************************/
/** 
*****************************************************************************/
Color* PrismaticJoint::pGetColor ()
{
	Color *pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrSlider;

	if (fSelected ())
		pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrSelected;

	return pclr;
}

/****************************************************************************/
/** 
*****************************************************************************/
void PrismaticJoint::vSetTransform (Scene *pScene)
{
	if (!pScene)
		return;

	if (pScene->pGetApplication()->eGetAppMode () == amDesign)
	{
		pScene->vPushMatrix (m_vecPosition, m_vecRotation);
	}
	else
	{
		Matrix4x4 mx;
		
		if (m_pBody1 && m_pBody2 && (m_pBody1 != m_pBody2))
			m_pDynamicsObject->vGetMatrix (mx, m_vecBody1EndPoint, m_vecBody2EndPoint);
		else
			mx.vSetIdentity ();

		pScene->vPushMatrix (mx);			
	}
}

/****************************************************************************/
/** 
*****************************************************************************/
void PrismaticJoint::vDrawMesh (Scene *pScene)
{
	if (!pScene)
		return;

	if (pScene->pGetApplication()->eGetAppMode () == amDesign)
	{
		pScene->vDrawCapsule (m_rLength, m_rDiameter / 2, 0.1);
	}
	else
	{
		real rStretch = m_pDynamicsObject->rGetExtension ();
			
		if (rStretch < 0) 
			rStretch = -rStretch;

		pScene->vDrawCapsule (rStretch + m_rLength, m_rDiameter / 2, 0.1);
	}
		
}

/****************************************************************************/
/** 
*****************************************************************************/
Color* SphericalJoint::pGetColor ()
{
	Color *pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrBallJoint;

	if (fSelected ())
		pclr = &m_pModel->pGetApplication()->m_Preferences.m_clrSelected;

	return pclr;
}

/****************************************************************************/
/** 
*****************************************************************************/
void SphericalJoint::vSetTransform (Scene *pScene)
{
	if (!pScene)
		return;

	if (pScene->pGetApplication()->eGetAppMode () == amDesign)
	{
		pScene->vPushMatrix (m_vecPosition, m_vecRotation);
	}
	else
	{
		VectorYPR vecRotation (0, 0, 0);
		VectorXYZ vecPosition;

		m_pDynamicsObject->vGetAnchor (vecPosition);

		pScene->vPushMatrix (vecPosition, vecRotation);
	}
}


/****************************************************************************/
/** 
*****************************************************************************/
void SphericalJoint::vDrawMesh (Scene *pScene)
{
	if (!pScene)
		return;

	pScene->vDrawSphere (m_rDiameter / 2);
}

// RenderModel.cpp ends here ->