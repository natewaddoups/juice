#define ExternalAPI __stdcall __declspec (dllexport) 
typedef real float;

/** This is the API between Juice and plug-in behavior implementations */ 

namespace BehaviorPlugin
{
	enum JointFlags
	{
		Invalid = 0,
		Hinge = 1,
		Slider = 2,
		Ball = 4,
		PositionMotor = 1 >> 8,
		VelocityMotor = 2 >> 8,
		GetPosition = 1 >> 16,
	};

	enum BodyFlags
	{
		Invalid = 0,
		YawSensor = 1,
		PitchSensor = 2,
		RollSensor = 4,
		Future = 8,
	};

	typedef struct 
	{
		const char *Name;
		JointFlags Flags;
		real Position;
		real Motor;
	} Joint;

	typedef struct
	{
		const char *Name;
		BodyFlags Flags;
		real Yaw;
		real Pitch;
		real Roll;
	} Body;

	typedef std::list<Joint*> JointList;
	typedef std::list<Body*> BodyList;

	// This will be called on each behavior implementation when the user 
	// opens a model.
	// The void* returned by this function will be passed into the other
	// functions.  Implementors are invited to use it for any purpose.
	void* LoadModel (const char *szModelName);

	// This will be called on each behavior implementation when the user
	// closes a model.
	// If the implementor allocated a data structure during LoadModel,
	// this would be the place to release the data structure.
	void UnloadModel (void *pUserData);

	// This will be called when a joint or body is added to or removed from
	// the model.
	void JointsChanged (JointList *pJoints, void *pUserData);
	void BodiesChanged (BodyList *pBodies, void *pUserData);

	// This will be called before a simulation begins.  The implementation
	// should set joint and body flags to tell Juice how to interpret the Motor
	// value and which body feedback values to return during the simulation.
	// The 'data file' parameter can be set by the user in the Juice UI, its
	// use is optional but implementors are encouraged to use it to reference
	// a data file.
	void ExternalAPI PrepareForSimulation (const char * szDataFile, 
		JointList *pJoints, BodyList *pBodies, void *pUserData);

	// With each invocation of Step, the implementation may read sensor data 
	// and write motor data from and to body and joint structures.
	void ExternalAPI Step (real rEnginePhase, Generic::UINT32 uMilliseconds, 
		ControlInput *pController, void *pUserData);

	// This will be called when a simulation ends.  It is provided for the
	// convenience of the implementor.
    void ExternalAPI EndSimulation (void *pUserData);
}