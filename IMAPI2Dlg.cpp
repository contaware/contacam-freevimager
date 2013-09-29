// IMAPI2Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "IMAPI2Dlg.h"
#include "SortableFileFind.h"
#include "DiscMaster.h"
#include "DiscFormatData.h"
#include "DiscFormatErase.h"
#include "DiscFormatEraseEvent.h"
#include "DiscRecorder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define CD_MEDIA				0
#define DVD_MEDIA				1
#define DL_DVD_MEDIA			2
#define CLIENT_NAME				_T("BurnSlideshow")
#define WM_BURN_STATUS_MESSAGE	WM_APP + 300
#define WM_BURN_FINISHED		WM_APP + 301

int CIMAPI2Dlg::CIMAPI2DlgThread::Work()
{
    ASSERT(m_pDlg);

	::CoInitialize(NULL);

    // Get the selected recording device from the combobox
    int selectedIndex = m_pDlg->m_cbDrive.GetCurSel();
    ASSERT(selectedIndex >= 0);
    if (selectedIndex < 0)
    {
        m_pDlg->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)_T("Error: No Device Selected"));
		::CoUninitialize();
        return 0;
    }

	// Get original disc recorder
    CDiscRecorder* pOrigDiscRecorder = ((CUImagerApp*)::AfxGetApp())->m_DiscRecorders2[selectedIndex];

    // Did user cancel?
    if (DoExit())
    {
        m_pDlg->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)(LPCTSTR)ML_STRING(1803, "User Canceled!"));
        ::CoUninitialize();
		return 0;
    }

	// Init Message
    m_pDlg->SendMessage(WM_BURN_STATUS_MESSAGE, 0, (LPARAM)(LPCTSTR)ML_STRING(1802, "Initializing Disc Recorder..."));

    // Create another disc recorder because we're in a different thread
    CDiscRecorder discRecorder;
    CString errorMessage;
    if (discRecorder.Initialize(pOrigDiscRecorder->GetUniqueId()))
    {
		// Exclusive access
        if (discRecorder.AcquireExclusiveAccess(true, CLIENT_NAME))
        {
			// Init disc format data
            CDiscFormatData discFormatData;
            if (discFormatData.Initialize(&discRecorder, CLIENT_NAME))
            {
                // Get the media type currently in the recording device
                IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;
                discFormatData.GetInterface()->get_CurrentPhysicalMediaType(&mediaType);

				// Is a media inserted?
				bool bMedia = discFormatData.IsMediaInsert();
				if (!bMedia)
				{
					m_pDlg->SendMessage(WM_BURN_FINISHED, 0, 
						(LPARAM)(LPCTSTR)ML_STRING(1814, "No Media Inserted"));
					discRecorder.EjectMedia();
					::CoUninitialize();
					return 0;
				}

				// Check whether disc is blank, if not try formatting it
				bool bBlank = discFormatData.IsMediaBlank();
				if (!bBlank)
				{
					CDiscFormatErase discFormatErase;
					if (discFormatErase.Initialize(&discRecorder, CLIENT_NAME))
					{
						if (!discFormatErase.Erase(m_pDlg->m_hWnd))
						{
							m_pDlg->SendMessage(WM_BURN_FINISHED, 0, 
								(LPARAM)(LPCTSTR)discFormatErase.GetErrorMessage());
							discRecorder.EjectMedia();
							::CoUninitialize();
							return 0;
						}
					}	
				}

                // Create the file system
                IStream* dataStream = NULL;
                if (!CreateMediaFileSystem(mediaType, &dataStream))
                {	// CreateMediaFileSystem reported error to UI
                    ::CoUninitialize();
					return 0;
                }

				// Close media
                discFormatData.SetCloseMedia(true);

                // Burn the data, this does all the work
                discFormatData.Burn(m_pDlg->m_hWnd, dataStream);

                // Release the IStream after burning
                dataStream->Release();

                // Eject Media when finished
                discRecorder.EjectMedia();
            }

			// Release exclusive access
            discRecorder.ReleaseExclusiveAccess();

            // Finished Burning, GetHresult will determine if it was successful or not
            m_pDlg->SendMessage(WM_BURN_FINISHED, discFormatData.GetHresult(), 
                (LPARAM)(LPCTSTR)discFormatData.GetErrorMessage());
        }
        else
        {
            errorMessage.Format(_T("Failed: %s is exclusive owner"),
                (LPCTSTR)discRecorder.ExclusiveAccessOwner());
            m_pDlg->SendMessage(WM_BURN_FINISHED, discRecorder.GetHresult(), 
                (LPARAM)(LPCTSTR)errorMessage);
        }
    }
    else
    {
        errorMessage.Format(_T("Failed to initialize recorder - Unique ID:%s"),
            (LPCTSTR)pOrigDiscRecorder->GetUniqueId());
        m_pDlg->SendMessage(WM_BURN_FINISHED, discRecorder.GetHresult(), 
            (LPARAM)(LPCTSTR)errorMessage);
    }

    ::CoUninitialize();
	return 0;
}

bool CIMAPI2Dlg:: CIMAPI2DlgThread::CreateMediaFileSystem(IMAPI_MEDIA_PHYSICAL_TYPE mediaType, IStream** ppDataStream)
{
    IFileSystemImage*		image = NULL;
    IFileSystemImageResult*	imageResult = NULL;
    IFsiDirectoryItem*		rootItem = NULL;
    CString					message;
    bool					returnVal = false;
	BSTR					VolumeLabelTemp = NULL;

	// Create File System Image
    HRESULT hr = ::CoCreateInstance(CLSID_MsftFileSystemImage,
        NULL, CLSCTX_ALL, __uuidof(IFileSystemImage), (void**)&image);
    if (FAILED(hr) || (image == NULL))
    {
        m_pDlg->SendMessage(WM_BURN_FINISHED, hr, (LPARAM)_T("Failed to create IFileSystemImage Interface"));
        goto cleanup;
    }
    m_pDlg->SendMessage(WM_BURN_STATUS_MESSAGE, 0, (LPARAM)(LPCTSTR)ML_STRING(1804, "Creating File System..."));
	image->put_FileSystemsToCreate((FsiFileSystems)(FsiFileSystemJoliet|FsiFileSystemISO9660));
	VolumeLabelTemp = m_pDlg->m_sVolumeLabel.AllocSysString();
    image->put_VolumeName(VolumeLabelTemp);
	::SysFreeString(VolumeLabelTemp);
    image->ChooseImageDefaultsForMediaType(mediaType);

    // Get the image root
    hr = image->get_Root(&rootItem);
    if (SUCCEEDED(hr))
    {
        // Add Directories and Files to the File System Image
		CSortableFileFind FileFind;
		CString sDir = m_pDlg->m_sDir;
		sDir.TrimRight(_T('\\'));
		FileFind.Init(sDir + _T("\\*"));
		int pos;
		for (pos = 0 ; pos < FileFind.GetDirsCount() ; pos++)
		{
			BSTR DirNameTemp = FileFind.GetDirName(pos).AllocSysString();
			hr = rootItem->AddTree(DirNameTemp, VARIANT_TRUE);
			::SysFreeString(DirNameTemp);
            if (FAILED(hr))
            {
                if (hr == IMAPI_E_IMAGE_SIZE_LIMIT)
				{
					message = ML_STRING(1805, "Error: No enough space on this media, choose a bigger one!");
					m_pDlg->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)(LPCTSTR)message);
				}
				else
				{
					message.Format(ML_STRING(1806, "Failed to add %s!"), (LPCTSTR)FileFind.GetDirName(pos));
					m_pDlg->SendMessage(WM_BURN_FINISHED, hr, (LPARAM)(LPCTSTR)message);
				}
                goto cleanup;
            }

			// Do Exit Thread?
            if (DoExit())
            {
                m_pDlg->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)(LPCTSTR)ML_STRING(1803, "User Canceled!"));
                hr = IMAPI_E_FSI_INTERNAL_ERROR;
				goto cleanup;
            }
		}
		for (pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
		{
			CString fileName = ::GetShortFileName(FileFind.GetFileName(pos));
            message.Format(ML_STRING(1807, "Adding %s to file system..."), (LPCTSTR)fileName);
            m_pDlg->SendMessage(WM_BURN_STATUS_MESSAGE, 0, (LPARAM)(LPCTSTR)message);

			IStream* pStream = NULL;
			CreateStream(FileFind.GetFileName(pos), &pStream);
            if (pStream)
            {
				BSTR fileNameTemp = fileName.AllocSysString();
                hr = rootItem->AddFile(fileNameTemp, pStream);
				::SysFreeString(fileNameTemp);
                if (FAILED(hr))
                {
                    if (hr == IMAPI_E_IMAGE_SIZE_LIMIT)
					{
						message = ML_STRING(1805, "Error: No enough space on this media, choose a bigger one!");
						m_pDlg->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)(LPCTSTR)message);
					}
					else
					{
						message.Format(ML_STRING(1806, "Failed to add %s!"), (LPCTSTR)fileName);
						m_pDlg->SendMessage(WM_BURN_FINISHED, hr, (LPARAM)(LPCTSTR)message);
					}
					pStream->Release();
                    goto cleanup;
                }
				else
					pStream->Release();
            }

            // Do Exit Thread?
            if (DoExit())
            {
                m_pDlg->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)(LPCTSTR)ML_STRING(1803, "User Canceled!"));
                hr = IMAPI_E_FSI_INTERNAL_ERROR;
				goto cleanup;
            }
		}

		// Create the result image
        if (SUCCEEDED(hr))
        {
            hr = image->CreateResultImage(&imageResult);
            if (SUCCEEDED(hr))
            {
                // Get the stream
                hr = imageResult->get_ImageStream(ppDataStream);
                if (SUCCEEDED(hr))
                    returnVal = true;
                else
                {
                    m_pDlg->SendMessage(WM_BURN_FINISHED, hr, 
                        (LPARAM)_T("Failed IFileSystemImageResult->get_ImageStream!"));
                }

            }
            else
            {
                m_pDlg->SendMessage(WM_BURN_FINISHED, hr, 
                    (LPARAM)_T("Failed IFileSystemImage->CreateResultImage!"));
            }
        }
    }
    else
        m_pDlg->SendMessage(WM_BURN_FINISHED, hr, (LPARAM)_T("Failed IFileSystemImage->getRoot"));

    // Cleanup
cleanup:
    if (image != NULL)
        image->Release();
    if (imageResult != NULL)
        imageResult->Release();
    if (rootItem != NULL)
        rootItem->Release();

    return returnVal;
}

/////////////////////////////////////////////////////////////////////////////
// CIMAPI2Dlg dialog


CIMAPI2Dlg::CIMAPI2Dlg(CWnd* pParent, const CString& sDir)
	: CDialog(CIMAPI2Dlg::IDD, pParent)
, m_cancelBurn(false)
, m_isBurning(false)
, m_prevTotalProgress(-1)
{
	//{{AFX_DATA_INIT(CIMAPI2Dlg)
	//}}AFX_DATA_INIT
	m_sVolumeLabel = CTime::GetCurrentTime().Format(_T("%Y_%m_%d"));
	m_sDir = sDir;
	m_IMAPI2DlgThread.SetDlg(this);
	CDialog::Create(CIMAPI2Dlg::IDD, pParent);
}


void CIMAPI2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIMAPI2Dlg)
	DDX_Control(pDX, IDC_PROGRESS_TEXT, m_ProgressText);
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressCtrl);
	DDX_Control(pDX, IDC_TIME_LEFT, m_TimeLeft);
	DDX_Control(pDX, IDC_ESTIMATED_TIME, m_EstimatedTime);
	DDX_Control(pDX, IDC_COMBO_DRIVE, m_cbDrive);
	DDX_Text(pDX, IDC_EDIT_VOLUME_LABEL, m_sVolumeLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIMAPI2Dlg, CDialog)
	//{{AFX_MSG_MAP(CIMAPI2Dlg)
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	ON_BN_CLICKED(IDC_BURN, OnBurn)
	ON_CBN_SELCHANGE(IDC_COMBO_DRIVE, OnSelchangeComboDrive)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IMAPI_UPDATE, OnImapiUpdate)
    ON_MESSAGE(WM_BURN_STATUS_MESSAGE, OnBurnStatusMessage)
    ON_MESSAGE(WM_BURN_FINISHED, OnBurnFinished)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIMAPI2Dlg message handlers

BOOL CIMAPI2Dlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    AddRecordersToComboBox();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIMAPI2Dlg::AddRecordersToComboBox()
{
    // Cleanup ComboBox
    m_cbDrive.ResetContent();

    // Add Devices to ComboBox
    for (int i = 0 ; i < ((CUImagerApp*)::AfxGetApp())->m_DiscRecorders2.GetSize() ; i++)
    {
        // Get the volume path(s), usually just 1
        CString volumeList;
        ULONG totalVolumePaths = ((CUImagerApp*)::AfxGetApp())->m_DiscRecorders2[i]->GetTotalVolumePaths();
        for (ULONG volIndex = 0; volIndex < totalVolumePaths; volIndex++)
        {
            if (volIndex)
                volumeList += _T(",");
            volumeList += ((CUImagerApp*)::AfxGetApp())->m_DiscRecorders2[i]->GetVolumePath(volIndex);
        }

        // Add String to ComboBox
        CString productId = ((CUImagerApp*)::AfxGetApp())->m_DiscRecorders2[i]->GetProductID();
        CString strName;
        strName.Format(_T("%s [%s]"), (LPCTSTR)volumeList, (LPCTSTR)productId);
        m_cbDrive.AddString(strName);
    }

	// Update ComboBox
    if (((CUImagerApp*)::AfxGetApp())->m_DiscRecorders2.GetSize() > 0)
    {
        m_cbDrive.SetCurSel(0);
        OnSelchangeComboDrive();
    }
}

void CIMAPI2Dlg::OnAbort() 
{
	if (m_isBurning && !m_cancelBurn)
	{
		m_IMAPI2DlgThread.Kill_NoBlocking();
		m_cancelBurn = true;
	}
}

void CIMAPI2Dlg::OnBurn() 
{
	if (!m_isBurning)
	{
		// Init
        m_cancelBurn = false;
        m_isBurning = true;
		m_prevTotalProgress = -1;
		m_ProgressCtrl.SetPos(0);
        UpdateData();
        EnableUI(FALSE);

		// The worker thread sends a closing message that the
		// UI thread processes through the OnBurnFinished() function.
		// This function enables the interface and resets the
		// m_isBurning flag, so we may be here with the worker
		// thread still running -> wait that it finishes!
		m_IMAPI2DlgThread.WaitDone_Blocking(2000);
		m_IMAPI2DlgThread.Start();
    }
}

void CIMAPI2Dlg::EnableUI(BOOL bEnable)
{
    m_cbDrive.EnableWindow(bEnable);
	CButton* pCancel = (CButton*)GetDlgItem(IDCANCEL);
	pCancel->EnableWindow(bEnable);
	if (bEnable)
	{
		pCancel->SetButtonStyle(pCancel->GetButtonStyle() | BS_DEFPUSHBUTTON);
		pCancel->SetFocus();
	}
    GetDlgItem(IDC_EDIT_VOLUME_LABEL)->EnableWindow(bEnable);
    GetDlgItem(IDC_BURN)->EnableWindow(bEnable);
	GetDlgItem(IDC_ABORT)->EnableWindow(!bEnable);
}

void CIMAPI2Dlg::OnSelchangeComboDrive() 
{
	// Clear
	m_isCdromSupported = false;
    m_isDvdSupported = false;
    m_isDualLayerDvdSupported = false;

	// Get selected drive
    int selectedIndex = m_cbDrive.GetCurSel();
    ASSERT(selectedIndex >= 0);
    if (selectedIndex < 0)
        return;

	// Init Disc Format Data
    CDiscFormatData discFormatData;
    if  (!discFormatData.Initialize(((CUImagerApp*)::AfxGetApp())->m_DiscRecorders2[selectedIndex], CLIENT_NAME))
        return;

    // Display Supported Media Types
    CString supportedMediaTypes;
    ULONG totalMediaTypes = discFormatData.GetTotalSupportedMediaTypes();
    for (ULONG volIndex = 0; volIndex < totalMediaTypes; volIndex++)
    {
        int mediaType = discFormatData.GetSupportedMediaType(volIndex);
        if (volIndex > 0)
            supportedMediaTypes += _T(", ");
        supportedMediaTypes += GetMediaTypeString(mediaType);
    }
    TRACE(supportedMediaTypes);
}

CString	CIMAPI2Dlg::GetMediaTypeString(int mediaType)
{
    switch (mediaType)
    {
		case IMAPI_MEDIA_TYPE_UNKNOWN:
		default:
			return _T("Unknown Media Type");

		case IMAPI_MEDIA_TYPE_CDROM:
			m_isCdromSupported = true;
			return _T("CD-ROM or CD-R/RW media");

		case IMAPI_MEDIA_TYPE_CDR:
			m_isCdromSupported = true;
			return _T("CD-R");

		case IMAPI_MEDIA_TYPE_CDRW:
			m_isCdromSupported = true;
			return _T("CD-RW");

		case IMAPI_MEDIA_TYPE_DVDROM:
			m_isDvdSupported = true;
			return _T("DVD ROM");

		case IMAPI_MEDIA_TYPE_DVDRAM:
			m_isDvdSupported = true;
			return _T("DVD-RAM");

		case IMAPI_MEDIA_TYPE_DVDPLUSR:
			m_isDvdSupported = true;
			return _T("DVD+R");

		case IMAPI_MEDIA_TYPE_DVDPLUSRW:
			m_isDvdSupported = true;
			return _T("DVD+RW");

		case IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER:
			m_isDualLayerDvdSupported = true;
			return _T("DVD+R Dual Layer");

		case IMAPI_MEDIA_TYPE_DVDDASHR:
			m_isDvdSupported = true;
			return _T("DVD-R");

		case IMAPI_MEDIA_TYPE_DVDDASHRW:
			m_isDvdSupported = true;
			return _T("DVD-RW");

		case IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER:
			m_isDualLayerDvdSupported = true;
			return _T("DVD-R Dual Layer");

		case IMAPI_MEDIA_TYPE_DISK:
			return _T("random-access writes");

		case IMAPI_MEDIA_TYPE_DVDPLUSRW_DUALLAYER:
			m_isDualLayerDvdSupported = true;
			return _T("DVD+RW DL");

		case IMAPI_MEDIA_TYPE_HDDVDROM:
			return _T("HD DVD-ROM");

		case IMAPI_MEDIA_TYPE_HDDVDR:
			return _T("HD DVD-R");

		case IMAPI_MEDIA_TYPE_HDDVDRAM:
			return _T("HD DVD-RAM");

		case IMAPI_MEDIA_TYPE_BDROM:
			return _T("Blu-ray DVD (BD-ROM)");

		case IMAPI_MEDIA_TYPE_BDR:
			return _T("Blu-ray media");

		case IMAPI_MEDIA_TYPE_BDRE:
			return _T("Blu-ray Rewritable media");
    }
}

BOOL CIMAPI2Dlg::CIMAPI2DlgThread::CreateStream(const CString& sPath, IStream** ppStream)
{
	HRESULT hr = S_FALSE;   
	typedef HRESULT (WINAPI * FPSHCREATESTREAMONFILEEX)(LPCWSTR pszFile,
														DWORD grfMode,
														DWORD dwAttributes,
														BOOL fCreate,
														IStream *pstmTemplate,
														IStream **ppstm);
	FPSHCREATESTREAMONFILEEX fpSHCreateStreamOnFileEx;
	HINSTANCE h = ::LoadLibrary(_T("shlwapi.dll"));
	if (!h)
		return FALSE;
	fpSHCreateStreamOnFileEx = (FPSHCREATESTREAMONFILEEX)::GetProcAddress(h, "SHCreateStreamOnFileEx");
	if (ppStream && fpSHCreateStreamOnFileEx)
	{
		hr = fpSHCreateStreamOnFileEx(	sPath, 
										STGM_READ|STGM_SHARE_DENY_NONE|STGM_DELETEONRELEASE,
										FILE_ATTRIBUTE_NORMAL, 
										FALSE, 
										NULL, 
										ppStream);
	}
	::FreeLibrary(h);
    return (hr == S_OK);
}

LRESULT CIMAPI2Dlg::OnBurnStatusMessage(WPARAM, LPARAM lpMessage)
{
    if (lpMessage != NULL)
        m_ProgressText.SetWindowText((LPCTSTR)lpMessage);
    return 0;
}

LRESULT CIMAPI2Dlg::OnBurnFinished(WPARAM hResult, LPARAM lpMessage)
{
    if (lpMessage != NULL && *((LPCTSTR)lpMessage) != _T('\0'))
    {
        if (SUCCEEDED((HRESULT)hResult))
            m_ProgressText.SetWindowText((LPCTSTR)lpMessage);
        else
		{
            CString text;
            text.Format(ML_STRING(1794, "%s - Error:0x%08X"), (LPCTSTR)lpMessage, hResult);
            m_ProgressText.SetWindowText(text);
        }
    }
    else
    {
        if (SUCCEEDED((HRESULT)hResult))
            m_ProgressText.SetWindowText(ML_STRING(1795, "Burn completed successfully"));
        else
        {
            CString message;
            message.Format(ML_STRING(1796, "Burn failed! Error: 0x%08X"), hResult);
            m_ProgressText.SetWindowText(message);
        }
    }

    EnableUI(TRUE);
	m_isBurning = false;

    return 0;
}

LRESULT CIMAPI2Dlg::OnImapiUpdate(WPARAM wParam, LPARAM lParam)
{
    IMAPI_FORMAT2_DATA_WRITE_ACTION currentAction = 
				(IMAPI_FORMAT2_DATA_WRITE_ACTION)wParam;
    PIMAPI_STATUS pImapiStatus = (PIMAPI_STATUS)lParam;

    switch (currentAction)
    {
		case IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA:
			m_ProgressText.SetWindowText(ML_STRING(1797, "Validating current media..."));
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA:
			m_ProgressText.SetWindowText(ML_STRING(1798, "Formatting media..."));
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE:
			m_ProgressText.SetWindowText(ML_STRING(1799, "Initializing hardware..."));
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER:
			m_ProgressText.SetWindowText(ML_STRING(1800, "Optimizing laser intensity..."));
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA:
			UpdateTimes(pImapiStatus->totalTime, pImapiStatus->remainingTime);
			UpdateProgress(TRUE, pImapiStatus->lastWrittenLba-pImapiStatus->startLba, pImapiStatus->sectorCount);
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION:
			m_ProgressText.SetWindowText(ML_STRING(1801, "Finalizing writing..."));
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED:
			m_ProgressText.SetWindowText(ML_STRING(1795, "Burn completed successfully"));
			break;

		case IMAPI_FORMAT2_ERASE:
			UpdateTimes(pImapiStatus->totalTime, pImapiStatus->remainingTime);
			UpdateProgress(FALSE, pImapiStatus->elapsedTime, pImapiStatus->totalTime);
			break;
    }

    return m_cancelBurn ? RETURN_CANCEL_WRITE : RETURN_CONTINUE;
}

void CIMAPI2Dlg::UpdateTimes(LONG totalTime, LONG remainingTime)
{
    // Set the estimated total time
    CString strText;
    if (totalTime > 0)
        strText.Format(_T("%d:%02d"), totalTime / 60, totalTime % 60);
    else
        strText = _T("0:00");
    m_EstimatedTime.SetWindowText(strText);

    // Set the estimated remaining time
    if (remainingTime > 0)
        strText.Format(_T("%d:%02d"), remainingTime / 60, remainingTime % 60);
    else
        strText = _T("0:00");
    m_TimeLeft.SetWindowText(strText);
}

void CIMAPI2Dlg::UpdateProgress(BOOL bBurn, LONG done, LONG total)
{
	// Init
    if (total != m_prevTotalProgress)
    {
        m_prevTotalProgress = total;
		if (total)
			m_ProgressCtrl.SetRange32(0, total);
		else
			m_ProgressCtrl.SetRange32(0, 1);
    }

	// Set Position
	if (total)
		m_ProgressCtrl.SetPos(MIN(done, total));
	else
		m_ProgressCtrl.SetPos(0);

	// Update Text
	CString text;
	if (bBurn)
	{
		if (done && total)
			text.Format(ML_STRING(1790, "Burn %d%%"), 100 * done / total);
		else
			text = ML_STRING(1791, "Burn...");
	}
	else
	{
		if (done && total)
			text.Format(ML_STRING(1792, "Erase %d%%"), 100 * done / total);
		else
			text = ML_STRING(1793, "Erase...");
	}
    m_ProgressText.SetWindowText(text);
}

BOOL CIMAPI2Dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
			case IDCANCEL:
				DestroyWindow();
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CIMAPI2Dlg::OnClose() 
{
	if (!m_isBurning)
		DestroyWindow();
	else
		::MessageBeep(0xFFFFFFFF);
}

void CIMAPI2Dlg::PostNcDestroy() 
{
	::AfxGetMainFrame()->m_pIMAPI2Dlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}
