//  ServiceThread.hpp
//  Service thread header file.
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  Available under the GNU license.
//  2000.03.19

#ifndef _SERVICETHREAD_H_
#define _SERVICETHREAD_H_

#define SERVICE_THREAD_TIMER_ID     0x6502

#define WM_USER_SET_TIMER           WM_USER + 0

VOID WINAPI ServiceMain( DWORD dwArgs, LPTSTR *ppszArgs);
VOID WINAPI ServiceHandler( DWORD dwControl );
BOOL UpdateServiceStatus( DWORD dwState );
DWORD WINAPI ServiceThread( LPVOID pVoid );
BOOL UpdateServiceStatus( DWORD dwState );
VOID CALLBACK TimerHandler( HWND hWnd, UINT uMsg, UINT uEventId, DWORD dwSystemTime );
void SetTimeoutTimer( DWORD dwTimeout );
void CleanStatusStruct( SERVICE_STATUS *pServiceStatus );

#endif // _SERVICETHREAD_H_