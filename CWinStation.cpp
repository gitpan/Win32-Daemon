//  CWinStation.cpp
//  C++ Class to manage Win32 Windows Stations
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  Available under the GNU license.
//  2000.03.19

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "constant.h"
#include "CWinStation.hpp"

////////////////////////////////////////////////////////////////////////////
CWinStation::CWinStation()
{
    m_hWinStation = NULL;
    m_hDesktop = NULL;
    GetThisStation();
}

////////////////////////////////////////////////////////////////////////////
CWinStation::~CWinStation()
{
}

////////////////////////////////////////////////////////////////////////////
void CWinStation::GetThisStation()
{
    // Call GetDesktopWindow() so that a window station and
    // is associated with the service.  THis only is a problem with pre-NT 4.0
    HWND hDesktopWnd = GetDesktopWindow(); 
    _ASSERT( hDesktopWnd );

    m_hWinStation   = GetProcessWindowStation(); 
    _ASSERT( m_hWinStation );
    m_hDesktop      = GetThreadDesktop( GetCurrentThreadId() ); 
    _ASSERT( m_hDesktop );
}

////////////////////////////////////////////////////////////////////////////
BOOL CWinStation::Set( LPCTSTR pszWindowStation, LPCTSTR pszDesktop )
{
    BOOL fResult = FALSE;
    HWINSTA hWinStationBackup = GetProcessWindowStation();
    DWORD dwPerms = MAXIMUM_ALLOWED 
                    | WINSTA_ACCESSCLIPBOARD 
                    | WINSTA_EXITWINDOWS; 
//                    | WINSTA_READATTRIBUTES 
//                    | WINSTA_READSCREEN; 
//                    | WINSTA_WRITEATTRIBUTES;
    HWINSTA hWinStation = OpenWindowStation( (LPTSTR) pszWindowStation, TRUE, dwPerms ); 
#ifdef _DEBUG
    TCHAR szBuffer[256];
    sprintf( szBuffer, "CWinStation::Set: Attempting to open the winstation to '%s'", pszWindowStation );
    ALERT( szBuffer );
#endif // _DEBUG
    if( NULL != hWinStation )  
    { 
        if( SetProcessWindowStation( hWinStation ) )
        {
            HDESK hDesktopBackup = GetThreadDesktop( GetCurrentThreadId() );
#ifdef _DEBUG
            sprintf( szBuffer, "CWinStation::Set: Attempting to open the Desktop to '%s'", pszDesktop );
            ALERT( szBuffer );
#endif // _DEBUG
            //  Get the new desktop: specify inheritance by new processes and max permissions
            DWORD dwPerms = MAXIMUM_ALLOWED 
                            | DESKTOP_SWITCHDESKTOP 
                            | DESKTOP_WRITEOBJECTS 
                            | DESKTOP_CREATEWINDOW
                            | DESKTOP_READOBJECTS;
            HDESK hDesktop = OpenDesktop( (LPTSTR) pszDesktop, 0, TRUE, dwPerms );
            if( NULL != hDesktop )
            {
#ifdef _DEBUG
                ALERT( "CWinStation::Set: Attempting to set the current winstation to previously opened one");
#endif // _DEBUG
                fResult = TRUE;
#ifdef _DEBUG
                ALERT( "CWinStation::Set: Attempting to set the current desktop to previously opened one");
#endif // _DEBUG
                if( TRUE == SetThreadDesktop( hDesktop ) )
                {
#ifdef _DEBUG
                    ALERT( "CWinStation::Set: Attempting to switch to the new desktop");
#endif // _DEBUG
                    fResult = SwitchDesktop( hDesktop );
                }

                if( TRUE == fResult )
                {
                    ALERT( "CWinStation::Set: Successfully set the desktop window & desktop" );
                }
                else
                {
#ifdef _DEBUG
                    sprintf( szBuffer, "CWinStation::Set: FAILED! Win32 Error: 0x%08x  Now resorting to backup versions of window station and desktop", GetLastError() );
                    ALERT( szBuffer );
#endif // _DEBUG
                    SetProcessWindowStation( hWinStationBackup );
                    SetThreadDesktop( hDesktopBackup );
                }
                CloseDesktop( hDesktop );
            }

#ifdef _DEBUG
            else
            {
                sprintf( szBuffer, "CWinStation::Set: FAILED! Win32 Error: 0x%08x", GetLastError() );
                ALERT( szBuffer );
            }
#endif // _DEBUG
            
        }
#ifdef _DEBUG
        else
        {
            sprintf( szBuffer, "CWinStation::Set: FAILED! Win32 Error: 0x%08x", GetLastError() );
            ALERT( szBuffer );
        }
#endif // _DEBUG
        CloseWindowStation( hWinStation );
    } 
#ifdef _DEBUG
    else
    {
        sprintf( szBuffer, "CWinStation::Set: FAILED! Win32 Error: 0x%08x", GetLastError() );
        ALERT( szBuffer );
    }
#endif // _DEBUG

    return( fResult );
}

////////////////////////////////////////////////////////////////////////////
BOOL CWinStation::Restore()
{
    BOOL fResult = FALSE;

    if( FALSE != SetProcessWindowStation( m_hWinStation ) )
    {
        if( FALSE != SetThreadDesktop( m_hDesktop ) )
        {
            fResult = SwitchDesktop( m_hDesktop );
        }
    }
    return( fResult );
}

