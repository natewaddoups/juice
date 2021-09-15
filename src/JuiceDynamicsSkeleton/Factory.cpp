/*****************************************************************************
<HEADER>
*****************************************************************************/

namespace Juice 
{ 
	namespace Dynamics 
	{ 
		class Factory;

		namespace ODE 
		{
			__declspec (dllexport) Dynamics::Factory* pCreateFactory ();
		};
	};
};

#include <JuiceEngine.h>
#include <DynamicsAPI.h>

#include <ode/ode.h>
#include <DynamicsODE.h>

using namespace Juice;
using namespace Juice::Dynamics::ODE;

Dynamics::Factory* Juice::Dynamics::ODE::pCreateFactory ()
{
	return new Factory;
}

Factory::Factory ()
{
}

/****************************************************************************/
/** This is a bit of a hack to suppress invalid ODE memory leak reports.  The
 ** dCloseODE function will be included in the next version of ODE, and then
 ** it won't be necessary to sneak the declaration in here.
*****************************************************************************/
extern "C" void dCloseODE ();

Factory::~Factory ()
{
	// any last-minute cleanup?
}


/// This is a helper function that builds an OpenGL-compatible matrix using 
/// ODE's position vector and orientation matrix.
void vBuildMatrix (Matrix4x4 &mx, const dReal *pPosition, const dReal *pOrientation)
{
	mx[0]  = pOrientation[0];
	mx[1]  = pOrientation[4];
	mx[2]  = pOrientation[8];
	mx[3]  = 0;
	mx[4]  = pOrientation[1];
	mx[5]  = pOrientation[5];
	mx[6]  = pOrientation[9];
	mx[7]  = 0;
	mx[8]  = pOrientation[2];
	mx[9]  = pOrientation[6];
	mx[10] = pOrientation[10];
	mx[11] = 0;
	mx[12] = pPosition[0];
	mx[13] = pPosition[1];
	mx[14] = pPosition[2];
	mx[15] = 1;
}

/*****************************************************************************
<FOOTER>
*****************************************************************************/
