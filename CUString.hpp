//  CUString.hpp
//  C++ Class to manage Win32 Windows Stations
//  (c) Dave Roth
//  Courtesy of Roth Consulting
//  http://www.roth.net/
//
//  
//  2000.03.19

#ifndef _CUString_H_
#define _CUString_H_

#define TYPE_ANSI               0
#define TYPE_UNICODE            1

// How much extra do we allocate per buffer?
#define DEFAULT_BUFFER_OVERHEAD 0x10    
#define DEFAULT_BUFFER_SIZE     0x80

class CUString
{
    public:
        CUString( DWORD dwLength = DEFAULT_BUFFER_SIZE );
        CUString( LPSTR pszString );
        CUString( LPWSTR pszString );
        ~CUString();

        operator LPSTR ();
        operator LPWSTR ();
        operator LPCSTR ();
        operator LPCWSTR ();
        operator PVOID ();

        operator = ( CUString &szuString );
        operator = ( LPSTR pszaString );
        operator = ( LPCSTR pszaString );
        operator = ( LPWSTR pszwString );    
        operator = ( LPCWSTR pszwString );    

    protected:
        static DWORD m_sdwInstance;

    private:
        void Init();
        BOOL AllocBuffer( BOOL bAnsiOrUnicode, DWORD dwLength );
        void UpdateAnsi( LPWSTR pszString );
        void UpdateUnicode( LPSTR pszString );
        void EmptyBuffers();
        void DeleteBuffers( int iBufferType = -1 );
        void DeleteBuffer( PVOID pBuffer );
#ifdef _DEBUG
        void CUString::LOG_DEBUG( LPTSTR pszText );
#endif  //  _DEBUG

        BOOL    m_fDirtyAnsi;
        BOOL    m_fDirtyUnicode;
        BOOL    m_fIsNull;
        DWORD   m_dwLength;
        DWORD   m_dwLengthAllocated;
        LPSTR   m_pszaBuffer;
        LPWSTR  m_pszwBuffer;

};

////////////////////////////////////////////////////////
inline CUString::operator = ( LPWSTR pszString )
{
    operator=( (LPCWSTR) pszString );
}

////////////////////////////////////////////////////////
inline CUString::operator = ( LPSTR pszString )
{
    operator=( (LPCSTR) pszString );
}

////////////////////////////////////////////////////////
inline CUString::operator LPCSTR ()
{
    return( (LPCSTR) operator LPSTR () );
}

////////////////////////////////////////////////////////
inline CUString::operator LPCWSTR ()
{
    return( (LPCWSTR) operator LPWSTR () );
}

#endif  //  _CUString_H_

