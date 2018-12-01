// CBatchProcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "PictureDoc.h"
#include "BatchProcDlg.h"
#include "PreviewFileDlg.h"
#include "Dib.h"
#include "BrowseDlg.h"
#include "SortableFileFind.h"
#include "SaveFileDlg.h"
#include "Tiff2Pdf.h"
#include "NoVistaFileDlg.h"
#include "GetDirContentSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CBatchProcDlg::CProcessThread::Work()
{
	ASSERT(m_pDlg);

	int pos;
	CSortableFileFind FileFind;
	int nFilesCount;
	CString sTempFileName(_T(""));
	CString sSrcFileName(_T(""));
	CString sSrcDirPath(_T(""));
	CString sDstFileName(_T(""));
	CString sDstFileNameSameExt(_T(""));
	CString sTempDstDirPath(_T(""));
	CString sOrigDstDirPath(_T(""));

	::CoInitialize(NULL);

	// Temp Destination Directory Path
	sTempDstDirPath = m_szTempDstDirPath;
	sTempDstDirPath.TrimRight(_T('\\'));

	// Original Destination Directory Path
	sOrigDstDirPath = m_szOrigDstDirPath;
	sOrigDstDirPath.TrimRight(_T('\\'));

	try
	{
		// Init File Find For Src Directory
		if (m_pDlg->m_nInputSelection == INPUT_DIR)
		{
			// Source Directory Path
			sSrcDirPath = m_pDlg->m_sSrc;
			sSrcDirPath.TrimRight(_T('\\'));

			// Init File Find
			if (m_pDlg->m_bRecursive)
			{
				if (!FileFind.InitRecursive(sSrcDirPath + _T("\\*")))
					throw (int)0;
				// Wait that the thread has finished
				if (FileFind.WaitRecursiveDone(GetKillEvent()) != 1)
					throw (int)0;
			}
			else
			{
				if (!FileFind.Init(sSrcDirPath + _T("\\*")))
					throw (int)0;
			}
		}

		// Create Dir Structure
		if (m_pDlg->m_nInputSelection == INPUT_DIR && m_pDlg->m_bRecursive)
		{
			for (pos = 0 ; pos < FileFind.GetDirsCount() ; pos++)
			{
				// Do Exit?
				if (DoExit())
					throw (int)0;

				// Source Directory
				CString sSrcDir = FileFind.GetDirName(pos);
				sSrcDir.TrimRight(_T('\\'));

				// Destination Directory
				CString sDstDir = sSrcDir;
				sDstDir = sDstDir.Mid(sSrcDirPath.GetLength() + 1);
				sDstDir = sTempDstDirPath + _T("\\") + sDstDir;
				if (!::CreateDir(sDstDir))
				{
					::ShowErrorMsg(::GetLastError(), TRUE);
					throw (int)0;
				}
			}
		}

		// Get files count and make sure that all the thumbs
		// have been fully loaded, otherwise if input file
		// and output file are the same the thumb load fails!
		if (m_pDlg->m_nInputSelection == INPUT_DIR)
		{
			nFilesCount = FileFind.GetFilesCount();
			for (int nItem = 0 ; nItem < m_pDlg->m_List.GetItemCount() ; nItem++)
			{
				CListElement* pListElement = (CListElement*)m_pDlg->m_List.GetItemData(nItem);
				if (pListElement)
				{
					while (	!pListElement->m_DibStatic.HasLoadHdrStarted()		||
							!pListElement->m_DibStatic.HasLoadHdrTerminated()	||
							!pListElement->m_DibStatic.HasLoadFullStarted()		||
							!pListElement->m_DibStatic.HasLoadFullTerminated())
					{
						::Sleep(THUMBLOAD_POLL_TIME);
						if (DoExit())
							throw (int)0;
					}
				}
			}
		}
		else
		{
			// The thumb load done check is performed
			// inside the following for-loop
			nFilesCount = m_pDlg->m_List.GetItemCount();
		}

		// Open Output File
		if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
			OpenOutputFile(nFilesCount);

		// Set first Output File flag
		m_bFirstOutputFile = TRUE;

		// Loop Through All Found Files
		int num = 1;
		for (pos = 0 ; pos < nFilesCount ; pos++)
		{
			// Do Exit?
			if (DoExit())
				throw (int)0;

			// Source File Name and Source Dir Path
			if (m_pDlg->m_nInputSelection == INPUT_DIR)
			{
				sSrcFileName = FileFind.GetFileName(pos);
				// Source Dir Path has been initialized above!
			}
			else
			{
				// Get File Name
				sSrcFileName =	m_pDlg->m_List.GetItemText(pos, LIST_PATH) +
								m_pDlg->m_List.GetItemText(pos, LIST_FILENAME);
				sSrcDirPath =	::GetDriveAndDirName(sSrcFileName);
				sSrcDirPath.TrimRight(_T('\\'));

				// Make sure that the thumb has been fully loaded,
				// otherwise if input file and output file are the
				// same the thumb load fails!
				CListElement* pListElement = (CListElement*)m_pDlg->m_List.GetItemData(pos);
				if (pListElement)
				{
					while (	!pListElement->m_DibStatic.HasLoadHdrStarted()		||
							!pListElement->m_DibStatic.HasLoadHdrTerminated()	||
							!pListElement->m_DibStatic.HasLoadFullStarted()		||
							!pListElement->m_DibStatic.HasLoadFullTerminated())
					{
						::Sleep(THUMBLOAD_POLL_TIME);
						if (DoExit())
							throw (int)0;
					}
				}
			}
			sSrcFileName.TrimRight(_T('\\'));

			// Skip the System's Thumbs.db File
			if (::GetShortFileName(sSrcFileName).CompareNoCase(THUMBS_DB) == 0)
				continue;

			// Destination File Name and Extension
			sDstFileName = sSrcFileName;
			sDstFileName = sDstFileName.Mid(sSrcDirPath.GetLength() + 1);
			sDstFileNameSameExt = sDstFileName = sTempDstDirPath + _T("\\") + sDstFileName;
			CString sDstExt = ::GetFileExt(sDstFileNameSameExt);
			if (m_pDlg->m_bConversion &&
				!(m_pDlg->m_nOutputSelection == OUTPUT_FILE && ::GetFileExt(m_pDlg->m_sOutputFileName) != _T(".zip")))
			{
				if (m_pDlg->m_nOptimizationSelection == AUTO_OPT)
				{
					sDstExt = CUImagerApp::ShrinkGetDstExt(::GetFileExt(sSrcFileName));
					sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
				}
				else
				{
					switch (m_pDlg->m_GeneralTab.m_nExtChangeType)
					{
						case CBatchProcGeneralTab::NO_CHANGE :
							sDstFileName = sDstFileNameSameExt;
							break;

						case CBatchProcGeneralTab::AUTO_CHANGE :
							sDstExt = CUImagerApp::ShrinkGetDstExt(::GetFileExt(sSrcFileName));
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;

						case CBatchProcGeneralTab::ALL_JPEG :
							sDstExt = _T(".jpg");
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;

						case CBatchProcGeneralTab::ALL_PNG :
							sDstExt = _T(".png");
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;

						case CBatchProcGeneralTab::ALL_GIF :
							sDstExt = _T(".gif");
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;

						case CBatchProcGeneralTab::ALL_TIFF :
							sDstExt = _T(".tif");
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;

						case CBatchProcGeneralTab::ALL_BMP :
							sDstExt = _T(".bmp");
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;

						case CBatchProcGeneralTab::ALL_PCX :
							sDstExt = _T(".pcx");
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;

						case CBatchProcGeneralTab::ALL_EMF :
							sDstExt = _T(".emf");
							sDstFileName = ::GetFileNameNoExt(sDstFileName) + sDstExt;
							break;
					
						default :
							break;
					}
				}
			}
			else
				sDstFileName = sDstFileNameSameExt;

			// Prepare rename strings
			if (m_pDlg->m_bRename)
			{
				DoRename(	num,
							sSrcFileName,
							sDstFileName,
							sDstFileNameSameExt);
			}

			// Copy File?
			if (m_pDlg->m_bRename && !m_pDlg->m_bConversion)
			{
				// Copy File
				if (Copy(sSrcFileName, sDstFileNameSameExt))
				{
					// Add File
					if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
					{
						AddToOutputFile(nFilesCount, sTempDstDirPath, sDstFileNameSameExt);
						m_pDlg->UpdateDstFileSize();
					}

					// Delete Input File
					if (m_bDeleteInputFile &&
						(sSrcDirPath.CompareNoCase(sOrigDstDirPath) == 0))
						::DeleteToRecycleBin(sSrcFileName);
				}

				// Progress
				Progress(pos, nFilesCount);

				continue;
			}

			// Conversion
			if (CUImagerApp::IsSupportedPictureFile(sSrcFileName))
			{
				// -1 : Just Copied
				// 0  : Error
				// 1  : Shrinked
				int nShrinkPictureRes;
				if ((nShrinkPictureRes = CUImagerApp::ShrinkPicture(sSrcFileName,
																	sDstFileName,
																	m_dwMaxSize,
																	m_bMaxSizePercent,
																	::IsJPEG(sDstFileName) ?
																	m_nJpegQuality :
																	m_nTiffJpegQuality,
																	m_bForceJpegQuality,
																	m_nTiffCompression,
																	m_bTiffForceCompression,
																	m_bShrinkPictures,
																	m_bSharpen,
																	m_bWorkOnAllPages,
																	NULL,
																	FALSE,
																	this)) == 0)
				{
					// Copy File
					if (Copy(sSrcFileName, sDstFileNameSameExt))
					{
						// Add File
						if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
						{
							AddToOutputFile(nFilesCount, sTempDstDirPath, sDstFileNameSameExt);
							m_pDlg->UpdateDstFileSize();
						}
					}
				}
				else
				{
					// Special Processing for Jpeg Files
					if (::IsJPEG(sDstFileName))
					{
						JpegProcessing(	(nShrinkPictureRes == 1) ? TRUE : FALSE,
										sDstFileName,
										sTempFileName);
					}

					// Special Processing for Tiff Files
					if (::IsTIFF(sDstFileName))
					{
						TiffProcessing(sDstFileName);
					}

					// Add File
					if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
					{
						AddToOutputFile(nFilesCount, sTempDstDirPath, sDstFileName);
						m_pDlg->UpdateDstFileSize();
					}
				}
			}
			else
			{
				// Copy File
				if (Copy(sSrcFileName, sDstFileNameSameExt))
				{
					// Add File
					if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
					{
						AddToOutputFile(nFilesCount, sTempDstDirPath, sDstFileNameSameExt);
						m_pDlg->UpdateDstFileSize();
					}
				}
			}

			// Progress
			Progress(pos, nFilesCount);
		}

		// Do Close Output
		if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
		{
			CloseOutputFile(false);
			m_pDlg->UpdateDstFileSize();
		}
		// Move from temp destination to original destination directory:
		// overwrite if target file exists and fail on copy error
		else if (!::MergeDirContent(sTempDstDirPath, sOrigDstDirPath, TRUE, FALSE))
		{
			::ShowErrorMsg(::GetLastError(), TRUE);
			throw (int)0;
		}

		// OK
		return OnExit(TRUE);
	}
	catch (int)
	{
		if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
		{
			CloseOutputFile(true);
			::DeleteFile(m_pDlg->m_sOutputFileName);
		}
		return OnExit(FALSE);
	}
	catch (CZipException* e)
	{
		e->ReportZipError();
		e->Delete();
		if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
		{
			CloseOutputFile(true);
			::DeleteFile(m_pDlg->m_sOutputFileName);
		}
		return OnExit(FALSE);
	}
	catch (CFileException* e)
	{
		e->ReportError(MB_ICONSTOP);
		e->Delete();
		if (sTempFileName != _T(""))
			::DeleteFile(sTempFileName);
		if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
		{
			CloseOutputFile(true);
			::DeleteFile(m_pDlg->m_sOutputFileName);
		}
		return OnExit(FALSE);
		
	}
	catch (CException* e)
	{
		e->ReportError(MB_ICONSTOP);
		e->Delete();
		if (m_pDlg->m_nOutputSelection == OUTPUT_FILE)
		{
			CloseOutputFile(true);
			::DeleteFile(m_pDlg->m_sOutputFileName);
		}
		return OnExit(FALSE);
	}
}

void CBatchProcDlg::CProcessThread::OpenOutputFile(int nFilesCount)
{			
	if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".zip"))
	{
		((CUImagerApp*)::AfxGetApp())->m_Zip.Open(m_pDlg->m_sOutputFileName, CZipArchive::create, 0);
	}
	else if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".gif"))
	{
		m_pAnimGifSave = new CAnimGifSave (	m_pDlg->m_sOutputFileName,
											((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
											TRUE,		// Dither Color Conversion
											255,		// Max Colors
											0,			// PlayTimes = 0: Infinite
														// PlayTimes = 1: Show All Frames One Time
														// PlayTimes = 2: Show All Frames Two Times
											RGB(0,0,0));// Black Background Color
		if (!m_pAnimGifSave)
			throw (int)0;
	}
	else if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".pdf"))
	{
		m_sTempOutputFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
													::GetFileNameNoExt(m_pDlg->m_sOutputFileName) + _T(".tif"));
	}
}

void CBatchProcDlg::CProcessThread::CloseOutputFile(bool bException)
{
	if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".zip"))
	{
		((CUImagerApp*)::AfxGetApp())->m_Zip.Close(bException);
	}
	else if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".gif"))
	{
		if (m_pAnimGifSave)
		{
			delete m_pAnimGifSave;
			m_pAnimGifSave = NULL;
		}
	}
	else if (::IsTIFF(m_pDlg->m_sOutputFileName))
	{
		if (m_TiffOutFile)
		{
			::TIFFClose(m_TiffOutFile);
			m_TiffOutFile = NULL;
		}
	}
	else if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".pdf"))
	{
		if (m_TiffOutFile)
		{
			::TIFFClose(m_TiffOutFile);
			m_TiffOutFile = NULL;
		}
		if (!bException)
		{
			if (!::Tiff2Pdf(m_sTempOutputFileName,
							m_pDlg->m_sOutputFileName,
							m_sPdfPaperSize,
							TRUE,						// Fit Window
							TRUE,						// Interpolate
							m_nTiffJpegQuality))
				::AfxMessageBox(ML_STRING(1357, "Error while converting to Pdf"), MB_ICONSTOP);
		}
		::DeleteFile(m_sTempOutputFileName);
	}
}

void CBatchProcDlg::CProcessThread::AddToOutputFile(int nFilesCount,
													CString sDirPath,
													CString sFileName)
{
	if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".zip"))
	{
		if (m_pDlg->m_bRecursive)
		{
			CString sStoredFileName = sFileName;
			sStoredFileName = sStoredFileName.Mid(sDirPath.GetLength() + 1);
			if (!((CUImagerApp*)::AfxGetApp())->m_Zip.AddNewFile(sFileName, sStoredFileName))
				throw (int)0;
		}
		else
		{
			if (!((CUImagerApp*)::AfxGetApp())->m_Zip.AddNewFile(sFileName, false)) // Do not store path
				throw (int)0;
		}
	}
	else 
	{
		// Check whether it is a supported picture file:
		// do not throw an error, skip it.
		if (!CUImagerApp::IsSupportedPictureFile(sFileName))
			return; 
		
		if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".gif"))
		{
			// Load Image
			if (!m_Dib.LoadImage(	sFileName,
									0,		// X Full Size Load
									0,		// Y Full Size Load
									0,		// Load Page 0
									TRUE,	// Decompress Bmps
									FALSE,	// Not Only Header
									NULL,	// No Progress Wnd
									FALSE,	// No Progress Send
									this))	// Thread
				throw (int)0;

			// Auto Orientate
			CDib::AutoOrientateDib(&m_Dib);
			
			// Clear Orientation
			m_Dib.GetExifInfo()->Orientation = 1;

			// Flatten
			if (m_Dib.HasAlpha() && m_Dib.GetBitCount() == 32)
			{
				m_Dib.SetBackgroundColor(RGB(0,0,0)); // Black background for GIFs is ok
				m_Dib.RenderAlphaWithSrcBackground();
				m_Dib.SetAlpha(FALSE);
			}

			// Save Frame
			if (!m_pAnimGifSave->SaveAsAnimGIF(&m_Dib, ::Round(1000.0 / m_dFrameRate)))
				throw (int)0;
		}
		else if (::IsTIFF(m_pDlg->m_sOutputFileName))
		{
			AddToOutputTiff(nFilesCount,
							sFileName,
							m_pDlg->m_sOutputFileName);
		}
		else if (::GetFileExt(m_pDlg->m_sOutputFileName) == _T(".pdf"))
		{
			AddToOutputPdf(	nFilesCount,
							sFileName,
							m_sTempOutputFileName);
		}
	}
	m_bFirstOutputFile = FALSE;
}

void CBatchProcDlg::CProcessThread::AddToOutputTiff(int nFilesCount,
													CString sInFileName,
													CString sOutFileName)
{
	if (::IsTIFF(sInFileName))
	{
		if (m_bTiffForceCompression)
		{
			if (m_bFirstOutputFile)
			{
				m_TiffOutFile = TIFFOpenW(sOutFileName, "w");    
				if (!m_TiffOutFile)
					throw (int)0;
			}
			if (!CDib::TIFFCopyAllPages(sInFileName,
										m_TiffOutFile,
										0, // We do not know the page number
										0, // We do not know how many pages
										m_bWorkOnAllPages ? -1 : 0,
										m_nTiffCompression,
										m_nTiffJpegQuality))
				throw (int)0;
		}
		else
		{
			if (nFilesCount == 1)
			{
				if (!::CopyFile(sInFileName, sOutFileName, FALSE))
					throw (int)0;
			}
			else if (nFilesCount > 1)
			{
				if (m_bFirstOutputFile)
				{
					m_TiffOutFile = TIFFOpenW(sOutFileName, "w");    
					if (!m_TiffOutFile)
						throw (int)0;
				}
				if (!CDib::TIFFCopyAllPages(sInFileName, m_TiffOutFile))
					throw (int)0;
			}
		}
	}
	else
	{
		// Load Image
		if (!m_Dib.LoadImage(	sInFileName,
								0,		// X Full Size Load
								0,		// Y Full Size Load
								0,		// Load Page 0
								TRUE,	// Decompress Bmps
								FALSE,	// Not Only Header
								NULL,	// No Progress Wnd
								FALSE,	// No Progress Send
								this))	// Thread
			throw (int)0;

		// Auto Orientate
		CDib::AutoOrientateDib(&m_Dib);
			
		// Clear Orientation
		m_Dib.GetExifInfo()->Orientation = 1;

		// Decide about the Compression Type
		int nCompression = m_nTiffCompression;
		if (::IsTIFF(sInFileName) && !m_bTiffForceCompression)
			nCompression = m_Dib.m_FileInfo.m_nCompression;

		// Save Multi-Page TIFF
		if (nFilesCount == 1)
		{
			if (!m_Dib.SaveTIFF(sOutFileName,
								nCompression,	
								m_nTiffJpegQuality,
								NULL,
								FALSE,
								this))
				throw (int)0;
		}
		else if (nFilesCount > 1)
		{
			// Save first TIFF page
			if (m_bFirstOutputFile)
			{
				if (!m_Dib.SaveFirstTIFF(sOutFileName,
										&m_TiffOutFile,
										0, // We do not know how many pages
										nCompression,
										m_nTiffJpegQuality,
										NULL,
										FALSE,
										this))
					throw (int)0;
			}
			// Save next TIFF page
			else
			{
				if (!m_Dib.SaveNextTIFF(m_TiffOutFile,
										0, // We do not know the page number
										0, // We do not know how many pages
										nCompression,
										m_nTiffJpegQuality,
										NULL,
										FALSE,
										this))
					throw (int)0;
			}
		}
	}
}

void CBatchProcDlg::CProcessThread::AddToOutputPdf(	int nFilesCount,
													CString sInFileName,
													CString sOutFileName)
{
	if (::IsTIFF(sInFileName))
	{
		if (m_bTiffForceCompression)
		{
			if (m_bFirstOutputFile)
			{
				m_TiffOutFile = TIFFOpenW(sOutFileName, "w");    
				if (!m_TiffOutFile)
					throw (int)0;
			}
			if (!CDib::TIFFCopyAllPages(sInFileName,
										m_TiffOutFile,
										0,			// We do not know the page number
										0,			// We do not know how many pages
										m_bWorkOnAllPages ? -1 : 0,
										m_nTiffCompression,
										m_nTiffJpegQuality,
										TRUE,		// Flatten all pages with a alpha channel
										RGB(255,255,255),// White background for PDFs is ok
										TRUE,		// Limit to 1,2,4 or 8 Bits per Sample because PDF is not supporting others!
										TRUE))		// YCbCr Jpegs inside Tiff are not supported by Tiff2Pdf -> re-encode
				throw (int)0;
		}
		else
		{
			// In case of nFilesCount == 1 do not use ::CopyFile() like for Tiff output
			// because it may be necessary
			// - to flatten the image
			// - or convert it to standard 1,2,4,8 Bits per Sample
			// - or recompress it because it is a YCbCr Jpeg!
			if (m_bFirstOutputFile)
			{
				m_TiffOutFile = TIFFOpenW(sOutFileName, "w");    
				if (!m_TiffOutFile)
					throw (int)0;
			}
			if (!CDib::TIFFCopyAllPages(sInFileName,
										m_TiffOutFile,
										0,			// We do not know the page number
										0,			// We do not know how many pages
										-1,			// Don't care because next param is -1
										-1,			// -1 do not change compression type
										m_nTiffJpegQuality,
										TRUE,		// Flatten all pages with a alpha channel
										RGB(255,255,255),// White background for PDFs is ok
										TRUE,		// Limit to 1,2,4 or 8 Bits per Sample because PDF is not supporting others!
										TRUE))		// YCbCr Jpegs inside Tiff are not supported by Tiff2Pdf -> re-encode
				throw (int)0;
		}
	}
	else
	{
		// Load Image
		if (!m_Dib.LoadImage(	sInFileName,
								0,		// X Full Size Load
								0,		// Y Full Size Load
								0,		// Load Page 0
								TRUE,	// Decompress Bmps
								FALSE,	// Not Only Header
								NULL,	// No Progress Wnd
								FALSE,	// No Progress Send
								this))	// Thread
			throw (int)0;

		// Auto Orientate
		CDib::AutoOrientateDib(&m_Dib);
		
		// Clear Orientation
		m_Dib.GetExifInfo()->Orientation = 1;

		// Flatten
		if (m_Dib.HasAlpha() && m_Dib.GetBitCount() == 32)
		{
			m_Dib.SetBackgroundColor(RGB(255,255,255)); // White background for PDFs is ok
			m_Dib.RenderAlphaWithSrcBackground();
			m_Dib.SetAlpha(FALSE);
		}

		// Decide about the Compression Type
		int nCompression = m_nTiffCompression;
		if (::IsTIFF(sInFileName) && !m_bTiffForceCompression)
			nCompression = m_Dib.m_FileInfo.m_nCompression;

		// Save Multi-Page TIFF
		if (nFilesCount == 1)
		{
			if (!m_Dib.SaveTIFF(sOutFileName,
								nCompression,	
								m_nTiffJpegQuality,
								NULL,
								FALSE,
								this))
				throw (int)0;
		}
		else if (nFilesCount > 1)
		{
			// Save first TIFF page
			if (m_bFirstOutputFile)
			{
				if (!m_Dib.SaveFirstTIFF(sOutFileName,
										&m_TiffOutFile,
										0, // We do not know how many pages
										nCompression,
										m_nTiffJpegQuality,
										NULL,
										FALSE,
										this))
					throw (int)0;
			}
			// Save next TIFF page
			else
			{
				if (!m_Dib.SaveNextTIFF(m_TiffOutFile,
										0, // We do not know the page number
										0, // We do not know how many pages
										nCompression,
										m_nTiffJpegQuality,
										NULL,
										FALSE,
										this))
					throw (int)0;
			}
		}
	}
}

void CBatchProcDlg::CProcessThread::DoRename(	int& num,
												const CString& sSrcFileName,
												CString& sDstFileName,
												CString& sDstFileNameSameExt)
{
	CString sRenamed = m_pDlg->m_sRename;

	// Find the # Digits Place Holder
	int nDigitsFirst = sRenamed.Find(_T('#'));
	while (nDigitsFirst >= 0)
	{
		int nDigitsCount = 1;
		for (int i = (nDigitsFirst + 1) ; i < sRenamed.GetLength() ; i++)
		{
			if (sRenamed[i] == _T('#'))
				nDigitsCount++;
			else
				break;
		}
		
		// Delete
		sRenamed.Delete(nDigitsFirst, nDigitsCount);

		// Substitute it with the number
		CString sNum, sFormat;
		sFormat.Format(_T("%%0%iu"), nDigitsCount);
		sNum.Format(sFormat, num);
		sRenamed.Insert(nDigitsFirst, sNum);

		// Next
		nDigitsFirst = sRenamed.Find(_T('#'));
	}

	// Inc. Count
	num++;
	
	// Find the File Name Place Holder
	CString sRenamedLower = sRenamed;
	sRenamedLower.MakeLower();
	nDigitsFirst = sRenamedLower.Find(_T("$file"));
	while (nDigitsFirst >= 0)
	{
		// Delete
		sRenamed.Delete(nDigitsFirst, 5);

		// Substitute it with the file name
		sRenamed.Insert(nDigitsFirst, ::GetShortFileNameNoExt(sDstFileName));

		// Next
		sRenamedLower = sRenamed;
		sRenamedLower.MakeLower();
		nDigitsFirst = sRenamed.Find(_T("$file"));
	}

	// Final Destination File Names
	sDstFileName =	::GetDriveAndDirName(sDstFileName) +
					sRenamed +
					::GetFileExt(sDstFileName);
	sDstFileNameSameExt =	::GetFileNameNoExt(sDstFileName) +
							::GetFileExt(sSrcFileName);
}

BOOL CBatchProcDlg::CProcessThread::Copy(	const CString& sSrcFileName,
											const CString& sDstFileNameSameExt)
{
	if (sSrcFileName.CompareNoCase(sDstFileNameSameExt) != 0)
	{
		if (!::CopyFile(sSrcFileName, sDstFileNameSameExt, FALSE))
		{
			::ShowErrorMsg(::GetLastError(), FALSE);
			return FALSE;
		}
		if (!::SetFileAttributes(sDstFileNameSameExt,
								FILE_ATTRIBUTE_NORMAL |
								FILE_ATTRIBUTE_TEMPORARY))
		{
			::ShowErrorMsg(::GetLastError(), FALSE);
			return FALSE;
		}
		return TRUE;
	}
	else
		return TRUE;
}

void CBatchProcDlg::CProcessThread::JpegProcessing(	BOOL bJpegHasBeenSaved,
													const CString& sDstFileName,
													CString& sTempFileName)
{
	// Auto Orientate Jpeg File
	if (m_bAutoOrientate)
	{
		if (!CDib::JPEGAutoOrientate(sDstFileName, ((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), FALSE))
			return;
	}

	// EXIF Thumbnail and Date / Time Offset
	if (!m_bRemoveExif)
	{
		switch (m_nExifThumbOperationType)
		{
			case CBatchProcJpegTab::THUMB_UPDATE :
			{
				// Jpeg Saving in ShrinkPicture() and JPEGAutoOrientate
				// update the EXIF Thumb, if they haven't been called
				// we need to update the EXIF Thumb here!
				if (!bJpegHasBeenSaved && !m_bAutoOrientate)
				{
					// Update Exif Width, Height & Thumb
					CDib::UpdateExifWidthHeightThumb(sDstFileName, FALSE);
				}
				break;
			}
			case CBatchProcJpegTab::THUMB_ADD_UPDATE :
			{
				// Temporary File
				sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

				// Add Thumb
				if (CDib::AddEXIFThumbnail(	sDstFileName,
											sTempFileName,
											FALSE))
				{
					// Remove and Rename Files
					CFile::Remove(sDstFileName);
					CFile::Rename(sTempFileName, sDstFileName);
				}
				// Add failes if the Thumb already exists
				// -> Update Thumb
				else
				{
					// Jpeg Saving in ShrinkPicture() and JPEGAutoOrientate
					// update the EXIF Thumb, if they haven't been called
					// we need to update the EXIF Thumb here!
					if (!bJpegHasBeenSaved && !m_bAutoOrientate)
					{
						// Update Exif Width, Height & Thumb
						CDib::UpdateExifWidthHeightThumb(sDstFileName, FALSE);
					}
				}
				
				break;
			}
			case CBatchProcJpegTab::THUMB_REMOVE :
			{
				// Temporary File
				sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

				// Remove Thumb
				if (CDib::RemoveEXIFThumbnail(	sDstFileName,
												sTempFileName,
												FALSE))
				{
					// Remove and Rename Files
					CFile::Remove(sDstFileName);
					CFile::Rename(sTempFileName, sDstFileName);
				}
				
				break;
			}

			default :
				break;
		}

		if (m_bExifTimeOffset)
		{
			CDib Dib;
			Dib.SetShowMessageBoxOnError(FALSE);
			if (Dib.JPEGLoadMetadata(sDstFileName))
			{
				CString sExifDateTime(Dib.GetExifInfo()->DateTime);
				if (sExifDateTime != _T(""))
				{
					// Use ole date time because it is not using daylight saving time
					COleDateTime OleDateTime = CMetadata::GetOleDateTimeFromExifString(sExifDateTime);
					if (m_nExifTimeOffsetSign)
						OleDateTime -= m_ExifTimeOffset;
					else
						OleDateTime += m_ExifTimeOffset;
					CDib::JPEGSetOleDateTimeInplace(sDstFileName, OleDateTime, FALSE);
				}
			}
		}
	}

	if (m_bRemoveCom)
	{
		// Temporary File
		sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

		// Remove COM Section
		if (CDib::JPEGRemoveSection(M_COM,
									NULL,
									0,
									sDstFileName,
									sTempFileName,
									FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}
	}
	
	if (m_bRemoveExif)
	{
		// Temporary File
		sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

		// Remove EXIF Section
		if (CDib::JPEGRemoveSection(M_EXIF_XMP,
									CMetadata::m_ExifHeader,
									EXIF_HEADER_SIZE,
									sDstFileName,
									sTempFileName,
									FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}
	}
	
	if (m_bRemoveIcc)
	{
		// Temporary File
		sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

		// Remove ICC Section
		if (CDib::JPEGRemoveSection(M_ICC,
									CMetadata::m_IccHeader,
									ICC_HEADER_SIZE,
									sDstFileName,
									sTempFileName,
									FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}
	}

	if (m_bRemoveXmp)
	{
		// Temporary File
		sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

		// Remove XMP Section
		if (CDib::JPEGRemoveSection(M_EXIF_XMP,
									CMetadata::m_XmpHeader,
									XMP_HEADER_SIZE,
									sDstFileName,
									sTempFileName,
									FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}
	}

	if (m_bRemoveIptc)
	{
		// Temporary File
		sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

		// Remove IPTC Section
		if (CDib::JPEGRemoveSection(M_IPTC,
									NULL,
									0,
									sDstFileName,
									sTempFileName,
									FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}
	}

	if (m_bRemoveJfif)
	{
		// Temporary File
		sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

		// Remove JFIF Section
		if (CDib::JPEGRemoveSection(M_JFIF,
									NULL,
									0,
									sDstFileName,
									sTempFileName,
									FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}
	}

	if (m_bRemoveOtherAppSections)
	{
		// Temporary Files
		sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sDstFileName);

		// Remove APP3 - APP12 Sections
		if (CDib::JPEGRemoveSections(	M_APP3,
										M_APP12,
										sDstFileName,
										sTempFileName,
										FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}

		// Remove APP14 - APP15 Sections
		if (CDib::JPEGRemoveSections(	M_APP14,
										M_APP15,
										sDstFileName,
										sTempFileName,
										FALSE))
		{
			// Remove and Rename Files
			CFile::Remove(sDstFileName);
			CFile::Rename(sTempFileName, sDstFileName);
		}
	}

	if (m_bMergeXmp)
	{
		CDib Dib;
		Dib.SetShowMessageBoxOnError(FALSE);
		if (Dib.JPEGLoadMetadata(sDstFileName))
		{
			// Import Xmp
			CString sExifDateTime(Dib.GetExifInfo()->DateTime);
			if (sExifDateTime != _T(""))
			{
				((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->SetExifDateCreated(
					CMetadata::GetDateTimeFromExifString(sExifDateTime));
			}
			else
				((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->ClearExifDateCreated();
			((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->GetFields(m_bIptcFromXmpPriority, *Dib.GetMetadata());

			// Write Iptc Legacy
			LPBYTE pData = NULL;
			int nSize = Dib.GetMetadata()->MakeIptcLegacySection(&pData);
			if (nSize > 0)
			{
				CDib::JPEGWriteSection(	M_IPTC,
										NULL,
										0,
										sDstFileName,
										((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),	
										nSize,
										pData,
										FALSE); // Do not show Message Box on error!
			}

			// Free
			if (pData)
			{
				delete [] pData;
				pData = NULL;
			}

			// Write Xmp
			nSize = Dib.GetMetadata()->MakeXmpSection(&pData);
			if (nSize > 0)
			{
				CDib::JPEGWriteSection(	M_EXIF_XMP,
										CMetadata::m_XmpHeader,
										XMP_HEADER_SIZE,
										sDstFileName,
										((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),	
										nSize,
										pData,
										FALSE); // Do not show Message Box on error!
			}

			// Free
			if (pData)
			{
				delete [] pData;
				pData = NULL;
			}
		}
	}
}

void CBatchProcDlg::CProcessThread::TiffProcessing(const CString& sDstFileName)
{
	if (m_bMergeXmp || m_bExifTimeOffset)
	{	
		// Load Tiff Header of Page 0
		CDib Dib;
		Dib.SetShowMessageBoxOnError(FALSE);

		// Load First Page
		int nPageNum = 0;
		if (!Dib.LoadTIFF(sDstFileName, 0, TRUE))
			return;
		do
		{
			// First update time
			if (m_bExifTimeOffset)
			{
				CString sExifDateTime(Dib.GetExifInfo()->DateTime);
				if (sExifDateTime != _T(""))
				{
					// Use ole date time because it is not using daylight saving time
					COleDateTime OleDateTime = CMetadata::GetOleDateTimeFromExifString(sExifDateTime);
					if (m_nExifTimeOffsetSign)
						OleDateTime -= m_ExifTimeOffset;
					else
						OleDateTime += m_ExifTimeOffset;
					CMetadata::FillExifOleDateTimeString(OleDateTime, Dib.GetExifInfo()->DateTime);
				}
			}

			// Now update metadata
			if (m_bMergeXmp)
			{
				// Import Xmp
				CString sExifDateTime(Dib.GetExifInfo()->DateTime);
				if (sExifDateTime != _T(""))
				{
					((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->SetExifDateCreated(
						CMetadata::GetDateTimeFromExifString(sExifDateTime));
				}
				else
					((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->ClearExifDateCreated();
				((CUImagerApp*)::AfxGetApp())->m_pXmpDlg->GetFields(m_bIptcFromXmpPriority, *Dib.GetMetadata());

				// Update Iptc Data
				if (Dib.GetMetadata()->m_pIptcLegacyData)
				{
					delete [] Dib.GetMetadata()->m_pIptcLegacyData;
					Dib.GetMetadata()->m_pIptcLegacyData = NULL;
				}
				Dib.GetMetadata()->m_dwIptcLegacySize =
					Dib.GetMetadata()->MakeIptcLegacyData(&(Dib.GetMetadata()->m_pIptcLegacyData));
				
				// Update Xmp
				Dib.GetMetadata()->UpdateXmpData(_T("image/tiff"));
			}

			// Save Tiff Metadatas
			if ((Dib.GetMetadata()->m_pIptcLegacyData &&
				Dib.GetMetadata()->m_dwIptcLegacySize > 0)	||
				(Dib.GetMetadata()->m_pXmpData &&
				Dib.GetMetadata()->m_dwXmpSize > 0)			||
				m_bExifTimeOffset)
			{	
				Dib.TIFFWriteMetadata(	sDstFileName,
										((CUImagerApp*)::AfxGetApp())->GetAppTempDir());
			}

			// Load Next Page?
			nPageNum++;
			if (m_bWorkOnAllPages && nPageNum < Dib.m_FileInfo.m_nImageCount)
			{
				if (!Dib.LoadTIFF(sDstFileName, nPageNum, TRUE))
					return;
			}
			else
				return;
		}
		while (TRUE);
	}
}

int CBatchProcDlg::CProcessThread::OnExit(BOOL bOk)
{
	::CoUninitialize();
	m_pDlg->PostMessage(WM_ONEXIT, bOk, 0);
	return 0;
}

LONG CBatchProcDlg::OnExitHandler(WPARAM wparam, LPARAM lparam)
{
	// Ok Flag
	BOOL bOk = (BOOL)wparam;

	// Delete Tmp Dir
	if (bOk)
		::DeleteDir(m_ProcessThread.m_szTempDstDirPath);

	// Reset Progress
	m_ProcessThread.m_nPrevPercentDone = -5;
	m_Progress.SetPos(-1);

	// Change Button Label
	CButton* pButton = (CButton*)GetDlgItem(IDOK);
	pButton->SetWindowText(_T("Start"));
	pButton->EnableWindow(TRUE);

	// Re-Enable All Controls
	EnableAllControls(TRUE, FALSE);

	// Update Controls
	UpdateControls();

	// Set Exited Flag
	m_bThreadExited = TRUE;

	// Close Dlg?
	if (m_bDoCloseDlg)
	{
		SaveSettings();
		ListDeleteAll();
		EndWaitCursor();
		DestroyWindow();
	}

	return 0;
}

void CBatchProcDlg::CProcessThread::Progress(	int nCurrentFilePos,
												int nTotalFiles)
{
	int nPercentDone = Round((double)(nCurrentFilePos) * 100.0 / (double)(nTotalFiles));

	if ((nPercentDone >= (m_nPrevPercentDone + 5)) ||
		(nPercentDone < m_nPrevPercentDone))
	{
		m_pDlg->m_Progress.SetPos(nPercentDone);
		m_nPrevPercentDone = nPercentDone;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBatchProcDlg dialog


CBatchProcDlg::CBatchProcDlg(CWnd* pParent)
	: CDialog(CBatchProcDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatchProcDlg)
	m_nOptimizationSelection = AUTO_OPT;
	m_bRecursive = TRUE;
	m_sSrc = _T("");
	m_sEditDst = _T("");
	m_sOutputFileName = _T("");
	m_nOutputSelection = OUTPUT_DIR;
	m_nInputSelection = INPUT_DIR;
	m_bRename = FALSE;
	m_sRename = _T("Picture #####");
	m_bConversion = FALSE;
	//}}AFX_DATA_INIT

	// Destination Directory
	m_sDst = _T("");

	// Sort Type
	m_nSortType = FILENAME_DES;

	// Exit Flags
	m_bDoCloseDlg = FALSE;
	m_bThreadExited = TRUE;

	// Enable Next LoadDibs Flag,
	// cleared when exiting
	m_bEnableNextLoadDibs = TRUE;

	// List Files Count
	m_nStoredListFilesCount = 0;

	// Dib Static Height and Width
	m_nDibStaticWidth = 0;
	m_nDibStaticHeight = 0;

	// Dibs Load Counts
	m_uiDibsHdrLoadTotCount = 0;
	m_uiDibsHdrLoadStartCount = 0;
	m_uiDibsHdrLoadDoneCount = 0;

	// Update Dibs Flag
	m_bEnableUpdateDibs = TRUE;

	// Output Dir Content Size
	m_OutDirContentSize.QuadPart = 0;

	// Output Dir Files Count
	m_nOutDirFilesCount = 0;

	// Critical Section
	::InitializeCriticalSection(&m_csOutDir);

	// ComCtl32.dll Major Version
	m_nComCtl32MajorVersion = GetComCtl32MajorVersion();

	// Init Tab
	m_nInitTab = 0;

	// Load Settings
	LoadSettings();

	// Create
	CDialog::Create(CBatchProcDlg::IDD, pParent);
}


void CBatchProcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchProcDlg)
	DDX_Control(pDX, IDC_TAB_ADV_SETTINGS, m_TabAdvSettings);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_LIST_INPUT, m_List);
	DDX_Radio(pDX, IDC_RADIO_OPTIMIZE_AUTO, m_nOptimizationSelection);
	DDX_Check(pDX, IDC_CHECK_RECURSIVE, m_bRecursive);
	DDX_Text(pDX, IDC_EDIT_SRCDIR, m_sSrc);
	DDX_Text(pDX, IDC_EDIT_DSTDIR, m_sEditDst);
	DDX_Text(pDX, IDC_EDIT_OUTPUT_FILE, m_sOutputFileName);
	DDX_Radio(pDX, IDC_RADIO_OUTPUT, m_nOutputSelection);
	DDX_Radio(pDX, IDC_RADIO_INPUT, m_nInputSelection);
	DDX_Check(pDX, IDC_CHECK_RENAME, m_bRename);
	DDX_Text(pDX, IDC_EDIT_RENAME, m_sRename);
	DDX_Check(pDX, IDC_CHECK_CONVERSION, m_bConversion);
	//}}AFX_DATA_MAP
	if (::IsWindow(m_GeneralTab.GetSafeHwnd()))
	{
		if (!m_GeneralTab.UpdateData(pDX->m_bSaveAndValidate))
			AfxThrowUserException();
	}
	if (::IsWindow(m_ShrinkTab.GetSafeHwnd()))
	{
		if (!m_ShrinkTab.UpdateData(pDX->m_bSaveAndValidate))
			AfxThrowUserException();
	}
	if (::IsWindow(m_JpegTab.GetSafeHwnd()))
	{
		if (!m_JpegTab.UpdateData(pDX->m_bSaveAndValidate))
			AfxThrowUserException();
	}
	if (::IsWindow(m_TiffTab.GetSafeHwnd()))
	{
		if (!m_TiffTab.UpdateData(pDX->m_bSaveAndValidate))
			AfxThrowUserException();
	}
}


BEGIN_MESSAGE_MAP(CBatchProcDlg, CDialog)
	//{{AFX_MSG_MAP(CBatchProcDlg)
	ON_BN_CLICKED(IDC_RADIO_OPTIMIZE_ADVANCED, OnRadioOptimizeAdvanced)
	ON_BN_CLICKED(IDC_RADIO_OPTIMIZE_AUTO, OnRadioOptimizeAuto)
	ON_BN_CLICKED(IDC_BUTTON_SRCDIR, OnButtonSrcDir)
	ON_BN_CLICKED(IDC_BUTTON_DSTDIR, OnButtonDstDir)
	ON_BN_CLICKED(IDC_BUTTON_OUTPUT_FILE, OnButtonDstFile)
	ON_BN_CLICKED(IDC_RADIO_OUTPUT, OnRadioOutput)
	ON_BN_CLICKED(IDC_RADIO_OUTPUT_FILE, OnRadioOutputFile)
	ON_BN_CLICKED(IDC_RADIO_INPUT, OnRadioInput)
	ON_BN_CLICKED(IDC_RADIO_INPUT_LIST, OnRadioInputList)
	ON_BN_CLICKED(IDC_CHECK_RENAME, OnCheckRename)
	ON_BN_CLICKED(IDC_BUTTON_LIST_ADD, OnButtonListAdd)
	ON_BN_CLICKED(IDC_BUTTON_LIST_DELETE, OnButtonListDelete)
	ON_BN_CLICKED(IDC_BUTTON_LIST_DOWN, OnButtonListDown)
	ON_BN_CLICKED(IDC_BUTTON_LIST_UP, OnButtonListUp)
	ON_BN_CLICKED(IDC_BUTTON_LIST_SELECTALL, OnButtonListSelectall)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_INPUT, OnItemchangedListInput)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_INPUT, OnDeleteitemListInput)
	ON_NOTIFY(LVN_INSERTITEM, IDC_LIST_INPUT, OnInsertitemListInput)
	ON_NOTIFY(LVN_DELETEALLITEMS, IDC_LIST_INPUT, OnDeleteallitemsListInput)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_INPUT, OnKeydownListInput)
	ON_BN_CLICKED(IDC_BUTTON_LIST_LOAD, OnButtonListLoad)
	ON_BN_CLICKED(IDC_BUTTON_LIST_SAVE, OnButtonListSave)
	ON_BN_CLICKED(IDC_CHECK_CONVERSION, OnCheckConversion)
	ON_EN_CHANGE(IDC_EDIT_DSTDIR, OnChangeEditDstdir)
	ON_EN_CHANGE(IDC_EDIT_OUTPUT_FILE, OnChangeEditOutputFileName)
	ON_EN_CHANGE(IDC_EDIT_SRCDIR, OnChangeEditSrcdir)
	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	// Change : ON_NOTIFY(HDN_ITEMCLICK, IDC_LIST_INPUT, OnItemclickListInput)
	// To:
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnItemclickListInput)
	ON_MESSAGE(WM_ONEXIT, OnExitHandler)
	ON_MESSAGE(WM_UPDATEDIBS_POSTPONE, OnUpdateDibsPostpone)
	ON_MESSAGE(WM_LOADDONE, OnLoadDone)
	ON_MESSAGE(WM_SET_DST_SIZE, OnSetDstSize)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchProcDlg message handlers

BOOL CBatchProcDlg::OnInitDialog() 
{
	int i;
	CDialog::OnInitDialog();

	// Init Change Notification Thread
	m_ChangeNotificationThread.SetDlg(this);
	if (::IsExistingDir(m_sDst))
		m_ChangeNotificationThread.Start();	// This Calls UpdateDstFolderSizes()

	// Update Sizes
	UpdateDstFileSize();

	// Subclass Links
	m_InputDirLabel.SubclassDlgItem(IDC_TEXT_INPUT_DIR, this);
	m_OutputDirLabel.SubclassDlgItem(IDC_TEXT_OUTPUT_DIR, this);
	m_OutputFileNameLabel.SubclassDlgItem(IDC_TEXT_OUTPUT_FILE, this);
	m_InputDirLabel.SetVisitedColor(RGB(0, 0, 255));
	m_OutputDirLabel.SetVisitedColor(RGB(0, 0, 255));
	m_OutputFileNameLabel.SetVisitedColor(RGB(0, 0, 255));

	// Set Dlg Wnd for Process Thread
	m_ProcessThread.SetDlg(this);

	// Progress
	m_Progress.SetRange(0, 100);

	// Uncheck the dangerous delete input file after in-place rename checkbox
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_DELETE_INPUTFILE);
	if (pCheck)
		pCheck->SetCheck(0);

	// Subclass Thumbnail Dib Static Controls
	for (i = 0 ; i < NUM_DIBS ; i++)
	{
		m_Dibs[i].SubclassDlgItem(IDC_DIB0 + i, this);
		m_Dibs[i].SetBackgroundColor(RGB(0xFF, 0xFF, 0xFF));
		m_Dibs[i].SetBorders(CRect(2,2,2,2));
		m_Dibs[i].SetBordersColor(RGB(0xFF, 0xFF, 0xFF));
		m_Dibs[i].SetCrossColor(RGB(0x80, 0x80, 0x80));
		m_Dibs[i].SetBusyTextColor(RGB(0x80, 0x80, 0x80));
		m_Dibs[i].SetBusyText(ML_STRING(1384, "Loading..."));
	}

	// Init Dib Static Width & Height
	CRect rcDibStaticClient;
	m_Dibs[0].GetClientRect(&rcDibStaticClient);
	m_nDibStaticWidth = rcDibStaticClient.Width();
	m_nDibStaticHeight = rcDibStaticClient.Height();

	// Setup the Tab Ctrl
	int nPageID = 0;
	m_GeneralTab.Create(IDD_BATCH_PROC_GENERALTAB, this);
	m_TabAdvSettings.AddSSLPage (ML_STRING(1785, "General"), nPageID++, &m_GeneralTab);
	m_ShrinkTab.Create(IDD_BATCH_PROC_SHRINKTAB, this);
	m_TabAdvSettings.AddSSLPage (ML_STRING(1786, "Shrink"), nPageID++, &m_ShrinkTab);
	m_JpegTab.Create(IDD_BATCH_PROC_JPEGTAB, this);
	m_TabAdvSettings.AddSSLPage (ML_STRING(1787, "Jpeg"), nPageID++, &m_JpegTab);
	m_TiffTab.Create(IDD_BATCH_PROC_TIFFTAB, this);
	m_TabAdvSettings.AddSSLPage (ML_STRING(1788, "Tiff / Pdf"), nPageID++, &m_TiffTab);
	m_TabAdvSettings.ActivateSSLPage(m_nInitTab);

	// List Ctrl
	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_List.SetMultiSelectImageRes(IDB_BITMAP_DRAGDROP_MULTI);
	m_List.SetSingleSelectImageRes(IDB_BITMAP_DRAGDROP_SINGLE);
	m_List.SetUseResDragImage(TRUE);
	CRect rcListClient;
	m_List.GetClientRect(&rcListClient);
	m_List.InsertColumn(0, ML_STRING(1860, "Path"), LVCFMT_LEFT, 80);
	m_List.InsertColumn(0, ML_STRING(1859, "Height"), LVCFMT_LEFT, 80);
	m_List.InsertColumn(0, ML_STRING(1858, "Width"), LVCFMT_LEFT, 80);
	m_List.InsertColumn(0, ML_STRING(1857, "Image Size"), LVCFMT_LEFT, 95);
	m_List.InsertColumn(0, ML_STRING(1856, "File Size"), LVCFMT_LEFT, 95);
	m_List.InsertColumn(0, ML_STRING(1855, "Date Modified"), LVCFMT_LEFT, 120);
	m_List.InsertColumn(0, ML_STRING(1854, "Date Created"), LVCFMT_LEFT, 120);
	m_List.InsertColumn(0, ML_STRING(1853, "Date Taken"), LVCFMT_LEFT, 120);
	m_List.InsertColumn(0, ML_STRING(1852, "Name"), LVCFMT_LEFT, rcListClient.Width());
	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_List.GetHeaderCtrl();
	if (pHeader)
	{
		// Create the image list and attach it to the header control
		VERIFY(m_HdrImageList.Create(IDB_HEADER_CTRL, 15, 2, RGB(255, 0, 255)));
		pHeader->SetImageList(&m_HdrImageList);

		// Make the header control track 
		pHeader->ModifyStyle(0, HDS_HOTTRACK);
	}

	// Load Stored File List?
	if (m_nStoredListFilesCount > 0)
	{
		// Disable All Controls, Including Process Button!
		EnableAllControls(FALSE, TRUE);
		
		// Load Input List From Registry
		m_bEnableUpdateDibs = FALSE;
		for (i = 0 ; i < m_nStoredListFilesCount ; i++)
		{
			CString sSection(_T("BatchDlg"));
			CWinApp* pApp = ::AfxGetApp();
			CString s;
			s.Format(_T("ListFile%i"), i+1);
			CString sFileName = pApp->GetProfileString(sSection, s, _T(""));
			if (::IsExistingFile(sFileName))
				ListAdd(m_List.GetItemCount(), sFileName);
		}

		// Start Loading Dibs
		StartLoadDibs();
	}
	else
	{
		// Update Controls
		UpdateControls();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBatchProcDlg::StartLoadDibs()
{
	// Reset Counters
	m_uiDibsHdrLoadStartCount = 0;
	m_uiDibsHdrLoadDoneCount = 0;

	// Count the Pictures to Load
	m_uiDibsHdrLoadTotCount = 0;
	for (int nItem = 0 ; nItem < m_List.GetItemCount() ; nItem++)
	{
		CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
		if (pListElement &&
			!pListElement->m_DibStatic.HasLoadHdrStarted())
			m_uiDibsHdrLoadTotCount++;
	}

	// Start Thread(s)
	if (StartNextLoadDibsHdr(((CUImagerApp*)::AfxGetApp())->m_nCoresCount) == 0)
	{
		// Re-Enable Controls
		EnableAllControls(TRUE, TRUE);
		UpdateControls();
		
		// Update Dibs
		m_bEnableUpdateDibs = TRUE;
		OnUpdateDibsPostpone(0, 0);
	}
}

void CBatchProcDlg::LoadSettings()
{
	CString sSection(_T("BatchDlg"));
	CWinApp* pApp = ::AfxGetApp();

	m_nOptimizationSelection = (int)pApp->GetProfileInt(sSection, _T("OptimizationSelection"), AUTO_OPT);
	m_ShrinkTab.m_nPixelsPercentSel = (int)pApp->GetProfileInt(sSection, _T("PixelsPercentSel"), 0);
	m_ShrinkTab.m_bShrinkingPictures = (BOOL)pApp->GetProfileInt(sSection, _T("ShrinkingPictures"), FALSE);
	m_ShrinkTab.m_nShrinkingPercent = (int)pApp->GetProfileInt(sSection, _T("ShrinkingPercent"), 50);
	m_ShrinkTab.m_nShrinkingPixels = (int)pApp->GetProfileInt(sSection, _T("ShrinkingPixels"), AUTO_SHRINK_MAX_SIZE);
	m_JpegTab.m_nJpegQuality = (int)pApp->GetProfileInt(sSection, _T("JpegQuality"), DEFAULT_JPEGCOMPRESSION);
	m_TiffTab.m_nJpegQuality = (int)pApp->GetProfileInt(sSection, _T("TiffJpegQuality"), DEFAULT_JPEGCOMPRESSION);
	m_TiffTab.m_nCompression = (int)pApp->GetProfileInt(sSection, _T("TiffCompression"), 2);
	m_TiffTab.m_bForceCompression = (BOOL)pApp->GetProfileInt(sSection, _T("TiffForceCompression"), FALSE);
	m_TiffTab.m_sPdfPaperSize = pApp->GetProfileString(sSection, _T("PdfPaperSize"), _T("Fit"));
	m_bRecursive = (BOOL)pApp->GetProfileInt(sSection, _T("Recursive"), TRUE);
	m_sSrc = pApp->GetProfileString(sSection, _T("Src"), _T(""));
	m_InputDirLabel.SetLink(m_sSrc);
	m_sDst = pApp->GetProfileString(sSection, _T("Dst"), _T(""));
	m_sEditDst = m_sDst;
	m_OutputDirLabel.SetLink(m_sDst);
	m_sOutputFileName = pApp->GetProfileString(sSection, _T("OutputFileName"), _T(""));
	m_OutputFileNameLabel.SetLink(m_sOutputFileName);
	m_nOutputSelection = (int)pApp->GetProfileInt(sSection, _T("OutputSelection"), OUTPUT_DIR);
	m_nInputSelection = (int)pApp->GetProfileInt(sSection, _T("InputSelection"), INPUT_DIR);
	m_JpegTab.m_nExifThumbOperationType = (int)pApp->GetProfileInt(sSection, _T("ExifThumbOperationType"), CBatchProcJpegTab::THUMB_NONE);
	m_GeneralTab.m_nExtChangeType = (int)pApp->GetProfileInt(sSection, _T("ExtChangeType"), CBatchProcGeneralTab::NO_CHANGE);
	m_GeneralTab.m_nIptcPriority = (int)pApp->GetProfileInt(sSection, _T("IptcPriority"), 0);
	m_JpegTab.m_bAutoOrientate = (BOOL)pApp->GetProfileInt(sSection, _T("AutoOrientate"), FALSE);
	m_JpegTab.m_bForceJpegQuality = (BOOL)pApp->GetProfileInt(sSection, _T("ForceJpegQuality"), FALSE);
	m_bRename = (BOOL)pApp->GetProfileInt(sSection, _T("Rename"), TRUE);
	m_sRename = pApp->GetProfileString(sSection, _T("RenamePattern"), _T("Picture #####"));
	m_bConversion = (BOOL)pApp->GetProfileInt(sSection, _T("Conversion"), FALSE);
	m_ShrinkTab.m_bSharpen = (BOOL)pApp->GetProfileInt(sSection, _T("Sharpen"), FALSE);
	m_TiffTab.m_bWorkOnAllPages = (BOOL)pApp->GetProfileInt(sSection, _T("WorkOnAllPages"), FALSE);
	m_JpegTab.m_bRemoveCom = (BOOL)pApp->GetProfileInt(sSection, _T("RemoveCom"), FALSE);
	m_JpegTab.m_bRemoveExif = (BOOL)pApp->GetProfileInt(sSection, _T("RemoveExif"), FALSE);
	m_JpegTab.m_bRemoveIcc = (BOOL)pApp->GetProfileInt(sSection, _T("RemoveIcc"), FALSE);
	m_JpegTab.m_bRemoveXmp = (BOOL)pApp->GetProfileInt(sSection, _T("RemoveXmp"), FALSE);
	m_JpegTab.m_bRemoveIptc = (BOOL)pApp->GetProfileInt(sSection, _T("RemoveIptc"), FALSE);
	m_JpegTab.m_bRemoveJfif = (BOOL)pApp->GetProfileInt(sSection, _T("RemoveJfif"), FALSE);
	m_JpegTab.m_bRemoveOtherAppSections = (BOOL)pApp->GetProfileInt(sSection, _T("RemoveOtherAppSections"), FALSE);
	m_GeneralTab.m_bMergeXmp = (BOOL)pApp->GetProfileInt(sSection, _T("MergeXmp"), FALSE);
	m_GeneralTab.m_bExifTimeOffset = (BOOL)pApp->GetProfileInt(sSection, _T("ExifTimeOffset"), FALSE);
	m_GeneralTab.m_nExifTimeOffsetSign = (int)pApp->GetProfileInt(sSection, _T("ExifTimeOffsetSign"), 0);
	m_GeneralTab.m_nExifTimeOffsetDays = (int)pApp->GetProfileInt(sSection, _T("ExifTimeOffsetDays"), 0);
	m_GeneralTab.m_nExifTimeOffsetHours = (int)pApp->GetProfileInt(sSection, _T("ExifTimeOffsetHours"), 0);
	m_GeneralTab.m_nExifTimeOffsetMin = (int)pApp->GetProfileInt(sSection, _T("ExifTimeOffsetMin"), 0);
	m_GeneralTab.m_nExifTimeOffsetSec = (int)pApp->GetProfileInt(sSection, _T("ExifTimeOffsetSec"), 0);
	m_nStoredListFilesCount = (int)pApp->GetProfileInt(sSection, _T("ListFilesCount"), 0);
	m_nInitTab = (int)pApp->GetProfileInt(sSection, _T("InitTab"), 0);
	unsigned int nSize = sizeof(m_GeneralTab.m_dFrameRate);
	double* pFrameRate = &m_GeneralTab.m_dFrameRate;
	pApp->GetProfileBinary(sSection, _T("FrameRate"), (LPBYTE*)&pFrameRate, &nSize);
	if (pFrameRate && (nSize > 0))
	{
		m_GeneralTab.m_dFrameRate = *pFrameRate;
		delete [] pFrameRate;
	}
	else
	{
		if (pFrameRate)
			delete [] pFrameRate;
		m_GeneralTab.m_dFrameRate = 1.0;
	}
}

void CBatchProcDlg::SaveSettings()
{
	CString sSection(_T("BatchDlg"));
	CWinApp* pApp = ::AfxGetApp();

	pApp->WriteProfileInt(sSection, _T("OptimizationSelection"), m_nOptimizationSelection);
	pApp->WriteProfileInt(sSection, _T("PixelsPercentSel"), m_ShrinkTab.m_nPixelsPercentSel);
	pApp->WriteProfileInt(sSection, _T("ShrinkingPictures"), m_ShrinkTab.m_bShrinkingPictures);
	pApp->WriteProfileInt(sSection, _T("ShrinkingPercent"), m_ShrinkTab.m_nShrinkingPercent);
	pApp->WriteProfileInt(sSection, _T("ShrinkingPixels"), m_ShrinkTab.m_nShrinkingPixels);
	pApp->WriteProfileInt(sSection, _T("JpegQuality"), m_JpegTab.m_nJpegQuality);
	pApp->WriteProfileInt(sSection, _T("TiffJpegQuality"), m_TiffTab.m_nJpegQuality);
	pApp->WriteProfileInt(sSection, _T("TiffCompression"), m_TiffTab.m_nCompression);
	pApp->WriteProfileInt(sSection, _T("TiffForceCompression"), m_TiffTab.m_bForceCompression);
	pApp->WriteProfileString(sSection, _T("PdfPaperSize"), m_TiffTab.m_sPdfPaperSize);
	pApp->WriteProfileInt(sSection, _T("Recursive"), m_bRecursive);
	pApp->WriteProfileString(sSection, _T("Src"), m_sSrc);
	pApp->WriteProfileString(sSection, _T("Dst"), m_sDst);
	pApp->WriteProfileString(sSection, _T("OutputFileName"), m_sOutputFileName);
	pApp->WriteProfileInt(sSection, _T("OutputSelection"), m_nOutputSelection);
	pApp->WriteProfileInt(sSection, _T("InputSelection"), m_nInputSelection);
	pApp->WriteProfileInt(sSection, _T("ExifThumbOperationType"), m_JpegTab.m_nExifThumbOperationType);
	pApp->WriteProfileInt(sSection, _T("ExtChangeType"), m_GeneralTab.m_nExtChangeType);
	pApp->WriteProfileInt(sSection, _T("IptcPriority"), m_GeneralTab.m_nIptcPriority);
	pApp->WriteProfileInt(sSection, _T("AutoOrientate"), m_JpegTab.m_bAutoOrientate);
	pApp->WriteProfileInt(sSection, _T("ForceJpegQuality"), m_JpegTab.m_bForceJpegQuality);
	pApp->WriteProfileInt(sSection, _T("Rename"), m_bRename);
	pApp->WriteProfileString(sSection, _T("RenamePattern"), m_sRename);
	pApp->WriteProfileInt(sSection, _T("Conversion"), m_bConversion);
	pApp->WriteProfileInt(sSection, _T("Sharpen"), m_ShrinkTab.m_bSharpen);
	pApp->WriteProfileInt(sSection, _T("WorkOnAllPages"), m_TiffTab.m_bWorkOnAllPages);
	pApp->WriteProfileInt(sSection, _T("RemoveCom"), m_JpegTab.m_bRemoveCom);
	pApp->WriteProfileInt(sSection, _T("RemoveExif"), m_JpegTab.m_bRemoveExif);
	pApp->WriteProfileInt(sSection, _T("RemoveIcc"), m_JpegTab.m_bRemoveIcc);
	pApp->WriteProfileInt(sSection, _T("RemoveXmp"), m_JpegTab.m_bRemoveXmp);
	pApp->WriteProfileInt(sSection, _T("RemoveIptc"), m_JpegTab.m_bRemoveIptc);
	pApp->WriteProfileInt(sSection, _T("RemoveJfif"), m_JpegTab.m_bRemoveJfif);
	pApp->WriteProfileInt(sSection, _T("RemoveOtherAppSections"), m_JpegTab.m_bRemoveOtherAppSections);
	pApp->WriteProfileInt(sSection, _T("MergeXmp"), m_GeneralTab.m_bMergeXmp);
	pApp->WriteProfileInt(sSection, _T("ExifTimeOffset"), m_GeneralTab.m_bExifTimeOffset);
	pApp->WriteProfileInt(sSection, _T("ExifTimeOffsetSign"), m_GeneralTab.m_nExifTimeOffsetSign);
	pApp->WriteProfileInt(sSection, _T("ExifTimeOffsetDays"), m_GeneralTab.m_nExifTimeOffsetDays);
	pApp->WriteProfileInt(sSection, _T("ExifTimeOffsetHours"), m_GeneralTab.m_nExifTimeOffsetHours);
	pApp->WriteProfileInt(sSection, _T("ExifTimeOffsetMin"), m_GeneralTab.m_nExifTimeOffsetMin);
	pApp->WriteProfileInt(sSection, _T("ExifTimeOffsetSec"), m_GeneralTab.m_nExifTimeOffsetSec);
	pApp->WriteProfileInt(sSection, _T("ListFilesCount"), m_List.GetItemCount());
	pApp->WriteProfileInt(sSection, _T("InitTab"), MAX(0, m_TabAdvSettings.GetSSLActivePage()));
	for (int i = 0 ; i < m_List.GetItemCount() ; i++)
	{
		CString s;
		s.Format(_T("ListFile%i"), i+1);
		pApp->WriteProfileString(sSection, s, m_List.GetItemText(i, LIST_PATH)
											+ m_List.GetItemText(i, LIST_FILENAME));
	}
	unsigned int nSize = sizeof(m_GeneralTab.m_dFrameRate);
	pApp->WriteProfileBinary(sSection, _T("FrameRate"), (LPBYTE)&(m_GeneralTab.m_dFrameRate), nSize);
}

void CBatchProcDlg::OnRadioOptimizeAdvanced() 
{
	UpdateControls();
}

void CBatchProcDlg::OnRadioOptimizeAuto() 
{
	UpdateControls();
}

void CBatchProcDlg::OnRadioInput() 
{
	UpdateControls();
	if (m_nInputSelection == INPUT_LIST)
		DragAcceptFiles(TRUE);
	else
		DragAcceptFiles(FALSE);
}

void CBatchProcDlg::OnRadioInputList() 
{
	UpdateControls();
	if (m_nInputSelection == INPUT_LIST)
		DragAcceptFiles(TRUE);
	else
		DragAcceptFiles(FALSE);
}

void CBatchProcDlg::OnChangeEditSrcdir() 
{
	UpdateData(TRUE);
	m_InputDirLabel.SetLink(m_sSrc);
}

void CBatchProcDlg::OnChangeEditDstdir() 
{
	UpdateData(TRUE);
	m_ChangeNotificationThread.Kill();
	m_OutputDirLabel.SetLink(m_sDst = m_sEditDst);
	if (::IsExistingDir(m_sDst))
		m_ChangeNotificationThread.Start();
	else
		UpdateDstFolderSizes();
}

void CBatchProcDlg::OnChangeEditOutputFileName() 
{
	UpdateData(TRUE);
	m_OutputFileNameLabel.SetLink(m_sOutputFileName);
	UpdateDstFileSize();
}

void CBatchProcDlg::OnRadioOutput() 
{
	UpdateControls();
	m_ChangeNotificationThread.DoUpdate();
}

void CBatchProcDlg::OnRadioOutputFile() 
{
	UpdateControls();
	UpdateDstFileSize();
}

void CBatchProcDlg::OnButtonSrcDir() 
{
	if (UpdateData(TRUE))
	{
		CBrowseDlg dlg(	this,
						&m_sSrc,
						_T("Select the Source Directory"));
		if (dlg.DoModal())
			m_InputDirLabel.SetLink(m_sSrc);

		UpdateData(FALSE);
	}
}

void CBatchProcDlg::OnButtonDstDir() 
{
	if (UpdateData(TRUE))
	{
		CBrowseDlg dlg(	this,
						&m_sEditDst,
						_T("Select the Destination Directory"),
						TRUE);
		if (dlg.DoModal())
		{
			m_ChangeNotificationThread.Kill();
			m_OutputDirLabel.SetLink(m_sDst = m_sEditDst);
			if (::IsExistingDir(m_sDst))
				m_ChangeNotificationThread.Start();
			else
				UpdateDstFolderSizes();
		}
		else
			m_sEditDst = m_sDst; // Restore

		UpdateData(FALSE);
	}
}

void CBatchProcDlg::OnButtonDstFile()
{
	if (UpdateData(TRUE))
	{
		TCHAR FileName[MAX_PATH] = _T("");
		if ( m_sOutputFileName == _T(""))
			 m_sOutputFileName = _T("Pictures.zip");
		_tcscpy(FileName, m_sOutputFileName);
		CSaveFileDlg fd(FALSE,
						NULL,
						NULL,
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						NULL,
						this);
		TCHAR defext[10] = _T("");
		LPTSTR lpPos = _tcsrchr(FileName, _T('.'));
		if (lpPos != NULL)
			_tcscpy(defext, lpPos+1);
		fd.m_ofn.lpstrFile = FileName;
		fd.m_ofn.nMaxFile = MAX_PATH;
		fd.m_ofn.lpstrDefExt = defext;
		fd.m_ofn.lpstrFilter =	_T("Zip File (*.zip)\0*.zip\0")
								_T("Animated GIF (*.gif)\0*.gif\0")						
								_T("Multi-Page TIFF (*.tif)\0*.tif\0")
								_T("Pdf Document (*.pdf)\0*.pdf\0");
		CString defextension = defext;
		defextension.MakeLower();
		if (defextension == _T("zip"))
		{
			fd.m_ofn.nFilterIndex = 1;
		}
		else if (defextension == _T("gif"))
		{
			fd.m_ofn.nFilterIndex = 2;
		}
		else if (::IsTIFFExt(defextension))
		{
			fd.m_ofn.nFilterIndex = 3;
		}
		else if (defextension == _T("pdf"))
		{
			fd.m_ofn.nFilterIndex = 4;
		}
		if (fd.DoModal() == IDOK)
		{
			m_sOutputFileName = FileName;
			m_OutputFileNameLabel.SetLink(m_sOutputFileName);
		}
		UpdateData(FALSE);
		UpdateDstFileSize();
	}
}

void CBatchProcDlg::OnCheckRename() 
{
	UpdateControls();
}

void CBatchProcDlg::OnCheckConversion() 
{
	UpdateControls();
}

void CBatchProcDlg::OnOK() 
{
	if (m_ProcessThread.IsAlive())
	{
		m_ProcessThread.Kill_NoBlocking();
		CButton* pButton = (CButton*)GetDlgItem(IDOK);
		pButton->EnableWindow(FALSE);
	}
	else
	{
		CString sOutputDirectory;

		if (!UpdateData(TRUE))
			return;

		// Input Dir?
		if (m_nInputSelection == INPUT_DIR)
		{
			// Check & Store Vars
			if (!::IsExistingDir(m_sSrc))
			{
				::AfxMessageBox(ML_STRING(1360, "Please Select a Valid Input Directory."),
												MB_OK | MB_ICONSTOP);
				return;
			}
		}
		// Input File List
		else
		{
			if (m_List.GetItemCount() <= 0)
			{
				::AfxMessageBox(ML_STRING(1361, "Please Add Input Files."),
												MB_OK | MB_ICONSTOP);
				return;
			}
		}

		// Output Dir?
		if (m_nOutputSelection == OUTPUT_DIR)
		{
			if (m_sDst == _T(""))
			{
				::AfxMessageBox(ML_STRING(1362, "Please Enter an Output Directory."),
												MB_OK | MB_ICONSTOP);
				return;
			}
			sOutputDirectory = m_sDst;
		}
		// Output File?
		else
		{
			if (m_sOutputFileName == _T(""))
			{
				::AfxMessageBox(ML_STRING(1363, "Please Enter an Output File Name."),
												MB_OK | MB_ICONSTOP);
				return;
			}
			sOutputDirectory = _T("");
		}

		// Rename Pattern Check
		if (m_bRename)
		{
			CString sRename(m_sRename);
			if ((sRename.Find(_T("$file")) < 0) &&
				(sRename.Find(_T('#'))) < 0)
			{
				::AfxMessageBox(ML_STRING(1364, "Rename string is incorrect.\n") +
								ML_STRING(1365, "It must contain at least a # char or the $file place holder."),
												MB_OK | MB_ICONSTOP);
					return;
			}
		}

		// Output Directory
		if (sOutputDirectory != _T(""))
		{
			// Trim
			sOutputDirectory.TrimRight(_T('\\'));
			m_sSrc.TrimRight(_T('\\'));

			// Check
			if ((m_nInputSelection == INPUT_DIR) && (m_sSrc.CompareNoCase(sOutputDirectory) == 0))
			{
				// Conversion
				if (m_bConversion)
				{
					if (::AfxMessageBox(ML_STRING(1366, "You have chosen the Same Path for Input and Output!\n") +
										ML_STRING(1367, "Do You want to Proceed?"),
										MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
						return;
				}
				// Rename
				else if (m_bRename)
				{
					CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_DELETE_INPUTFILE);
					if (pCheck && (pCheck->GetCheck() == 1))
					{
						if (::AfxMessageBox(ML_STRING(1366, "You have chosen the Same Path for Input and Output!\n") +
											ML_STRING(1368, "Files will be Renamed in place.\n") +
											ML_STRING(1369, "Are You Sure You Want to Proceed?"),
											MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
							return;
					}
					else
					{
						if (::AfxMessageBox(ML_STRING(1366, "You have chosen the Same Path for Input and Output!\n") +
											ML_STRING(1367, "Do You want to Proceed?"),
											MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
							return;
					}
				}
			}

			// If not existing
			if (!::IsExistingDir(sOutputDirectory))
			{
				// Create Dir
				if (!::CreateDir(sOutputDirectory))
				{
					::ShowErrorMsg(::GetLastError(), TRUE);
					return;
				}
				else
				{
					// Start Change Notification Thread
					m_ChangeNotificationThread.Start();
				}
			}
		}
		
		// Create & Empty Temp Dir
		CString sTempBatchOutDir;
		sTempBatchOutDir.Format(_T("BatchOut%X"), ::GetCurrentProcessId());
		sTempBatchOutDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempBatchOutDir;
		if (!::IsExistingDir(sTempBatchOutDir))
		{
			if (!::CreateDir(sTempBatchOutDir))
			{
				::ShowErrorMsg(::GetLastError(), TRUE);
				return;
			}
		}
		else
		{
			if (!::DeleteDirContent(sTempBatchOutDir))
			{
				::AfxMessageBox(ML_STRING(1225, "Error While Deleting The Temporary Folder."), MB_OK | MB_ICONSTOP);
				return;
			}
		}

		// Change Button Label
		CButton* pButton = (CButton*)GetDlgItem(IDOK);
		pButton->SetWindowText(_T("Cancel"));

		// Init
		m_bDoCloseDlg = FALSE;
		m_bThreadExited = FALSE;

		// Convert?
		if (m_bConversion)
		{
			if (m_nOptimizationSelection == AUTO_OPT) // Auto
			{
				m_ProcessThread.m_szTempDstDirPath = sTempBatchOutDir;
				m_ProcessThread.m_szOrigDstDirPath = sOutputDirectory;
				m_ProcessThread.m_dwMaxSize = AUTO_SHRINK_MAX_SIZE;
				m_ProcessThread.m_bMaxSizePercent = FALSE;
				m_ProcessThread.m_bShrinkPictures = TRUE;
				m_ProcessThread.m_bSharpen = TRUE;
				m_ProcessThread.m_bWorkOnAllPages = TRUE;
				m_ProcessThread.m_bAutoOrientate = TRUE;
				m_ProcessThread.m_nJpegQuality = DEFAULT_JPEGCOMPRESSION;
				m_ProcessThread.m_bForceJpegQuality = TRUE;
				m_ProcessThread.m_nExifThumbOperationType = CBatchProcJpegTab::THUMB_UPDATE;
				m_ProcessThread.m_bRemoveCom = FALSE;
				m_ProcessThread.m_bRemoveExif = FALSE;
				m_ProcessThread.m_bRemoveIcc = FALSE;
				m_ProcessThread.m_bRemoveXmp = FALSE;
				m_ProcessThread.m_bRemoveIptc = FALSE;
				m_ProcessThread.m_bRemoveJfif = FALSE;
				m_ProcessThread.m_bRemoveOtherAppSections = FALSE;
				m_ProcessThread.m_bMergeXmp = FALSE;
				m_ProcessThread.m_bExifTimeOffset = FALSE;
				m_ProcessThread.m_nExifTimeOffsetSign = 0;
				m_ProcessThread.m_ExifTimeOffset = COleDateTimeSpan(0,0,0,0);
				m_ProcessThread.m_bDeleteInputFile = FALSE;
				m_ProcessThread.m_nTiffJpegQuality = DEFAULT_JPEGCOMPRESSION; 
				m_ProcessThread.m_nTiffCompression = COMPRESSION_JPEG;
				m_ProcessThread.m_bTiffForceCompression = TRUE;
				m_ProcessThread.m_sPdfPaperSize = _T("Fit");
				m_ProcessThread.m_bIptcFromXmpPriority = TRUE;
				m_ProcessThread.m_dFrameRate = DEFAULT_OUTFILE_FRAMERATE;
				if (!m_ProcessThread.Start(THREAD_PRIORITY_BELOW_NORMAL))
					m_bThreadExited = TRUE;
			}
			else
			{
				m_ProcessThread.m_szTempDstDirPath = sTempBatchOutDir;
				m_ProcessThread.m_szOrigDstDirPath = sOutputDirectory;
				m_ProcessThread.m_dwMaxSize = (m_ShrinkTab.m_nPixelsPercentSel == 0) ? m_ShrinkTab.m_nShrinkingPixels : m_ShrinkTab.m_nShrinkingPercent;
				m_ProcessThread.m_bMaxSizePercent = (m_ShrinkTab.m_nPixelsPercentSel == 1);
				m_ProcessThread.m_bShrinkPictures = m_ShrinkTab.m_bShrinkingPictures;
				m_ProcessThread.m_bSharpen = m_ShrinkTab.m_bSharpen;
				m_ProcessThread.m_bWorkOnAllPages = m_TiffTab.m_bWorkOnAllPages;
				m_ProcessThread.m_bAutoOrientate = m_JpegTab.m_bAutoOrientate;
				m_ProcessThread.m_nJpegQuality = m_JpegTab.m_nJpegQuality;
				m_ProcessThread.m_bForceJpegQuality = m_JpegTab.m_bForceJpegQuality;
				m_ProcessThread.m_nExifThumbOperationType = m_JpegTab.m_nExifThumbOperationType;
				m_ProcessThread.m_bRemoveCom = m_JpegTab.m_bRemoveCom;
				m_ProcessThread.m_bRemoveExif = m_JpegTab.m_bRemoveExif;
				m_ProcessThread.m_bRemoveIcc = m_JpegTab.m_bRemoveIcc;
				m_ProcessThread.m_bRemoveXmp = m_JpegTab.m_bRemoveXmp;
				m_ProcessThread.m_bRemoveIptc = m_JpegTab.m_bRemoveIptc;
				m_ProcessThread.m_bRemoveJfif = m_JpegTab.m_bRemoveJfif;
				m_ProcessThread.m_bRemoveOtherAppSections = m_JpegTab.m_bRemoveOtherAppSections;
				m_ProcessThread.m_bMergeXmp = m_GeneralTab.m_bMergeXmp;
				m_ProcessThread.m_bExifTimeOffset = m_GeneralTab.m_bExifTimeOffset;
				m_ProcessThread.m_nExifTimeOffsetSign = m_GeneralTab.m_nExifTimeOffsetSign;
				m_ProcessThread.m_ExifTimeOffset = COleDateTimeSpan(m_GeneralTab.m_nExifTimeOffsetDays,
																	m_GeneralTab.m_nExifTimeOffsetHours,
																	m_GeneralTab.m_nExifTimeOffsetMin,
																	m_GeneralTab.m_nExifTimeOffsetSec);
				m_ProcessThread.m_bDeleteInputFile = FALSE;
				m_ProcessThread.m_nTiffJpegQuality = m_TiffTab.m_nJpegQuality;
				switch (m_TiffTab.m_nCompression)
				{	
					case 0 : 
						m_ProcessThread.m_nTiffCompression = COMPRESSION_NONE;
						break;
					case 1 : 
						m_ProcessThread.m_nTiffCompression = COMPRESSION_CCITTFAX4;
						break;
					case 2 : 
						m_ProcessThread.m_nTiffCompression = COMPRESSION_LZW;
						break;
					case 3 : 
						m_ProcessThread.m_nTiffCompression = COMPRESSION_JPEG;
						break;
					default :
						m_ProcessThread.m_nTiffCompression = COMPRESSION_NONE;
						break;
				}
				m_ProcessThread.m_bTiffForceCompression = m_TiffTab.m_bForceCompression;
				m_ProcessThread.m_sPdfPaperSize = m_TiffTab.m_sPdfPaperSize;
				m_ProcessThread.m_bIptcFromXmpPriority = m_GeneralTab.m_nIptcPriority == 0 ? TRUE : FALSE;
				m_ProcessThread.m_dFrameRate = m_GeneralTab.m_dFrameRate;
				if (!m_ProcessThread.Start(THREAD_PRIORITY_BELOW_NORMAL))
					m_bThreadExited = TRUE;
			}
		}
		else
		{
			m_ProcessThread.m_szTempDstDirPath = sTempBatchOutDir;
			m_ProcessThread.m_szOrigDstDirPath = sOutputDirectory;
			m_ProcessThread.m_dwMaxSize = 0;
			m_ProcessThread.m_bMaxSizePercent = FALSE;
			m_ProcessThread.m_bShrinkPictures = FALSE;
			m_ProcessThread.m_bSharpen = FALSE;
			m_ProcessThread.m_bWorkOnAllPages = FALSE;
			m_ProcessThread.m_bAutoOrientate = FALSE;
			m_ProcessThread.m_nJpegQuality = DEFAULT_JPEGCOMPRESSION;
			m_ProcessThread.m_bForceJpegQuality = FALSE;
			m_ProcessThread.m_nExifThumbOperationType = CBatchProcJpegTab::THUMB_UPDATE;
			m_ProcessThread.m_bRemoveCom = FALSE;
			m_ProcessThread.m_bRemoveExif = FALSE;
			m_ProcessThread.m_bRemoveIcc = FALSE;
			m_ProcessThread.m_bRemoveXmp = FALSE;
			m_ProcessThread.m_bRemoveIptc = FALSE;
			m_ProcessThread.m_bRemoveJfif = FALSE;
			m_ProcessThread.m_bRemoveOtherAppSections = FALSE;
			m_ProcessThread.m_bMergeXmp = FALSE;
			m_ProcessThread.m_bExifTimeOffset = FALSE;
			m_ProcessThread.m_nExifTimeOffsetSign = 0;
			m_ProcessThread.m_ExifTimeOffset = COleDateTimeSpan(0,0,0,0);
			CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_DELETE_INPUTFILE);
			if (m_bRename && pCheck && (pCheck->GetCheck() == 1))
				m_ProcessThread.m_bDeleteInputFile = TRUE;
			else
				m_ProcessThread.m_bDeleteInputFile = FALSE;
			m_ProcessThread.m_nTiffJpegQuality = DEFAULT_JPEGCOMPRESSION; 
			m_ProcessThread.m_nTiffCompression = COMPRESSION_LZW;
			m_ProcessThread.m_bTiffForceCompression = FALSE;
			m_ProcessThread.m_sPdfPaperSize = _T("Fit");
			m_ProcessThread.m_bIptcFromXmpPriority = TRUE;
			m_ProcessThread.m_dFrameRate = DEFAULT_OUTFILE_FRAMERATE;
			if (!m_ProcessThread.Start(THREAD_PRIORITY_BELOW_NORMAL))
				m_bThreadExited = TRUE;
		}

		// Disable All Controls Except Process Button!
		if (!m_bThreadExited)
			EnableAllControls(FALSE, FALSE);
	}
}

BOOL CBatchProcDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CBatchProcDlg::UpdateControls()
{
	CSpinButtonCtrl* pSpin;
	CEdit* pEdit;
	CButton* pRadio;
	CButton* pCheck;
	CButton* pButton;
	CDragDropListCtrl* pList;
	CComboBox* pCombo;

	UpdateData(TRUE);
	
	// Conversion Radio Buttons
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_OPTIMIZE_AUTO);
	if (pRadio)
		pRadio->EnableWindow(m_bConversion);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_OPTIMIZE_ADVANCED);
	if (pRadio)
		pRadio->EnableWindow(m_bConversion);

	// If No Conversion or Auto Optimize Conversion
	if (!m_bConversion ||
		(m_nOptimizationSelection == AUTO_OPT))
	{
		pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PIXELS);
		if (pSpin)
			pSpin->EnableWindow(FALSE);
		pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PERCENT);
		if (pSpin)
			pSpin->EnableWindow(FALSE);
		pSpin = (CSpinButtonCtrl*)m_JpegTab.GetDlgItem(IDC_SPIN_QUALITY);
		if (pSpin)
			pSpin->EnableWindow(FALSE);
		pSpin = (CSpinButtonCtrl*)m_TiffTab.GetDlgItem(IDC_SPIN_QUALITY);
		if (pSpin)
			pSpin->EnableWindow(FALSE);

		pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PIXELS);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PERCENT);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_EDIT_QUALITY);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_LABEL_JPEGQUALITY);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_LABEL_REMOVE);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_LABEL_JPEGQUALITY);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_EDIT_QUALITY);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_LABEL_PAPER_SIZE);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pCombo = (CComboBox*)m_TiffTab.GetDlgItem(IDC_COMBO_PAPER_SIZE);
		if (pCombo)
			pCombo->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_IPTC_PRIORITY);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_COMBO_EXIF_TIMEOFFSET_SIGN);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_DAYS);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_DAYS);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_HOURS);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_HOURS);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_MIN);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_MIN);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_SEC);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_SEC);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_FRAMERATE);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_FRAMERATE);
		if (pEdit)
			pEdit->EnableWindow(FALSE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_FRAMES_SEC);
		if (pEdit)
			pEdit->EnableWindow(FALSE);

		pCheck = (CButton*)m_ShrinkTab.GetDlgItem(IDC_CHECK_SHRINKINGPICTURES);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_AUTOORIENTATE);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_FORCEQUALITY);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_ShrinkTab.GetDlgItem(IDC_CHECK_SHARPEN);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_TiffTab.GetDlgItem(IDC_CHECK_WORKONALLPAGES);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_EXIF);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_ICC);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_XMP);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_COM);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_IPTC);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_JFIF);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_OTHER_APPSECTIONS);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_GeneralTab.GetDlgItem(IDC_CHECK_MERGE_XMP);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_GeneralTab.GetDlgItem(IDC_CHECK_EXIF_TIMEOFFSET);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
		pCheck = (CButton*)m_TiffTab.GetDlgItem(IDC_CHECK_FORCECOMPRESSION);
		if (pCheck)
			pCheck->EnableWindow(FALSE);

		pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PIXELS);
		if (pRadio)
			pRadio->EnableWindow(FALSE);
		pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PERCENT);
		if (pRadio)
			pRadio->EnableWindow(FALSE);
		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION);
		if (pRadio)
			pRadio->EnableWindow(FALSE);
		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_CCITTFAX4);
		if (pRadio)
			pRadio->EnableWindow(FALSE);
		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_LZW);
		if (pRadio)
			pRadio->EnableWindow(FALSE);
		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_JPEG);
		if (pRadio)
			pRadio->EnableWindow(FALSE);
		pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCFROMXMP_PRIORITY);
		if (pRadio)
			pRadio->EnableWindow(FALSE);
		pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCLEGACY_PRIORITY);
		if (pRadio)
			pRadio->EnableWindow(FALSE);

		pCombo = (CComboBox*)m_GeneralTab.GetDlgItem(IDC_COMBO_EXTENSION);
		if (pCombo)
			pCombo->EnableWindow(FALSE);
		pCombo = (CComboBox*)m_JpegTab.GetDlgItem(IDC_COMBO_EXIF_THUMBNAIL);
		if (pCombo)
			pCombo->EnableWindow(FALSE);

		pButton = (CButton*)m_GeneralTab.GetDlgItem(IDC_BUTTON_MERGE_XMP);
		if (pButton)
			pButton->EnableWindow(FALSE);
	}
	else
	{
		pSpin = (CSpinButtonCtrl*)m_JpegTab.GetDlgItem(IDC_SPIN_QUALITY);
		if (pSpin)
			pSpin->EnableWindow(TRUE);
		pSpin = (CSpinButtonCtrl*)m_TiffTab.GetDlgItem(IDC_SPIN_QUALITY);
		if (pSpin)
			pSpin->EnableWindow(TRUE);

		pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_EDIT_QUALITY);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_LABEL_JPEGQUALITY);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_LABEL_REMOVE);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_LABEL_JPEGQUALITY);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_EDIT_QUALITY);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_LABEL_PAPER_SIZE);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pCombo = (CComboBox*)m_TiffTab.GetDlgItem(IDC_COMBO_PAPER_SIZE);
		if (pCombo)
			pCombo->EnableWindow(TRUE);

		pCheck = (CButton*)m_ShrinkTab.GetDlgItem(IDC_CHECK_SHRINKINGPICTURES);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		if (pCheck->GetCheck())
		{
			pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PIXELS);
			if (pRadio)
				pRadio->EnableWindow(TRUE);
			pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PERCENT);
			if (pRadio)
				pRadio->EnableWindow(TRUE);

			pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PIXELS);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PERCENT);
			if (pEdit)
				pEdit->EnableWindow(TRUE);

			pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PIXELS);
			if (pSpin)
				pSpin->EnableWindow(TRUE);
			pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PERCENT);
			if (pSpin)
				pSpin->EnableWindow(TRUE);

			pCheck = (CButton*)m_ShrinkTab.GetDlgItem(IDC_CHECK_SHARPEN);
			if (pCheck)
				pCheck->EnableWindow(TRUE);
		}
		else
		{
			pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PIXELS);
			if (pRadio)
				pRadio->EnableWindow(FALSE);
			pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PERCENT);
			if (pRadio)
				pRadio->EnableWindow(FALSE);

			pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PIXELS);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PERCENT);
			if (pEdit)
				pEdit->EnableWindow(FALSE);

			pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PIXELS);
			if (pSpin)
				pSpin->EnableWindow(FALSE);
			pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PERCENT);
			if (pSpin)
				pSpin->EnableWindow(FALSE);

			pCheck = (CButton*)m_ShrinkTab.GetDlgItem(IDC_CHECK_SHARPEN);
			if (pCheck)
				pCheck->EnableWindow(FALSE);
		}

		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_AUTOORIENTATE);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_FORCEQUALITY);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_EXIF);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_ICC);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_XMP);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_COM);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_IPTC);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_JFIF);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_OTHER_APPSECTIONS);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_TiffTab.GetDlgItem(IDC_CHECK_FORCECOMPRESSION);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		pCheck = (CButton*)m_TiffTab.GetDlgItem(IDC_CHECK_WORKONALLPAGES);
		if (pCheck)
			pCheck->EnableWindow(TRUE);

		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION);
		if (pRadio)
			pRadio->EnableWindow(TRUE);
		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_CCITTFAX4);
		if (pRadio)
			pRadio->EnableWindow(TRUE);
		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_LZW);
		if (pRadio)
			pRadio->EnableWindow(TRUE);
		pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_JPEG);
		if (pRadio)
			pRadio->EnableWindow(TRUE);
		
		pCombo = (CComboBox*)m_JpegTab.GetDlgItem(IDC_COMBO_EXIF_THUMBNAIL);
		if (pCombo)
			pCombo->EnableWindow(TRUE);
		pCombo = (CComboBox*)m_GeneralTab.GetDlgItem(IDC_COMBO_EXTENSION);
		if (pCombo)
			pCombo->EnableWindow(TRUE);
		
		pCheck = (CButton*)m_GeneralTab.GetDlgItem(IDC_CHECK_MERGE_XMP);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		if (pCheck->GetCheck())
		{
			pButton = (CButton*)m_GeneralTab.GetDlgItem(IDC_BUTTON_MERGE_XMP);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCFROMXMP_PRIORITY);
			if (pRadio)
				pRadio->EnableWindow(TRUE);
			pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCLEGACY_PRIORITY);
			if (pRadio)
				pRadio->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_IPTC_PRIORITY);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
		}
		else
		{
			pButton = (CButton*)m_GeneralTab.GetDlgItem(IDC_BUTTON_MERGE_XMP);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCFROMXMP_PRIORITY);
			if (pRadio)
				pRadio->EnableWindow(FALSE);
			pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCLEGACY_PRIORITY);
			if (pRadio)
				pRadio->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_IPTC_PRIORITY);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
		}
		pCheck = (CButton*)m_GeneralTab.GetDlgItem(IDC_CHECK_EXIF_TIMEOFFSET);
		if (pCheck)
			pCheck->EnableWindow(TRUE);
		if (pCheck->GetCheck())
		{
			pCombo = (CComboBox*)m_GeneralTab.GetDlgItem(IDC_COMBO_EXIF_TIMEOFFSET_SIGN);
			if (pCombo)
				pCombo->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_DAYS);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_DAYS);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_HOURS);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_HOURS);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_MIN);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_MIN);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_SEC);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_SEC);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
		}
		else
		{
			pCombo = (CComboBox*)m_GeneralTab.GetDlgItem(IDC_COMBO_EXIF_TIMEOFFSET_SIGN);
			if (pCombo)
				pCombo->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_DAYS);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_DAYS);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_HOURS);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_HOURS);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_MIN);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_MIN);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_SEC);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_SEC);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
		}
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_FRAMERATE);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_FRAMERATE);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
		pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_FRAMES_SEC);
		if (pEdit)
			pEdit->EnableWindow(TRUE);
	}

	switch (m_nInputSelection)
	{
		case INPUT_DIR :
		{
			DragAcceptFiles(FALSE);

			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SRCDIR);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pList = (CDragDropListCtrl*)GetDlgItem(IDC_LIST_INPUT);
			if (pList)
				pList->EnableWindow(FALSE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_SRCDIR);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pCheck = (CButton*)GetDlgItem(IDC_CHECK_RECURSIVE);
			if (pCheck)
				pCheck->EnableWindow(TRUE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_ADD);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_DELETE);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_SELECTALL);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_UP);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_DOWN);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_LOAD);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_SAVE);
			if (pButton)
				pButton->EnableWindow(FALSE);

			break;
		}

		case INPUT_LIST :
		{
			DragAcceptFiles(TRUE);

			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SRCDIR);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pList = (CDragDropListCtrl*)GetDlgItem(IDC_LIST_INPUT);
			if (pList)
				pList->EnableWindow(TRUE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_SRCDIR);
			if (pButton)
				pButton->EnableWindow(FALSE);
			pCheck = (CButton*)GetDlgItem(IDC_CHECK_RECURSIVE);
			if (pCheck)
				pCheck->EnableWindow(FALSE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_ADD);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_DELETE);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_SELECTALL);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_UP);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_DOWN);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_LOAD);
			if (pButton)
				pButton->EnableWindow(TRUE);
			pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_SAVE);
			if (pButton)
				pButton->EnableWindow(TRUE);

			break;
		}

		default:
			break;
	}

	switch (m_nOutputSelection)
	{
		case OUTPUT_DIR :
		{
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DSTDIR);
			if (pEdit)
				pEdit->EnableWindow(TRUE);
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_OUTPUT_FILE);
			if (pEdit)
				pEdit->EnableWindow(FALSE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_DSTDIR);
			if (pButton)
				pButton->EnableWindow(TRUE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_OUTPUT_FILE);
			if (pButton)
				pButton->EnableWindow(FALSE);

			break;
		}

		case OUTPUT_FILE :
		{
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DSTDIR);
			if (pEdit)
				pEdit->EnableWindow(FALSE);
			pEdit = (CEdit*)GetDlgItem(IDC_EDIT_OUTPUT_FILE);
			if (pEdit)
				pEdit->EnableWindow(TRUE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_DSTDIR);
			if (pButton)
				pButton->EnableWindow(FALSE);

			pButton = (CButton*)GetDlgItem(IDC_BUTTON_OUTPUT_FILE);
			if (pButton)
				pButton->EnableWindow(TRUE);

			break;
		}

		default:
			break;
	}

	// Rename
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_DELETE_INPUTFILE);
	if (pCheck)
	{
		pCheck->EnableWindow(m_bRename && !m_bConversion);
		if (!m_bRename || m_bConversion)
			pCheck->SetCheck(0);
	}
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_RENAME);
	if (pEdit)
		pEdit->EnableWindow(m_bRename);
	pEdit = (CEdit*)GetDlgItem(IDC_LABEL_ONEDIGIT);
	if (pEdit)
		pEdit->EnableWindow(m_bRename);
	pEdit = (CEdit*)GetDlgItem(IDC_LABEL_ORIGINALFILENAME);
	if (pEdit)
		pEdit->EnableWindow(m_bRename);
}

void CBatchProcDlg::EnableAllControls(BOOL bEnable, BOOL bIncludeProcessButton)
{
	CSpinButtonCtrl* pSpin;
	CEdit* pEdit;
	CButton* pRadio;
	CButton* pCheck;
	CButton* pButton;
	CDragDropListCtrl* pList;
	CComboBox* pCombo;
	
	UpdateData(TRUE);

	// Spin Controls
	pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PIXELS);
	if (pSpin)
		pSpin->EnableWindow(bEnable);
	pSpin = (CSpinButtonCtrl*)m_ShrinkTab.GetDlgItem(IDC_SPIN_PERCENT);
	if (pSpin)
		pSpin->EnableWindow(bEnable);
	pSpin = (CSpinButtonCtrl*)m_JpegTab.GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->EnableWindow(bEnable);
	pSpin = (CSpinButtonCtrl*)m_TiffTab.GetDlgItem(IDC_SPIN_QUALITY);
	if (pSpin)
		pSpin->EnableWindow(bEnable);

	// Edit Controls
	pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PIXELS);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_ShrinkTab.GetDlgItem(IDC_EDIT_PERCENT);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_EDIT_QUALITY);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SRCDIR);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DSTDIR);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_OUTPUT_FILE);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_RENAME);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_LABEL_JPEGQUALITY);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_JpegTab.GetDlgItem(IDC_LABEL_REMOVE);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_LABEL_JPEGQUALITY);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_EDIT_QUALITY);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_LABEL_ONEDIGIT);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_LABEL_ORIGINALFILENAME);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DST_SIZE);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_IPTC_PRIORITY);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_COMBO_EXIF_TIMEOFFSET_SIGN);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_DAYS);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_DAYS);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_HOURS);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_HOURS);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_MIN);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_MIN);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_EXIF_TIMEOFFSET_SEC);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_EXIF_TIMEOFFSET_SEC);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_FRAMERATE);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_EDIT_FRAMERATE);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_GeneralTab.GetDlgItem(IDC_LABEL_FRAMES_SEC);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	pEdit = (CEdit*)m_TiffTab.GetDlgItem(IDC_LABEL_PAPER_SIZE);
	if (pEdit)
		pEdit->EnableWindow(bEnable);
	
	// Check Buttons
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_RECURSIVE);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_ShrinkTab.GetDlgItem(IDC_CHECK_SHRINKINGPICTURES);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_AUTOORIENTATE);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_FORCEQUALITY);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_ShrinkTab.GetDlgItem(IDC_CHECK_SHARPEN);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_TiffTab.GetDlgItem(IDC_CHECK_WORKONALLPAGES);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_RENAME);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_DELETE_INPUTFILE);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_CONVERSION);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_EXIF);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_XMP);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_ICC);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_COM);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_IPTC);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_JFIF);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_JpegTab.GetDlgItem(IDC_CHECK_REMOVE_OTHER_APPSECTIONS);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_GeneralTab.GetDlgItem(IDC_CHECK_MERGE_XMP);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_GeneralTab.GetDlgItem(IDC_CHECK_EXIF_TIMEOFFSET);
	if (pCheck)
		pCheck->EnableWindow(bEnable);
	pCheck = (CButton*)m_TiffTab.GetDlgItem(IDC_CHECK_FORCECOMPRESSION);
	if (pCheck)
		pCheck->EnableWindow(bEnable);

	// Radio Buttons
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_INPUT);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_INPUT_LIST);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_OUTPUT);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_OUTPUT_FILE);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_OPTIMIZE_AUTO);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)GetDlgItem(IDC_RADIO_OPTIMIZE_ADVANCED);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PIXELS);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_ShrinkTab.GetDlgItem(IDC_RADIO_PERCENT);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_CCITTFAX4);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_LZW);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_TiffTab.GetDlgItem(IDC_RADIO_COMPRESSION_JPEG);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCFROMXMP_PRIORITY);
	if (pRadio)
		pRadio->EnableWindow(bEnable);
	pRadio = (CButton*)m_GeneralTab.GetDlgItem(IDC_RADIO_IPTCLEGACY_PRIORITY);
	if (pRadio)
		pRadio->EnableWindow(bEnable);

	// Buttons
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_DSTDIR);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_OUTPUT_FILE);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_SRCDIR);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_ADD);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_DELETE);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_SELECTALL);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_UP);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_DOWN);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_LOAD);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)GetDlgItem(IDC_BUTTON_LIST_SAVE);
	if (pButton)
		pButton->EnableWindow(bEnable);
	pButton = (CButton*)m_GeneralTab.GetDlgItem(IDC_BUTTON_MERGE_XMP);
	if (pButton)
		pButton->EnableWindow(bEnable);
	if (bIncludeProcessButton)
	{
		pButton = (CButton*)GetDlgItem(IDOK);
		if (pButton)
			pButton->EnableWindow(bEnable);
	}

	// Combo Box
	pCombo = (CComboBox*)m_GeneralTab.GetDlgItem(IDC_COMBO_EXTENSION);
	if (pCombo)
		pCombo->EnableWindow(bEnable);
	pCombo = (CComboBox*)m_JpegTab.GetDlgItem(IDC_COMBO_EXIF_THUMBNAIL);
	if (pCombo)
		pCombo->EnableWindow(bEnable);
	pCombo = (CComboBox*)m_TiffTab.GetDlgItem(IDC_COMBO_PAPER_SIZE);
	if (pCombo)
		pCombo->EnableWindow(bEnable);

	// List Control
	pList = (CDragDropListCtrl*)GetDlgItem(IDC_LIST_INPUT);
	if (pList)
		pList->EnableWindow(bEnable);

	// Accept Drops
	if (bEnable && (m_nInputSelection == INPUT_LIST))
		DragAcceptFiles(TRUE);
	else
		DragAcceptFiles(FALSE);
}

BOOL CBatchProcDlg::IsAlreadyInList(CString sFileName, int nStart/*=-1*/) 
{
	CString sShortFileName = ::GetShortFileName(sFileName);
	CString sPath = ::GetDriveAndDirName(sFileName);
	LVFINDINFO FindInfo;
	FindInfo.flags = LVFI_STRING;
    FindInfo.psz = sShortFileName;
	int nItem = m_List.FindItem(&FindInfo, nStart);
	
	if (nItem >= 0)
	{
		// Make Sure the Path is also the same
		if (m_List.GetItemText(nItem, LIST_PATH) == sPath) 
			return TRUE;
		else
			return IsAlreadyInList(sFileName, nItem);
	}
	else
		return FALSE;
}

void CBatchProcDlg::SetExifDateColumn(int nItem)
{
	CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
	if (pListElement)
	{
		CTime Time;
		CString sTime;
		BOOL bUpdateColumnWidth;
		LVITEM lvItem;
		lvItem.iItem = nItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_EXIFDATE;
		if (pListElement->m_pDibHdr->GetExifInfo() &&
			CString(pListElement->m_pDibHdr->GetExifInfo()->DateTime) != _T(""))
		{
			Time = CMetadata::GetDateTimeFromExifString(CString(pListElement->m_pDibHdr->GetExifInfo()->DateTime));
			sTime = ::MakeDateLocalFormat(Time) + _T(" ") + ::MakeTimeLocalFormat(Time, TRUE);
			lvItem.pszText = sTime.GetBuffer(sTime.GetLength());
			bUpdateColumnWidth = TRUE;
		}
		else
		{
			lvItem.pszText = _T("");
			bUpdateColumnWidth = FALSE;
		}
		m_List.SetItem(&lvItem);
		sTime.ReleaseBuffer();
		if (bUpdateColumnWidth)
			m_List.SetColumnWidth(LIST_EXIFDATE, LVSCW_AUTOSIZE);
	}
}

void CBatchProcDlg::SetCreatedDateColumn(int nItem)
{
	CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
	if (pListElement)
	{
		CTime Time;
		CString sTime;
		LVITEM lvItem;
		lvItem.iItem = nItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_CREATEDDATE;
		Time = GetCreatedFileTime(pListElement->GetFileName());
		sTime = ::MakeDateLocalFormat(Time) + _T(" ") + ::MakeTimeLocalFormat(Time, TRUE);
		lvItem.pszText = sTime.GetBuffer(sTime.GetLength());
		m_List.SetItem(&lvItem);
		sTime.ReleaseBuffer();
		m_List.SetColumnWidth(LIST_CREATEDDATE, LVSCW_AUTOSIZE);
	}
}

void CBatchProcDlg::SetModifiedDateColumn(int nItem)
{
	CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
	if (pListElement)
	{
		CTime Time;
		CString sTime;
		LVITEM lvItem;
		lvItem.iItem = nItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_MODIFIEDDATE;
		Time = GetModifiedFileTime(pListElement->GetFileName());
		sTime = ::MakeDateLocalFormat(Time) + _T(" ") + ::MakeTimeLocalFormat(Time, TRUE);
		lvItem.pszText = sTime.GetBuffer(sTime.GetLength());
		m_List.SetItem(&lvItem);
		sTime.ReleaseBuffer();
		m_List.SetColumnWidth(LIST_MODIFIEDDATE, LVSCW_AUTOSIZE);
	}
}

void CBatchProcDlg::SetFileSizeColumn(int nItem)
{
	CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
	if (pListElement)
	{
		ULARGE_INTEGER FileSize;
		CString sFileSize;
		LVITEM lvItem;
		lvItem.iItem = nItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_FILESIZE;
		FileSize = ::GetFileSize64(pListElement->GetFileName());
		sFileSize.Format(_T("%I64d"),	(FileSize.QuadPart >= 1024) ?
										FileSize.QuadPart >> 10 :
										FileSize.QuadPart);
		sFileSize = ::FormatIntegerNumber(sFileSize);
		sFileSize += (FileSize.QuadPart >= 1024) ?
					_T(" ") + ML_STRING(1243, "KB") : _T(" ") + ML_STRING(1244, "Bytes");
		lvItem.pszText = sFileSize.GetBuffer(sFileSize.GetLength());
		m_List.SetItem(&lvItem);
		sFileSize.ReleaseBuffer();
	}
}

void CBatchProcDlg::SetImageSizeColumn(int nItem)
{
	CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
	if (pListElement)
	{
		CString sImageSize;
		LVITEM lvItem;
		lvItem.iItem = nItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_IMAGESIZE;
		if (pListElement->m_pDibHdr->GetImageSize() > 0)
		{
			sImageSize.Format(_T("%i"),	(pListElement->m_pDibHdr->GetImageSize() >= 1024) ?
										pListElement->m_pDibHdr->GetImageSize() >> 10 :
										pListElement->m_pDibHdr->GetImageSize());
			sImageSize = ::FormatIntegerNumber(sImageSize);
			sImageSize += (pListElement->m_pDibHdr->GetImageSize() >= 1024) ?
						_T(" ") + ML_STRING(1243, "KB") : _T(" ") + ML_STRING(1244, "Bytes");
			lvItem.pszText = sImageSize.GetBuffer(sImageSize.GetLength());
		}
		else
			lvItem.pszText = _T("");
		m_List.SetItem(&lvItem);
		sImageSize.ReleaseBuffer();
	}
}

void CBatchProcDlg::SetWidthColumn(int nItem)
{
	CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
	if (pListElement)
	{
		CString sWidth;
		LVITEM lvItem;
		lvItem.iItem = nItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_WIDTH;
		if (pListElement->m_pDibHdr->GetBMIH() &&
			pListElement->m_pDibHdr->GetWidth() > 0)
		{
			sWidth.Format(_T("%i"), pListElement->m_pDibHdr->GetWidth());
			lvItem.pszText = sWidth.GetBuffer(sWidth.GetLength());
		}
		else
			lvItem.pszText = _T("");
		m_List.SetItem(&lvItem);
		sWidth.ReleaseBuffer();
	}
}

void CBatchProcDlg::SetHeightColumn(int nItem)
{
	CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
	if (pListElement)
	{
		CString sHeight;
		LVITEM lvItem;
		lvItem.iItem = nItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_HEIGHT;
		if (pListElement->m_pDibHdr->GetBMIH() &&
			pListElement->m_pDibHdr->GetHeight() > 0)
		{
			sHeight.Format(_T("%i"), pListElement->m_pDibHdr->GetHeight());
			lvItem.pszText = sHeight.GetBuffer(sHeight.GetLength());
		}
		else
			lvItem.pszText = _T("");
		m_List.SetItem(&lvItem);
		sHeight.ReleaseBuffer();
	}
}

BOOL CBatchProcDlg::ListAdd(int nItem, CString sFileName)
{
	if (!IsAlreadyInList(sFileName))
	{
		// Vars
		LVITEM lvItem;
		lvItem.iItem = nItem;

		// Allocate Pointers & Init Dib Static
		CListElement* pListElement = (CListElement*)new CListElement;
		if (!pListElement)
			return FALSE;
		pListElement->m_pDibHdr = new CDib;
		if (!pListElement->m_pDibHdr)
		{
			delete pListElement;
			return FALSE;
		}
		pListElement->m_pDibFull = new CDib;
		if (!pListElement->m_pDibFull)
		{
			delete pListElement->m_pDibHdr;
			delete pListElement;
			return FALSE;
		}
		pListElement->m_pAlphaRenderedDib = new CDib;
		if (!pListElement->m_pAlphaRenderedDib)
		{
			delete pListElement->m_pDibHdr;
			delete pListElement->m_pDibFull;
			delete pListElement;
			return FALSE;
		}
		pListElement->m_pcsDibHdr = new CTryEnterCriticalSection;
		if (!pListElement->m_pcsDibHdr)
		{
			delete pListElement->m_pAlphaRenderedDib;
			delete pListElement->m_pDibHdr;
			delete pListElement->m_pDibFull;
			delete pListElement;
			return FALSE;
		}
		pListElement->m_pcsDibFull = new CTryEnterCriticalSection;
		if (!pListElement->m_pcsDibFull)
		{
			delete pListElement->m_pcsDibHdr;
			delete pListElement->m_pAlphaRenderedDib;
			delete pListElement->m_pDibHdr;
			delete pListElement->m_pDibFull;
			delete pListElement;
			return FALSE;
		}
		pListElement->m_DibStatic.SetBorders(CRect(2,2,2,2));
		pListElement->m_DibStatic.SetNotifyHwnd(GetSafeHwnd());
		pListElement->m_DibStatic.SetDibHdrPointer(pListElement->m_pDibHdr);
		pListElement->m_DibStatic.SetDibFullPointer(pListElement->m_pDibFull);
		pListElement->m_DibStatic.SetAlphaRenderedDibPointer(pListElement->m_pAlphaRenderedDib);
		pListElement->m_DibStatic.SetDibHdrCS(pListElement->m_pcsDibHdr);
		pListElement->m_DibStatic.SetDibFullCS(pListElement->m_pcsDibFull);
		pListElement->m_DibStatic.SetThumbLoadThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);

		// Short File Name
		pListElement->m_sShortFileName = ::GetShortFileName(sFileName);
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iSubItem = LIST_FILENAME;
		lvItem.pszText = pListElement->m_sShortFileName.GetBuffer(pListElement->m_sShortFileName.GetLength());
		lvItem.lParam = (LPARAM)pListElement;
		m_List.InsertItem(&lvItem);
		pListElement->m_sShortFileName.ReleaseBuffer();
		m_List.SetColumnWidth(LIST_FILENAME, LVSCW_AUTOSIZE);

		// Path
		pListElement->m_sPath = ::GetDriveAndDirName(sFileName);
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = LIST_PATH;
		lvItem.pszText = pListElement->m_sPath.GetBuffer(pListElement->m_sPath.GetLength());
		m_List.SetItem(&lvItem);
		pListElement->m_sPath.ReleaseBuffer();
		m_List.SetColumnWidth(LIST_PATH, LVSCW_AUTOSIZE);

		// EXIF Date
		SetExifDateColumn(nItem);

		// Created Date
		SetCreatedDateColumn(nItem);

		// Modified Date
		SetModifiedDateColumn(nItem);

		// File Size
		SetFileSizeColumn(nItem);

		// Image Size
		SetImageSizeColumn(nItem);

		// Width
		SetWidthColumn(nItem);

		// Height
		SetHeightColumn(nItem);

		return TRUE;
	}
	else
		return FALSE;
}

void CBatchProcDlg::OnButtonListAdd() 
{
	TCHAR* FileNames = new TCHAR[MAX_FILEDLG_PATH];
	TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
	FileNames[0] = _T('\0');
	InitDir[0] = _T('\0');
	((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir.TrimRight(_T('\\'));
	if (::IsExistingDir(((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir))
		_tcscpy(InitDir, (LPCTSTR)((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir);
	else
		_tcscpy(InitDir, (LPCTSTR)::GetSpecialFolderPath(CSIDL_MYPICTURES));
	CPreviewFileDlg dlgFile(TRUE,
							((CUImagerApp*)::AfxGetApp())->m_bFileDlgPreview,
							NULL,
							NULL,
							NULL,
							::AfxGetMainFrame());
	dlgFile.m_ofn.lpstrInitialDir = InitDir;
	dlgFile.m_ofn.lpstrDefExt = _T("bmp");
	dlgFile.m_ofn.lpstrCustomFilter = NULL;
	dlgFile.m_ofn.lpstrFilter = 
				_T("Supported Files (*.bmp;*.gif;*.jpg;*.tif;*.png;*.pcx;*.emf)\0")
				_T("*.bmp;*.dib;*.gif;*.png;*.jpg;*.jpeg;*.jpe;*.thm;*.tif;*.tiff;*.jfx;*.pcx;*.emf\0")
				_T("All Files (*.*)\0*.*\0")
				_T("Windows Bitmap (*.bmp;*.dib)\0*.bmp;*.dib\0")
				_T("Graphics Interchange Format (*.gif)\0*.gif\0")
				_T("Portable Network Graphics (*.png)\0*.png\0")
				_T("JPEG File Interchange Format (*.jpg;*.jpeg;*.jpe;*.thm)\0*.jpg;*.jpeg;*.jpe;*.thm\0")
				_T("Tag Image File Format (*.tif;*.tiff;*.jfx)\0*.tif;*.tiff;*.jfx\0")
				_T("PC Paintbrush (*.pcx)\0*.pcx\0")
				_T("Enhanced Metafile (*.emf)\0*.emf\0");
	dlgFile.m_ofn.Flags |= OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;
	dlgFile.m_ofn.lpstrFile = FileNames;
	dlgFile.m_ofn.nMaxFile = MAX_FILEDLG_PATH;

	// Open File Dialog
	if (dlgFile.DoModal() == IDOK)
	{
		// Disable All Controls, Including Process Button!
		EnableAllControls(FALSE, TRUE);

		// Update preview flag
		((CUImagerApp*)::AfxGetApp())->m_bFileDlgPreview = dlgFile.m_bPreview;
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
														_T("FileDlgPreview"),
														((CUImagerApp*)::AfxGetApp())->m_bFileDlgPreview);

		// Get File(s)
		TCHAR* sSource = FileNames;
		TCHAR FileName[MAX_PATH];
		FileName[0] = _T('\0');
		TCHAR Path[MAX_PATH];
		Path[0] = _T('\0');
		_tcscpy(Path, (LPCTSTR)sSource);
		while (*sSource != 0)
			sSource++;
		sSource++; // Skip the 0.
		CString sBuf;
		int nItem;
		POSITION pos = m_List.GetFirstSelectedItemPosition();
		if (pos == NULL)
			nItem = 0;
		else
			nItem = m_List.GetNextSelectedItem(pos) + 1;
		if (*sSource == 0) // If two zeros -> single file selected
		{
			if (::IsExistingFile(Path))
			{
				// List Add
				m_bEnableUpdateDibs = FALSE;
				if (ListAdd(nItem, Path))
				{
					// Start Loading Dib
					StartLoadDibs();
				}
				else
				{
					// Re-Enable Update Dibs
					m_bEnableUpdateDibs = TRUE;

					// Re-Enable Controls
					EnableAllControls(TRUE, TRUE);
					UpdateControls();
				}
			}
			else
			{
				// Re-Enable Controls
				EnableAllControls(TRUE, TRUE);
				UpdateControls();
			}

			// Store Last Opened Directory
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];
			_tsplitpath(Path, szDrive, szDir, NULL, NULL);
			((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir = CString(szDrive) + CString(szDir);
			((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir.TrimRight(_T('\\'));
			((CUImagerApp*)::AfxGetApp())->WriteProfileString(	_T("GeneralApp"),
																_T("LastOpenedDir"),
																((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir);
		}
		else // multiple files selected
		{
			m_bEnableUpdateDibs = FALSE;
			while (*sSource != 0) // If 0 -> end of file list.
			{
				_tcscpy(FileName, (LPCTSTR)Path);
				_tcscat(FileName, (LPCTSTR)_T("\\"));
				_tcscat(FileName, (LPCTSTR)sSource);

				if (::IsExistingFile(FileName))
				{
					if (ListAdd(nItem, FileName))
						nItem++;
				}
				
				while (*sSource != 0)
					sSource++;
				sSource++; // Skip the 0.
			}

			// Store Last Opened Directory
			((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir = CString(Path);
			((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir.TrimRight(_T('\\'));
			((CUImagerApp*)::AfxGetApp())->WriteProfileString(	_T("GeneralApp"),
																_T("LastOpenedDir"),
																((CUImagerApp*)::AfxGetApp())->m_sLastOpenedDir);

			// Start Loading Dibs
			StartLoadDibs();
		}
	}
	else
	{
		((CUImagerApp*)::AfxGetApp())->m_bFileDlgPreview = dlgFile.m_bPreview;
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
														_T("FileDlgPreview"),
														((CUImagerApp*)::AfxGetApp())->m_bFileDlgPreview);
	}

	// Free
	delete [] FileNames;
	delete [] InitDir;
}

void CBatchProcDlg::DeleteListElement(CBatchProcDlg::CListElement* pListElement)
{
	if (pListElement)
	{
		// Remove Post Done Window
		pListElement->m_DibStatic.SetNotifyHwnd(NULL);

		// Kill Threads
		pListElement->m_DibStatic.GetThumbLoadThread()->Kill();
		pListElement->m_DibStatic.GetGifAnimationThread()->Kill();

		// Delete Dib Hdr
		if (pListElement->m_pDibHdr)
		{
			pListElement->m_DibStatic.SetDibHdrPointer(NULL);
			delete pListElement->m_pDibHdr;
		}
		
		// Delete Dib Full
		if (pListElement->m_pDibFull)
		{
			pListElement->m_DibStatic.SetDibFullPointer(NULL);
			delete pListElement->m_pDibFull;
		}

		// Delete Alpha Rendered Dib
		if (pListElement->m_pAlphaRenderedDib)
		{
			pListElement->m_DibStatic.SetAlphaRenderedDibPointer(NULL);
			delete pListElement->m_pAlphaRenderedDib;
		}

		// Delete Hdr Critical Section
		if (pListElement->m_pcsDibHdr)
		{
			pListElement->m_DibStatic.SetDibHdrCS(NULL);
			delete pListElement->m_pcsDibHdr;
		}

		// Delete Full Critical Section
		if (pListElement->m_pcsDibFull)
		{
			pListElement->m_DibStatic.SetDibFullCS(NULL);
			delete pListElement->m_pcsDibFull;
		}

		// Delete List Element
		delete pListElement;
	}
}

void CBatchProcDlg::ListDeleteAll() 
{
	// Clear Dibs
	ClearDibs();

	// Disable Updates
	m_bEnableUpdateDibs = FALSE;

	// Delete All
	for (int nItem = 0 ; nItem < m_List.GetItemCount() ; nItem++)
	{
		CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
		DeleteListElement(pListElement);
	}
	m_List.DeleteAllItems();

	// Update Dibs
	m_bEnableUpdateDibs = TRUE;
	OnUpdateDibsPostpone(0, 0);
}

void CBatchProcDlg::OnButtonListDelete() 
{
	int nItem = 0;
	POSITION pos;
	if (m_List.GetSelectedCount() > 0)
	{
		// Clear Dibs, do not paint
		// -> paint later when calling UpdateDibs
		ClearDibs(FALSE);

		// Disable Updates
		m_bEnableUpdateDibs = FALSE;

		// Delete
		while (pos = m_List.GetFirstSelectedItemPosition())
		{
			nItem = m_List.GetNextSelectedItem(pos);
			CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
			DeleteListElement(pListElement);
			m_List.DeleteItem(nItem);
		}

		// New Selection
		if (m_List.GetItemCount() > 0)
		{
			if (nItem >= m_List.GetItemCount())
				nItem--;
			m_List.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
		}

		// Update Dibs
		m_bEnableUpdateDibs = TRUE;
		OnUpdateDibsPostpone(0, 0);
	}
}

BOOL CBatchProcDlg::ListLoad(CString sFileName) 
{
	LPBYTE pData = NULL;
	try
	{
		// BOM
		const unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};

		// Open File
		CFile f(sFileName,
				CFile::modeRead |
				CFile::shareDenyWrite);
		DWORD dwLength = (DWORD)f.GetLength();
		if (dwLength == 0)
			return FALSE;

		// Allocate Buffer
		pData = new BYTE [dwLength];
		if (!pData)
			return FALSE;
		
		// Read Data
		dwLength = f.Read(pData, dwLength);
		CString s;
		if (dwLength >= 3)
		{
			if (memcmp(pData, BOM, 3) == 0)
				s = ::FromUTF8(pData + 3, dwLength - 3);
			else
				s = ::FromUTF8(pData, dwLength);
		}

		// Empty List
		if (m_List.GetItemCount() > 0)
			ListDeleteAll();
	
		// Get Lines
		int nItem = 0;
		m_bEnableUpdateDibs = FALSE;
		int nIndex;
		while ((nIndex = s.FindOneOf(_T("\r\n"))) >= 0)
		{
			sFileName = s.Left(nIndex);
			if (::IsExistingFile(sFileName))
			{
				if (ListAdd(nItem, sFileName))
					nItem++;
			}
			s.Delete(0, nIndex);
			s.TrimLeft(_T("\r\n")); // Remove '\r' or '\n' or a combination of them
		}

		// Last Line
		if (::IsExistingFile(s))
		{
			if (ListAdd(nItem, s))
				nItem++;
		}

		// Start Loading Dibs
		StartLoadDibs();

		// Free
		delete [] pData;

		return TRUE;
	}
	catch (CFileException* e)
	{
		if (pData)
			delete [] pData;
		e->ReportError();
		e->Delete();

		// Re-Enable Controls
		EnableAllControls(TRUE, TRUE);
		
		return FALSE;
	}
}

void CBatchProcDlg::OnButtonListLoad() 
{
	CNoVistaFileDlg fd(	TRUE,
						_T("txt"),
						_T(""),
						OFN_HIDEREADONLY, // Hides the Read Only check box
						_T("Text Files (*.txt)|*.txt||"));
	if (fd.DoModal() == IDOK)
	{
		// Disable All Controls, Including Process Button!
		EnableAllControls(FALSE, TRUE);
		
		// Load List
		ListLoad(fd.GetPathName());
	}
}

BOOL CBatchProcDlg::ListSave(CString sFileName) 
{
	CString s(_T("\r\n"));	// To be backwards compatible!
							// FreeVimager <= 1.1.0 will read BOM + "\r\n" as a bad file name
							// if no "\r\n" FreeVimager <= 1.1.0 would discard the first file name!
	for (int nItem = 0 ; nItem < m_List.GetItemCount() ; nItem++)
	{
		CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
		if (pListElement)
			s = s + pListElement->GetFileName() + _T("\r\n");
	}
	
	LPBYTE pData = NULL;
	try
	{
		int nSize = ::ToUTF8(s, &pData);
		if (pData)
		{
			const unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
			CFile f(sFileName,
					CFile::modeCreate		|
					CFile::modeWrite		|
					CFile::shareDenyWrite);
			f.Write(BOM, 3);
			f.Write(pData, nSize);
			delete [] pData;
			return TRUE;
		}
		else
			return FALSE;
	}
	catch (CFileException* e)
	{
		if (pData)
			delete [] pData;
		e->ReportError();
		e->Delete();
		return FALSE;
	}
}

void CBatchProcDlg::OnButtonListSave() 
{
	if (m_List.GetItemCount() <= 0)
		return;

	CNoVistaFileDlg fd(	FALSE,
						_T("txt"),
						_T("Files List.txt"),
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						_T("Text Files (*.txt)|*.txt||"));
	if (fd.DoModal() == IDOK)
		ListSave(fd.GetPathName());
}

void CBatchProcDlg::OnKeydownListInput(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

	if (pLVKeyDow)
	{
		if (pLVKeyDow->wVKey == VK_DELETE)
		{
			OnButtonListDelete();
		}
	}

	*pResult = 0;
}

void CBatchProcDlg::OnButtonListDown() 
{
	if (m_List.GetSelectedCount() > 0)
	{
		CPoint ptBottomScroll(8, 0);
		CRect rcClient;
		m_List.GetClientRect(rcClient);
		int col;
		int nColumns = 1;
		CHeaderCtrl* pHeader = (CHeaderCtrl*)m_List.GetHeaderCtrl();
		if (pHeader)
			nColumns = pHeader->GetItemCount();
		ptBottomScroll.y = rcClient.bottom - 6;
		int nBottomScrollItem = m_List.HitTest(ptBottomScroll);
		CString* Strings = new CString[nColumns];
		DWORD dwData;
		for (int nItem = m_List.GetItemCount() - 2 ; nItem >= 0  ; nItem--)
		{
			if (m_List.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED)
			{
				// Disable Updates
				m_bEnableUpdateDibs = FALSE;

				// Scroll-Down
				if (nItem == nBottomScrollItem)
					m_List.SendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), NULL);

				// Reached the Bottom of List?
				if (m_List.GetItemState(nItem + 1, LVIS_SELECTED) == LVIS_SELECTED)
				{
					m_bEnableUpdateDibs = TRUE;
					continue;
				}

				// Old Item
				m_List.SetItemState(nItem, ~LVIS_SELECTED, LVIS_SELECTED);
				for (col = 0 ; col < nColumns ; col++)
					Strings[col] = m_List.GetItemText(nItem, col);
				dwData = m_List.GetItemData(nItem);
				m_List.DeleteItem(nItem);

				// New Item
				m_List.InsertItem(nItem + 1, Strings[0]);
				for (col = 1 ; col < nColumns ; col++)
					m_List.SetItemText(nItem + 1, col, Strings[col]);
				m_List.SetItemData(nItem + 1, dwData);
				m_bEnableUpdateDibs = TRUE;
				m_List.SetItemState(nItem + 1, LVIS_SELECTED, LVIS_SELECTED);
			}
		}

		// Free
		delete [] Strings;
	}
}

void CBatchProcDlg::OnButtonListUp() 
{
	CPoint ptTopScroll(8, 0);
	CRect rcClient;
	m_List.GetClientRect(rcClient);
	int col;
	int nColumns = 1;
	CRect rcHeader;
	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_List.GetHeaderCtrl();
	if (pHeader)
	{
		nColumns = pHeader->GetItemCount();
		pHeader->GetClientRect(rcHeader);
		rcClient.top += rcHeader.Height();
	}
	ptTopScroll.y = rcClient.top + 6;
	int nTopScrollItem = m_List.HitTest(ptTopScroll);
	CString* Strings = new CString[nColumns];
	int nItem;
	DWORD dwData;
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	while (pos)
	{
		if ((nItem = m_List.GetNextSelectedItem(pos)) > 0)
		{
			// Disable Updates
			m_bEnableUpdateDibs = FALSE;

			// Scroll-Up
			if (nItem == nTopScrollItem)
				m_List.SendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), NULL);

			// Reached the Top of List?
			if (m_List.GetItemState(nItem - 1, LVIS_SELECTED) == LVIS_SELECTED)
			{
				m_bEnableUpdateDibs = TRUE;
				continue;
			}

			// Old Item
			m_List.SetItemState(nItem, ~LVIS_SELECTED, LVIS_SELECTED);
			for (col = 0 ; col < nColumns ; col++)
				Strings[col] = m_List.GetItemText(nItem, col);
			dwData = m_List.GetItemData(nItem);
			m_List.DeleteItem(nItem);

			// New Item
			m_List.InsertItem(nItem - 1, Strings[0]);
			for (col = 1 ; col < nColumns ; col++)
				m_List.SetItemText(nItem - 1, col, Strings[col]);
			m_List.SetItemData(nItem - 1, dwData);
			m_bEnableUpdateDibs = TRUE;
			m_List.SetItemState(nItem - 1, LVIS_SELECTED, LVIS_SELECTED);
		}
	}

	// Free
	delete [] Strings;
}

void CBatchProcDlg::OnButtonListSelectall() 
{
	m_List.SetFocus();
	for (int nItem = 0 ; nItem < m_List.GetItemCount() ; nItem++)
		m_List.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
}

CTime CBatchProcDlg::GetCreatedFileTime(CString sFileName)
{
	CFileStatus FileStatus;
	::GetFileStatus(sFileName, FileStatus);
	return FileStatus.m_ctime;
}

CTime CBatchProcDlg::GetModifiedFileTime(CString sFileName)
{
	CFileStatus FileStatus;
	::GetFileStatus(sFileName, FileStatus);
	return FileStatus.m_mtime;
}

// Return a negative value if the first item should precede the second,
// a positive value if the first item should follow the second,
// or zero if the two items are equivalent
int CBatchProcDlg::Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListElement* pListElement1 = (CListElement*)lParam1;
	CListElement* pListElement2 = (CListElement*)lParam2;
	ASSERT(pListElement1);
	ASSERT(pListElement2);
	int nSortType = (int)lParamSort;
	CTime Time1, Time2;

	switch (nSortType)
	{
		case FILENAME_ASC :
			return ::CompareNatural(&pListElement1->m_sShortFileName, &pListElement2->m_sShortFileName);
			
		case FILENAME_DES :
			return ::CompareNatural(&pListElement2->m_sShortFileName, &pListElement1->m_sShortFileName);
	
		case EXIFDATE_ASC :
		{
			if (pListElement1->m_pDibHdr->GetExifInfo() &&
				CString(pListElement1->m_pDibHdr->GetExifInfo()->DateTime) != _T(""))
				Time1 = CMetadata::GetDateTimeFromExifString(CString(pListElement1->m_pDibHdr->GetExifInfo()->DateTime));
			else
			{
				CTime CreatedFileTime(GetCreatedFileTime(pListElement1->GetFileName()));
				CTime ModifiedFileTime(GetModifiedFileTime(pListElement1->GetFileName()));
				Time1 = (ModifiedFileTime < CreatedFileTime ? ModifiedFileTime : CreatedFileTime);
			}
			if (pListElement2->m_pDibHdr->GetExifInfo() &&
				CString(pListElement2->m_pDibHdr->GetExifInfo()->DateTime) != _T(""))
				Time2 = CMetadata::GetDateTimeFromExifString(CString(pListElement2->m_pDibHdr->GetExifInfo()->DateTime));
			else
			{
				CTime CreatedFileTime(GetCreatedFileTime(pListElement2->GetFileName()));
				CTime ModifiedFileTime(GetModifiedFileTime(pListElement2->GetFileName()));
				Time2 = (ModifiedFileTime < CreatedFileTime ? ModifiedFileTime : CreatedFileTime);
			}
			if (Time1 < Time2)
				return -1;
			else if (Time1 > Time2)
				return 1;
			else
				return 0;
		}
		case EXIFDATE_DES :
		{
			if (pListElement1->m_pDibHdr->GetExifInfo() &&
				CString(pListElement1->m_pDibHdr->GetExifInfo()->DateTime) != _T(""))
				Time1 = CMetadata::GetDateTimeFromExifString(CString(pListElement1->m_pDibHdr->GetExifInfo()->DateTime));
			else
			{
				CTime CreatedFileTime(GetCreatedFileTime(pListElement1->GetFileName()));
				CTime ModifiedFileTime(GetModifiedFileTime(pListElement1->GetFileName()));
				Time1 = (ModifiedFileTime < CreatedFileTime ? ModifiedFileTime : CreatedFileTime);
			}
			if (pListElement2->m_pDibHdr->GetExifInfo() &&
				CString(pListElement2->m_pDibHdr->GetExifInfo()->DateTime) != _T(""))
				Time2 = CMetadata::GetDateTimeFromExifString(CString(pListElement2->m_pDibHdr->GetExifInfo()->DateTime));
			else
			{
				CTime CreatedFileTime(GetCreatedFileTime(pListElement2->GetFileName()));
				CTime ModifiedFileTime(GetModifiedFileTime(pListElement2->GetFileName()));
				Time2 = (ModifiedFileTime < CreatedFileTime ? ModifiedFileTime : CreatedFileTime);
			}
			if (Time1 < Time2)
				return 1;
			else if (Time1 > Time2)
				return -1;
			else
				return 0;
		}
		case CREATEDDATE_ASC :
		{
			Time1 = GetCreatedFileTime(pListElement1->GetFileName());
			Time2 = GetCreatedFileTime(pListElement2->GetFileName());
			if (Time1 < Time2)
				return -1;
			else if (Time1 > Time2)
				return 1;
			else
				return 0;
		}
		case CREATEDDATE_DES :
		{
			Time1 = GetCreatedFileTime(pListElement1->GetFileName());
			Time2 = GetCreatedFileTime(pListElement2->GetFileName());
			if (Time1 < Time2)
				return 1;
			else if (Time1 > Time2)
				return -1;
			else
				return 0;
		}
		case MODIFIEDDATE_ASC :
		{
			Time1 = GetModifiedFileTime(pListElement1->GetFileName());
			Time2 = GetModifiedFileTime(pListElement2->GetFileName());
			if (Time1 < Time2)
				return -1;
			else if (Time1 > Time2)
				return 1;
			else
				return 0;
		}
		case MODIFIEDDATE_DES :
		{
			Time1 = GetModifiedFileTime(pListElement1->GetFileName());
			Time2 = GetModifiedFileTime(pListElement2->GetFileName());
			if (Time1 < Time2)
				return 1;
			else if (Time1 > Time2)
				return -1;
			else
				return 0;
		}
		case FILESIZE_ASC :
		{
			if (pListElement1->m_pDibHdr->GetFileSize() < pListElement2->m_pDibHdr->GetFileSize())
				return -1;
			else if (pListElement1->m_pDibHdr->GetFileSize() > pListElement2->m_pDibHdr->GetFileSize())
				return 1;
			else
				return 0;
		}
		case FILESIZE_DES :
		{
			if (pListElement1->m_pDibHdr->GetFileSize() < pListElement2->m_pDibHdr->GetFileSize())
				return 1;
			else if (pListElement1->m_pDibHdr->GetFileSize() > pListElement2->m_pDibHdr->GetFileSize())
				return -1;
			else
				return 0;
		}
		case IMAGESIZE_ASC :
		{
			if (pListElement1->m_pDibHdr->GetImageSize() < pListElement2->m_pDibHdr->GetImageSize())
				return -1;
			else if (pListElement1->m_pDibHdr->GetImageSize() > pListElement2->m_pDibHdr->GetImageSize())
				return 1;
			else
				return 0;
		}
		case IMAGESIZE_DES :
		{
			if (pListElement1->m_pDibHdr->GetImageSize() < pListElement2->m_pDibHdr->GetImageSize())
				return 1;
			else if (pListElement1->m_pDibHdr->GetImageSize() > pListElement2->m_pDibHdr->GetImageSize())
				return -1;
			else
				return 0;
		}
		case WIDTH_ASC :
		{
			if (pListElement1->m_pDibHdr->GetWidth() < pListElement2->m_pDibHdr->GetWidth())
				return -1;
			else if (pListElement1->m_pDibHdr->GetWidth() > pListElement2->m_pDibHdr->GetWidth())
				return 1;
			else
				return 0;
		}
		case WIDTH_DES :
		{
			if (pListElement1->m_pDibHdr->GetWidth() < pListElement2->m_pDibHdr->GetWidth())
				return 1;
			else if (pListElement1->m_pDibHdr->GetWidth() > pListElement2->m_pDibHdr->GetWidth())
				return -1;
			else
				return 0;
		}
		case HEIGHT_ASC :
		{
			if (pListElement1->m_pDibHdr->GetHeight() < pListElement2->m_pDibHdr->GetHeight())
				return -1;
			else if (pListElement1->m_pDibHdr->GetHeight() > pListElement2->m_pDibHdr->GetHeight())
				return 1;
			else
				return 0;
		}
		case HEIGHT_DES :
		{
			if (pListElement1->m_pDibHdr->GetHeight() < pListElement2->m_pDibHdr->GetHeight())
				return 1;
			else if (pListElement1->m_pDibHdr->GetHeight() > pListElement2->m_pDibHdr->GetHeight())
				return -1;
			else
				return 0;
		}
		case PATH_ASC :
			return ::CompareNatural(&pListElement1->m_sPath, &pListElement2->m_sPath);

		case PATH_DES :
			return ::CompareNatural(&pListElement2->m_sPath, &pListElement1->m_sPath);

		default :
			return 0;
	}
}

void CBatchProcDlg::OnItemclickListInput(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY*)pNMHDR;

	UpdateData(TRUE);

	if (m_List.GetItemCount() <= 0)
		return;

	if (phdn)
	{
		CHeaderCtrl* pHeader = (CHeaderCtrl*)m_List.GetHeaderCtrl();
		if (pHeader)
		{
			// Remove All Images
			HDITEM hdi;
			hdi.mask = HDI_FORMAT;
			for (int i = 0 ; i < pHeader->GetItemCount() ; i++)
			{
				if (pHeader->GetItem(i, &hdi))
				{
					hdi.fmt &= ~HDF_BITMAP_ON_RIGHT;
					hdi.fmt &= ~HDF_IMAGE;
					hdi.fmt &= ~HDF_LEFT;
					hdi.fmt &= ~HDF_CENTER;
					hdi.fmt &= ~HDF_RIGHT;
					hdi.fmt &= ~HDF_SORTUP;
					hdi.fmt &= ~HDF_SORTDOWN;
					hdi.iImage = 0;
					pHeader->SetItem(i, &hdi);
				}
			}

			// Handle Column Click
			switch (phdn->iItem)
			{
				// File Name
				case LIST_FILENAME :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == FILENAME_ASC)
					{
						m_nSortType = FILENAME_DES;
						if (pHeader->GetItem(LIST_FILENAME, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_FILENAME, &hdi);
						}
					}
					else
					{
						m_nSortType = FILENAME_ASC;
						if (pHeader->GetItem(LIST_FILENAME, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_FILENAME, &hdi);
						}

					}
					break;
				}

				// EXIF Date
				case LIST_EXIFDATE :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == EXIFDATE_ASC)
					{
						m_nSortType = EXIFDATE_DES;
						if (pHeader->GetItem(LIST_EXIFDATE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_EXIFDATE, &hdi);
						}
					}
					else
					{
						m_nSortType = EXIFDATE_ASC;
						if (pHeader->GetItem(LIST_EXIFDATE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_EXIFDATE, &hdi);
						}

					}
					break;
				}

				// Created Date
				case LIST_CREATEDDATE :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == CREATEDDATE_ASC)
					{
						m_nSortType = CREATEDDATE_DES;
						if (pHeader->GetItem(LIST_CREATEDDATE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_CREATEDDATE, &hdi);
						}
					}
					else
					{
						m_nSortType = CREATEDDATE_ASC;
						if (pHeader->GetItem(LIST_CREATEDDATE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_CREATEDDATE, &hdi);
						}

					}
					break;
				}

				// Modified Date
				case LIST_MODIFIEDDATE :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == MODIFIEDDATE_ASC)
					{
						m_nSortType = MODIFIEDDATE_DES;
						if (pHeader->GetItem(LIST_MODIFIEDDATE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_MODIFIEDDATE, &hdi);
						}
					}
					else
					{
						m_nSortType = MODIFIEDDATE_ASC;
						if (pHeader->GetItem(LIST_MODIFIEDDATE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_MODIFIEDDATE, &hdi);
						}

					}
					break;
				}

				// File Size
				case LIST_FILESIZE :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == FILESIZE_ASC)
					{
						m_nSortType = FILESIZE_DES;
						if (pHeader->GetItem(LIST_FILESIZE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_FILESIZE, &hdi);
						}
					}
					else
					{
						m_nSortType = FILESIZE_ASC;
						if (pHeader->GetItem(LIST_FILESIZE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_FILESIZE, &hdi);
						}

					}
					break;
				}

				// Image Size
				case LIST_IMAGESIZE :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == IMAGESIZE_ASC)
					{
						m_nSortType = IMAGESIZE_DES;
						if (pHeader->GetItem(LIST_IMAGESIZE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_IMAGESIZE, &hdi);
						}
					}
					else
					{
						m_nSortType = IMAGESIZE_ASC;
						if (pHeader->GetItem(LIST_IMAGESIZE, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_IMAGESIZE, &hdi);
						}

					}
					break;
				}

				// Width
				case LIST_WIDTH :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == WIDTH_ASC)
					{
						m_nSortType = WIDTH_DES;
						if (pHeader->GetItem(LIST_WIDTH, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_WIDTH, &hdi);
						}
					}
					else
					{
						m_nSortType = WIDTH_ASC;
						if (pHeader->GetItem(LIST_WIDTH, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_WIDTH, &hdi);
						}

					}
					break;
				}

				// Height
				case LIST_HEIGHT :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == HEIGHT_ASC)
					{
						m_nSortType = HEIGHT_DES;
						if (pHeader->GetItem(LIST_HEIGHT, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_HEIGHT, &hdi);
						}
					}
					else
					{
						m_nSortType = HEIGHT_ASC;
						if (pHeader->GetItem(LIST_HEIGHT, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_HEIGHT, &hdi);
						}

					}
					break;
				}

				// Path
				case LIST_PATH :
				{
					hdi.mask = HDI_IMAGE | HDI_FORMAT;
					if (m_nSortType == PATH_ASC)
					{
						m_nSortType = PATH_DES;
						if (pHeader->GetItem(LIST_PATH, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTDOWN;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 1;
							pHeader->SetItem(LIST_PATH, &hdi);
						}
					}
					else
					{
						m_nSortType = PATH_ASC;
						if (pHeader->GetItem(LIST_PATH, &hdi))
						{
							if (m_nComCtl32MajorVersion >= 6)
								hdi.fmt |= HDF_LEFT | HDF_SORTUP;
							else
								hdi.fmt |= HDF_LEFT | HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
							hdi.iImage = 0;
							pHeader->SetItem(LIST_PATH, &hdi);
						}

					}
					break;
				}
			
				default :
					break;
			}

			// Sort
			m_List.SortItems(Compare, m_nSortType);

			// Update Dibs
			UpdateDibs();
		}
	}
	
	*pResult = 0;
}

void CBatchProcDlg::OnDropFiles(HDROP hDropInfo) 
{
	// Get the number of pathnames 
	// (files or folders) dropped
	UINT uiNumFilesDropped = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	// Insert Position
	int nItem;
	POSITION pos = m_List.GetFirstSelectedItemPosition();
	if (pos == NULL)
		nItem = 0;
	else
		nItem = m_List.GetNextSelectedItem(pos) + 1;

	// Disable All Controls, Including Process Button!
	EnableAllControls(FALSE, TRUE);

	// Iterate through them
	CString sPath;
	m_bEnableUpdateDibs = FALSE;
	for (UINT uiFile = 0 ; uiFile < uiNumFilesDropped ; uiFile++)
	{
		// Get the pathname
		UINT uiSize = ::DragQueryFile(hDropInfo, uiFile, NULL, 0) + 1;
		LPTSTR p = sPath.GetBuffer(uiSize);
		::DragQueryFile(hDropInfo, uiFile, p, uiSize);
		sPath.ReleaseBuffer();

		// If is File -> Append to List
		if (::IsExistingFile(sPath))
		{
			if (::GetFileExt(sPath) == _T(".txt"))
			{
				m_bEnableUpdateDibs = TRUE;
				ListLoad(sPath);
				::DragFinish(hDropInfo);
				return;
			}
			else
			{
				if (ListAdd(nItem, sPath))
					nItem++;
			}
		}
	}
	
	// Windows allocates the memory for 
	// file information, so we must clean it up
	::DragFinish(hDropInfo);

	// Start Loading Dibs
	StartLoadDibs();
}

void CBatchProcDlg::ClearDibs(BOOL bPaint/*=TRUE*/)
{
	for (int i = 0 ; i < NUM_DIBS ; i++)
	{
		m_Dibs[i].SetDibHdrPointer(NULL);
		m_Dibs[i].SetDibFullPointer(NULL);
		m_Dibs[i].SetAlphaRenderedDibPointer(NULL);
		m_Dibs[i].SetDibHdrCS(NULL);
		m_Dibs[i].SetDibFullCS(NULL);
		m_Dibs[i].SetBackgroundColor(RGB(0xFF,0xFF,0xFF));
		m_Dibs[i].SetImageBackgroundColor(RGB(0xFF,0xFF,0xFF));
		m_Dibs[i].SetUseImageBackgroundColor(FALSE);
		m_Dibs[i].SetBordersColor(RGB(0xFF,0xFF,0xFF));
		if (bPaint)
			m_Dibs[i].PaintDib();
	}
}

LONG CBatchProcDlg::OnUpdateDibsPostpone(WPARAM wparam, LPARAM lparam)
{
	if (m_List.GetItemCount() <= 0)
		ClearDibs();
	else
	{
		int nItem, nHighlightItem;
		POSITION pos = m_List.GetFirstSelectedItemPosition();
		if (pos == NULL)
		{
			nHighlightItem = -1;
			nItem = 0;
		}
		else if (m_List.IsDragging())
			nItem = nHighlightItem = m_List.GetDropIndex();
		else
			nItem = nHighlightItem = m_List.GetNextSelectedItem(pos);
		int nOffset = 0;
		if (nItem == 1)
			nOffset = 1;
		else if (nItem >= 2)
			nOffset = 2;
		if ((nItem + 3) >= m_List.GetItemCount())
			nOffset += ((nItem + 3) - m_List.GetItemCount() + 1);
		nItem -= nOffset;
		if (nItem < 0)
			nItem = 0;
		for (int i = 0 ; i < NUM_DIBS ; i++)
		{
			CListElement* pListElement = NULL;
			if (nItem < m_List.GetItemCount() &&
				(pListElement = (CListElement*)m_List.GetItemData(nItem)))
			{
				m_Dibs[i].SetDibHdrPointer(pListElement->m_pDibHdr);
				m_Dibs[i].SetDibFullPointer(pListElement->m_pDibFull);
				m_Dibs[i].SetAlphaRenderedDibPointer(pListElement->m_pAlphaRenderedDib);
				m_Dibs[i].SetDibHdrCS(pListElement->m_pcsDibHdr);
				m_Dibs[i].SetDibFullCS(pListElement->m_pcsDibFull);
				m_Dibs[i].SetBackgroundColor(pListElement->m_DibStatic.GetBackgroundColor());
				m_Dibs[i].SetImageBackgroundColor(pListElement->m_DibStatic.GetImageBackgroundColor());
				m_Dibs[i].SetUseImageBackgroundColor(pListElement->m_DibStatic.GetUseImageBackgroundColor());
			}
			else
			{
				m_Dibs[i].SetDibHdrPointer(NULL);
				m_Dibs[i].SetDibFullPointer(NULL);
				m_Dibs[i].SetAlphaRenderedDibPointer(NULL);
				m_Dibs[i].SetDibHdrCS(NULL);
				m_Dibs[i].SetDibFullCS(NULL);
				m_Dibs[i].SetBackgroundColor(RGB(0xFF,0xFF,0xFF));
				m_Dibs[i].SetImageBackgroundColor(RGB(0xFF,0xFF,0xFF));
				m_Dibs[i].SetUseImageBackgroundColor(FALSE);
			}
			if (nItem == nHighlightItem)
				m_Dibs[i].SetBordersColor(RGB(0,0,0xFF));
			else
				m_Dibs[i].SetBordersColor(RGB(0xFF,0xFF,0xFF));
			m_Dibs[i].PaintDib();
			nItem++;
		}
	}

	return 0;
}

__forceinline void CBatchProcDlg::UpdateDibs()
{
	if (!m_bEnableUpdateDibs)
		return;
	else
	{
		// When selection changes there is a moment where
		// no selection at all is available. Not handling
		// this moment avoids flickering between the
		// X-drawing and real Image-drawing.
		::PostMessage(	GetSafeHwnd(),
						WM_UPDATEDIBS_POSTPONE,
						0, 0);
	}
}

LONG CBatchProcDlg::OnLoadDone(WPARAM wparam, LPARAM lparam)
{
	if ((int)wparam == CDibStatic::HDRLOAD_DOEXIT)	
		return 0;

	if ((int)wparam == CDibStatic::FULLLOAD_HDRDONE)
	{
		// Paint
		for (int i = 0 ; i < NUM_DIBS ; i++)
			m_Dibs[i].PaintDib();
	}

	if (((int)wparam == CDibStatic::FULLLOAD_DOEXIT)	||
		((int)wparam == CDibStatic::FULLLOAD_HDRERROR)	||
		((int)wparam == CDibStatic::FULLLOAD_FULLERROR)	||
		((int)wparam == CDibStatic::FULLLOAD_FULLDONE))
	{
		// Paint
		for (int i = 0 ; i < NUM_DIBS ; i++)
			m_Dibs[i].PaintDib();

		// Start Next Full Dib Load
		StartNextLoadDibsFull(1);
	}
	
	if (((int)wparam == CDibStatic::HDRLOAD_ERROR)	||
		((int)wparam == CDibStatic::HDRLOAD_HDRDONE))
	{
		CDibStatic* pDibStatic = (CDibStatic*)lparam;

		// Update columns with header infos
		int nItem;
		if ((nItem = DibStaticToItem(pDibStatic)) >= 0)
		{
			SetExifDateColumn(nItem);
			SetImageSizeColumn(nItem);
			SetWidthColumn(nItem);
			SetHeightColumn(nItem);
			
			// Set File Size for Sorting
			CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
			if (pListElement && (pListElement->m_pDibHdr->GetFileSize() == 0))
			{
				ULARGE_INTEGER FileSize = ::GetFileSize64(pListElement->GetFileName());
				pListElement->m_pDibHdr->SetFileSize((DWORD)FileSize.QuadPart);
			}
		}

		// Start Next Header Load
		StartNextLoadDibsHdr(1);

		// Inc. Done Count
		m_uiDibsHdrLoadDoneCount++;

		// Was Last Load Done?
		if (m_uiDibsHdrLoadStartCount == m_uiDibsHdrLoadDoneCount)
		{	
			// Reset Progress
			m_Progress.SetPos(-1);

			// Start Thumbnail Dib Load
			StartNextLoadDibsFull(((CUImagerApp*)::AfxGetApp())->m_nCoresCount + 1);

			// Re-Enable Controls
			EnableAllControls(TRUE, TRUE);
			UpdateControls();

			// Update Dibs
			m_bEnableUpdateDibs = TRUE;
			OnUpdateDibsPostpone(0, 0);
		}
		else
		{
			// Set Progress
			m_Progress.SetPos(Round(m_uiDibsHdrLoadDoneCount * 100.0 /
									m_uiDibsHdrLoadTotCount));
		}
	}

	return 0;
}

int CBatchProcDlg::DibStaticToItem(CDibStatic* pDibStatic)
{
	if (pDibStatic)
	{
		for (int nItem = 0 ; nItem < m_List.GetItemCount() ; nItem++)
		{
			CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
			if (pListElement)
			{
				if (&pListElement->m_DibStatic == pDibStatic)
					return nItem;
			}
		}
	}

	return -1;
}

int CBatchProcDlg::StartNextLoadDibsHdr(int nNumOfLoads/*=1*/)
{
	// This Flag is set when exiting
	if (!m_bEnableNextLoadDibs)
		return 0;

	int nNumOfLoadsCount = 0;
	for (int nItem = 0 ; nItem < m_List.GetItemCount() ; nItem++)
	{
		CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
		if (pListElement &&
			!pListElement->m_DibStatic.HasLoadHdrStarted())
		{
			m_uiDibsHdrLoadStartCount++;

			if (!pListElement->m_DibStatic.Load(pListElement->GetFileName(),
												TRUE, FALSE, FALSE,
												m_nDibStaticWidth,
												m_nDibStaticHeight))
			{
				// Should Never Enter Here!
				// -> Fake the Failed Thread Run:
				pListElement->m_DibStatic.SetLoadHdrStarted(TRUE);
				pListElement->m_DibStatic.SetLoadHdrTerminated(TRUE);
				OnLoadDone((WPARAM)(CDibStatic::HDRLOAD_ERROR),
									(LPARAM)&pListElement->m_DibStatic);
			}

			if (++nNumOfLoadsCount >= nNumOfLoads)
				break;
		}
	}

	return nNumOfLoadsCount;
}

int CBatchProcDlg::StartNextLoadDibsFull(int nNumOfLoads/*=1*/)
{
	// This Flag is set when exiting
	if (!m_bEnableNextLoadDibs)
		return 0;

	int nNumOfLoadsCount = 0;
	for (int nItem = 0 ; nItem < m_List.GetItemCount() ; nItem++)
	{
		CListElement* pListElement = (CListElement*)m_List.GetItemData(nItem);
		if (pListElement &&
			pListElement->m_DibStatic.HasLoadHdrTerminated() &&
			!pListElement->m_DibStatic.HasLoadFullStarted())
		{
			// No Need to start a full load because thumbnail
			// comes from EXIF header load?
			if (pListElement->m_pDibHdr->GetThumbnailDib() &&
				pListElement->m_pDibHdr->GetThumbnailDib()->IsValid())
			{
				// Fake a Thread Run:
				pListElement->m_DibStatic.SetLoadFullStarted(TRUE);
				pListElement->m_DibStatic.SetLoadFullTerminated(TRUE);
				*pListElement->m_pDibFull = *pListElement->m_pDibHdr;
				OnLoadDone((WPARAM)(CDibStatic::FULLLOAD_HDRDONE),
									(LPARAM)&pListElement->m_DibStatic);
				OnLoadDone((WPARAM)(CDibStatic::FULLLOAD_FULLDONE),
									(LPARAM)&pListElement->m_DibStatic);
			}
			else
			{
				if (!pListElement->m_DibStatic.Load(pListElement->GetFileName(),
													FALSE, FALSE, FALSE,
													m_nDibStaticWidth,
													m_nDibStaticHeight))
				{
					// Should Never Enter Here!
					// -> Fake the Failed Thread Run:
					pListElement->m_DibStatic.SetLoadFullStarted(TRUE);
					pListElement->m_DibStatic.SetLoadFullTerminated(TRUE);	
					OnLoadDone((WPARAM)(CDibStatic::FULLLOAD_HDRERROR),
										(LPARAM)&pListElement->m_DibStatic);
				}
			}

			if (++nNumOfLoadsCount >= nNumOfLoads)
				break;
		}
	}

	return nNumOfLoadsCount;
}

void CBatchProcDlg::OnItemchangedListInput(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView)
	{
		// Update Dibs
		UpdateDibs();
	}

	*pResult = 0;
}

void CBatchProcDlg::OnInsertitemListInput(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if (pNMListView)
	{
		// Update Dibs
		UpdateDibs();
	}
	
	*pResult = 0;
}

void CBatchProcDlg::OnDeleteitemListInput(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if (pNMListView)
	{
		// Update Dibs
		UpdateDibs();
	}
	
	*pResult = 0;
}

void CBatchProcDlg::OnDeleteallitemsListInput(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if (pNMListView)
	{
		// Update Dibs
		UpdateDibs();
	}

	*pResult = 0;
}

// Starting with version 4.71, the Shell and common
// controls DLLs, among others, began exporting DllGetVersion
int CBatchProcDlg::GetComCtl32MajorVersion()
{
	int ret = 4;

	typedef HRESULT (CALLBACK *DLLGETVERSION)(DLLVERSIONINFO*);
	DLLGETVERSION pDLLGETVERSION = NULL;
	HMODULE hModComCtl = ::LoadLibrary(_T("comctl32.dll"));
    if (hModComCtl)
    {
        pDLLGETVERSION = (DLLGETVERSION)(
			::GetProcAddress(hModComCtl, "DllGetVersion"));
        if (pDLLGETVERSION)
        {
            DLLVERSIONINFO dvi = {0};
            dvi.cbSize = sizeof dvi;
            if (pDLLGETVERSION(&dvi) == NOERROR)
            {
                ret = dvi.dwMajorVersion;
            }
        }
		::FreeLibrary(hModComCtl);                 
    }

	return ret;
}

int CBatchProcDlg::CChangeNotificationThread::Work() 
{
	ASSERT(m_pDlg);

	DWORD Event;

	m_hFindChangeNotification = ::FindFirstChangeNotification(	m_pDlg->m_sDst,
																TRUE,
																FILE_NOTIFY_CHANGE_SIZE			|
																FILE_NOTIFY_CHANGE_FILE_NAME	|
																FILE_NOTIFY_CHANGE_DIR_NAME);
	m_hEventArray[1] = m_hFindChangeNotification;
	
	// First Time Calc. & Display Size
	m_pDlg->UpdateDstFolderSizes(this);

	while (TRUE)
    {   
		Event = ::WaitForMultipleObjects(3, m_hEventArray, FALSE, FSCHANGES_TIMEOUT);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		::FindCloseChangeNotification(m_hFindChangeNotification);
										m_hFindChangeNotification = INVALID_HANDLE_VALUE;
										return 0;

			// Find Change Notification Event
			case WAIT_OBJECT_0 + 1 :	// Get rid of all similar events that occur shortly after this
										do
										{
											::FindNextChangeNotification(m_hFindChangeNotification);
											Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, SUCCESSIVE_FSCHANGES_TIMEOUT);
											switch (Event)
											{
												// Shutdown Event
												case WAIT_OBJECT_0 :		::FindCloseChangeNotification(m_hFindChangeNotification);
																			m_hFindChangeNotification = INVALID_HANDLE_VALUE;
																			return 0;

												// Find Change Notification Event
												case WAIT_OBJECT_0 + 1 :	break;

												// Timeout
												case WAIT_TIMEOUT :			break;

												// Error
												default:					::FindCloseChangeNotification(m_hFindChangeNotification);
																			m_hFindChangeNotification = INVALID_HANDLE_VALUE;
																			return 0;
											}
										}
										while (Event == (WAIT_OBJECT_0 + 1));

										m_pDlg->UpdateDstFolderSizes(this);

										break;

			// Do Update Event
			case WAIT_OBJECT_0 + 2 :	::ResetEvent(m_hDoUpdateEvent);
										m_pDlg->UpdateDstFolderSizes(this);
										break;

			// Timeout
			case WAIT_TIMEOUT :			m_pDlg->UpdateDstFolderSizes(this);
										break;

			// Error
			default:					::FindCloseChangeNotification(m_hFindChangeNotification);
										m_hFindChangeNotification = INVALID_HANDLE_VALUE;
										return 0;
		}

		// Check whether Folder has been deleted
		if (!::IsExistingDir(m_pDlg->m_sDst))
			break;
    }

	// Close
	::FindCloseChangeNotification(m_hFindChangeNotification);
	m_hFindChangeNotification = INVALID_HANDLE_VALUE;

	return 0;
}

LONG CBatchProcDlg::OnSetDstSize(WPARAM wparam, LPARAM lparam)
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DST_SIZE);
	if (pEdit)
	{
		if (m_nOutputSelection == OUTPUT_DIR)
			pEdit->SetWindowText(m_sDstDirSize);
		else
			pEdit->SetWindowText(m_sDstFileSize);
	}
	return 1;
}

void CBatchProcDlg::UpdateDstFolderSizes(CWorkerThread* pThread/*=NULL*/)
{
	if (m_nOutputSelection == OUTPUT_DIR)
	{
		ULARGE_INTEGER OutDirContentSize;
		int nOutDirFilesCount = 0;
		if (::IsExistingDir(m_sDst))
		{
			OutDirContentSize = ::GetDirContentSize(m_sDst,
													&nOutDirFilesCount,
													pThread);
		}
		else	
			OutDirContentSize.QuadPart = 0;
		::EnterCriticalSection(&m_csOutDir);
		m_OutDirContentSize.LowPart = OutDirContentSize.LowPart;
		m_OutDirContentSize.HighPart = OutDirContentSize.HighPart;
		m_nOutDirFilesCount = nOutDirFilesCount;
		::LeaveCriticalSection(&m_csOutDir);
		CString sContentSize, sFilesCount;
		if (OutDirContentSize.QuadPart >= (1024*1024*1024))
			sContentSize.Format(_T("%I64d ") + ML_STRING(1826, "GB"), OutDirContentSize.QuadPart >> 30);
		else if (OutDirContentSize.QuadPart >= (1024*1024))
			sContentSize.Format(_T("%I64d ") + ML_STRING(1825, "MB"), OutDirContentSize.QuadPart >> 20);
		else if (OutDirContentSize.QuadPart >= 1024)
			sContentSize.Format(_T("%I64d ") + ML_STRING(1243, "KB"), OutDirContentSize.QuadPart >> 10);
		else
			sContentSize.Format(_T("%I64d ") + ML_STRING(1244, "Bytes"), OutDirContentSize.QuadPart);
		if (nOutDirFilesCount == 1)
			sFilesCount.Format(_T("%u ") + ML_STRING(1574, "File"), nOutDirFilesCount);
		else
			sFilesCount.Format(_T("%u ") + ML_STRING(1575, "Files"), nOutDirFilesCount);
		m_sDstDirSize = sContentSize + _T('\n') + sFilesCount;
		PostMessage(WM_SET_DST_SIZE, 0, 0);
	}
}

void CBatchProcDlg::UpdateDstFileSize()
{
	if (m_nOutputSelection == OUTPUT_FILE)
	{
		ULARGE_INTEGER FileSize;
		int nFilesCount = 1;
		if (::IsExistingFile(m_sOutputFileName))
		{
			FileSize = ::GetFileSize64(m_sOutputFileName);
			if (::GetFileExt(m_sOutputFileName) == _T(".zip"))
			{
				BOOL bZipNotOpen;	
				if (bZipNotOpen = ((CUImagerApp*)::AfxGetApp())->m_Zip.IsClosed())
				{
					((CUImagerApp*)::AfxGetApp())->m_Zip.Open(m_sOutputFileName,
										CZipArchive::openReadOnly, 0);
				}
				nFilesCount = ((CUImagerApp*)::AfxGetApp())->m_Zip.GetNoEntries();
				if (bZipNotOpen)
					((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
			}
		}
		else
		{
			FileSize.QuadPart = 0;
			nFilesCount = 0;
		}
		CString sFileSize, sFilesCount;
		if (FileSize.QuadPart >= (1024*1024*1024))
			sFileSize.Format(_T("%I64d ") + ML_STRING(1826, "GB"), FileSize.QuadPart >> 30);
		else if (FileSize.QuadPart >= (1024*1024))
			sFileSize.Format(_T("%I64d ") + ML_STRING(1825, "MB"), FileSize.QuadPart >> 20);
		else if (FileSize.QuadPart >= 1024)
			sFileSize.Format(_T("%I64d ") + ML_STRING(1243, "KB"), FileSize.QuadPart >> 10);
		else
			sFileSize.Format(_T("%I64d ") + ML_STRING(1244, "Bytes"), FileSize.QuadPart);
		if (nFilesCount == 1)
			sFilesCount.Format(_T("%u ") + ML_STRING(1574, "File"), nFilesCount);
		else
			sFilesCount.Format(_T("%u ") + ML_STRING(1575, "Files"), nFilesCount);
		m_sDstFileSize = sFileSize + _T('\n') + sFilesCount;
		PostMessage(WM_SET_DST_SIZE, 0, 0);
	}
}

void CBatchProcDlg::OnCancel() 
{
	OnClose();
}

void CBatchProcDlg::OnClose() 
{
	if (UpdateData(TRUE))
	{
		BeginWaitCursor();
		m_ChangeNotificationThread.Kill();
		m_bEnableNextLoadDibs = FALSE;
		if (!m_bThreadExited)
		{
			m_ProcessThread.Kill_NoBlocking();
			EnableAllControls(FALSE, TRUE);
			m_bDoCloseDlg = TRUE;
		}
		else
		{
			SaveSettings();
			ListDeleteAll();
			EndWaitCursor();
			DestroyWindow();
		}
	}
}

void CBatchProcDlg::PostNcDestroy() 
{
	::AfxGetMainFrame()->m_pBatchProcDlg = NULL;
	delete this;
	CDialog::PostNcDestroy();
}
