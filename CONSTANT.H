//  Constant.h
//  Constant support for Win32 Perl extensions
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  Available under the GNU license.
//  2000.03.19

#ifndef _CONSTANT_H_
#define _CONSTANT_H_

#include <crtdbg.h>

enum eConstantType { NotDefined = 0, NotPresent, String, Numeric };

#ifdef _DEBUG
    #define ALERT(x)            {                           \
                                    TCHAR _TEMP_szBuffer[ 256 ];  \
                                    wsprintf( _TEMP_szBuffer, TEXT( "(%04d) %s\n" ), GetCurrentThreadId(), (x) ); \
                                    LogToFile( _TEMP_szBuffer );      \
                                }
#else
    #define ALERT(x)
#endif // _DEBUG


#define SERVICE_CONTROL_USER_DEFINED    0x80

//  Default handler timeout before we auto-set the service state is 10 seconds
#define DEFAULT_HANDLER_TIMEOUT_VALUE   5

//  The handler timeout value scale. This is the multiplier for the timeout value.
#define DEFAULT_HANDLER_TIMEOUT_SCALE   1000

eConstantType Constant( LPTSTR pszConstant, LPVOID *ppBuffer );
LPTSTR GetConstantName( DWORD dwIndex );
DWORD GetTotalConstants();
void CountConstants();

typedef struct tagConstStruct
{
    LPTSTR m_Name;
    LPVOID m_pBuffer;
    eConstantType m_eType;
} ConstantStruct;

// What user bits can we use for SetServiceBits?
#define USER_SERVICE_BITS_1     0x00004000 
#define USER_SERVICE_BITS_2     0x00008000 
#define USER_SERVICE_BITS_3     0x00400000 
#define USER_SERVICE_BITS_4     0x00800000 
#define USER_SERVICE_BITS_5     0x01000000 
#define USER_SERVICE_BITS_6     0x02000000 
#define USER_SERVICE_BITS_7     0x04000000 
#define USER_SERVICE_BITS_8     0x08000000 
#define USER_SERVICE_BITS_9     0x10000000 
#define USER_SERVICE_BITS_10    0x20000000
 
#define USER_SERVICE_BITS_MASK  0x3FC0C000

extern int		 giThread;
extern int		 iTheList;
extern TCHAR     gszModulePath[];
extern HANDLE    ghServiceThread;
extern DWORD     gServiceThreadId;
extern DWORD     gdwServiceBits;
extern DWORD     gdwLastError;
extern DWORD     gdwState;
extern DWORD     gdwTimeoutState;
extern DWORD     ghTimer;
extern DWORD     gdwHandlerTimeout;
extern DWORD     gdwLastControlMessage;
extern HINSTANCE ghDLL;
extern SERVICE_STATUS_HANDLE ghService;  
extern SERVICE_STATUS gServiceStatus;
extern HANDLE  ghLogFile;

void LogToFile( LPTSTR pszMessage );
BOOL KillTimer();
int My_SetServiceBits( SERVICE_STATUS_HANDLE hService, DWORD dwServiceBits, BOOL bSetBitsOn, BOOL bUpdateImmediately );

#endif _CONSTANT_H_