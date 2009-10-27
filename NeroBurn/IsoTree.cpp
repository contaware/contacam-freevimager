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
|* PROGRAM: IsoTrack.cpp
|*
|* PURPOSE: ISO tree handling
******************************************************************************/


#include "stdafx.h"
#include "IsoTree.h"
#include "FindFile.h"
#include "ExitCode.h"

// This function is used solely for debug purposes in order to print the
// whole ISO tree.
// 
void CIsoTree::DebugPrintIsoTrack(const NERO_ISO_ITEM * pItem, int iLevel)
{
	while (pItem)
	{
		// Indent each level a little bit.
		// 
		for (int i = 0; i <= iLevel; i ++)
		{
			printf ("  ");
		}

		printf((pItem->isDirectory)? "[%s]\n": "%s\n", (pItem->longFileName != NULL)? pItem->longFileName: pItem->fileName);

		if (pItem->isDirectory)
		{
			DebugPrintIsoTrack (pItem->subDirFirstItem, iLevel + 1);
		}

		pItem = pItem->nextItem;
	}
}

// A helper to get the correct filename in a NERO_ISO_ITEM.
// 
inline LPCSTR GetFilename(const NERO_ISO_ITEM * pItem)
{
	return (pItem->longFileName != NULL)? pItem->longFileName: pItem->fileName;
}

// A helper to get the correct filename source path in a NERO_ISO_ITEM.
// 
inline LPCSTR GetFilenameSourcePath(const NERO_ISO_ITEM * pItem)
{
	return (pItem->longSourceFilePath != NULL)? pItem->longSourceFilePath: pItem->sourceFilePath;
}

// This function deletes the iso tree recursively.

void CIsoTree::DeleteIsoItemTree(NERO_ISO_ITEM * pItem)
{
	// First free our own long filename strings, then free the whole tree.
	// 
	FreeOurOwnResources (pItem);
	NeroFreeIsoItemTree (pItem);
}

void CIsoTree::FreeOurOwnResources(NERO_ISO_ITEM * pItem)
{
	// Step through the tree until the 
	// ISO item tree pointer becomes NULL

	while (NULL != pItem)
	{
		NERO_ISO_ITEM* pNextItem = pItem->nextItem;

		if (pItem->isDirectory)
		{
			// We have encountered another ISO item tree;
			// recurse another level.

			FreeOurOwnResources (pItem->subDirFirstItem);
		}

		// It the item is not a reference, free its associated long
		// filenames, if any.
		// 
		if (!pItem->isReference)
		{
			delete pItem->longFileName;
			delete (char *) pItem->longSourceFilePath;
		}

		pItem = pNextItem;
	}
}


// This function searches for a specified path and recursively adds
// all files and directories that are found.

CExitCode CIsoTree::CreateIsoTree(bool bRecursive, LPCSTR psFilename, NERO_ISO_ITEM ** ppItem, int iLevel)
{
	// CFindFiles is a helper class for file and subdirectory handling 

	CFindFiles ff (psFilename);

	*ppItem = NULL;

	if (!ff.IsValidEntry())
	{
		if (0 == iLevel)
		{
			// If we haven't found any entries and we are on the
			// first level of recursion then this should be
			// reported as an error.

			return EXITCODE_FILE_NOT_FOUND;
		}
		else
		{
			// If we are on a level other than first, it is ok
			// not to find any entries. This simply means we
			// stumbled upon an empty directory somewhere in a tree.

			return EXITCODE_OK;
		}
	}

	char sPath[MAX_PATH];

	// Make sure that we have no relative path names, but only absolute paths

	if (NULL == _fullpath (sPath, psFilename, sizeof (sPath)))
	{
		// Our path buffer is too small. Bail out!

		return EXITCODE_INTERNAL_ERROR;
	}

	// Find the last blackslash and remove it if found.
	// This will leave us with a root directory.

	LPSTR psBackslash = strrchr (sPath, '\\');
	if (NULL != psBackslash)
	{
		*psBackslash = '\0';
	}

	do
	{
		std::string sNewPath;

		sNewPath = sPath;

		sNewPath += "\\";
		sNewPath += ff.GetName ();

		if (ff.IsSubDir())
		{
			// Here we handle subdirectories

			// strcmp returns 0 on equal strings.
			// Proceed if name contains none of "." or ".."

			if ((0 != strcmp (ff.GetName (), ".")) && (0 != strcmp (ff.GetName (), "..")))
			{
				// Append a wildcard to the path and do a recursive search.

				sNewPath += "\\";
				sNewPath += ff.GetWildcard ();

				NERO_ISO_ITEM * pNewItem = NeroCreateIsoItem ();
				if (NULL == pNewItem)
				{
					DeleteIsoItemTree (*ppItem);
					return EXITCODE_OUT_OF_MEMORY;
				}

				// Attach this item to the beginning of the list.

				if (*ppItem != NULL)
				{
					pNewItem->nextItem = *ppItem;
				}
				*ppItem = pNewItem;

				pNewItem->isDirectory = TRUE;
				time_t t = ff.GetCreateTime ();
				pNewItem->entryTime = *localtime (&t);

				StoreFileName (pNewItem->fileName, pNewItem->longFileName, ff.GetName ());

				// If we the user wants recursive search, only then do we
				// actually recurse at deeper levels otherwise ignore directories
				// alltogether.
				// 
				if (bRecursive)
				{
					// Create an ISO item tree at a deeper level

					EXITCODE code = CreateIsoTree (bRecursive, sNewPath.c_str (), &pNewItem->subDirFirstItem, iLevel + 1);
					if (EXITCODE_OK != code)
					{
						DeleteIsoItemTree (*ppItem);
						return code;
					}

					// We don't allow empty directories. This should probably
					// be regulated by a command line switch.
					// 
					if (pNewItem->subDirFirstItem == NULL)
					{
						// If the newly added directory is empty, remove it!
						// We first detach it from the list and then
						// deallocate it.
						// 
						*ppItem = pNewItem->nextItem;
						pNewItem->nextItem = NULL;
						DeleteIsoItemTree (pNewItem);
					}
				}
			}
		}
		else
		{
			// Here we handle regular files

			NERO_ISO_ITEM * pNewItem = NeroCreateIsoItem ();
			if (NULL == pNewItem)
			{
				DeleteIsoItemTree (*ppItem);
				return EXITCODE_OUT_OF_MEMORY;
			}

			StoreFileName (pNewItem->sourceFilePath, (char *) pNewItem->longSourceFilePath, sNewPath.c_str ());
			
			pNewItem->isDirectory = FALSE;
			time_t t = ff.GetCreateTime ();
			pNewItem->entryTime = *localtime (&t);

			StoreFileName (pNewItem->fileName, pNewItem->longFileName, ff.GetName ());
			
			// Attach this item to the beginning of the list.

			if (*ppItem != NULL)
			{
				pNewItem->nextItem = *ppItem;
			}
			*ppItem = pNewItem;
		}

		ff.FindNext ();
	}
	while (ff.IsValidEntry ());

	return EXITCODE_OK;
}

void CIsoTree::StoreFileName(char (& shortFilename) [252], char * psLongFilename, LPCSTR psFilename)
{
	// If filename can fit in the old field, store it there
	// to prevent unnecessary allocation. If not, allocate
	// a sufficient buffer to hold the string.
	// 
	int iLen = strlen (psFilename);

	if (iLen < sizeof (shortFilename))
	{
		strcpy (shortFilename, psFilename);
	}
	else
	{
		psLongFilename = new char [iLen + 1];
		if (psLongFilename != NULL)
		{
			strcpy (psLongFilename, psFilename);
		}
	}
}
