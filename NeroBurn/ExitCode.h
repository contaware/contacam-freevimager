/******************************************************************************
|* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
|* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
|* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
|* PARTICULAR PURPOSE.
|* 
|* Copyright 1995-2005 Nero AG. All Rights Reserved.
|*-----------------------------------------------------------------------------
|* NeroSDK / NeroCmd
|*
|* PROGRAM: ExitCodes.h
|*
|* PURPOSE: Supported exit codes and translation to textual representation
******************************************************************************/


#ifndef _EXIT_CODE_H_
#define _EXIT_CODE_H_


// These are the supported error codes.

enum EXITCODE {
	EXITCODE_OK = 0,
	EXITCODE_BAD_USAGE,
	EXITCODE_INTERNAL_ERROR,
	EXITCODE_NEROAPI_DLL_NOT_FOUND,
	EXITCODE_NO_SERIAL_NUMBER,
	EXITCODE_BAD_SERIAL_NUMBER,
	EXITCODE_NO_CD_INSERTED,
	EXITCODE_NO_SUPPORT_FOR_CDRW,
	EXITCODE_ERROR_ERASING_CDRW,
	EXITCODE_ERROR_OBTAINING_AVAILABLE_DRIVES,
	EXITCODE_MISSING_DRIVENAME,
	EXITCODE_ERROR_OPENING_DRIVE,
	EXITCODE_DRIVE_NOT_FOUND,
	EXITCODE_UNKNOWN_CD_FORMAT,
	EXITCODE_INVALID_DRIVE,
	EXITCODE_BURN_FAILED,
	EXITCODE_FUNCTION_NOT_ALLOWED,
	EXITCODE_DRIVE_NOT_ALLOWED,
	EXITCODE_ERROR_GETTING_CD_INFO,
	EXITCODE_TRACK_NOT_FOUND,
	EXITCODE_UNKNOWN_FILE_TYPE,
	EXITCODE_DAE_FAILED,
	EXITCODE_ERROR_OPENING_FILE,
	EXITCODE_OUT_OF_MEMORY,
	EXITCODE_ERROR_DETERMINING_LENGTH_OF_FILE,
	EXITCODE_EJECT_FAILED,
	EXITCODE_BAD_IMPORT_SESSION_NUMBER,
	EXITCODE_FAILED_TO_CREATE_ISO_TRACK,
	EXITCODE_FILE_NOT_FOUND,
	EXITCODE_UNKNOWN,
	EXITCODE_USER_ABORTED,
	EXITCODE_DEMOVERSION_EXPIRED,
	EXITCODE_BAD_MESSAGE_FILE,
	EXITCODE_ERROR_DOING_ESTIMATION,
	EXITCODE_ERROR_GETTING_VMS_INFO,
	EXITCODE_ROBO_DRIVER_NOTFOUND,
	EXITCODE_ROBO_NOTSUPPORTED,
};


// Translate the numeric error code into a textual representation.

class CExitCode
{
protected:
	EXITCODE m_ExitCode;
	LPSTR m_psNeroLastError;

public:
	CExitCode (EXITCODE code);
	~CExitCode ();
	LPCSTR GetTextualExitCode (void) const;
	LPCSTR GetLastError (void) const {return (m_psNeroLastError)? m_psNeroLastError: "";}

	operator EXITCODE () const {return m_ExitCode;}
	CExitCode & operator= (const CExitCode & code);
	CExitCode & operator= (const EXITCODE code);
	
protected:
	void GetLastErrorLogLine (void);

};

#endif
