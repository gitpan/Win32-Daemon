//  Constant.cpp
//  Constant support for Win32 Perl extensions
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

#include <winspool.h>
#include <LMACCESS.H>
#include <LM.H>
#include <LMAUDIT.H>
#include <LMERR.H>
#include <LMERRLOG.H>

#include "constant.h" 

static DWORD gdwConstTotal = 0;
static ConstantStruct gsConst[] =
{
    { TEXT( "SERVICE_CONTROL_USER_DEFINED" ),   (LPVOID) (SERVICE_CONTROL_USER_DEFINED),    Numeric },
    { TEXT( "SERVICE_STOPPED" ),                (LPVOID) (SERVICE_STOPPED),                 Numeric },
    { TEXT( "SERVICE_RUNNING" ),                (LPVOID) (SERVICE_RUNNING),                 Numeric },
    { TEXT( "SERVICE_PAUSED" ),                 (LPVOID) (SERVICE_PAUSED),                  Numeric },
    { TEXT( "SERVICE_START_PENDING" ),          (LPVOID) (SERVICE_START_PENDING),           Numeric },
    { TEXT( "SERVICE_STOP_PENDING" ),           (LPVOID) (SERVICE_STOP_PENDING),            Numeric },
    { TEXT( "SERVICE_CONTINUE_PENDING" ),       (LPVOID) (SERVICE_CONTINUE_PENDING),        Numeric },
    { TEXT( "SERVICE_PAUSE_PENDING" ),          (LPVOID) (SERVICE_PAUSE_PENDING),           Numeric },
    { TEXT( "SERVICE_INTERROGATE" ),			(LPVOID) (SERVICE_INTERROGATE),				Numeric },

    { TEXT( "SERVICE_CONTROL_SHUTDOWN" ),		(LPVOID) (SERVICE_CONTROL_SHUTDOWN),        Numeric },
    { TEXT( "SERVICE_CONTROL_PARAMCHANGE" ),	(LPVOID) (SERVICE_CONTROL_PARAMCHANGE),     Numeric },
    { TEXT( "SERVICE_CONTROL_NETBINDADD" ),		(LPVOID) (SERVICE_CONTROL_NETBINDADD),      Numeric },
    { TEXT( "SERVICE_CONTROL_NETBINDADD" ),		(LPVOID) (SERVICE_CONTROL_NETBINDADD),      Numeric },
    { TEXT( "SERVICE_CONTROL_NETBINDENABLE" ),	(LPVOID) (SERVICE_CONTROL_NETBINDENABLE),   Numeric },
    { TEXT( "SERVICE_CONTROL_NETBINDDISABLE" ),	(LPVOID) (SERVICE_CONTROL_NETBINDDISABLE),  Numeric },


    //  Service bits available to a script
    { TEXT( "USER_SERVICE_BITS_1" ),			(LPVOID) (USER_SERVICE_BITS_1),				Numeric },
    { TEXT( "USER_SERVICE_BITS_2" ),			(LPVOID) (USER_SERVICE_BITS_2),				Numeric },
    { TEXT( "USER_SERVICE_BITS_3" ),			(LPVOID) (USER_SERVICE_BITS_3),				Numeric },
    { TEXT( "USER_SERVICE_BITS_4" ),			(LPVOID) (USER_SERVICE_BITS_4),				Numeric },
    { TEXT( "USER_SERVICE_BITS_5" ),			(LPVOID) (USER_SERVICE_BITS_5),				Numeric },
    { TEXT( "USER_SERVICE_BITS_6" ),			(LPVOID) (USER_SERVICE_BITS_6),				Numeric },
    { TEXT( "USER_SERVICE_BITS_7" ),			(LPVOID) (USER_SERVICE_BITS_7),				Numeric },
    { TEXT( "USER_SERVICE_BITS_8" ),			(LPVOID) (USER_SERVICE_BITS_8),				Numeric },
    { TEXT( "USER_SERVICE_BITS_9" ),			(LPVOID) (USER_SERVICE_BITS_9),				Numeric },
    { TEXT( "USER_SERVICE_BITS_10" ),		    (LPVOID) (USER_SERVICE_BITS_10),			Numeric },

    // Terminating structure. Leave this here!
    { NULL,                             (LPVOID) NULL,                          Numeric } 
};

eConstantType Constant( LPTSTR pszConstant, LPVOID *ppBuffer )
{
    eConstantType eResult = NotDefined;
    DWORD dwIndex = 0;

    while( NULL != gsConst[ dwIndex ].m_Name )
    {
        if( NULL == gsConst[ dwIndex ].m_Name )
        {
            break;
        }

        if( *pszConstant == *gsConst[ dwIndex ].m_Name )
        {
            int iResult = _tcsicmp( gsConst[ dwIndex ].m_Name, pszConstant );
            if( 0 == iResult )
            {
                *ppBuffer = gsConst[ dwIndex ].m_pBuffer;
                eResult = gsConst[ dwIndex ].m_eType;
                break;
            }
            /*
                //  This code segment is commented out so that we don't run into 
                //  the problem of a constant being out of alpha order hence not
                //  resolving.
            else if( 0 < iResult )
            {
                    //  We have passed the spot where this constant
                    //  *should* have been if it were in alpha order
                break;
            }
            */
        }
        dwIndex++;
    }
    return( eResult );
}

LPTSTR GetConstantName( DWORD dwIndex )
{
    LPTSTR pszBuffer = NULL;
    if( gdwConstTotal > dwIndex )
    {
        pszBuffer = gsConst[ dwIndex ].m_Name;
    }
    return( pszBuffer );
}

DWORD GetTotalConstants()
{
    return( gdwConstTotal );
}

void CountConstants()
{
   gdwConstTotal = 0;
   while( NULL != gsConst[ gdwConstTotal++ ].m_Name ){};  
}

void LogToFile( LPTSTR pszMessage )
{
    if( ghLogFile )
    {
        DWORD dwWritten = 0;
        DWORD dwBufferSize = 0;
        TCHAR szBuffer[ 256 ];
        SYSTEMTIME Time;
        GetLocalTime( &Time );
        wsprintf( szBuffer, TEXT( "[%04d.%02d.%02d %d:%02d:%02d] %s\n" ), 
                Time.wYear,
                Time.wMonth,
                Time.wDay,
                Time.wHour,
                Time.wMinute,
                Time.wSecond,
                (LPTSTR) pszMessage ); 

        dwBufferSize = lstrlen( szBuffer );
        WriteFile( ghLogFile, 
                    (LPCVOID) (szBuffer),
                    dwBufferSize, 
                    &dwWritten, 
                    NULL );
    }
}
