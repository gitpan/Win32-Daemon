//  Daemon.cpp
//  Header for the Win32::Daemon Perl extension.
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  Available under the GNU license.
//  2000.03.19

#include "Win32Perl.h"
 
#ifndef _DAEMON_H
#   define _DAEMON_H 

#   ifndef WIN32
#       ifdef _WIN32
#           define WIN32   
#       endif // _WIN32  
#   endif // WIN32  
  
///////////////////////////////////////////////////////////////////////////////////////////
//	Begin resource compiler macro block

    //  Include the version information...
    #include "version.h"

    #define	EXTENSION_NAME			"Daemon"

    #define	EXTENSION_PARENT_NAMESPACE		"Win32"
    #define EXTENSION				EXTENSION_PARENT_NAMESPACE "::" EXTENSION_NAME
    
    #define	EXTENSION_FILE_NAME		EXTENSION_NAME

    #define	EXTENSION_VERSION		VERSION
    #define	EXTENSION_AUTHOR		"Dave Roth <rothd@roth.net>"

    #define	COPYRIGHT_YEAR			"2000"
    #define	COPYRIGHT_NOTICE		"Copyright (c) " COPYRIGHT_YEAR

    #define COMPANY_NAME			"Roth Consulting\r\nhttp://www.roth.net/consult"

    #define	VERSION_TYPE			"Beta"
//	End resource compiler macro block
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//  These are members that will be defined in the blessed Perl Object.
//  These are only intended to be used by this extension (and maybe it's
//  brethern). I would not suggest other applications rely on this since it
//  may change.
#define KEYWORD_SERVICE_NAME            TEXT( "name" )
#define KEYWORD_SERVICE_DISPLAY_NAME    TEXT( "display" )
#define KEYWORD_SERVICE_BINARY_PATH     TEXT( "path" )
#define KEYWORD_SERVICE_ACCOUNT_UID     TEXT( "user" )
#define KEYWORD_SERVICE_ACCOUNT_PWD     TEXT( "password" )
#define KEYWORD_SERVICE_PARAMETERS      TEXT( "parameters" )


///////////////////////////////////////////////////////////////////////////////
//  Define a method to report exceptions
#ifdef DEBUG
    #define REPORT_EXCEPTION    _tprintf( TEXT( "Error! An Exception has been caught.\n" ) )
#else   //  DEBUG
    #define REPORT_EXCEPTION    
#endif  //  DEBUG

HINSTANCE	ghDLL = 0;
int		giThread = 0;
int		iTheList = 0;
TCHAR   gszModulePath[ MAX_PATH ];
SERVICE_STATUS_HANDLE ghService;  
SERVICE_STATUS gServiceStatus;
HANDLE  ghServiceThread;
DWORD   gServiceThreadId;
DWORD   gdwServiceBits;
DWORD   gdwLastError;
DWORD   gdwState = 0;
DWORD   gdwTimeoutState = SERVICE_START_PENDING;
DWORD   ghTimer = 0;
DWORD   gdwHandlerTimeout = DEFAULT_HANDLER_TIMEOUT_VALUE;
DWORD   gdwLastControlMessage = SERVICE_START_PENDING;
static  CWinStation gWindowStation;

HANDLE  ghLogFile = NULL;


#endif // _DAEMON_H

