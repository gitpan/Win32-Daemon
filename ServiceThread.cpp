//  ServiceThread.cpp
//  Wickedly sinister hack for a service manager thread.
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  Available under the GNU license.
//  2000.03.19

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lmaccess.h>   //  Service stuff
#include <lmserver.h>   //  Service stuff
#include <lmapibuf.h>
#include <LMERR.H>      //  For the NERR_Succes macro

#include <stdio.h> // REmove. ONly used for sprintf for debugging.

#include "constant.h"
#include "CWinStation.hpp"
#include "ServiceThread.hpp"

#include <crtdbg.h>

static SERVICE_TABLE_ENTRY gpServiceTable[] =
{
    {
        TEXT( "GROWL!" ), ServiceMain
    },
    {
        NULL,   NULL
    }
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ServiceMain()
//  Called by the Service Manager.
VOID WINAPI ServiceMain( DWORD dwArgs, LPTSTR *ppszArgs)
{
    HWND hWnd = NULL;
    LPCTSTR pszServiceName = ppszArgs[0];
    
    ALERT( "ServiceMain: function started. Passed inn Name is..." );
    ALERT( pszServiceName );
    ALERT( "ServiceMain: About to call RegisterServiceCtrlHandler()..." );

// Test to see if we can cause the message loop to start queuing...
SetTimeoutTimer( 10 );
    
    CleanStatusStruct( &gServiceStatus );
    gServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        
    ghService = RegisterServiceCtrlHandler( pszServiceName, ServiceHandler );
    if( 0 != ghService )
    {
        ALERT( "ServiceMain: Just came out of RegisterServiceCtrlHandler()" );

        // If the state has not yet changed then push start everything...b
        if( 0 == gdwState )
        {
            gdwState = SERVICE_START_PENDING;
        }

        {
            char szBuffer[256];
            sprintf( szBuffer, "ServiceMain: About to call My_SetServiceBits with gdwServiceBits=0x%08x", gdwServiceBits );
            ALERT( szBuffer );
        }

        if( 0 != gdwServiceBits )
        {
            My_SetServiceBits( ghService, gdwServiceBits, TRUE, TRUE );
        }
        
//  This block removed since the Perl script should be the one to report we are running
//        gdwState = SERVICE_RUNNING;
//        UpdateServiceStatus( gdwState );    

/*
        do
        {
            Sleep( 750 );
        }
h        while( SERVICE_STOPPED != gdwState );
*/    
        ALERT( "ServiceMain: Entering message loop" );

        // Call a Win32 User level function to create a message queue
        GetDesktopWindow();
        GetWindow( NULL, GW_HWNDFIRST );
        
        if( 1 )
        {
            MSG Message;
            int iResult = 1;

            while( iResult )
            { 
ALERT( "ServiceMain: Just enetered the message loop" );

                try
                {
                    iResult = GetMessage( &Message, (HWND) NULL, 0, 0 );
                }
                catch (...)
                {
                    ALERT( "ServiceMain: Ouch!!! We caught an exception!" );
                }

ALERT( "ServiceMain: Get a message " );
                if( -1 == iResult )
                {
ALERT( "ServiceMain: OOOPS! GetMessage ended in error (-1 )" );
//                    _ASSERT( -1 != iResult );
                }
    TCHAR szBuffer[256];
    wsprintf( szBuffer, "Got message: 0x%08x", Message.message );
    ALERT( szBuffer );

                switch( Message.message )
                {

                case WM_USER_SET_TIMER:
ALERT( "ServiceMain: Setting timer" );
                    ghTimer = ::SetTimer( NULL, SERVICE_THREAD_TIMER_ID, Message.wParam * DEFAULT_HANDLER_TIMEOUT_SCALE, TimerHandler );
                    break;

                case WM_QUERYENDSESSION:
                case WM_ENDSESSION:
                case WM_TIMER:
                    ALERT( "ServiceMain: HandlerTimeoutTimer due to WM_TIMER." );     
                    KillTimer();
                    gdwState = gdwTimeoutState;
                    UpdateServiceStatus( gdwTimeoutState );

                default:
    ALERT( "ServiceMain: Dispatching message." );
                    TranslateMessage( &Message ); 
                    DispatchMessage( &Message ); 
                }
            } 
        }
        ALERT( "ServiceMain: Just left the message loop." );
        UpdateServiceStatus( gdwState );    

    }
    else
    {   
        gdwState = SERVICE_STOPPED;
#ifdef _DEBUG
        TCHAR szBuffer[ 100 ];
        wsprintf( szBuffer, TEXT( "ServiceMain: ERROR! 0x08x" ), GetLastError() );
        ALERT( szBuffer );
#endif // _DEBUG
    }

    ALERT( "ServiceMain: Shutting down ServiceMain()!" );
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID WINAPI ServiceHandler( DWORD dwControl )
{
    LPTSTR pszCommand = "";
    DWORD dwState = gdwState;
    BOOL fUseTimer = FALSE;
#ifdef _DEBUG
    TCHAR szBuffer[ 256 ];
  
#endif // _DEBUG

    ALERT( "ServiceHandler: Incoming service control message..." );

    switch( dwControl )
    {
    case SERVICE_CONTROL_STOP:
        pszCommand = "STOP";
        dwState = SERVICE_STOP_PENDING;
        fUseTimer = TRUE;
        gdwTimeoutState = SERVICE_STOPPED;
        break;
    
    case SERVICE_CONTROL_PAUSE:
        pszCommand = "PAUSE";
        dwState = SERVICE_PAUSE_PENDING;
        fUseTimer = TRUE;
        gdwTimeoutState = SERVICE_PAUSED;
        break;    
    
    case SERVICE_CONTROL_CONTINUE:
        pszCommand = "CONTINUE";
        dwState = SERVICE_CONTINUE_PENDING;
        fUseTimer = TRUE;
        gdwTimeoutState = SERVICE_RUNNING;
        break;    
    
    case SERVICE_CONTROL_INTERROGATE:
        pszCommand = "INTERROGATE";
		dwState = SERVICE_INTERROGATE;
        //  No gdwTimoutState for this state
        break;    
    
    case SERVICE_CONTROL_SHUTDOWN:
        pszCommand = "SHUTDOWN";
        dwState = SERVICE_CONTROL_SHUTDOWN;
        //  No gdwTimeoutState for this state
        break;    

    //  Win2k control codes...
    case SERVICE_CONTROL_PARAMCHANGE:
        pszCommand = "PARAMCHANGE";
		dwState = SERVICE_CONTROL_PARAMCHANGE;
        break;    

    case SERVICE_CONTROL_NETBINDADD:
        pszCommand = "NETBINDADD";
		dwState = SERVICE_CONTROL_NETBINDADD;
        break;    

    case SERVICE_CONTROL_NETBINDREMOVE:
        pszCommand = "NETBINDREMOVE";
		dwState = SERVICE_CONTROL_NETBINDREMOVE;
        break;    

    case SERVICE_CONTROL_NETBINDENABLE:
        pszCommand = "NETBINDENABLE";
		dwState = SERVICE_CONTROL_NETBINDENABLE;
        break;    

    case SERVICE_CONTROL_NETBINDDISABLE:
        pszCommand = "NETBINDDISABLE";
		dwState = SERVICE_CONTROL_NETBINDDISABLE;
        break;    


    //  User defined control codes...there are 128 of them
    case SERVICE_CONTROL_USER_DEFINED:
    case SERVICE_CONTROL_USER_DEFINED + 0x01:
    case SERVICE_CONTROL_USER_DEFINED + 0x4f:

    default:
        pszCommand = "DEFAULT";
        dwState = dwControl;
        break;    
    }

    gdwLastControlMessage = dwControl;

#ifdef _DEBUG
    wsprintf( szBuffer, "ServiceHandler: message => %s (0x%0x)", pszCommand, dwControl );
    ALERT( szBuffer );
#endif // _DEBUG

//	TODO:
//	We should set an alarm to for some configurable timeout value so that
//	in case the perl script does not process the request we will change
//	the state automatically	

    if( FALSE != fUseTimer )
    {
        SetTimeoutTimer( gdwHandlerTimeout );
    }
    
    gdwState = dwState;    
    UpdateServiceStatus( gdwState );
} 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetTimeoutTimer( DWORD dwTimeout )
{
    KillTimer();
//    ghTimer = SetTimer( NULL, SERVICE_THREAD_TIMER_ID, gdwHandlerTimeout * DEFAULT_HANDLER_TIMEOUT_SCALE, TimerHandler );
    
    PostThreadMessage( (DWORD) ghServiceThread, WM_USER_SET_TIMER, (WPARAM) dwTimeout * DEFAULT_HANDLER_TIMEOUT_SCALE, (LPARAM) NULL );
/*
    // This block would be great IF we could get the thread message queue created
    // 
    while( ! PostThreadMessage( (DWORD) ghServiceThread, WM_USER_SET_TIMER, (WPARAM) dwTimeout * DEFAULT_HANDLER_TIMEOUT_SCALE, (LPARAM) NULL ) )
    {
        ALERT( "SetTimeoutTimer: Could not post thread message. Sleep and try again." )
        Sleep( 1000 );
    }
*/
#ifdef _DEBUG
    TCHAR szBuffer[ 256 ];
    wsprintf( szBuffer, "Setting timer will value of %d. Timer # is %d", gdwHandlerTimeout, ghTimer );
    ALERT( szBuffer );
#endif // _DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CleanStatusStruct( SERVICE_STATUS *pServiceStatus )
{
    if( NULL != pServiceStatus )
    {
        ZeroMemory( pServiceStatus, sizeof( SERVICE_STATUS ) );

        pServiceStatus->dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
        pServiceStatus->dwControlsAccepted =    SERVICE_ACCEPT_STOP 
                                                | SERVICE_ACCEPT_PAUSE_CONTINUE
                                                | SERVICE_ACCEPT_SHUTDOWN;
        pServiceStatus->dwWin32ExitCode = NO_ERROR;
    }
}           

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID CALLBACK TimerHandler( HWND hWnd, UINT uMsg, UINT uEventId, DWORD dwSystemTime )
{
    ALERT( "HandlerTimeoutTimer callback called." );
    KillTimer();
    gdwState = gdwTimeoutState;
    UpdateServiceStatus( gdwTimeoutState );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL KillTimer()
{
    BOOL fResult = FALSE;

    if( ghTimer )
    {
        ALERT( "Killing timer due to a new pending command" );
        fResult = ::KillTimer( NULL, ghTimer );
        ghTimer = 0;
    }
    return( fResult );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UpdateServiceStatus( DWORD dwState )
{
    BOOL fResult = FALSE;
    SERVICE_STATUS Status;
    CleanStatusStruct( &Status );
    gdwState = Status.dwCurrentState = dwState;

#ifdef _DEBUG
    TCHAR szBuffer[ 256];
    wsprintf( szBuffer, "UpdateServiceStatus: Updating service status to 0x%08x", gdwState );
    ALERT( szBuffer );
#endif // _DEBUG


    fResult = SetServiceStatus( ghService, &Status );
#ifdef _DEBUG
    if( fResult )
    {
        ALERT( "UpdateServiceStatus: update was successful" );
    }
    else
    {
        ALERT( "UpdateServiceStatus: update failed" );
    }
#endif

    return( fResult );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ServiceThread( LPVOID pVoid )
{   
    DWORD dwResult = 0;

    ALERT( "ServiceThread: Starting" );
    ALERT( "ServiceThrad: Calling StartServiceCtrlDispatcher()..." );

    if( FALSE != StartServiceCtrlDispatcher( (LPSERVICE_TABLE_ENTRY) gpServiceTable ) )
    {
        //  Successful
        dwResult = 1;
    }
    else
    {
        dwResult = 0;   
    }

    ALERT( "ServiceThread: Finished with StartServiceCtrlDispatcher()" );

    UpdateServiceStatus( SERVICE_STOPPED );

    ALERT( "ServiceThraed: ENDING THE SERVICE THREAD!!!!!!!!!!" );
//    ExitThread( 0 );
    return( dwResult );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

