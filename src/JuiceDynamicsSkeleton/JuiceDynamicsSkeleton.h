// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the JUICEDYNAMICSSKELETON_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// JUICEDYNAMICSSKELETON_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef JUICEDYNAMICSSKELETON_EXPORTS
#define JUICEDYNAMICSSKELETON_API __declspec(dllexport)
#else
#define JUICEDYNAMICSSKELETON_API __declspec(dllimport)
#endif

// This class is exported from the JuiceDynamicsSkeleton.dll
class JUICEDYNAMICSSKELETON_API CJuiceDynamicsSkeleton {
public:
	CJuiceDynamicsSkeleton(void);
	// TODO: add your methods here.
};

extern JUICEDYNAMICSSKELETON_API int nJuiceDynamicsSkeleton;

JUICEDYNAMICSSKELETON_API int fnJuiceDynamicsSkeleton(void);
