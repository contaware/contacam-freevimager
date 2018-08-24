// CreditsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "CreditsDlg.h"
extern "C"
{
#ifdef VIDEODEVICEDOC
#include "libavutil/ffversion.h"
#endif
#include "jversion.h"
#include "tiffio.h"
#include "png.h"
#include "zlib.h"
#include "gif_lib.h"
#include "icc.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg dialog


CCreditsDlg::CCreditsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreditsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreditsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCreditsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreditsDlg)
	DDX_Control(pDX, IDC_EDIT_CREDITS, m_Credits);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreditsDlg, CDialog)
	//{{AFX_MSG_MAP(CCreditsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg message handlers

BOOL CCreditsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

#ifdef VIDEODEVICEDOC
	// ffmpeg
	CString sffmpeg = _T("ffmpeg ") + CString(FFMPEG_VERSION);
	sffmpeg += CString(_T('\n')) + _T("http://www.ffmpeg.org");

	// Smtp
	CString ssmtp(_T("mailsend"));
	ssmtp += CString(_T('\n')) + _T("http://github.com/muquit/mailsend");
#endif

	// CMDITabs
	CString smditabs(_T("CMDITabs by Christian Rodemeyer"));
	smditabs += CString(_T('\n')) + _T("http://www.codeproject.com/articles/1438/cmditabs");

	// CToasterWnd v1.18
	CString stoaster(_T("CToasterWnd v1.18"));
	stoaster += CString(_T('\n')) + _T("http://www.naughter.com");

	// AVIR
	CString savir(_T("AVIR image resizing algorithm designed by\nAleksey Vaneev version 1.9"));
	savir += CString(_T('\n')) + _T("http://github.com/avaneev/avir");

	// libjpeg
	CString slibjpeg = _T("libjpeg ") + CString(JVERSION);
	slibjpeg += CString(_T('\n')) + _T("http://www.ijg.org");

	// libtiff
	CString slibtiff(::TIFFGetVersion());
	int i = slibtiff.Find(_T('\n'));
	if (i >= 0)
		slibtiff = slibtiff.Left(i);
	slibtiff += CString(_T('\n')) + _T("http://www.libtiff.org");

	// libpng
	CString slibpng(::png_get_header_version(NULL));
	slibpng.Trim();
	slibpng += CString(_T('\n')) + _T("http://www.libpng.org");

	// zlib
	CString szlib = _T("zlib ") + CString(::zlibVersion());
	szlib += CString(_T('\n')) + _T("http://www.zlib.net");

	// giflib
	CString sgiflib(GIF_LIB_VERSION);
	sgiflib.Trim();
	sgiflib.TrimRight(_T(','));
	sgiflib = _T("giflib ") + sgiflib;
	sgiflib += CString(_T('\n')) + _T("http://sourceforge.net/projects/giflib");

	// icclib
	CString sicclib = _T("icclib ") + CString(ICCLIB_VERSION_STR);
	sicclib += CString(_T('\n')) + _T("http://www.argyllcms.com/icclibsrc.html");

	// unicode nsis
	CString snsis(_T("Unicode NSIS"));
	snsis += CString(_T('\n')) + _T("http://www.scratchpaper.com");

	// Output credits
	CString sCredits =	
#ifdef VIDEODEVICEDOC
						sffmpeg		+ _T("\n\n") +
						ssmtp		+ _T("\n\n") +
#endif
						smditabs	+ _T("\n\n") +
						stoaster	+ _T("\n\n") +
						savir		+ _T("\n\n") +
						slibjpeg	+ _T("\n\n") +
						slibtiff	+ _T("\n\n") +
						slibpng		+ _T("\n\n") +
						szlib		+ _T("\n\n") +
						sgiflib		+ _T("\n\n") +
						sicclib		+ _T("\n\n") +
						snsis;
	m_Credits.SetWindowText(sCredits);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
