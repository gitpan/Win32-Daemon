//  Daemon.cpp
//  Perl stub for the Win32::Daemon Perl extension.
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  Available under the GNU license.
//  2000.03.19

#define WIN32_LEAN_AND_MEAN

#ifdef __BORLANDC__
typedef wchar_t wctype_t; /* in tchar.h, but unavailable unless _UNICODE */
#endif
 
#include <windows.h>
#include <tchar.h>
#include <wtypes.h>
#include <stdio.h>		//	Gurusamy's right, Borland is brain damaged!
#include <math.h>		//	Gurusamy's right, MS is brain damaged!
#include <time.h>

//	Use headers that define the security stuff
#include <lmaccess.h>
#include <lmserver.h>
#include <lmapibuf.h>
#include <LMERR.H>      //  For the NERR_Succes macro


//	If we are building with the core distribution headers we can not define
//	the function names using C++ because of name mangling
#if defined(__cplusplus) && !defined(PERL_OBJECT)
extern "C" {
#endif
    #include "EXTERN.h"
    #include "perl.h"
    #include "XSub.h"

#if defined(__cplusplus) && !defined(PERL_OBJECT)
}
#endif

#define _PERMS_

#include "constant.h"
#include "CWinStation.hpp"
#include "ServiceThread.hpp"
#include "daemon.h"


#define SET_SERVICE_BITS_LIBRARY    TEXT( "AdvApi32.dll" )
#define SET_SERVICE_BITS_FUNCTION   TEXT( "SetServiceBits" )

/*----------------------- M I S C   F U N C T I O N S -------------------*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int My_SetServiceBits( SERVICE_STATUS_HANDLE hService, DWORD dwServiceBits, BOOL bSetBitsOn, BOOL bUpdateImmediately )
{
    int iResult = 0;
    HINSTANCE hInstance;
    DWORD dwError;
    DWORD dwMask = USER_SERVICE_BITS_MASK;      // Default mask. These bits are not reserved by MS. We can use these w/o problems w/MS products.

    hInstance = LoadLibrary( SET_SERVICE_BITS_LIBRARY );
    if( NULL != hInstance )
    {
        typedef BOOL (CALLBACK *fSetServiceBits) ( SERVICE_STATUS_HANDLE hServiceGoo, DWORD dwBits, BOOL bBitsOn, BOOL bUpdate );
        fSetServiceBits pSetServiceBits = NULL;
        pSetServiceBits= (fSetServiceBits) GetProcAddress( hInstance, SET_SERVICE_BITS_FUNCTION );
        if( NULL != pSetServiceBits )
        {
            // Note:
            // We clear all user defined bits before applying our bits. If this sucks
            // then using something like Win32::Lanman. We only want to track our bits
            // and not mess with others.

            //  First clear all user defined bits...
            // My_SetServiceBits() will mask out the user bits from the passed in DWORD.
            (*pSetServiceBits)( ghService, 0xFFFFFFFF, FALSE, FALSE );
    
            // Mask those lovely bits!
            dwServiceBits &= dwMask;
            iResult = (*pSetServiceBits)( hService, dwServiceBits, bSetBitsOn, bUpdateImmediately );
        }
    }

    return( iResult );    
}

/*----------------------- P E R L   F U N C T I O N S -------------------*/
//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_Constant )
{ 
	dXSARGS;
    EXTENSION_VARS;
	char *pszName = NULL;
    LPVOID pBuffer = NULL;
    DWORD dwValue;
    eConstantType eResult;

	if( 2 != items )
	{
		croak( "Usage: " EXTENSION "::Constant( $Name, $Arg )\n" );
    }
	
	pszName = (char*) SvPV( ST( 0 ), na );

	eResult = Constant( pszName, &pBuffer );
    switch( eResult )
    {
        case String:
            sv_setpv( ST( 1 ), (char*) pBuffer );
            break;

        case Numeric:
            sv_setiv( ST( 1 ), (IV) pBuffer );
            break;
    }

        //  Return the result type.
    PUSH_IV( eResult );


    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_StartService )
{
	dXSARGS;
    EXTENSION_VARS;
	
    //    BeginServiceThread()
    ALERT( "Daemon::StartService: About to start the service thread..." );
    
    ghServiceThread = CreateThread( NULL, 0, ServiceThread, 0, 0, &gServiceThreadId );
    
    ALERT( "Daemon::StartService: Thread has been created (falling back to perl)." );

	PUSH_IV( ghServiceThread );

    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_StopService )
{
	dXSARGS;
    EXTENSION_VARS;
    DWORD dwTermStatus;
    BOOL fResult = FALSE;
	
    if( FALSE != GetExitCodeThread( ghServiceThread, &dwTermStatus ) )
    {
        if( STILL_ACTIVE == dwTermStatus )
        {
            UpdateServiceStatus( SERVICE_STOP_PENDING );
            Sleep( 1000 );
            GetExitCodeThread( ghServiceThread, &dwTermStatus );
            if( STILL_ACTIVE == dwTermStatus )
            {
                TerminateThread( ghServiceThread, 0 );
            }
        }
        UpdateServiceStatus( SERVICE_STOPPED );
        fResult = TRUE;
    }

	PUSH_IV( fResult );
	EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_GetVersion )
{
	dXSARGS;
    EXTENSION_VARS;
	
	PUSH_PV( VERSION_STRING );
	
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_CreateService )
{
	dXSARGS;
    EXTENSION_VARS;
    HV *pHv = NULL;
    BOOL fResult = FALSE;
    
    if( 1 != items )
    {
	    croak( "Usage: " EXTENSION "::CreateService( \\%ServiceInfo )\n" );
    }

    if( NULL != ( pHv = EXTRACT_HV( ST( 0 ) ) ) )
    {
        TCHAR szBuffer[ MAX_PATH * 2 ];
        TCHAR szBinaryPath[ 256 ];
        LPTSTR pszServiceName = HASH_GET_PV( pHv, KEYWORD_SERVICE_NAME );
        LPTSTR pszDisplayName = HASH_GET_PV( pHv, KEYWORD_SERVICE_DISPLAY_NAME );
        LPTSTR pszUser =        HASH_GET_PV( pHv, KEYWORD_SERVICE_ACCOUNT_UID );
        LPTSTR pszPwd =         HASH_GET_PV( pHv, KEYWORD_SERVICE_ACCOUNT_PWD );
        LPTSTR pszBinaryPath =  HASH_GET_PV( pHv, KEYWORD_SERVICE_BINARY_PATH );
        LPTSTR pszParameters =  HASH_GET_PV( pHv, KEYWORD_SERVICE_PARAMETERS );
        LPTSTR pszLoadOrder = NULL;
        DWORD  dwTag = 0;
        LPTSTR pszDepend = NULL;
        DWORD dwAccess = SERVICE_ALL_ACCESS;
        DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS;
        DWORD dwStartType = SERVICE_AUTO_START;
        DWORD dwErrorControl = SERVICE_ERROR_IGNORE;
        SC_HANDLE hSc;

        if( _tcschr( pszBinaryPath, ' ' ) )
        {
            _tcscpy( szBinaryPath, "\"" );
            _tcscat( szBinaryPath, pszBinaryPath );
            _tcscat( szBinaryPath, "\"" );
            pszBinaryPath = szBinaryPath;
        }

        if( 0 != _tcscmp( TEXT( "" ), pszParameters ) )
        {
            _tcscpy( szBuffer, pszBinaryPath );
            _tcscat( szBuffer, TEXT( " " ) );
            _tcscat( szBuffer, pszParameters );
            pszBinaryPath = szBuffer;
        }

        if( 0 == _tcscmp( TEXT( "" ), pszUser ) )
        {
            pszUser = NULL;    
            pszPwd = NULL;
        }

        hSc = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if( NULL != hSc )
        {
            SC_LOCK sclLock = LockServiceDatabase( hSc );
            if( NULL != sclLock )
            {
                SC_HANDLE hService = CreateService( hSc,
                                                    pszServiceName,
                                                    pszDisplayName,
                                                    dwAccess,
                                                    dwServiceType,
                                                    dwStartType,
                                                    dwErrorControl,
                                                    pszBinaryPath,
                                                    pszLoadOrder,
                                                    NULL,
                                                    pszDepend,
                                                    pszUser,
                                                    pszPwd );
                if( NULL != hService )
                {
                    fResult = TRUE;
                    CloseServiceHandle( hService );
                }
                else
                {
                    gdwLastError = GetLastError();
                }
                UnlockServiceDatabase( sclLock );
            }
            CloseServiceHandle( hSc );
        }
    }

	PUSH_IV( 0 != fResult );
	
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_DeleteService )
{
	dXSARGS;
    EXTENSION_VARS;
    HV *pHv = NULL;
    BOOL fResult = FALSE;
    LPTSTR pszServiceName = NULL;
    
    if( 1 != items )
    {
	    croak( "Usage: " EXTENSION "::DeleteService( $ServiceName )\n" );
    }

    if( NULL != ( pszServiceName = SvPV( ST( 0 ), na ) ) )
    {
        SC_HANDLE hSc = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if( NULL != hSc )
        {
            SC_LOCK sclLock = LockServiceDatabase( hSc );
            if( NULL != sclLock )
            {
                SC_HANDLE hService = OpenService( hSc, 
                                                    pszServiceName,
                                                    DELETE );
                if( NULL != hService )
                {
                    fResult = DeleteService( hService );
                    CloseServiceHandle( hService );
                }
                UnlockServiceDatabase( sclLock );
            }
            CloseServiceHandle( hSc );
        }
    }

	PUSH_IV( 0 != fResult );
	
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_SetServiceBits )
{
	dXSARGS;
    EXTENSION_VARS;
    BOOL fResult = FALSE;
    DWORD dwBits = 0;
    SERVICE_STATUS_HANDLE hService = ghService;
	
    if( ( 1 > items ) || ( 2 < items ) )
	{
		croak( "Usage: SetServiceBits( $Value, [$hServiceHandle] )\n" );
    }
    
    dwBits = SvIV( ST( 0 ) );

    if( 2 == items )
    {
        hService = SvIV( ST( 1 ) );
    }

    if( NULL != hService )
    {

        // SetServiceBits() for some reason will not link. The link lib
        // is AdvApi32.dll which is in the link list but alas it errors out.
        // So we need to fix this later. For now we hack...load the dll, get the
        // proc then call it.

        //  Now set our bits...
        fResult = My_SetServiceBits( (SERVICE_STATUS_HANDLE) ghService, (DWORD) dwBits,(BOOL) TRUE, (BOOL) TRUE );
    }
    else
    {
        // Set the gdwServiceBits so that when the service is formally running
        // it will set the bits.
        gdwServiceBits = dwBits;
        fResult = 1;
    }
	PUSH_IV( fResult );
    EXTENSION_RETURN;
}


//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_GetLastError )
{
	dXSARGS;
    EXTENSION_VARS;

    PUSH_IV( gdwLastError );
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_State )
{
	dXSARGS;
    EXTENSION_VARS;

    if( 0 != gdwState )
    {
	    if( 1 == items )
	    {
		    DWORD dwState = dwState = SvIV( ST( 0 ) );
            KillTimer();
		    UpdateServiceStatus( dwState );
	    }
    }

    PUSH_IV( gdwState );
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_LastControlMessage )
{
	dXSARGS;
    EXTENSION_VARS;

    PUSH_IV( gdwLastControlMessage );
    EXTENSION_RETURN;
}
	
//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_ShowService )
{
	dXSARGS;
    EXTENSION_VARS;
    BOOL fResult;
    LPCTSTR pszWindowStation = TEXT( "Winsta0" );
    LPCTSTR pszDesktop = TEXT( "Default" );

    if( 3 < items )
    {
		croak( "Usage: ShowService( $WindowStationName, [$DesktopName] )\n" );
    }

    if( items )
    {
        pszWindowStation = SvPV( ST( 0 ), na );
    }
    if( 1 < items )
    {
        pszDesktop = SvPV( ST( 0 ), na );
    }

    // Free your mind...and the current console. :)
    // Do it before the winstation/desktop switch otherwise if you free it later you will 
    // see a brief flash of a console.
    FreeConsole();

    fResult = gWindowStation.Set( pszWindowStation, pszDesktop );
#ifdef _DEBUG
    TCHAR szBuffer[256];
    wsprintf( szBuffer, 
              TEXT( "ShowService: Setting window station %s\\%s resulted in %d" ),
              pszWindowStation,
              pszDesktop,
              fResult );
    ALERT( szBuffer );
#endif

    // Allocate a new console for output.
    AllocConsole();

    PUSH_IV( fResult );
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_HideService )
{
	dXSARGS;
    EXTENSION_VARS;
    BOOL fResult;

    fResult = gWindowStation.Set( TEXT( "Service-x0-3e7$" ), TEXT( "Default" ) );

    PUSH_IV( fResult );
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_RestoreService )
{
	dXSARGS;
    EXTENSION_VARS;
    BOOL fResult;

    fResult = gWindowStation.Restore();

    PUSH_IV( fResult );
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_Timeout )
{
	dXSARGS;
    EXTENSION_VARS;
    DWORD dwResult = 0;

    if( 1 < items )
    {
        croak( TEXT( "Usage: " EXTENSION "::Timeout( [$Timeout] )\n" ) );
    }

    if( items )
    {
        gdwHandlerTimeout = SvIV( ST( 0 ) );
    }
    dwResult = gdwHandlerTimeout;

    PUSH_IV( dwResult );
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( XS_WIN32__Daemon_GetServiceHandle )
{
	dXSARGS;
    EXTENSION_VARS;
    DWORD dwResult = 0;

    if( items )
    {
        croak( TEXT( "Usage: " EXTENSION "::GetServiceHandle()\n" ) );
    }

    PUSH_IV( ghService );
    EXTENSION_RETURN;
}

//////////////////////////////////////////////////////////////////
XS( boot_Win32__Daemon )
{
	dXSARGS;
	LPTSTR file = __FILE__;
	int	retcode = 1;

//    gWindowStation.Set( TEXT( "WinSta0" ), TEXT( "Default" ) );
//  AllocConsole();

	newXS( EXTENSION "::Constant",			XS_WIN32__Daemon_Constant, file );
    newXS( EXTENSION "::StartService",      XS_WIN32__Daemon_StartService, file );
    newXS( EXTENSION "::StopService",       XS_WIN32__Daemon_StopService, file );
    newXS( EXTENSION "::SetServiceBits",    XS_WIN32__Daemon_SetServiceBits, file );
    newXS( EXTENSION "::CreateService",     XS_WIN32__Daemon_CreateService, file );
    newXS( EXTENSION "::DeleteService",     XS_WIN32__Daemon_DeleteService, file );
    newXS( EXTENSION "::GetLastError",      XS_WIN32__Daemon_GetLastError, file );
    newXS( EXTENSION "::State",				XS_WIN32__Daemon_State, file );
    newXS( EXTENSION "::QueryLastMessage",  XS_WIN32__Daemon_LastControlMessage, file );
    newXS( EXTENSION "::ShowService",   	XS_WIN32__Daemon_ShowService, file );
    newXS( EXTENSION "::HideService",       XS_WIN32__Daemon_HideService, file );
    newXS( EXTENSION "::RestoreService",    XS_WIN32__Daemon_RestoreService, file );
    newXS( EXTENSION "::Timeout",           XS_WIN32__Daemon_Timeout, file );
    newXS( EXTENSION "::GetServiceHandle",  XS_WIN32__Daemon_GetServiceHandle, file );

    ST(0) = &sv_yes;
	XSRETURN( retcode );

}			

/* ===============  DLL Specific  Functions  ===================  */

//////////////////////////////////////////////////////////////////			  
BOOL WINAPI DllMain( HINSTANCE  hinstDLL, DWORD fdwReason, LPVOID  lpvReserved )
{
	BOOL	bResult = TRUE;

	switch( fdwReason )
    {
		case DLL_PROCESS_ATTACH:
			ghDLL = hinstDLL;
            CountConstants();
            gdwLastError = 0;
            ghService = 0;  
            ghServiceThread = 0;
            gServiceThreadId = 0;
            gdwServiceBits = 0;
            gdwLastError = 0;

            // Set the state to 0. This way we know when the service thread actually
            // starts because it will set the state to SERVICE_START_PENDING
            gdwState = 0;
            ZeroMemory( &gServiceStatus, sizeof( gServiceStatus ) );

            if( ! GetModuleFileName( ghDLL, gszModulePath, sizeof( gszModulePath ) ) )
            {
                _tcscpy( gszModulePath, TEXT( "" ) );
            }

#ifdef _DEBUG
            ghLogFile = CreateFile( TEXT( "c:\\temp\\perlserv.log" ),
                                            GENERIC_READ | GENERIC_WRITE,
                                            FILE_SHARE_READ,
                                            NULL,
                                            CREATE_ALWAYS,
                                            FILE_ATTRIBUTE_NORMAL,
                                            NULL );
            if( NULL != ghLogFile )
            {
                TCHAR szBuffer[ 256 ];
                struct tm *Time;
                time_t Clock;
                DWORD dwWritten;

                time( &Clock );                 
                Time = localtime( &Clock );  
                _tcscpy( szBuffer, _tasctime( Time ) );
                _tcscat( szBuffer, TEXT( "\n\n" ) );
                WriteFile( ghLogFile, szBuffer, _tcslen( szBuffer ) * sizeof( TCHAR ), &dwWritten, NULL );
            }
#endif // _DEBUG
    		break;

		case DLL_THREAD_ATTACH:
			giThread++;
			break;
			 
		case DLL_THREAD_DETACH:
			giThread--;
			break;
			 	
		case DLL_PROCESS_DETACH:
#ifdef _DEBUG
            if( ghLogFile )
            {
                CloseHandle( ghLogFile );
            }
#endif // _DEBUG
			break;

		default:
			break;
	}
	return ( bResult );
}

