#include "stdafx.h"
#include "..\Helpers.h"
#include "Colorspace.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColor
const CColor::DNamedColor CColor::m_namedColor[CColor::numNamedColors] =
{
	{aliceblue            , _T("aliceblue")},
	{antiquewhite         , _T("antiquewhite")},
	{aqua                 , _T("aqua")},
	{aquamarine           , _T("aquamarine")},
	{azure                , _T("azure")},
	{beige                , _T("beige")},
	{bisque               , _T("bisque")},
	{black                , _T("black")},
	{blanchedalmond       , _T("blanchedalmond")},
	{blue                 , _T("blue")},
	{blueviolet           , _T("blueviolet")},
	{brown                , _T("brown")},
	{burlywood            , _T("burlywood")},
	{cadetblue            , _T("cadetblue")},
	{chartreuse           , _T("chartreuse")},
	{chocolate            , _T("chocolate")},
	{coral                , _T("coral")},
	{cornflower           , _T("cornflower")},
	{cornsilk             , _T("cornsilk")},
	{crimson              , _T("crimson")},
	{cyan                 , _T("cyan")},
	{darkblue             , _T("darkblue")},
	{darkcyan             , _T("darkcyan")},
	{darkgoldenrod        , _T("darkgoldenrod")},
	{darkgray             , _T("darkgray")},
	{darkgreen            , _T("darkgreen")},
	{darkkhaki            , _T("darkkhaki")},
	{darkmagenta          , _T("darkmagenta")},
	{darkolivegreen       , _T("darkolivegreen")},
	{darkorange           , _T("darkorange")},
	{darkorchid           , _T("darkorchid")},
	{darkred              , _T("darkred")},
	{darksalmon           , _T("darksalmon")},
	{darkseagreen         , _T("darkseagreen")},
	{darkslateblue        , _T("darkslateblue")},
	{darkslategray        , _T("darkslategray")},
	{darkturquoise        , _T("darkturquoise")},
	{darkviolet           , _T("darkviolet")},
	{deeppink             , _T("deeppink")},
	{deepskyblue          , _T("deepskyblue")},
	{dimgray              , _T("dimgray")},
	{dodgerblue           , _T("dodgerblue")},
	{firebrick            , _T("firebrick")},
	{floralwhite          , _T("floralwhite")},
	{forestgreen          , _T("forestgreen")},
	{fuchsia              , _T("fuchsia")},
	{gainsboro            , _T("gainsboro")},
	{ghostwhite           , _T("ghostwhite")},
	{gold                 , _T("gold")},
	{goldenrod            , _T("goldenrod")},
	{gray                 , _T("gray")},
	{green                , _T("green")},
	{greenyellow          , _T("greenyellow")},
	{honeydew             , _T("honeydew")},
	{hotpink              , _T("hotpink")},
	{indianred            , _T("indianred")},
	{indigo               , _T("indigo")},
	{ivory                , _T("ivory")},
	{khaki                , _T("khaki")},
	{lavender             , _T("lavender")},
	{lavenderblush        , _T("lavenderblush")},
	{lawngreen            , _T("lawngreen")},
	{lemonchiffon         , _T("lemonchiffon")},
	{lightblue            , _T("lightblue")},
	{lightcoral           , _T("lightcoral")},
	{lightcyan            , _T("lightcyan")},
	{lightgoldenrodyellow , _T("lightgoldenrodyellow")},
	{lightgreen           , _T("lightgreen")},
	{lightgrey            , _T("lightgrey")},
	{lightpink            , _T("lightpink")},
	{lightsalmon          , _T("lightsalmon")},
	{lightseagreen        , _T("lightseagreen")},
	{lightskyblue         , _T("lightskyblue")},
	{lightslategray       , _T("lightslategray")},
	{lightsteelblue       , _T("lightsteelblue")},
	{lightyellow          , _T("lightyellow")},
	{lime                 , _T("lime")},
	{limegreen            , _T("limegreen")},
	{linen                , _T("linen")},
	{magenta              , _T("magenta")},
	{maroon               , _T("maroon")},
	{mediumaquamarine     , _T("mediumaquamarine")},
	{mediumblue           , _T("mediumblue")},
	{mediumorchid         , _T("mediumorchid")},
	{mediumpurple         , _T("mediumpurple")},
	{mediumseagreen       , _T("mediumseagreen")},
	{mediumslateblue      , _T("mediumslateblue")},
	{mediumspringgreen    , _T("mediumspringgreen")},
	{mediumturquoise      , _T("mediumturquoise")},
	{mediumvioletred      , _T("mediumvioletred")},
	{midnightblue         , _T("midnightblue")},
	{mintcream            , _T("mintcream")},
	{mistyrose            , _T("mistyrose")},
	{moccasin             , _T("moccasin")},
	{navajowhite          , _T("navajowhite")},
	{navy                 , _T("navy")},
	{oldlace              , _T("oldlace")},
	{olive                , _T("olive")},
	{olivedrab            , _T("olivedrab")},
	{orange               , _T("orange")},
	{orangered            , _T("orangered")},
	{orchid               , _T("orchid")},
	{palegoldenrod        , _T("palegoldenrod")},
	{palegreen            , _T("palegreen")},
	{paleturquoise        , _T("paleturquoise")},
	{palevioletred        , _T("palevioletred")},
	{papayawhip           , _T("papayawhip")},
	{peachpuff            , _T("peachpuff")},
	{peru                 , _T("peru")},
	{pink                 , _T("pink")},
	{plum                 , _T("plum")},
	{powderblue           , _T("powderblue")},
	{purple               , _T("purple")},
	{red                  , _T("red")},
	{rosybrown            , _T("rosybrown")},
	{royalblue            , _T("royalblue")},
	{saddlebrown          , _T("saddlebrown")},
	{salmon               , _T("salmon")},
	{sandybrown           , _T("sandybrown")},
	{seagreen             , _T("seagreen")},
	{seashell             , _T("seashell")},
	{sienna               , _T("sienna")},
	{silver               , _T("silver")},
	{skyblue              , _T("skyblue")},
	{slateblue            , _T("slateblue")},
	{slategray            , _T("slategray")},
	{snow                 , _T("snow")},
	{springgreen          , _T("springgreen")},
	{steelblue            , _T("steelblue")},
	{tan                  , _T("tan")},
	{teal                 , _T("teal")},
	{thistle              , _T("thistle")},
	{tomato               , _T("tomato")},
	{turquoise            , _T("turquoise")},
	{violet               , _T("violet")},
	{wheat                , _T("wheat")},
	{white                , _T("white")},
	{whitesmoke           , _T("whitesmoke")},
	{yellow               , _T("yellow")},
	{yellowgreen          , _T("yellowgreen")}
};

LPCTSTR CColor::GetNameFromIndex(int i)
{
	if (0 <= i && i < numNamedColors)
		return m_namedColor[i].name;
	else
		return m_namedColor[0].name;
}

CColor CColor::GetColorFromIndex(int i)
{
	if (0 <= i && i < numNamedColors)
		return m_namedColor[i].color;
	else
		return m_namedColor[0].color;
}

CColor CColor::FromString(LPCTSTR pcColor)
{
	CColor t;
	t.SetString(pcColor);
	return t;
}

CColor::CColor(COLORREF cr)
: m_bIsRGB(true), m_bIsHLS(false), m_colorref(cr)
{}

CColor::operator COLORREF() const
{
	const_cast<CColor*>(this)->ToRGB();
	return m_colorref;
}

// String conversion to RRGGBB format
CString CColor::GetString() const 
{
	CString color;
	color.Format(_T("%02X%02X%02X"), GetRed(), GetGreen(), GetBlue());
	return color;
}

bool CColor::SetString(LPCTSTR pcColor) 
{
	ASSERT(pcColor);
	int r, g, b;
	if (_stscanf(pcColor, _T("%2x%2x%2x"), &r, &g, &b) != 3) 
	{
		m_color[c_red] = m_color[c_green] = m_color[c_blue] = 0;
		return false;
	}
	else
	{
		m_color[c_red]   = static_cast<unsigned char>(r);
		m_color[c_green] = static_cast<unsigned char>(g);
		m_color[c_blue]  = static_cast<unsigned char>(b);
		m_bIsRGB = true;
		m_bIsHLS = false;
		return true;
	}
}

CString CColor::GetName() const
{
	const_cast<CColor*>(this)->ToRGB();
	int i = numNamedColors; 
	while (i-- && m_colorref != m_namedColor[i].color);
	if (i < 0) 
		return _T("#") + GetString();
	else
		return m_namedColor[i].name;
}
