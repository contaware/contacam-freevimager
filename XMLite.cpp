// XMLite.cpp: implementation of the XMLite class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLite.h"
#include <iostream>
#include <string>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const TCHAR chXMLTagOpen		= _T('<');
static const TCHAR chXMLTagClose	= _T('>');
static const TCHAR chXMLTagPre	= _T('/');
static const TCHAR chXMLEscape = _T('\\');	// for value field escape

static const TCHAR szXMLPIOpen[] = _T("<?");
static const TCHAR szXMLPIClose[] = _T("?>");
static const TCHAR szXMLCommentOpen[] = _T("<!--");
static const TCHAR szXMLCommentClose[] = _T("-->");
static const TCHAR szXMLCDATAOpen[] = _T("<![CDATA[");
static const TCHAR szXMLCDATAClose[] = _T("]]>");

static const XENTITY x_EntityTable[] = {
		{ _T('&'), _T("&amp;"), 5 } ,
		{ _T('\"'), _T("&quot;"), 6 } ,
		{ _T('\''), _T("&apos;"), 6 } ,
		{ _T('<'), _T("&lt;"), 4 } ,
		{ _T('>'), _T("&gt;"), 4 } ,
		{ _T('\n'), _T("&#xA;"), 5 } ,
		{ _T('\r'), _T("&#xD;"), 5 } 
	};

PARSEINFO piDefault;
DISP_OPT optDefault;
XENTITYS entityDefault((LPXENTITY)x_EntityTable, sizeof(x_EntityTable)/sizeof(x_EntityTable[0]) );
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//========================================================
// Name   : _tcschrs
// Desc   : same with _tcspbrk 
// Param  :
// Return :
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcschrs( LPCTSTR psz, LPCTSTR pszchs )
{
	while( psz && *psz )
	{
		if( _tcschr( pszchs, *psz ) )
			return (LPTSTR)psz;
		psz++;
	}
	return NULL;
}

//========================================================
// Name   : _tcsskip
// Desc   : skip space
// Param  : 
// Return : skiped string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcsskip( LPCTSTR psz )
{
	while( psz && _istspace(*psz) ) psz++;
		
	return (LPTSTR)psz;
}

//========================================================
// Name   : _tcsechr
// Desc   : similar with _tcschr with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcsechr( LPCTSTR psz, int ch, int escape )
{
	LPTSTR pch = (LPTSTR)psz;

	while( pch && *pch )
	{
		if( escape != 0 && *pch == escape )
			pch++;
		else
		if( *pch == ch ) 
			return (LPTSTR)pch;
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _tcselen
// Desc   : similar with _tcslen with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
int _tcselen( int escape, LPTSTR srt, LPTSTR end = NULL ) 
{
	int len = 0;
	LPTSTR pch = srt;
	if( end==NULL ) end = (LPTSTR)sizeof(long);
	LPTSTR prev_escape = NULL;
	while( pch && *pch && pch<end )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			len++;
		}
		pch++;
	}
	return len;
}

//========================================================
// Name   : _tcsecpy
// Desc   : similar with _tcscpy with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tcsecpy( LPTSTR psz, int escape, LPTSTR srt, LPTSTR end = NULL )
{
	LPTSTR pch = srt;
	if( end==NULL ) end = (LPTSTR)sizeof(long);
	LPTSTR prev_escape = NULL;
	while( pch && *pch && pch<end )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			*psz++ = *pch;
		}

		pch++;
	}

	*psz = _T('\0');
}

//========================================================
// Name   : _tcsepbrk
// Desc   : similar with _tcspbrk with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcsepbrk( LPCTSTR psz, LPCTSTR chset, int escape )
{
	LPTSTR pch = (LPTSTR)psz;
	LPTSTR prev_escape = NULL;
	while( pch && *pch )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( _tcschr( chset, *pch ) )
				return (LPTSTR)pch;		
		}
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _tcsenicmp
// Desc   : similar with _tcsnicmp with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
int _tcsenicmp( LPCTSTR psz, LPCTSTR str, int len, int escape )
{
	LPTSTR pch = (LPTSTR)psz;
	LPTSTR prev_escape = NULL;
	LPTSTR des = (LPTSTR)str;
	int i = 0;
	
	while( pch && *pch && i < len )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( tolower(*pch) != tolower(des[i]) )
				break;
			i++;
		}
		pch ++;
	}
	
	// find
	if( i == len )
		return 0;
	if( psz && psz[i] > des[i] )
		return 1;
	return -1;
}

//========================================================
// Name   : _tcsenistr
// Desc   : similar with _tcsistr with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcsenistr( LPCTSTR psz, LPCTSTR str, int len, int escape )
{
	LPTSTR pch = (LPTSTR)psz;
	LPTSTR prev_escape = NULL;
	LPTSTR des = (LPTSTR)str;
	int i = 0;
	
	while( pch && *pch )
	{
		if( escape != 0 && *pch == escape && prev_escape == NULL )
			prev_escape = pch;
		else
		{
			prev_escape = NULL;
			if( _tcsenicmp( pch, str, len, escape ) == 0 )
				return (LPTSTR)pch;
		}
		pch++;
	}
	return pch;
}

//========================================================
// Name   : _tcseistr
// Desc   : similar with _tcsistr with escape process
// Param  : escape - will be escape character
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tcseistr( LPCTSTR psz, LPCTSTR str, int escape )
{
	int len = _tcslen( str );
	return _tcsenistr( psz, str, len, escape );
}

//========================================================
// Name   : _SetString
// Desc   : put string of (psz~end) on ps string
// Param  : trim - will be trim?
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _SetString( LPTSTR psz, LPTSTR end, CString* ps, bool trim = FALSE, int escape = 0 )
{
	//trim
	if( trim )
	{
		while( psz && psz < end && _istspace(*psz) ) psz++;
		while( (end-1) && psz < (end-1) && _istspace(*(end-1)) ) end--;
	}
	int len = end - psz;
	if( len <= 0 ) return;
	if( escape )
	{
		len = _tcselen( escape, psz, end );
		LPTSTR pss = ps->GetBufferSetLength( len );
		_tcsecpy( pss, escape, psz, end );
		ps->ReleaseBuffer();
		ps->FreeExtra();
	}
	else
	{
		LPTSTR pss = ps->GetBufferSetLength(len + 1 );
		_tmemcpy( pss, psz, len );
		pss[len] = _T('\0');
		ps->ReleaseBuffer();
		ps->FreeExtra();
	}
	// The following makes strings with only white spaces
	// empty strings. If trim is set that's already done
	// at the beginning of this function. If that behaviour
	// is not wished just comment the following two lines.
	if(XIsEmptyString(*ps))
		ps->Empty();
}

_tagXMLNode::~_tagXMLNode()
{
	Close();
}

void _tagXMLNode::Close()
{
	unsigned int i;
	for( i = 0 ; i < childs.size(); i++)
	{
		LPXNode p = childs[i];
		if( p )
		{
			delete p; childs[i] = NULL;
		}
	}
	childs.clear();
	
	for( i = 0 ; i < attrs.size(); i++)
	{
		LPXAttr p = attrs[i];
		if( p )
		{
			delete p; attrs[i] = NULL;
		}
	}
	attrs.clear();
}
	
// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
//========================================================
// Name   : LoadAttributes
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pi = parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tagXMLNode::LoadAttributes( LPCTSTR pszAttrs , LPPARSEINFO pi /*= &piDefault*/)
{
	LPTSTR xml = (LPTSTR)pszAttrs;

	while( xml && *xml )
	{
		if( xml = _tcsskip( xml ) )
		{
			// close tag
			if( *xml == chXMLTagClose || *xml == chXMLTagPre )
				// wel-formed tag
				return xml;

			// XML Attr Name
			TCHAR* pEnd = _tcspbrk( xml, _T(" =") );
			if( pEnd == NULL ) 
			{
				// error
				if( pi->error_occur == false ) 
				{
					pi->error_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_ATTR_NO_VALUE;
					pi->error_string.Format(_T("<%s> attribute has error "), (LPCTSTR)name);
				}
				return NULL;
			}
			
			LPXAttr attr = new XAttr;
			attr->parent = this;

			// XML Attr Name
			_SetString( xml, pEnd, &attr->name );
			
			// add new attribute
			attrs.push_back( attr );
			xml = pEnd;
			
			// XML Attr Value
			if( xml = _tcsskip( xml ) )
			{
				if ( *xml == _T('=') )
				{
					if( xml = _tcsskip( ++xml ) )
					{
						// if " or '
						// or none quote
						int quote = *xml;
						if( quote == _T('"') || quote == _T('\'') )
							pEnd = _tcsechr( ++xml, quote, chXMLEscape );
						else
						{
							//attr= value> 
							// none quote mode
							pEnd = _tcsepbrk( xml, _T(" >"), chXMLEscape );
						}

						bool trim = pi->trim_value;
						TCHAR escape = pi->escape_value;	
						_SetString( xml, pEnd, &attr->value, trim, escape );
						xml = pEnd;
						// ATTRVALUE 
						if( pi->entity_value && pi->entitys )
							attr->value = pi->entitys->Ref2Entity(attr->value);

						if( quote == _T('"') || quote == _T('\'') )
							xml++;
					}
				}
			}
		}
	}

	// not wel-formed tag
	return NULL;
}

// attr1="value1" attr2='value2' attr3=value3 />
//                                            ^- return pointer
//========================================================
// Name   : LoadAttributes
// Desc   : loading attribute plain xml text
// Param  : pszAttrs - xml of attributes
//          pszEnd - last string
//          pi = parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPTSTR _tagXMLNode::LoadAttributes( LPCTSTR pszAttrs, LPCTSTR pszEnd, LPPARSEINFO pi /*= &piDefault*/ )
{
	LPTSTR xml = (LPTSTR)pszAttrs;

	while( xml && *xml )
	{
		if( xml = _tcsskip( xml ) )
		{
			// close tag
			if( xml >= pszEnd )
				// wel-formed tag
				return xml;

			// XML Attr Name
			TCHAR* pEnd = _tcspbrk( xml, _T(" =") );
			if( pEnd == NULL ) 
			{
				// error
				if( pi->error_occur == false ) 
				{
					pi->error_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_ATTR_NO_VALUE;
					pi->error_string.Format(_T("<%s> attribute has error "), (LPCTSTR)name);
				}
				return NULL;
			}
			
			LPXAttr attr = new XAttr;
			attr->parent = this;

			// XML Attr Name
			_SetString( xml, pEnd, &attr->name );
			
			// add new attribute
			attrs.push_back( attr );
			xml = pEnd;
			
			// XML Attr Value
			if( xml = _tcsskip( xml ) )
			{
				if (*xml == _T('='))
				{
					if( xml = _tcsskip( ++xml ) )
					{
						// if " or '
						// or none quote
						int quote = *xml;
						if( quote == _T('"') || quote == _T('\'') )
							pEnd = _tcsechr( ++xml, quote, chXMLEscape );
						else
						{
							//attr= value> 
							// none quote mode
							pEnd = _tcsepbrk( xml, _T(" >"), chXMLEscape );
						}

						bool trim = pi->trim_value;
						TCHAR escape = pi->escape_value;	
						_SetString( xml, pEnd, &attr->value, trim, escape );
						xml = pEnd;
						// ATTRVALUE 
						if( pi->entity_value && pi->entitys )
							attr->value = pi->entitys->Ref2Entity(attr->value);

						if( quote == _T('"') || quote == _T('\'') )
							xml++;
					}
				}
			}
		}
	}

	// not wel-formed tag
	return NULL;
}

// <?xml version="1.0"?>
//                      ^- return pointer
//========================================================
// Name   : LoadProcessingInstrunction
// Desc   : loading processing instruction
// Param  : pszXml - PI string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPTSTR _tagXMLNode::LoadProcessingInstrunction( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// find the end of pi
	LPTSTR end = _tcsenistr( pszXml, szXMLPIClose, sizeof(szXMLPIClose) / sizeof(TCHAR) - 1, pi ? pi->escape_value : 0 );
	if( end == NULL )
		return NULL;

	// process pi
	if( doc )
	{
		LPTSTR xml = (LPTSTR)pszXml;

		LPXNode node = new XNode;
		node->parent = this;
		node->doc = doc;
		node->type = XNODE_PI;
		
		xml += sizeof(szXMLPIOpen) / sizeof(TCHAR) - 1;
		TCHAR* pTagEnd = _tcspbrk( xml, _T(" ?>") );
		_SetString( xml, pTagEnd, &node->name );
		xml = pTagEnd;
		
		node->LoadAttributes( xml, end, pi );

		doc->childs.push_back( node );
	}

	end += sizeof(szXMLPIClose) / sizeof(TCHAR) - 1;
	return end;
}

// <!-- comment -->
//                 ^- return pointer
//========================================================
// Name   : LoadComment
// Desc   : loading comment
// Param  : pszXml - comment string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPTSTR _tagXMLNode::LoadComment( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// find the end of comment
	LPTSTR end = _tcsenistr( pszXml, szXMLCommentClose, sizeof(szXMLCommentClose) / sizeof(TCHAR) - 1, pi ? pi->escape_value : 0 );
	if( end == NULL )
		return NULL;

	// process comment
	LPXNode par = parent;
	if( parent == NULL && doc )
		par = (LPXNode)&doc;
	if( par )
	{
		LPTSTR xml = (LPTSTR)pszXml;
		xml += sizeof(szXMLCommentOpen) / sizeof(TCHAR) - 1;
		
		LPXNode node = new XNode;
		node->parent = this;
		node->doc = doc;
		node->type = XNODE_COMMENT;
		node->name = _T("#COMMENT");
		_SetString( xml, end, &node->value, FALSE );

		par->childs.push_back( node );
	}

	end += sizeof(szXMLCommentClose) / sizeof(TCHAR) - 1;
	return end;
}

// <![CDATA[ cdata ]]>
//                    ^- return pointer
//========================================================
// Name   : LoadCDATA
// Desc   : loading CDATA
// Param  : pszXml - CDATA string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPTSTR _tagXMLNode::LoadCDATA( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// find the end of CDATA
	LPTSTR end = _tcsenistr( pszXml, szXMLCDATAClose, sizeof(szXMLCDATAClose) / sizeof(TCHAR) - 1, pi ? pi->escape_value : 0 );
	if( end == NULL )
		return NULL;

	// process CDATA
	LPXNode par = parent;
	if( parent == NULL && doc )
		par = (LPXNode)&doc;
	if( par )
	{
		LPTSTR xml = (LPTSTR)pszXml;
		xml += sizeof(szXMLCDATAOpen) / sizeof(TCHAR) - 1;
		
		LPXNode node = new XNode;
		node->parent = this;
		node->doc = doc;
		node->type = XNODE_CDATA;
		node->name = _T("#CDATA");
		_SetString( xml, end, &node->value, FALSE );

		par->childs.push_back( node );
	}

	end += sizeof(szXMLCDATAClose) / sizeof(TCHAR) - 1;
	return end;
}

//========================================================
// Name   : LoadOtherNodes
// Desc   : internal function for loading PI/CDATA/Comment
// Param  : node - current xml node
//          pbRet - error occur
//          pszXml - CDATA string
//          pi - parser information
// Return : advanced string pointer. (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-14
//========================================================
LPTSTR LoadOtherNodes( LPXNode node, bool* pbRet, LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	LPTSTR xml = (LPTSTR)pszXml;
	bool do_other_type = true;
	*pbRet = false;

	while( xml && do_other_type )
	{
		do_other_type = false;

		xml = _tcsskip( xml );
		LPTSTR prev = xml;
		// is PI( Processing Instruction ) Node?
		if( _tcsnicmp( xml, szXMLPIOpen, sizeof(szXMLPIOpen) / sizeof(TCHAR) - 1 ) == 0 )
		{
			// processing instrunction parse
			// return pointer is next node of pi
			xml = node->LoadProcessingInstrunction( xml, pi );
			//if( xml == NULL )
			//	return NULL;
			// restart xml parse
		}

		if( xml != prev )
			do_other_type = true;
		xml = _tcsskip( xml );
		prev = xml;

		// is comment Node?
		if( _tcsnicmp( xml, szXMLCommentOpen, sizeof(szXMLCommentOpen) / sizeof(TCHAR) - 1 ) == 0 )
		{
			// processing comment parse
			// return pointer is next node of comment
			xml = node->LoadComment( xml, pi );
			// comment node is terminal node
			if( node->parent && node->parent->type != XNODE_DOC 
				&& xml != prev )
			{
				*pbRet = true;
				return xml;
			}
			// restart xml parse when this node is root doc node
		}

		if( xml != prev )
			do_other_type = true;

		xml = _tcsskip( xml );
		prev = xml;
		// is CDATA Node?
		if( _tcsnicmp( xml, szXMLCDATAOpen, sizeof(szXMLCDATAOpen) / sizeof(TCHAR) - 1 ) == 0 )
		{
			// processing CDATA parse
			// return pointer is next node of CDATA
			xml = node->LoadCDATA( xml, pi );
			// CDATA node is terminal node
			if( node->parent && node->parent->type != XNODE_DOC 
				&& xml != prev )
			{
				*pbRet = true;
				return xml;
			}
			// restart xml parse when this node is root doc node
		}

		if( xml != prev )
			do_other_type = true;
	}

	return xml;
}

// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
//========================================================
// Name   : Load
// Desc   : load xml plain text
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tagXMLNode::Load( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
	// Close it
	Close();

	LPTSTR xml = (LPTSTR)pszXml;

	xml = _tcschr( xml, chXMLTagOpen );
	if( xml == NULL )
		return NULL;

	// Close Tag
	if( *(xml+1) == chXMLTagPre ) // </Close
		return xml;

	// Load Other Node before <Tag>(pi, comment, CDATA etc)
	bool bRet = false;
	LPTSTR ret = NULL;
	ret = LoadOtherNodes( this, &bRet, xml, pi );
	if( ret != NULL ) 
		xml = ret;
	if( bRet ) 
		return xml;
	if( xml[0] == _T('\0') ) // Oli Added this check
		return xml;

	// XML Node Tag Name Open
	xml++;
	TCHAR* pTagEnd = _tcspbrk( xml, _T(" />\t\r\n") );
	_SetString( xml, pTagEnd, &name );
	xml = pTagEnd;
	// Generate XML Attributte List
	if( xml = LoadAttributes( xml, pi ) )
	{
		// alone tag <TAG ... />
		if( *xml == chXMLTagPre )
		{
			xml++;
			if( *xml == chXMLTagClose )
				// wel-formed tag
				return ++xml;
			else
			{
				// error: <TAG ... / >
				if( pi->error_occur == false ) 
				{
					pi->error_occur = true;
					pi->error_pointer = xml;
					pi->error_code = PIE_ALONE_NOT_CLOSED;
					pi->error_string = _T("Element must be closed.");
				}
				// not wel-formed tag
				return NULL;
			}
		}
		else
		// open/close tag <TAG ..> ... </TAG>
		//                        ^- current pointer
		{
			// if text value is not exist, then assign value
			if( XIsEmptyString( value ) )
			{
				// Text Value 
				TCHAR* pEnd = _tcsechr( ++xml, chXMLTagOpen, chXMLEscape );
				if( pEnd == NULL ) 
				{
					// error cos not exist CloseTag </TAG>
					if( pi->error_occur == false ) 
					{
						pi->error_occur = true;
						pi->error_pointer = xml;
						pi->error_code = PIE_NOT_CLOSED;
						pi->error_string.Format(_T("it must be closed with </%s>"), (LPCTSTR)name);
					}
					return NULL;
				}
				
				bool trim = pi->trim_value;
				TCHAR escape = pi->escape_value;
				_SetString( xml, pEnd, &value, trim, escape );

				xml = pEnd;
				// TEXTVALUE reference
				if( pi->entity_value && pi->entitys )
					value = pi->entitys->Ref2Entity(value);
			}

			// generate child nodes
			while( xml && *xml )
			{
				LPXNode node = new XNode;
				node->parent = this;
				node->doc = doc;
				node->type = type;
				
				xml = node->Load( xml,pi );
				if( node->name.IsEmpty() == FALSE )
				{
					childs.push_back( node );

				}
				else
				{
					delete node;
				}

				// open/close tag <TAG ..> ... </TAG>
				//                             ^- current pointer
				// CloseTag case
				if( xml && *xml && *(xml+1) && *xml == chXMLTagOpen && *(xml+1) == chXMLTagPre )
				{
					// </Close>
					xml+=2; // C
					
					if( xml = _tcsskip( xml ) )
					{
						CString closename;
						TCHAR* pEnd = _tcspbrk( xml, _T(" >") );
						if( pEnd == NULL ) 
						{
							if( pi->error_occur == false ) 
							{
								pi->error_occur = true;
								pi->error_pointer = xml;
								pi->error_code = PIE_NOT_CLOSED;
								pi->error_string.Format(_T("it must be closed with </%s>"), (LPCTSTR)name);
							}
							// error
							return NULL;
						}
						_SetString( xml, pEnd, &closename );
						if( closename == this->name )
						{
							// wel-formed open/close
							xml = pEnd+1;
							// return '>' or ' ' after pointer
							return xml;
						}
						else
						{
							xml = pEnd+1;
							// 2004.6.15 - example <B> alone tag
							// now it can parse with attribute 'force_arse'
							if( pi->force_parse == false )
							{
								// not welformed open/close
								if( pi->error_occur == false ) 
								{
									pi->error_occur = true;
									pi->error_pointer = xml;
									pi->error_code = PIE_NOT_NESTED;
									pi->error_string.Format(_T("'<%s> ... </%s>' is not wel-formed."), (LPCTSTR)name, (LPCTSTR)closename);
								}
								return NULL;
							}
						}
					}
				}
				else	// Alone child Tag Loaded
				{
					if( xml && XIsEmptyString( value ) && *xml !=chXMLTagOpen )
					{
						// Text Value 
						TCHAR* pEnd = _tcsechr( xml, chXMLTagOpen, chXMLEscape );
						if( pEnd == NULL ) 
						{
							// error cos not exist CloseTag </TAG>
							if( pi->error_occur == false )  
							{
								pi->error_occur = true;
								pi->error_pointer = xml;
								pi->error_code = PIE_NOT_CLOSED;
								pi->error_string.Format(_T("it must be closed with </%s>"), (LPCTSTR)name);
							}
							return NULL;
						}
						
						bool trim = pi->trim_value;
						TCHAR escape = pi->escape_value;
						_SetString( xml, pEnd, &value, trim, escape );

						xml = pEnd;
						//TEXTVALUE
						if( pi->entity_value && pi->entitys )
							value = pi->entitys->Ref2Entity(value);
					}
				}
			}
		}
	}

	return xml;
}

// <?xml version='1.0'?>
// <TAG attr1="value1" attr2='value2' attr3=value3 >
// </TAG>
// or
// <TAG />
//        ^- return pointer
//========================================================
// Name   : Load
// Desc   : load xml plain text for xml document
// Param  : pszXml - plain xml text
//          pi = parser information
// Return : advanced string pointer  (error return NULL)
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPTSTR _tagXMLDocument::Load( LPCTSTR pszXml, LPPARSEINFO pi /*= NULL*/ )
{
	LPXNode node = new XNode;
	node->parent = (LPXNode)this;
	node->type = XNODE_ELEMENT;
	node->doc = this;
	LPTSTR end;
	
	if( pi == NULL )
		pi = &parse_info;

	if( (end = node->Load( pszXml, pi )) == NULL )
	{
		delete node;
		return NULL;
	}

	if( node->name.IsEmpty() == FALSE ) // Oli Added this check
	{
		childs.push_back( node );

		// Load Other Node after </Tag>(pi, comment, CDATA etc)
		LPTSTR ret;
		bool bRet = false;
		ret = LoadOtherNodes( node, &bRet, end, pi );
		if( ret != NULL ) 
			end = ret;
	}
	else
		delete node;

	return end;
}

LPXNode	_tagXMLDocument::GetRoot()
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end() ; ++(it) )
	{
		LPXNode node = *it;
		if( node->type == XNODE_ELEMENT )
			return node;
	}
	return NULL;
}

//========================================================
// Name   : GetXML
// Desc   : convert plain xml text from parsed xml attribute
// Param  :
// Return : converted plain string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
CString _tagXMLAttr::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
	tostringstream os;
	os << (LPCTSTR)name << _T("=") << opt->value_quotation_mark 
		<< (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value) 
		<< opt->value_quotation_mark << _T(" ");
	return os.str().c_str();
}

//========================================================
// Name   : GetXML
// Desc   : convert plain xml text from parsed xml node
// Param  :
// Return : converted plain string
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
CString _tagXMLNode::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
	tostringstream os;

	// tab
	if( opt && opt->newline )
	{
		if( opt && opt->newline )
			os << opt->newline_type;
		for( int i = 0 ; i < opt->tab_base ; i++)
			os << _T('\t');
	}

	if( type == XNODE_DOC )
	{
		for( unsigned int i = 0 ; i < childs.size(); i++ )
			os << (LPCTSTR)childs[i]->GetXML( opt );
		return os.str().c_str();
	}
	else
	if( type == XNODE_PI )
	{
		// <?TAG
		os << szXMLPIOpen << (LPCTSTR)name;
		// <?TAG Attr1="Val1" 
		if( attrs.empty() == false ) os << _T(' ');
		for( unsigned int i = 0 ; i < attrs.size(); i++ )
		{
			os << (LPCTSTR)attrs[i]->GetXML(opt);
		}
		//?>
		os << szXMLPIClose;	
		return os.str().c_str();
	}
	else
	if( type == XNODE_COMMENT )
	{
		// <--comment
		os << szXMLCommentOpen << (LPCTSTR)value;
		//-->
		os << szXMLCommentClose;	
		return os.str().c_str();
	}
	else
	if( type == XNODE_CDATA )
	{
		// <--comment
		os << szXMLCDATAOpen << (LPCTSTR)value;
		//-->
		os << szXMLCDATAClose;	
		return os.str().c_str();
	}

	// <TAG
	os << _T('<') << (LPCTSTR)name;

	// <TAG Attr1="Val1" 
	if( attrs.empty() == false ) os << _T(' ');
	for( unsigned int i = 0 ; i < attrs.size(); i++ )
	{
		os << (LPCTSTR)attrs[i]->GetXML(opt);
	}
	
	if( childs.empty() && value.IsEmpty() )
	{
		// <TAG Attr1="Val1"/> alone tag 
		os << _T("/>");	
	}
	else
	{
		// <TAG Attr1="Val1"> and get child
		os << _T('>');
		if( opt && opt->newline && !childs.empty() )
		{
			opt->tab_base++;
		}

		for( unsigned int i = 0 ; i < childs.size(); i++ )
			os << (LPCTSTR)childs[i]->GetXML( opt );
		
		// Text Value
		if( value != _T("") )
		{
			if( opt )
			{
				if( opt->newline && !childs.empty() )
				{
					if( opt && opt->newline )
						os << opt->newline_type;
					for( int i = 0 ; i < opt->tab_base ; i++)
						os << _T('\t');
				}
				os << (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value);
			}
		}

		// </TAG> CloseTag
		if( opt && opt->newline && !childs.empty() )
		{
			os << opt->newline_type;
			for( int i = 0 ; i < opt->tab_base-1 ; i++)
				os << _T('\t');
		}
		os << _T("</") << (LPCTSTR)name << _T('>');

		if( opt && opt->newline )
		{
			if( !childs.empty() )
				opt->tab_base--;
		}
	}
	
	return os.str().c_str();
}

//========================================================
// Name   : GetText
// Desc   : get current value
// Param  :
// Return : string value
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2004-06-15
//========================================================
CString _tagXMLNode::GetText( LPDISP_OPT opt /*= &optDefault*/ )
{
	tostringstream os;

	if( type == XNODE_DOC )
	{
		for( unsigned int i = 0 ; i < childs.size(); i++ )
			os << (LPCTSTR)childs[i]->GetText( opt );
	}
	else
	if( type == XNODE_PI )
	{
		// no text
	}
	else
	if( type == XNODE_COMMENT )
	{
		// no text
	}
	else
	if( type == XNODE_CDATA )
	{
		os << (LPCTSTR)value;
	}
	else
	if( type == XNODE_ELEMENT )
	{
		if( childs.empty() && value.IsEmpty() )
		{
			// no text
		}
		else
		{
			// childs text
			for( unsigned int i = 0 ; i < childs.size(); i++ )
				os << (LPCTSTR)childs[i]->GetText();
			
			// Text Value
			os << (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value):value);
		}
	}
	
	return os.str().c_str();
}

void _tagXMLNode::ClearValueIfHasChild()
{
	if (!childs.empty() && !value.IsEmpty())
		value = _T("");
	for (unsigned int i = 0 ; i < childs.size(); i++ )
		childs[i]->ClearValueIfHasChild();
}

void _tagXMLNode::ResetSpaceOnlyValues(const CString& sResetValue/*=_T("")*/)
{
	BOOL bClearValue = TRUE;
	if (value != _T(""))
	{
		LPTSTR p = value.GetBuffer(value.GetLength() + 1);
		while (p && *p)
		{
			if (!_istspace(*p))
			{
				bClearValue = FALSE;
				break;
			}
			p++;
		}
		value.ReleaseBuffer();
		if (bClearValue)
			value = sResetValue;
	}
	for (unsigned int i = 0 ; i < childs.size(); i++ )
		childs[i]->ResetSpaceOnlyValues(sResetValue);
}

//========================================================
// Name   : GetAttr
// Desc   : get attribute with attribute name
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr	_tagXMLNode::GetAttr( LPCTSTR attrname )
{
	for( unsigned int i = 0 ; i < attrs.size(); i++ )
	{
		LPXAttr attr = attrs[i];
		if( attr )
		{
			if( attr->name == attrname )
				return attr;
		}
	}
	return NULL;
}

//========================================================
// Name   : GetAttrs
// Desc   : find attributes with attribute name, return its list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XAttrs _tagXMLNode::GetAttrs( LPCTSTR name )
{
	XAttrs attrs;
	for( unsigned int i = 0 ; i < attrs.size(); i++ )
	{
		LPXAttr attr = attrs[i];
		if( attr )
		{
			if( attr->name == name )
				attrs.push_back( attr );
		}
	}
	return attrs;
}

XAttrs _tagXMLNode::GetAttrs()
{
	return attrs;
}

//========================================================
// Name   : GetAttrValue
// Desc   : get attribute with attribute name, return its value
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPCTSTR	_tagXMLNode::GetAttrValue( LPCTSTR attrname )
{
	LPXAttr attr = GetAttr( attrname );
	return attr ? (LPCTSTR)attr->value : NULL;
}

LPCTSTR	_tagXMLNode::GetAttrName( LPCTSTR attrvalue )
{
	for( unsigned int i = 0 ; i < attrs.size(); i++ )
	{
		LPXAttr attr = attrs[i];
		if( attr )
		{
			if( attr->value == attrvalue )
				return attr->name;
		}
	}
	return NULL;
}

//========================================================
// Name : GetAttrCount
// Desc : get attributes node count
// Param :
// Return : 0 return if no attributes
//--------------------------------------------------------
// Coder Date Desc
// bro 2005-05-06
//========================================================
int _tagXMLNode::GetAttrCount()
{
	return attrs.size();
}

XNodes _tagXMLNode::GetChilds()
{
	return childs;
}

//========================================================
// Name   : GetChilds
// Desc   : Find childs with name and return childs list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XNodes _tagXMLNode::GetChilds( LPCTSTR name )
{
	XNodes nodes;
	for( unsigned int i = 0 ; i < childs.size(); i++ )
	{
		LPXNode node = childs[i];
		if( node )
		{
			if( node->name == name )
				nodes.push_back( node );
		}
	}
	return nodes;	
}

//========================================================
// Name   : GetChild
// Desc   : get child node with index
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::GetChild( int i )
{
	if( i >= 0 && i < (int)childs.size() )
		return childs[i];
	return NULL;
}

//========================================================
// Name   : GetChildCount
// Desc   : get child node count
// Param  :
// Return : 0 return if no child
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-12-26
//========================================================
int	_tagXMLNode::GetChildCount()
{
	return childs.size();
}

//========================================================
// Name   : GetChild
// Desc   : Find child with name and return child
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::GetChild( LPCTSTR name )
{
	for( unsigned int i = 0 ; i < childs.size(); i++ )
	{
		LPXNode node = childs[i];
		if( node )
		{
			if( node->name == name )
				return node;
		}
	}
	return NULL;
}

//========================================================
// Name   : GetChildValue
// Desc   : Find child with name and return child's value
// Param  :
// Return : NULL return if no child.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPCTSTR	_tagXMLNode::GetChildValue( LPCTSTR name )
{
	LPXNode node = GetChild( name );
	return (node != NULL)? (LPCTSTR)node->value : NULL;
}

CString	_tagXMLNode::GetChildText( LPCTSTR name, LPDISP_OPT opt /*= &optDefault*/ )
{
	LPXNode node = GetChild( name );
	return (node != NULL)? node->GetText(opt) : _T("");
}

LPXAttr _tagXMLNode::GetChildAttr( LPCTSTR name, LPCTSTR attrname )
{
	LPXNode node = GetChild(name);
	return node ? node->GetAttr(attrname) : NULL;
}

LPCTSTR _tagXMLNode::GetChildAttrValue( LPCTSTR name, LPCTSTR attrname )
{
	LPXAttr attr = GetChildAttr( name, attrname );
	return attr ? (LPCTSTR)attr->value : NULL;
}

//========================================================
// Name   : Find
// Desc   : find node with tag name from it's all childs
// Param  :
// Return : NULL return if no found node.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::Find( LPCTSTR name, BOOL bOnlyDirectChilds/*=FALSE*/ )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		if( child->name == name )
			return child;

		if (!bOnlyDirectChilds)
		{
			LPXNode find = child->Find( name );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

LPXNode	_tagXMLNode::Find( LPCTSTR name, LPCTSTR attrname, BOOL bOnlyDirectChilds/*=FALSE*/ )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		if( child->name == name )
		{
			XAttrs attrs = child->GetAttrs();
			for( unsigned int i = 0 ; i < attrs.size(); i ++ )
			{
				LPXAttr attr = attrs[i];
				if (attr->name == attrname)
					return child;
			}
		}

		if (!bOnlyDirectChilds)
		{
			LPXNode find = child->Find( name, attrname );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

LPXNode	_tagXMLNode::Find( LPCTSTR name, LPCTSTR attrname, LPCTSTR attrvalue, BOOL bOnlyDirectChilds/*=FALSE*/ )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		if( child->name == name )
		{
			XAttrs attrs = child->GetAttrs();
			for( unsigned int i = 0 ; i < attrs.size(); i ++ )
			{
				LPXAttr attr = attrs[i];
				if (attr->name == attrname && 
					attr->value == attrvalue)
					return child;
			}
		}

		if (!bOnlyDirectChilds)
		{
			LPXNode find = child->Find( name, attrname, attrvalue );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

LPXNode	_tagXMLNode::FindByAttrName( LPCTSTR name, LPCTSTR attrname, BOOL bOnlyDirectChilds/*=FALSE*/ )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		if( child->name == name )
		{
			XAttrs attrs = child->GetAttrs();
			for( unsigned int i = 0 ; i < attrs.size(); i ++ )
			{
				LPXAttr attr = attrs[i];
				if (attr->name == attrname)
					return child;
			}
		}

		if (!bOnlyDirectChilds)
		{
			LPXNode find = child->FindByAttrName( name, attrname );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

LPXNode	_tagXMLNode::FindByAttrValue( LPCTSTR name, LPCTSTR attrvalue, BOOL bOnlyDirectChilds/*=FALSE*/ )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		if( child->name == name )
		{
			XAttrs attrs = child->GetAttrs();
			for( unsigned int i = 0 ; i < attrs.size(); i ++ )
			{
				LPXAttr attr = attrs[i];
				if (attr->value == attrvalue)
					return child;
			}
		}

		if (!bOnlyDirectChilds)
		{
			LPXNode find = child->FindByAttrValue( name, attrvalue );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

LPXNode	_tagXMLNode::FindByAttrName( LPCTSTR attrname, BOOL bOnlyDirectChilds/*=FALSE*/ )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		XAttrs attrs = child->GetAttrs();
		for( unsigned int i = 0 ; i < attrs.size(); i ++ )
		{
			LPXAttr attr = attrs[i];
			if (attr->name == attrname)
				return child;
		}

		if (!bOnlyDirectChilds)
		{
			LPXNode find = child->FindByAttrName( attrname );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

LPXNode	_tagXMLNode::FindByAttrValue( LPCTSTR attrvalue, BOOL bOnlyDirectChilds/*=FALSE*/ )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end(); ++(it))
	{
		LPXNode child = *it;
		XAttrs attrs = child->GetAttrs();
		for( unsigned int i = 0 ; i < attrs.size(); i ++ )
		{
			LPXAttr attr = attrs[i];
			if (attr->value == attrvalue)
				return child;
		}

		if (!bOnlyDirectChilds)
		{
			LPXNode find = child->FindByAttrValue( attrvalue );
			if( find != NULL )
				return find;
		}
	}

	return NULL;
}

//========================================================
// Name   : GetChildIterator
// Desc   : get child nodes iterator
// Param  :
// Return : NULL return if no childs.
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XNodes::iterator _tagXMLNode::GetChildIterator( LPXNode node )
{
	XNodes::iterator it = childs.begin();
	for( ; it != childs.end() ; ++(it) )
	{
		if( *it == node )
			return it;
	}
	return childs.end();
}

//========================================================
// Name   : AppendChild
// Desc   : add node
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::AppendChild( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	return AppendChild( CreateNode( name, value ) );
}

//========================================================
// Name   : AppendChild
// Desc   : add node
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::AppendChild( LPXNode node )
{
	node->parent = this;
	node->doc = doc;
	childs.push_back( node );
	return node;
}

//========================================================
// Name   : RemoveChild
// Desc   : detach node and delete object
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
bool _tagXMLNode::RemoveChild( LPXNode node )
{
	XNodes::iterator it = GetChildIterator( node );
	if (it != childs.end())
	{
		delete *it;
		childs.erase( it );
		return true;
	}
	return false;
}

//========================================================
// Name   : GetAttr
// Desc   : get attribute with index in attribute list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::GetAttr( int i )
{
	if( i >= 0 && i < (int)attrs.size() )
		return attrs[i];
	return NULL;
}

//========================================================
// Name   : GetAttrIterator
// Desc   : get attribute iterator
// Param  : 
// Return : std::vector<LPXAttr>::iterator
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
XAttrs::iterator _tagXMLNode::GetAttrIterator( LPXAttr attr )
{
	XAttrs::iterator it = attrs.begin();
	for( ; it != attrs.end() ; ++(it) )
	{
		if( *it == attr )
			return it;
	}
	return attrs.end();
}

//========================================================
// Name   : AppendAttr
// Desc   : add attribute
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::AppendAttr( LPXAttr attr )
{
	attr->parent = this;
	attrs.push_back( attr );
	return attr;
}

//========================================================
// Name   : RemoveAttr
// Desc   : detach attribute and delete object
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
bool _tagXMLNode::RemoveAttr( LPXAttr attr )
{
	XAttrs::iterator it = GetAttrIterator( attr );
	if (it != attrs.end())
	{
		delete *it;
		attrs.erase( it );
		return true;
	}
	return false;
}

//========================================================
// Name   : CreateNode
// Desc   : Create node object and return it
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::CreateNode( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	LPXNode node = new XNode;
	node->name = name;
	node->value = value;
	return node;
}

//========================================================
// Name   : CreateAttr
// Desc   : create Attribute object and return it
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::CreateAttr( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	LPXAttr attr = new XAttr;
	attr->name = name;
	attr->value = value;
	return attr;
}

//========================================================
// Name   : AppendAttr
// Desc   : add attribute
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::AppendAttr( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
	return AppendAttr( CreateAttr( name, value ) );
}

//========================================================
// Name   : DetachChild
// Desc   : no delete object, just detach in list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode _tagXMLNode::DetachChild( LPXNode node )
{
	XNodes::iterator it = GetChildIterator( node );
	if (it != childs.end())
	{
		childs.erase( it );
		return node;
	}
	return NULL;
}

//========================================================
// Name   : DetachAttr
// Desc   : no delete object, just detach in list
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXAttr _tagXMLNode::DetachAttr( LPXAttr attr )
{
	XAttrs::iterator it = GetAttrIterator( attr );
	if (it != attrs.end())
	{
		attrs.erase( it );
		return attr;
	}
	return NULL;
}

//========================================================
// Name   : CopyNode
// Desc   : copy current level node with own attributes
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tagXMLNode::CopyNode( LPXNode node )
{
	Close();

	doc = node->doc;
	parent = node->parent;
	name = node->name;
	value = node->value;
	type = node->type;

	// copy attributes
	for( unsigned int i = 0 ; i < node->attrs.size(); i++)
	{
		LPXAttr attr = node->attrs[i];
		if( attr )
			AppendAttr( attr->name, attr->value );
	}
}

//========================================================
// Name   : _CopyBranch
// Desc   : recursive internal copy branch 
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tagXMLNode::_CopyBranch( LPXNode node )
{
	CopyNode( node );

	for( unsigned int i = 0 ; i < node->childs.size(); i++)
	{
		LPXNode child = node->childs[i];
		if( child )
		{
			LPXNode mychild = new XNode;
			mychild->CopyNode( child );
			AppendChild( mychild );

			mychild->_CopyBranch( child );
		}
	}
}

//========================================================
// Name   : AppendChildBranch
// Desc   : add child branch ( deep-copy )
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
LPXNode	_tagXMLNode::AppendChildBranch( LPXNode node )
{
	LPXNode child = new XNode;
	child->CopyBranch( node );

	return AppendChild( child );
}

//========================================================
// Name   : CopyBranch
// Desc   : copy branch ( deep-copy )
// Param  :
// Return : 
//--------------------------------------------------------
// Coder    Date                      Desc
// bro      2002-10-29
//========================================================
void _tagXMLNode::CopyBranch( LPXNode branch )
{
	Close();
	
	_CopyBranch( branch );
}


_tagXMLEntitys::_tagXMLEntitys( LPXENTITY entities, int count )
{
	for( int i = 0; i < count; i++)
		push_back( entities[i] );
}

LPXENTITY _tagXMLEntitys::GetEntity( int entity )
{
	for( unsigned int i = 0 ; i < size(); i++ )
	{
		if( at(i).entity == entity )
			return LPXENTITY(&at(i));
	}
	return NULL;
}

LPXENTITY _tagXMLEntitys::GetEntity( LPTSTR entity )
{
	for( unsigned int i = 0 ; i < size(); i++ )
	{
		LPTSTR ref = (LPTSTR)at(i).ref;
		LPTSTR ps = entity;
		while( ref && *ref )
			if( *ref++ != *ps++ )
				break;
		if( ref && !*ref )	// found!
			return LPXENTITY(&at(i));
	}
	return NULL;
}

int _tagXMLEntitys::GetEntityCount( LPCTSTR str )
{
	int nCount = 0;
	LPTSTR ps = (LPTSTR)str;
	while( ps && *ps )
		if( GetEntity( *ps++ ) ) nCount ++;
	return nCount;
}

int _tagXMLEntitys::Ref2Entity( LPCTSTR estr, LPTSTR str, int strlen )
{
	LPTSTR pes = (LPTSTR)estr;
	LPTSTR ps = str;
	LPTSTR ps_end = ps+strlen;
	while( pes && *pes && ps < ps_end )
	{
		LPXENTITY ent = GetEntity( pes );
		if( ent )
		{
			// copy entity meanning char
			*ps = ent->entity;
			pes += ent->ref_len;
		}
		else
			*ps = *pes++;	// default character copy
		ps++;
	}
	*ps = _T('\0');
	
	// total copied characters
	return ps-str;	
}

int _tagXMLEntitys::Entity2Ref( LPCTSTR str, LPTSTR estr, int estrlen )
{
	LPTSTR ps = (LPTSTR)str;
	LPTSTR pes = (LPTSTR)estr;
	LPTSTR pes_end = pes+estrlen;
	while( ps && *ps && pes < pes_end )
	{
		LPXENTITY ent = GetEntity( *ps );
		if( ent )
		{
			// copy entity string
			LPTSTR ref = (LPTSTR)ent->ref;
			while( ref && *ref )
				*pes++ = *ref++;
		}
		else
			*pes++ = *ps;	// default character copy
		ps++;
	}
	*pes = _T('\0');
	
	// total copied characters
	return pes-estr;
}

CString _tagXMLEntitys::Ref2Entity( LPCTSTR estr )
{
	CString es;
	if( estr )
	{
		int len = _tcslen(estr);
		LPTSTR esbuf = es.GetBufferSetLength( len+1 );
		if( esbuf )
			Ref2Entity( estr, esbuf, len );
		es.ReleaseBuffer();
		es.FreeExtra();
	}
	return es;
}

CString _tagXMLEntitys::Entity2Ref( LPCTSTR str )
{
	CString s;
	if( str )
	{
		int nEntityCount = GetEntityCount(str);
		if( nEntityCount == 0 )
			return CString(str);
		int len = _tcslen(str) + nEntityCount*10 ;
		LPTSTR sbuf = s.GetBufferSetLength( len+1 );
		if( sbuf )
			Entity2Ref( str, sbuf, len );
		s.ReleaseBuffer();
		s.FreeExtra();
	}
	return s;
}

CString XRef2Entity( LPCTSTR estr )
{
	return entityDefault.Ref2Entity( estr );
}

CString XEntity2Ref( LPCTSTR str )
{
	return entityDefault.Entity2Ref( str );
}
