//  CWinStation.hpp
//  C++ Class to manage Win32 Windows Stations
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  Available under the GNU license.
//  2000.03.19

#ifndef _CWINSTATION_H_
#define _CWINSTATION_H_

class CWinStation
{
public:
    CWinStation();
    ~CWinStation();
    BOOL Set( LPCTSTR pszWindowStation, LPCTSTR pszDesktop );
    BOOL Restore();

private:
    void GetThisStation();

    HWINSTA m_hWinStation;
    DWORD   m_dwThreadId;
    HDESK   m_hDesktop;

};

#endif // _CWINSTATION_H_