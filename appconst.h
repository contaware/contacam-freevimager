// appconst.h : application constants
//

#if !defined(AFX_APPCONST_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_APPCONST_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// *** Remember To Change the Version in this file                     ***
// *** Remember To Change the Version in uImager.rc2                   ***
// *** Remember To Change the Version in RemoteCam and RemoteCamViewer ***
// *** (version and about dialog resources)                            ***
// *** Remember To Change the Version in the Installer                 ***
// *** Update the History.txt files                                    ***

#ifdef VIDEODEVICEDOC
#define APPNAME_EXT										_T("ContaCam.exe")
#define APPNAME_NOEXT									_T("ContaCam")
#define APPNAME_NOEXT_LOW								_T("contacam")
#define APPNAME_NOEXT_ASCII								"ContaCam"
#define SERVICENAME_EXT									_T("ContaCamService.exe")
#define SERVICENAME_NOEXT								_T("ContaCamService")
#define APPMUTEXNAME									_T("ContaCamAppMutex")
#define BROSERAUTORUN_NAME								_T("ContaCamBrowser")
#define FAQ_ONLINE_PAGE									_T("http://www.contaware.com/contacamfaq/")
#else
#define APPNAME_EXT										_T("FreeVimager.exe")
#define APPNAME_NOEXT									_T("FreeVimager")
#define APPNAME_NOEXT_LOW								_T("freevimager")
#define APPNAME_NOEXT_ASCII								"FreeVimager"
#define APPMUTEXNAME									_T("FreeVimagerAppMutex")
#define FAQ_ONLINE_PAGE									_T("http://www.contaware.com/freevimagerfaq/")
#endif
#define APPVERSION										_T("4.0.9")
#define SLIDESHOWNAME									_T("Start.exe")
#define SLIDESHOWSETTINGSNAME							_T("Settings.xml")
#define NEROBURNNAME									_T("NeroBurn.exe")

// Company
#define MYCOMPANY										_T("Contaware")
#define MYCOMPANY_WEB									_T("contaware.com")

// Trace and Log files
#define LOGNAME_EXT										_T("log.txt")
#define TRACENAME_EXT									_T("trace.txt")
#define LOG_FILE										(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(LOGNAME_EXT))
#define TRACE_FILE										(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(TRACENAME_EXT))
#define MAX_LOG_FILE_SIZE								8388608 /* 8MB */


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPCONST_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)

