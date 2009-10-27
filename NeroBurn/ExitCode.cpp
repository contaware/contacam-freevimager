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
|* PROGRAM: ExitCode.cpp
|*
|* PURPOSE: Translation of numeric error code
******************************************************************************/


#include "stdafx.h"
#include "ExitCode.h"

// The constructor
//
CExitCode::CExitCode (EXITCODE code)
{
	// Save the error code...
	// 
	m_ExitCode = code;
	m_psNeroLastError = NULL;

	// ... and obtain its text description.
	// 
	GetLastErrorLogLine ();
}

CExitCode::~CExitCode ()
{
	// Free the error string. free() will handle NULL just fine.
	// 
	free (m_psNeroLastError);
}

void CExitCode::GetLastErrorLogLine (void)
{
	if (m_ExitCode != EXITCODE_OK)
	{
		LPSTR psError = NeroGetLastError ();

		// Only proceed if there was an error
		// and no previous error has occured.
		
		if (NULL != psError)
		{
			// First free the previous error string, if it exists.
			//
			free (m_psNeroLastError);

			// Then duplicate the Nero error string by using C-runtime
			// function "strdup".
			//
			m_psNeroLastError = strdup (psError);

			// And finally release the Nero error string with a NeroAPI
			// function.
			//
			NeroFreeMem (psError);
		}
	}
	else
	{
		free (m_psNeroLastError);
		m_psNeroLastError = NULL;
	}
}

// The assignment operator if the source is another CExitCode.
//
CExitCode & CExitCode::operator= (const CExitCode & code)
{
	// Just copy the code and make a copy of the error string.
	// 
	m_ExitCode = code.m_ExitCode;
	m_psNeroLastError = strdup (code.m_psNeroLastError);

	return *this;
}

// The assignment operator if the source is a plain EXITCODE constant.
// 
CExitCode & CExitCode::operator= (const EXITCODE code)
{
	// Save the code itself...
	// 
	m_ExitCode = code;

	// ... and find out a textual error.
	// 
	GetLastErrorLogLine ();

	return *this;
}

// Translate the numeric error code into a textual representation.

LPCSTR CExitCode::GetTextualExitCode (void) const
{
	static struct {
		EXITCODE code;
		LPCSTR message;
	} errors[] = {
		EXITCODE_UNKNOWN,							"Unknown error.",
		EXITCODE_OK,								"Ok.",
		EXITCODE_BAD_USAGE,							"Bad usage!",
		EXITCODE_INTERNAL_ERROR,					"Internal error!",
		EXITCODE_NEROAPI_DLL_NOT_FOUND,				"NEROAPI.DLL was not found!",
		EXITCODE_NO_SERIAL_NUMBER,					"Serial number was not found!",
		EXITCODE_BAD_SERIAL_NUMBER,					"Serial number is invalid!",
		EXITCODE_NO_CD_INSERTED,					"No CD was inserted!",
		EXITCODE_NO_SUPPORT_FOR_CDRW,				"CDRW is not supported!",
		EXITCODE_ERROR_ERASING_CDRW,				"There was an error erasing CDRW!",
		EXITCODE_ERROR_OBTAINING_AVAILABLE_DRIVES,	"There was an error obtaining a list of available drives!",
		EXITCODE_MISSING_DRIVENAME,					"Drive name is missing!",
		EXITCODE_ERROR_OPENING_DRIVE,				"There was an error while trying to access the drive!",
		EXITCODE_DRIVE_NOT_FOUND,					"Drive was not found!",
		EXITCODE_UNKNOWN_CD_FORMAT,					"Unknown CD format!",
		EXITCODE_INVALID_DRIVE,						"Invalid drive!",
		EXITCODE_BURN_FAILED,						"Burn process failed!",
		EXITCODE_FUNCTION_NOT_ALLOWED,				"Function was not allowed!",
		EXITCODE_DRIVE_NOT_ALLOWED,					"Drive was not allowed!",
		EXITCODE_ERROR_GETTING_CD_INFO,				"There was an error obtaining CD info!",
		EXITCODE_TRACK_NOT_FOUND,					"Track was not found!",
		EXITCODE_UNKNOWN_FILE_TYPE,					"File type is unknown!",
		EXITCODE_DAE_FAILED,						"Digital audio extraction failed!",
		EXITCODE_ERROR_OPENING_FILE,				"There was an error opening the file!",
		EXITCODE_OUT_OF_MEMORY,						"Out of memory!",
		EXITCODE_ERROR_DETERMINING_LENGTH_OF_FILE,	"Error determining the length of input file!",
		EXITCODE_EJECT_FAILED,						"Eject/Load failed!",
		EXITCODE_BAD_IMPORT_SESSION_NUMBER,			"Invalid session number to import!",
		EXITCODE_FAILED_TO_CREATE_ISO_TRACK,		"Failed to create ISO track!",
		EXITCODE_FILE_NOT_FOUND,					"Specified file was not found!",
		EXITCODE_USER_ABORTED,						"User aborted!",
		EXITCODE_DEMOVERSION_EXPIRED,				"Demo version has expired!",
		EXITCODE_BAD_MESSAGE_FILE,					"Bad message file!",
		EXITCODE_ERROR_DOING_ESTIMATION,			"Error doing track size estimation!",
		EXITCODE_ERROR_GETTING_VMS_INFO,			"There was an error obtaining virtual multisession info!",
		EXITCODE_ROBO_DRIVER_NOTFOUND,				"Could not find specified robot driver!",
		EXITCODE_ROBO_NOTSUPPORTED,					"Specified robot option is not supported!",
	};

	for (int i = 0; i < sizeof (errors)/sizeof (errors[0]); i ++)
	{
		if (m_ExitCode == errors[i].code)
		{
			return errors[i].message;
		}
	}

	return errors[0].message;
}
