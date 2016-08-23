// appconst.h : application constants
//

#if !defined(AFX_APPCONST_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_APPCONST_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

// *** Remember To Change the Version in this file                     ***
// *** Remember To Change the Version in uImager.rc2                   ***
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
#define MANUAL_ONLINE_PAGE								_T("http://www.contaware.com/contacammanual/")
#else
#define APPNAME_EXT										_T("FreeVimager.exe")
#define APPNAME_NOEXT									_T("FreeVimager")
#define APPNAME_NOEXT_LOW								_T("freevimager")
#define APPNAME_NOEXT_ASCII								"FreeVimager"
#define APPMUTEXNAME									_T("FreeVimagerAppMutex")
#define FAQ_ONLINE_PAGE									_T("http://www.contaware.com/freevimagerfaq/")
#define MANUAL_ONLINE_PAGE								_T("http://www.contaware.com/freevimagermanual/")
#endif
#define APPVERSION										_T("5.1.0beta3")

// Company
#define MYCOMPANY										_T("Contaware")
#define MYCOMPANY_WEB									_T("contaware.com")

// This file contains the configuration files location, the temporary folder location
// and tells the program whether to use the registry or an ini file 
#define MASTERCONFIG_INI_NAME_EXT						_T("MasterConfig.ini")

// Log file
#define LOGNAME_EXT										_T("log.txt")
#define MAX_LOG_FILE_SIZE								268435456 /* 256MB */

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPCONST_H__FFD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)

