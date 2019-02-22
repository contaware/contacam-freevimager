#if !defined(AFX_DESCRIPTIONCOMBOBOX_H__8DA6632A_D989_4EC8_ABD0_AB43877EF0C1__INCLUDED_)
#define AFX_DESCRIPTIONCOMBOBOX_H__8DA6632A_D989_4EC8_ABD0_AB43877EF0C1__INCLUDED_

#pragma once

// For the Combo-Box in dialog editor:
// 1. Set "Has String" to True
// 2. Set "Owner Draw" to Variable
class CDescriptionComboBox : public CComboBox
{
public:
    CDescriptionComboBox();
    virtual ~CDescriptionComboBox();
	CStringArray m_NameArray;

private:
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    BOOL m_bEditHeightSet;
	CFont m_HostFont;
};

#endif // !defined(AFX_DESCRIPTIONCOMBOBOX_H__8DA6632A_D989_4EC8_ABD0_AB43877EF0C1__INCLUDED_)
