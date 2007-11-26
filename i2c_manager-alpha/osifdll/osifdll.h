
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OSIFDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OSIFDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef OSIFDLL_EXPORTS
#define OSIFDLL_API __declspec(dllexport)
#else
#define OSIFDLL_API __declspec(dllimport)
#endif

// This class is exported from the osifdll.dll
class OSIFDLL_API COsifdll {
public:
	COsifdll(void);
	// TODO: add your methods here.
};

extern OSIFDLL_API int nOsifdll;

OSIFDLL_API int fnOsifdll(void);

