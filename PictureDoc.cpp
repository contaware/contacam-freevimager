#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "ToolBarChildFrm.h"
#include "SaveFileDlg.h"
#include "RotationFlippingDlg.h"
#include "ResizingDpiDlg.h"
#include "AnimGifSaveSmallDlg.h"
#include "MonochromeConversionDlg.h"
#include "SharpenDlg.h"
#include "AddBordersDlg.h"
#include "SoftBordersDlg.h"
#include "ColorButtonPicker.h"
#include "XThemeHelper.h"
#include <math.h>
#include "Quantizer.h"
#include "DecreaseBppDlg.h"
#include "RenameDlg.h"
#include "TransAlpha.h"
#include "PostDelayedMessage.h"
#include "DeletePageDlg.h"
#include "Tiff2Pdf.h"
#include "NoVistaFileDlg.h"
#include "AVRec.h"
#include "HelpersAudio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "msimg32.lib")

/////////////////////////////////////////////////////////////////////////////
// PicturDoc

IMPLEMENT_DYNCREATE(CPictureDoc, CUImagerDoc)

BEGIN_MESSAGE_MAP(CPictureDoc, CUImagerDoc)
	//{{AFX_MSG_MAP(CPictureDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_SLIDESHOW_DELAY_2, OnSlideshowDelay2)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_DELAY_2, OnUpdateSlideshowDelay2)
	ON_COMMAND(ID_SLIDESHOW_DELAY_3, OnSlideshowDelay3)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_DELAY_3, OnUpdateSlideshowDelay3)
	ON_COMMAND(ID_SLIDESHOW_DELAY_4, OnSlideshowDelay4)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_DELAY_4, OnUpdateSlideshowDelay4)
	ON_COMMAND(ID_SLIDESHOW_DELAY_5, OnSlideshowDelay5)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_DELAY_5, OnUpdateSlideshowDelay5)
	ON_COMMAND(ID_SLIDESHOW_DELAY_10, OnSlideshowDelay10)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_DELAY_10, OnUpdateSlideshowDelay10)
	ON_COMMAND(ID_SLIDESHOW_DELAY_30, OnSlideshowDelay30)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_DELAY_30, OnUpdateSlideshowDelay30)
	ON_COMMAND(ID_SLIDESHOW_DELAY_60, OnSlideshowDelay60)
	ON_UPDATE_COMMAND_UI(ID_SLIDESHOW_DELAY_60, OnUpdateSlideshowDelay60)
	ON_COMMAND(ID_EDIT_ROTATE_90CW, OnEditRotate90cw)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ROTATE_90CW, OnUpdateEditRotate90cw)
	ON_COMMAND(ID_EDIT_ROTATE_90CCW, OnEditRotate90ccw)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ROTATE_90CCW, OnUpdateEditRotate90ccw)
	ON_COMMAND(ID_EDIT_ROTATE_FLIP, OnEditRotateFlip)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ROTATE_FLIP, OnUpdateEditRotateFlip)
	ON_COMMAND(ID_EDIT_RESIZE, OnEditResize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_RESIZE, OnUpdateEditResize)
	ON_COMMAND(ID_EDIT_HLS, OnEditHls)
	ON_UPDATE_COMMAND_UI(ID_EDIT_HLS, OnUpdateEditHls)
	ON_COMMAND(ID_EDIT_GRAYSCALE, OnEditGrayscale)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GRAYSCALE, OnUpdateEditGrayscale)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_PLAY_SLIDESHOW, OnPlayPlay)
	ON_UPDATE_COMMAND_UI(ID_PLAY_SLIDESHOW, OnUpdatePlayPlay)
	ON_COMMAND(ID_STOP_SLIDESHOW, OnPlayStop)
	ON_UPDATE_COMMAND_UI(ID_STOP_SLIDESHOW, OnUpdatePlayStop)
	ON_COMMAND(ID_PLAY_LOOP, OnPlayLoop)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LOOP, OnUpdatePlayLoop)
	ON_COMMAND(ID_VIEW_NEXT_PICTURE, OnViewNextPicture)
	ON_COMMAND(ID_VIEW_PREVIOUS_PICTURE, OnViewPreviousPicture)
	ON_COMMAND(ID_FILE_INFO, OnFileInfo)
	ON_UPDATE_COMMAND_UI(ID_FILE_INFO, OnUpdateFileInfo)
	ON_COMMAND(ID_PLAY_TRANSITION_OFF, OnPlayTransitionOff)
	ON_UPDATE_COMMAND_UI(ID_PLAY_TRANSITION_OFF, OnUpdatePlayTransitionOff)
	ON_COMMAND(ID_PLAY_TRANSITION_RANDOM, OnPlayTransitionRandom)
	ON_UPDATE_COMMAND_UI(ID_PLAY_TRANSITION_RANDOM, OnUpdatePlayTransitionRandom)
	ON_COMMAND(ID_PLAY_TRANSITION_CURTAININ, OnPlayTransitionCurtainin)
	ON_UPDATE_COMMAND_UI(ID_PLAY_TRANSITION_CURTAININ, OnUpdatePlayTransitionCurtainin)
	ON_COMMAND(ID_PLAY_TRANSITION_ROOLIN, OnPlayTransitionRoolin)
	ON_UPDATE_COMMAND_UI(ID_PLAY_TRANSITION_ROOLIN, OnUpdatePlayTransitionRoolin)
	ON_COMMAND(ID_PLAY_TRANSITION_SLIDEIN, OnPlayTransitionSlidein)
	ON_UPDATE_COMMAND_UI(ID_PLAY_TRANSITION_SLIDEIN, OnUpdatePlayTransitionSlidein)
	ON_COMMAND(ID_PLAY_TRANSITION_CHESSBOARD, OnPlayTransitionChessboard)
	ON_UPDATE_COMMAND_UI(ID_PLAY_TRANSITION_CHESSBOARD, OnUpdatePlayTransitionChessboard)
	ON_COMMAND(ID_EDIT_CROP, OnEditCrop)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CROP, OnUpdateEditCrop)
	ON_COMMAND(ID_EDIT_CROP_APPLY, OnEditCropApply)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CROP_APPLY, OnUpdateEditCropApply)
	ON_COMMAND(ID_EDIT_CROP_CANCEL, OnEditCropCancel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CROP_CANCEL, OnUpdateEditCropCancel)
	ON_COMMAND(ID_EDIT_TO24BITS, OnEditTo24bits)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO24BITS, OnUpdateEditTo24bits)
	ON_COMMAND(ID_EDIT_TO32BITS, OnEditTo32bits)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO32BITS, OnUpdateEditTo32bits)
	ON_COMMAND(ID_EDIT_TO16BITS, OnEditTo16bits)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO16BITS, OnUpdateEditTo16bits)
	ON_COMMAND(ID_EDIT_TO1BIT, OnEditTo1bit)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO1BIT, OnUpdateEditTo1bit)
	ON_COMMAND(ID_PLAY_ANIMATION, OnPlayAnimation)
	ON_UPDATE_COMMAND_UI(ID_PLAY_ANIMATION, OnUpdatePlayAnimation)
	ON_COMMAND(ID_STOP_ANIMATION, OnPlayStopAnimation)
	ON_UPDATE_COMMAND_UI(ID_STOP_ANIMATION, OnUpdatePlayStopAnimation)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_VIEW_NEXT_PAGE_FRAME, OnViewNextPageFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXT_PAGE_FRAME, OnUpdateViewNextPageFrame)
	ON_COMMAND(ID_VIEW_PREVIOUS_PAGE_FRAME, OnViewPreviousPageFrame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIOUS_PAGE_FRAME, OnUpdateViewPreviousPageFrame)
	ON_COMMAND(ID_EDIT_TO1BIT_DITHER_ERRDIFF, OnEditTo1bitDitherErrDiff)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO1BIT_DITHER_ERRDIFF, OnUpdateEditTo1bitDitherErrDiff)
	ON_COMMAND(ID_EDIT_TO8BITS, OnEditTo8bits)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO8BITS, OnUpdateEditTo8bits)
	ON_COMMAND(ID_EDIT_TO4BITS, OnEditTo4bits)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO4BITS, OnUpdateEditTo4bits)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_PLAY_TRANSITION_BLEND, OnPlayTransitionBlend)
	ON_UPDATE_COMMAND_UI(ID_PLAY_TRANSITION_BLEND, OnUpdatePlayTransitionBlend)
	ON_COMMAND(ID_EDIT_REDEYE, OnEditRedeye)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDEYE, OnUpdateEditRedeye)
	ON_COMMAND(ID_EDIT_UPDATE_EXIFTHUMB, OnEditUpdateExifthumb)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UPDATE_EXIFTHUMB, OnUpdateEditUpdateExifthumb)
	ON_COMMAND(ID_EDIT_ADD_EXIFTHUMB, OnEditAddExifthumb)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_EXIFTHUMB, OnUpdateEditAddExifthumb)
	ON_COMMAND(ID_EDIT_REMOVE_EXIFTHUMB, OnEditRemoveExifthumb)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_EXIFTHUMB, OnUpdateEditRemoveExifthumb)
	ON_COMMAND(ID_EDIT_REMOVE_EXIF, OnEditRemoveExif)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_EXIF, OnUpdateEditRemoveExif)
	ON_COMMAND(ID_EDIT_REMOVE_JFIF, OnEditRemoveJfif)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_JFIF, OnUpdateEditRemoveJfif)
	ON_COMMAND(ID_EDIT_REMOVE_OTHERAPP, OnEditRemoveOtherApp)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_OTHERAPP, OnUpdateEditRemoveOtherApp)
	ON_COMMAND(ID_EDIT_REMOVE_COM, OnEditRemoveCom)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_COM, OnUpdateEditRemoveCom)
	ON_COMMAND(ID_EDIT_CLEAR_EXIF_ORIENTATE, OnEditClearExifOrientate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR_EXIF_ORIENTATE, OnUpdateEditClearExifOrientate)
	ON_COMMAND(ID_BACKGROUND_COLOR, OnBackgroundColor)
	ON_UPDATE_COMMAND_UI(ID_BACKGROUND_COLOR, OnUpdateBackgroundColor)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_EDIT_CROP_LOSSLESS, OnEditCropLossless)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CROP_LOSSLESS, OnUpdateEditCropLossless)
	ON_COMMAND(ID_VIEW_STRETCH_HALFTONE, OnViewStretchHalftone)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STRETCH_HALFTONE, OnUpdateViewStretchHalftone)
	ON_COMMAND(ID_EDIT_FILTER_SHARPEN, OnEditFilterSharpen)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FILTER_SHARPEN, OnUpdateEditFilterSharpen)
	ON_COMMAND(ID_EDIT_FILTER_SOFTEN, OnEditFilterSoften)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FILTER_SOFTEN, OnUpdateEditFilterSoften)
	ON_COMMAND(ID_EDIT_REMOVE_IPTC, OnEditRemoveIptc)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_IPTC, OnUpdateEditRemoveIptc)
	ON_COMMAND(ID_VIEW_BACKGROUND_COLOR_MENU, OnViewBackgroundColorMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACKGROUND_COLOR_MENU, OnUpdateViewBackgroundColorMenu)
	ON_COMMAND(ID_EDIT_ADD_BORDERS, OnEditAddBorders)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_BORDERS, OnUpdateEditAddBorders)
	ON_COMMAND(ID_EDIT_SOFT_BORDERS, OnEditSoftBorders)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SOFT_BORDERS, OnUpdateEditSoftBorders)
	ON_COMMAND(ID_EDIT_NEGATIVE, OnEditNegative)
	ON_UPDATE_COMMAND_UI(ID_EDIT_NEGATIVE, OnUpdateEditNegative)
	ON_COMMAND(ID_FILE_SAVE_COPY_AS, OnFileSaveCopyAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_COPY_AS, OnUpdateFileSaveCopyAs)
	ON_COMMAND(ID_EDIT_REMOVE_XMP, OnEditRemoveXmp)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_XMP, OnUpdateEditRemoveXmp)
	ON_COMMAND(ID_EDIT_REMOVE_ICC, OnEditRemoveIcc)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE_ICC, OnUpdateEditRemoveIcc)
	ON_COMMAND(ID_VIEW_ENABLE_OSD, OnViewEnableOsd)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ENABLE_OSD, OnUpdateViewEnableOsd)
	ON_COMMAND(ID_OSD_BKGCOLOR_IMAGE, OnOsdBkgcolorImage)
	ON_COMMAND(ID_OSD_BKGCOLOR_SELECT, OnOsdBkgcolorSelect)
	ON_COMMAND(ID_OSD_FONTSIZE_SMALL, OnOsdFontsizeSmall)
	ON_UPDATE_COMMAND_UI(ID_OSD_FONTSIZE_SMALL, OnUpdateOsdFontsizeSmall)
	ON_COMMAND(ID_OSD_FONTSIZE_MEDIUM, OnOsdFontsizeMedium)
	ON_UPDATE_COMMAND_UI(ID_OSD_FONTSIZE_MEDIUM, OnUpdateOsdFontsizeMedium)
	ON_COMMAND(ID_OSD_FONTSIZE_BIG, OnOsdFontsizeBig)
	ON_UPDATE_COMMAND_UI(ID_OSD_FONTSIZE_BIG, OnUpdateOsdFontsizeBig)
	ON_COMMAND(ID_OSD_OFFTIMEOUT_3, OnOsdOfftimeout3)
	ON_UPDATE_COMMAND_UI(ID_OSD_OFFTIMEOUT_3, OnUpdateOsdOfftimeout3)
	ON_COMMAND(ID_OSD_OFFTIMEOUT_4, OnOsdOfftimeout4)
	ON_UPDATE_COMMAND_UI(ID_OSD_OFFTIMEOUT_4, OnUpdateOsdOfftimeout4)
	ON_COMMAND(ID_OSD_OFFTIMEOUT_5, OnOsdOfftimeout5)
	ON_UPDATE_COMMAND_UI(ID_OSD_OFFTIMEOUT_5, OnUpdateOsdOfftimeout5)
	ON_COMMAND(ID_OSD_OFFTIMEOUT_6, OnOsdOfftimeout6)
	ON_UPDATE_COMMAND_UI(ID_OSD_OFFTIMEOUT_6, OnUpdateOsdOfftimeout6)
	ON_COMMAND(ID_OSD_OFFTIMEOUT_7, OnOsdOfftimeout7)
	ON_UPDATE_COMMAND_UI(ID_OSD_OFFTIMEOUT_7, OnUpdateOsdOfftimeout7)
	ON_COMMAND(ID_OSD_OFFTIMEOUT_INFINITE, OnOsdOfftimeoutInfinite)
	ON_UPDATE_COMMAND_UI(ID_OSD_OFFTIMEOUT_INFINITE, OnUpdateOsdOfftimeoutInfinite)
	ON_COMMAND(ID_OSD_FONTCOLOR_SELECT, OnOsdFontcolorSelect)
	ON_UPDATE_COMMAND_UI(ID_OSD_BKGCOLOR_IMAGE, OnUpdateOsdBkgcolorImage)
	ON_COMMAND(ID_OSD_OPACITY_100, OnOsdOpacity100)
	ON_UPDATE_COMMAND_UI(ID_OSD_OPACITY_100, OnUpdateOsdOpacity100)
	ON_COMMAND(ID_OSD_OPACITY_80, OnOsdOpacity80)
	ON_UPDATE_COMMAND_UI(ID_OSD_OPACITY_80, OnUpdateOsdOpacity80)
	ON_COMMAND(ID_OSD_OPACITY_60, OnOsdOpacity60)
	ON_UPDATE_COMMAND_UI(ID_OSD_OPACITY_60, OnUpdateOsdOpacity60)
	ON_COMMAND(ID_OSD_OPACITY_40, OnOsdOpacity40)
	ON_UPDATE_COMMAND_UI(ID_OSD_OPACITY_40, OnUpdateOsdOpacity40)
	ON_COMMAND(ID_OSD_DISPLAY_DATE, OnOsdDisplayDate)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_DATE, OnUpdateOsdDisplayDate)
	ON_COMMAND(ID_OSD_DISPLAY_LOCATION, OnOsdDisplayLocation)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_LOCATION, OnUpdateOsdDisplayLocation)
	ON_COMMAND(ID_OSD_DISPLAY_HEADLINE_DESCRIPTION, OnOsdDisplayHeadlineDescription)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_HEADLINE_DESCRIPTION, OnUpdateOsdDisplayHeadlineDescription)
	ON_COMMAND(ID_OSD_DISPLAY_FLASH, OnOsdDisplayFlash)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_FLASH, OnUpdateOsdDisplayFlash)
	ON_COMMAND(ID_OSD_DISPLAY_EXPOSURETIME, OnOsdDisplayExposuretime)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_EXPOSURETIME, OnUpdateOsdDisplayExposuretime)
	ON_COMMAND(ID_OSD_DISPLAY_APERTURE, OnOsdDisplayAperture)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_APERTURE, OnUpdateOsdDisplayAperture)
	ON_COMMAND(ID_OSD_CLOSE, OnOsdClose)
	ON_COMMAND(ID_OSD_DISPLAY_FILENAME, OnOsdDisplayFilename)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_FILENAME, OnUpdateOsdDisplayFilename)
	ON_COMMAND(ID_OSD_DISPLAY_SIZESCOMPRESSION, OnOsdDisplaySizescompression)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_SIZESCOMPRESSION, OnUpdateOsdDisplaySizescompression)
	ON_COMMAND(ID_OSD_DEFAULTS, OnOsdDefaults)
	ON_COMMAND(ID_OSD_DISPLAY_METADATADATE, OnOsdDisplayMetadatadate)
	ON_UPDATE_COMMAND_UI(ID_OSD_DISPLAY_METADATADATE, OnUpdateOsdDisplayMetadatadate)
	ON_COMMAND(ID_OSD_AUTOSIZE, OnOsdAutosize)
	ON_UPDATE_COMMAND_UI(ID_OSD_AUTOSIZE, OnUpdateOsdAutosize)
	ON_COMMAND(ID_EDIT_TO15BITS, OnEditTo15bits)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO15BITS, OnUpdateEditTo15bits)
	ON_COMMAND(ID_VIEW_NO_BORDERS, OnViewNoBorders)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NO_BORDERS, OnUpdateViewNoBorders)
	ON_UPDATE_COMMAND_UI(ID_OSD_DEFAULTS, OnUpdateOsdDefaults)
	ON_UPDATE_COMMAND_UI(ID_OSD_FONTCOLOR_SELECT, OnUpdateOsdFontcolorSelect)
	ON_UPDATE_COMMAND_UI(ID_OSD_BKGCOLOR_SELECT, OnUpdateOsdBkgcolorSelect)
	ON_COMMAND(ID_EDIT_PALETTE, OnEditPalette)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PALETTE, OnUpdateEditPalette)
	ON_COMMAND(ID_VIEW_LAYERED_DLG, OnViewLayeredDlg)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LAYERED_DLG, OnUpdateViewLayeredDlg)
	ON_COMMAND(ID_LAYEREDDLG_MAXSIZE_0_66, OnLayereddlgMaxsize066)
	ON_COMMAND(ID_LAYEREDDLG_MAXSIZE_0_50, OnLayereddlgMaxsize050)
	ON_COMMAND(ID_LAYEREDDLG_MAXSIZE_0_33, OnLayereddlgMaxsize033)
	ON_COMMAND(ID_LAYEREDDLG_MAXSIZE_0_25, OnLayereddlgMaxsize025)
	ON_COMMAND(ID_LAYEREDDLG_OPACITY_100, OnLayereddlgOpacity100)
	ON_COMMAND(ID_LAYEREDDLG_OPACITY_80, OnLayereddlgOpacity80)
	ON_COMMAND(ID_LAYEREDDLG_OPACITY_60, OnLayereddlgOpacity60)
	ON_COMMAND(ID_LAYEREDDLG_OPACITY_40, OnLayereddlgOpacity40)
	ON_COMMAND(ID_LAYEREDDLG_CLOSE, OnLayereddlgClose)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_MAXSIZE_0_66, OnUpdateLayereddlgMaxsize066)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_MAXSIZE_0_50, OnUpdateLayereddlgMaxsize050)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_MAXSIZE_0_33, OnUpdateLayereddlgMaxsize033)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_MAXSIZE_0_25, OnUpdateLayereddlgMaxsize025)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_OPACITY_100, OnUpdateLayereddlgOpacity100)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_OPACITY_80, OnUpdateLayereddlgOpacity80)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_OPACITY_60, OnUpdateLayereddlgOpacity60)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_OPACITY_40, OnUpdateLayereddlgOpacity40)
	ON_COMMAND(ID_EDIT_COLORS_COUNT, OnEditColorsCount)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COLORS_COUNT, OnUpdateEditColorsCount)
	ON_COMMAND(ID_EDIT_TEXT_TO_ALPHA, OnEditTextToAlpha)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TEXT_TO_ALPHA, OnUpdateEditTextToAlpha)
	ON_COMMAND(ID_LAYEREDDLG_PASTE, OnLayereddlgPaste)
	ON_COMMAND(ID_LAYEREDDLG_OPACITY_20, OnLayereddlgOpacity20)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_OPACITY_20, OnUpdateLayereddlgOpacity20)
	ON_COMMAND(ID_EDIT_TO32BITS_ALPHA, OnEditTo32bitsAlpha)
	ON_UPDATE_COMMAND_UI(ID_EDIT_TO32BITS_ALPHA, OnUpdateEditTo32bitsAlpha)
	ON_COMMAND(ID_FILE_COPY_TO, OnFileCopyTo)
	ON_COMMAND(ID_FILE_MOVE_TO, OnFileMoveTo)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_0125, OnLayereddlgSize0125)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_0125, OnUpdateLayereddlgSize0125)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_025, OnLayereddlgSize025)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_025, OnUpdateLayereddlgSize025)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_050, OnLayereddlgSize050)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_050, OnUpdateLayereddlgSize050)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_100, OnLayereddlgSize100)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_100, OnUpdateLayereddlgSize100)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_200, OnLayereddlgSize200)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_200, OnUpdateLayereddlgSize200)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_400, OnLayereddlgSize400)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_400, OnUpdateLayereddlgSize400)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_800, OnLayereddlgSize800)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_800, OnUpdateLayereddlgSize800)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_TOPLEFT, OnLayereddlgSizeTopleft)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_TOPLEFT, OnUpdateLayereddlgSizeTopleft)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_TOPRIGHT, OnLayereddlgSizeTopright)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_TOPRIGHT, OnUpdateLayereddlgSizeTopright)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_BOTTOMLEFT, OnLayereddlgSizeBottomleft)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_BOTTOMLEFT, OnUpdateLayereddlgSizeBottomleft)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_BOTTOMRIGHT, OnLayereddlgSizeBottomright)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_BOTTOMRIGHT, OnUpdateLayereddlgSizeBottomright)
	ON_COMMAND(ID_LAYEREDDLG_SIZE_1600, OnLayereddlgSize1600)
	ON_UPDATE_COMMAND_UI(ID_LAYEREDDLG_SIZE_1600, OnUpdateLayereddlgSize1600)
	ON_COMMAND(ID_FILE_SAVE_AS_PDF, OnFileSaveAsPdf)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS_PDF, OnUpdateFileSaveAsPdf)
	ON_COMMAND(ID_EDIT_PASTE_INTO_TOPLEFT, OnEditPasteIntoTopleft)
	ON_COMMAND(ID_EDIT_PASTE_INTO_TOPRIGHT, OnEditPasteIntoTopright)
	ON_COMMAND(ID_EDIT_PASTE_INTO_BOTTOMLEFT, OnEditPasteIntoBottomleft)
	ON_COMMAND(ID_EDIT_PASTE_INTO_BOTTOMRIGHT, OnEditPasteIntoBottomright)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_INTO_TOPLEFT, OnUpdateEditPasteIntoTopleft)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_INTO_TOPRIGHT, OnUpdateEditPasteIntoTopright)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_INTO_BOTTOMLEFT, OnUpdateEditPasteIntoBottomleft)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_INTO_BOTTOMRIGHT, OnUpdateEditPasteIntoBottomright)
	ON_COMMAND(ID_EDIT_PASTE_INTO_FILE, OnEditPasteIntoFile)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_INTO_FILE, OnUpdateEditPasteIntoFile)
	ON_COMMAND(ID_EDIT_ROTATE_180, OnEditRotate180)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ROTATE_180, OnUpdateEditRotate180)
	ON_COMMAND(ID_EDIT_PASTE_INTO_FILE_HELP, OnEditPasteIntoFileHelp)
	ON_COMMAND(ID_PLAY_RANDOM, OnPlayRandom)
	ON_UPDATE_COMMAND_UI(ID_PLAY_RANDOM, OnUpdatePlayRandom)
	ON_COMMAND(ID_VIEW_MAP, OnViewMap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAP, OnUpdateViewMap)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_RENAME, OnEditRename)
	ON_COMMAND(ID_FILE_EXTRACT, OnFileExtract)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXTRACT, OnUpdateFileExtract)
	ON_COMMAND(ID_FILE_RELOAD, OnFileReload)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPictureDoc::CSlideShowThread::CSlideShowThread() 
{
	m_pDoc = NULL;
	m_sStartName = _T("");
	m_nMilliSecondsDelay = DEFAULT_SLIDESHOW_DELAY;
	m_hNextPictureEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hPreviousPictureEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hFirstPictureEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hLastPictureEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hSlideshowTimerEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventArray[0]	= GetKillEvent();
	m_hEventArray[1]	= m_hPreviousPictureEvent;
	m_hEventArray[2]	= m_hFirstPictureEvent;
	m_hEventArray[3]	= m_hLastPictureEvent;
	m_hEventArray[4]	= m_hNextPictureEvent;
	m_hEventArray[5]	= m_hSlideshowTimerEvent;
	m_uiSlideshowTimerId = 0;
	m_bRecursive = FALSE;
	m_bLoop = FALSE;
	m_bRandom = FALSE;
	m_bSlideshowLoadPictureDone = TRUE;
}

CPictureDoc::CSlideShowThread::~CSlideShowThread() 
{
	Kill();
	if (m_uiSlideshowTimerId)
	{
		::timeKillEvent(m_uiSlideshowTimerId);
		m_uiSlideshowTimerId = 0;
	}
	::CloseHandle(m_hNextPictureEvent);
	m_hNextPictureEvent = NULL;
	::CloseHandle(m_hPreviousPictureEvent);
	m_hPreviousPictureEvent = NULL;
	::CloseHandle(m_hFirstPictureEvent);
	m_hFirstPictureEvent = NULL;
	::CloseHandle(m_hLastPictureEvent);
	m_hLastPictureEvent = NULL;
	::CloseHandle(m_hSlideshowTimerEvent);
	m_hSlideshowTimerEvent = NULL;
}

void CPictureDoc::CSlideShowThread::RunSlideshow()
{
	// Kill Timer
	if (m_uiSlideshowTimerId)
	{
		::timeKillEvent(m_uiSlideshowTimerId);
		m_uiSlideshowTimerId = 0;
	}

	// Start Thread
	if (!IsRunning())
		Start();

	// Setup Timer
	m_uiSlideshowTimerId = ::timeSetEvent(	m_nMilliSecondsDelay,
											DEFAULT_TIMER_RESOLUTION,
											(LPTIMECALLBACK)m_hSlideshowTimerEvent,
											0,
											TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);

	// Avoid jpeg full loading
	m_pDoc->m_bDoJPEGGet = FALSE;
}

void CPictureDoc::CSlideShowThread::PauseSlideshow()
{
	// Kill Timer
	if (m_uiSlideshowTimerId)
	{
		::timeKillEvent(m_uiSlideshowTimerId);
		m_uiSlideshowTimerId = 0;
	}

	// Start Thread
	if (!IsRunning())
		Start();

	// Enable jpeg full loading
	if (!m_pDoc->m_bDoJPEGGet)
	{	
		m_pDoc->m_bDoJPEGGet = TRUE;
		m_pDoc->JPEGGet();
		::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
						WM_THREADSAFE_UPDATEIMAGEINFO,
						(WPARAM)TRUE, (LPARAM)0);
	}
}

void CPictureDoc::CSlideShowThread::LoadPicture(LPCTSTR sFileName, BOOL bNext)
{
	// We load the new picture only if the previous load completed.
	// Note that the previous load could still be running in case
	// that this thread exited (see below)
	while (!m_bSlideshowLoadPictureDone)
	{
		// Shutdown?
		if (::WaitForSingleObject(GetKillEvent(), SLIDESHOW_LOADPICTURE_WAIT) == WAIT_OBJECT_0)
			return;
	}

	// Load
	if (!m_pDoc->m_bClosing)
	{
		// Clear flag
		m_bSlideshowLoadPictureDone = FALSE;

		// We do not use SendMessage to avoid dead
		// locks on exiting the application!
		if (::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_SLIDESHOW_LOAD_PICTURE,
							(WPARAM)(new CString(sFileName)),
							(LPARAM)bNext))
		{
			// The main UI thread tries to load the picture and when done
			// always sets m_bSlideshowLoadPictureDone. Here we wait until
			// done; we stop waiting when exiting this thread!
			while (!m_bSlideshowLoadPictureDone)
			{
				// Shutdown?
				if (::WaitForSingleObject(GetKillEvent(), SLIDESHOW_LOADPICTURE_WAIT) == WAIT_OBJECT_0)
					return;
			}
		}
		else
			m_bSlideshowLoadPictureDone = TRUE;
	}
}

int CPictureDoc::CSlideShowThread::Work() 
{
	ASSERT(m_pDoc);

	CString sStartFileName;

	// Dir or File?
	DWORD dwAttrib = ::GetFileAttributes(m_sStartName);
	if (dwAttrib != 0xFFFFFFFF)
	{
		// Directory
		if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
		{
			m_pDoc->m_sDirName = m_sStartName;
			sStartFileName = _T("");
		}
		// Normal File
		else
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];
			TCHAR szExt[_MAX_EXT];
			TCHAR szName[_MAX_FNAME];
			_tsplitpath(m_sStartName, szDrive, szDir, szName, szExt);
			m_pDoc->m_sDirName = CString(szDrive) + CString(szDir);
			sStartFileName = CString(szName) + CString(szExt);
		}
	}
	else
	{
		m_pDoc->CloseDocumentForce();
		return 0;
	}
	
	// Adjust Directory Name
	m_pDoc->m_sDirName.TrimRight(_T('\\'));

	// Init Recursive File Find
	if (m_bRecursive)
	{
		if (!m_pDoc->m_FileFind.InitRecursive(m_pDoc->m_sDirName + _T("\\*")))
		{
			m_pDoc->CloseDocumentForce();
			return 0;
		}
	}
	// Init Normal File Find
	else
	{
		if (!m_pDoc->m_FileFind.Init(m_pDoc->m_sDirName + _T("\\*")))
		{
			m_pDoc->CloseDocumentForce();
			return 0;
		}
	}

	// SlideShow returns FALSE if an error occurs and
	// TRUE if it's time to Shutdown the Thread
	if (!SlideShow(sStartFileName))
		m_pDoc->CloseDocumentForce();
	
	return 0;
}

void CPictureDoc::CSlideShowThread::NextPicture()
{
	::SetEvent(m_hNextPictureEvent);
}

void CPictureDoc::CSlideShowThread::PreviousPicture()
{
	::SetEvent(m_hPreviousPictureEvent);
}

void CPictureDoc::CSlideShowThread::FirstPicture()
{
	::SetEvent(m_hFirstPictureEvent);
}

void CPictureDoc::CSlideShowThread::LastPicture()
{
	::SetEvent(m_hLastPictureEvent);
}

BOOL CPictureDoc::CSlideShowThread::SlideShow(CString sStartFileName)
{
	DWORD Event;

	// Never Show Message Box On Error 
	m_pDoc->SetShowMessageBoxOnError(FALSE);

	// Do Jpeg Get when not playing
	if (m_uiSlideshowTimerId == 0)
		m_pDoc->m_bDoJPEGGet = TRUE;
	else
		m_pDoc->m_bDoJPEGGet = FALSE;

	// Wait that the String Array of m_FileFind is filling up
	if (m_bRecursive)
	{
		int nRet = m_pDoc->m_FileFind.WaitRecursiveStarted(GetKillEvent());
		if (nRet == -1)						// shutdown event
			return TRUE;
		else if (nRet == 0 || nRet == -2)	// no files or error
			return FALSE;
	}

	// Find First File
    if (!m_pDoc->m_FileFind.FindFirstFile())
		return FALSE;

	// Find Init File
	if (sStartFileName != _T(""))
	{
		do
		{
			if (sStartFileName.CompareNoCase(m_pDoc->m_FileFind.GetShortFileName()) == 0)
				break;
		}
		while (m_pDoc->m_FileFind.FindNextFile());

		// Update
		::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
						WM_THREADSAFE_SETDOCUMENTTITLE,
						0, 0);
		::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
						WM_THREADSAFE_UPDATEIMAGEINFO,
						(WPARAM)TRUE, (LPARAM)0);
	}
	else
		LoadPicture(m_pDoc->m_FileFind.GetFileName(), TRUE);

	// Do Slideshow
	while (TRUE)
    {   
		Event = ::WaitForMultipleObjects(6, m_hEventArray, FALSE, SLIDESHOW_DIRCHECK_TIMEOUT);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		return TRUE;

			// Previous Picture Event
			case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hPreviousPictureEvent);
										if (!m_pDoc->m_FileFind.FindPreviousFile())
											LastPicture();
										else
											LoadPicture(m_pDoc->m_FileFind.GetFileName(), FALSE);
										break;

			// First Picture Event
			case WAIT_OBJECT_0 + 2 :	::ResetEvent(m_hFirstPictureEvent);
										if (m_pDoc->m_FileFind.FindFirstFile())
											LoadPicture(m_pDoc->m_FileFind.GetFileName(), TRUE);
										break;

			// Last Picture Event
			case WAIT_OBJECT_0 + 3 :	::ResetEvent(m_hLastPictureEvent);
										if (m_pDoc->m_FileFind.FindLastFile())
											LoadPicture(m_pDoc->m_FileFind.GetFileName(), FALSE);
										break;

			// Next Picture
			case WAIT_OBJECT_0 + 4 :	::ResetEvent(m_hNextPictureEvent);
										if (!::IsExistingFile(m_pDoc->m_FileFind.GetFileName())) // in case picture was deleted
										{
											m_pDoc->m_FileFind.DeleteFileName(m_pDoc->m_FileFind.GetFilePosition()); // it jumps to first position if it was at last
											if (m_pDoc->m_FileFind.GetFilesCount() <= 0)
												return FALSE;
											else
												LoadPicture(m_pDoc->m_FileFind.GetFileName(), TRUE);
										}
										else
										{
											if (!m_pDoc->m_FileFind.FindNextFile())
												FirstPicture();
											else
												LoadPicture(m_pDoc->m_FileFind.GetFileName(), TRUE);
										}
										break;

			// Slideshow Timer
			case WAIT_OBJECT_0 + 5 :	::ResetEvent(m_hSlideshowTimerEvent);
										BOOL res;
										if (m_bRandom)
											res = m_pDoc->m_FileFind.FindRandomFile();
										else
											res = m_pDoc->m_FileFind.FindNextFile();
										if (!res)
										{
											if (m_bLoop)
												FirstPicture();
											else
												::PostMessage(m_pDoc->GetView()->GetSafeHwnd(), WM_THREADSAFE_PAUSESLIDESHOW, (WPARAM)0, (LPARAM)0);
										}
										else
											LoadPicture(m_pDoc->m_FileFind.GetFileName(), TRUE);
										break;

			// Timeout
			case WAIT_TIMEOUT :			break;

			default:					return FALSE;
		}

		// Make Sure the Dir Exists!
		// (CD Eject or entire dir delete)
		if (!::IsExistingDir(m_pDoc->m_sDirName))
			return FALSE;
    }
}

BOOL CPictureDoc::CLayeredDlgThread::DoIt(CWorkerThread* pThread/*=NULL*/) 
{
	ASSERT(m_pDoc);

	// Convert to 32 bits if necessary
	if (m_Dib.GetBitCount() != 32)
		m_Dib.ConvertTo32bits();

	// Before using the UpdateLayeredWindow we have to
	// pre-multiply the rgb channels with the alpha channel
	for (unsigned int y = 0; y < m_Dib.GetHeight() ; y++)
	{
		LPBYTE p = m_Dib.GetBits() + 4 * y * m_Dib.GetWidth();
		for (unsigned int x = 0 ; x < m_Dib.GetWidth() ; x++)
		{
			if (!m_Dib.HasAlpha())
				p[3] = 255;
			p[0] = (int)p[0] * (int)p[3] / 255;
			p[1] = (int)p[1] * (int)p[3] / 255;
			p[2] = (int)p[2] * (int)p[3] / 255;
			p += 4;
		}
	}

	// Be sure that the alpha flag is set
	m_Dib.SetAlpha(TRUE);

	// Adapt image
	if (m_nMaxsizePercent)
	{
		// Shrink if necessary
		CSize szMonitor = ::AfxGetMainFrame()->GetMonitorSize();
		int nMaxSizeX = m_nMaxsizePercent * szMonitor.cx / 100;
		int nMaxSizeY = m_nMaxsizePercent * szMonitor.cy / 100;
		if (nMaxSizeX + LAYERED_DLG_LEFTBORDER + LAYERED_DLG_RIGHTBORDER > szMonitor.cx)
			nMaxSizeX = szMonitor.cx - LAYERED_DLG_LEFTBORDER - LAYERED_DLG_RIGHTBORDER;
		if (nMaxSizeY + LAYERED_DLG_TOPBORDER + LAYERED_DLG_BOTTOMBORDER > szMonitor.cy)
			nMaxSizeY = szMonitor.cy - LAYERED_DLG_TOPBORDER - LAYERED_DLG_BOTTOMBORDER;
		if ((int)m_Dib.GetWidth() > nMaxSizeX || (int)m_Dib.GetHeight() > nMaxSizeY)
		{
			if (!m_Dib.StretchBitsFitRect(	nMaxSizeX,
											nMaxSizeY,
											NULL,
											NULL,
											TRUE,
											pThread,
											!m_pDoc->m_bStretchModeHalftone))
				return FALSE;
		}
	}
	else
	{
		// Crop if necessary
		CSize szMonitor = ::AfxGetMainFrame()->GetMonitorSize();
		int nMaxSizeX = LAYERED_DLG_BOUNDARY_PERCENT * szMonitor.cx / 100;
		int nMaxSizeY = LAYERED_DLG_BOUNDARY_PERCENT * szMonitor.cy / 100;
		if (nMaxSizeX + LAYERED_DLG_LEFTBORDER + LAYERED_DLG_RIGHTBORDER > szMonitor.cx)
			nMaxSizeX = szMonitor.cx - LAYERED_DLG_LEFTBORDER - LAYERED_DLG_RIGHTBORDER;
		if (nMaxSizeY + LAYERED_DLG_TOPBORDER + LAYERED_DLG_BOTTOMBORDER > szMonitor.cy)
			nMaxSizeY = szMonitor.cy - LAYERED_DLG_TOPBORDER - LAYERED_DLG_BOTTOMBORDER;
		int nWidth = m_Dib.GetWidth();
		int nHeight = m_Dib.GetHeight();
		nWidth *= m_nSizePerthousand / 1000; 
		nHeight *= m_nSizePerthousand / 1000;
		if (nWidth > nMaxSizeX && nHeight > nMaxSizeY)
		{
			DWORD w, h;
			switch (m_pDoc->m_nLayeredDlgOrigin)
			{
				// Top-Left
				case 0 :
					if (!m_Dib.Crop(0, 0,
									MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand)),
									MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand))))
						return FALSE;
					break;
				// Top-Right
				case 1 :
					w = MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand));
					if (!m_Dib.Crop(m_Dib.GetWidth() - w,
									0,
									w,
									MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand))))
						return FALSE;
					break;
				// Bottom-Left
				case 2 :
					h = MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand));
					if (!m_Dib.Crop(0,
									m_Dib.GetHeight() - h,
									MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand)),
									h))
						return FALSE;
					break;
				// Bottom-Right
				default :
					w = MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand));
					h = MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand));
					if (!m_Dib.Crop(m_Dib.GetWidth() - w,
									m_Dib.GetHeight() - h,
									w,
									h))
						return FALSE;
					break;
			}
		}
		else if (nWidth > nMaxSizeX)
		{
			DWORD w;
			switch (m_pDoc->m_nLayeredDlgOrigin)
			{
				// Top-Left
				case 0 :
					if (!m_Dib.Crop(0, 0,
									MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand)),
									m_Dib.GetHeight()))
						return FALSE;
					break;
				// Top-Right
				case 1 :
					w = MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand));
					if (!m_Dib.Crop(m_Dib.GetWidth() - w,
									0,
									w,
									m_Dib.GetHeight()))
						return FALSE;
					break;
				// Bottom-Left
				case 2 :
					if (!m_Dib.Crop(0, 0,
									MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand)),
									m_Dib.GetHeight()))
						return FALSE;
					break;
				// Bottom-Right
				default :
					w = MIN(m_Dib.GetWidth(), (DWORD)(nMaxSizeX * 1000 / m_nSizePerthousand));
					if (!m_Dib.Crop(m_Dib.GetWidth() - w,
									0,
									w,
									m_Dib.GetHeight()))
						return FALSE;
					break;
			}
		}
		else if (nHeight > nMaxSizeY)
		{
			DWORD h;
			switch (m_pDoc->m_nLayeredDlgOrigin)
			{
				// Top-Left
				case 0 :
					if (!m_Dib.Crop(0, 0,
									m_Dib.GetWidth(),
									MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand))))
						return FALSE;
					break;
				// Top-Right
				case 1 :
					if (!m_Dib.Crop(0, 0,
									m_Dib.GetWidth(),
									MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand))))
						return FALSE;
					break;
				// Bottom-Left
				case 2 :
					h = MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand));
					if (!m_Dib.Crop(0,
									m_Dib.GetHeight() - h,
									m_Dib.GetWidth(),
									h))
						return FALSE;
					break;
				// Bottom-Right
				default :
					h = MIN(m_Dib.GetHeight(), (DWORD)(nMaxSizeY * 1000 / m_nSizePerthousand));
					if (!m_Dib.Crop(0,
									m_Dib.GetHeight() - h,
									m_Dib.GetWidth(),
									h))
						return FALSE;
					break;
			}
		}

		// Stretch if necessary
		if (m_nSizePerthousand != 1000)
		{
			if (!m_Dib.StretchBits(	m_Dib.GetWidth() * m_nSizePerthousand / 1000,
									m_Dib.GetHeight() * m_nSizePerthousand / 1000,
									NULL,
									NULL,
									TRUE,
									pThread,
									!m_pDoc->m_bStretchModeHalftone))
				return FALSE;
		}
	}

	// Add borders
	if (!m_Dib.AddBorders(	LAYERED_DLG_LEFTBORDER,
							LAYERED_DLG_TOPBORDER,
							LAYERED_DLG_RIGHTBORDER,
							LAYERED_DLG_BOTTOMBORDER,
							0,
							NULL,
							NULL,
							TRUE,
							pThread))
		return FALSE;

	// The DCs
	HDC hScreenDC = ::GetDC(NULL);
	HDC hMemDC = ::CreateCompatibleDC(hScreenDC);
	HGDIOBJ hOldBitmap = ::SelectObject(hMemDC, m_Dib.GetSafeHandle());

	// Get the window rectangle
	CRect rcDlg;
	m_pDoc->m_pLayeredDlg->GetWindowRect(&rcDlg);

	// Calculate the new window position/size based on the bitmap size
	POINT ptWindowScreenPosition;
	ptWindowScreenPosition.x = rcDlg.left;
	ptWindowScreenPosition.y = rcDlg.top;
	SIZE szWindow;
	szWindow.cx = m_Dib.GetWidth();
	szWindow.cy = m_Dib.GetHeight();

	// Setup the blend function
	BLENDFUNCTION blendPixelFunction= {AC_SRC_OVER, 0, (BYTE)m_pDoc->m_nLayeredDlgOpacity, AC_SRC_ALPHA};
	POINT ptSrc;
	ptSrc.x = 0;
	ptSrc.y = 0;

	// Perform the alpha blend
	BOOL bRet = m_pDoc->m_pUpdateLayeredWindow(	m_pDoc->m_pLayeredDlg->GetSafeHwnd(),
												hScreenDC,
												&ptWindowScreenPosition,
												&szWindow,
												hMemDC,
												&ptSrc,
												0,
												&blendPixelFunction,
												ULW_ALPHA);

	// Clean-up
	::SelectObject(hMemDC, hOldBitmap);

	return bRet;
}

void CPictureDoc::CJpegThread::OnExit()
{
	// Not Calculated
	ResetJpegCompressionQuality();
	CleanUp();
}

void CPictureDoc::CJpegThread::CleanUp()
{
	if (m_pDoc)
	{
		// ENTER CS
		m_pDoc->m_csLoadFullJpegDib.EnterCriticalSection();

		m_pDoc->m_bLoadFullJpegTransitionWorker = FALSE;

		if (m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC)
		{
			if (m_pDoc->GetView()->m_hLoadFullJpegTransitionOldBitmap)
			{
				::SelectObject(	m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC,
								m_pDoc->GetView()->m_hLoadFullJpegTransitionOldBitmap);
				m_pDoc->GetView()->m_hLoadFullJpegTransitionOldBitmap = NULL;
			}
			::DeleteDC(m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC);
			m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC = NULL;
		}

		// LEAVE CS
		m_pDoc->m_csLoadFullJpegDib.LeaveCriticalSection();
	}
}

int CPictureDoc::CJpegThread::Work() 
{
	ASSERT(m_pDoc);

	DWORD Event;
	CDib Dib;

	// Current Monitor Bpp
	int nCurrentMonitorBpp = ::AfxGetMainFrame()->GetMonitorBpp();

	// Current Monitor Area
	CSize szCurrentMonitor = ::AfxGetMainFrame()->GetMonitorSize();
	int nCurrentMonitorArea = szCurrentMonitor.cx * szCurrentMonitor.cy;

	// No Message Box On Error
	Dib.SetShowMessageBoxOnError(FALSE);

	// This flag is disabled when first opening a document,
	// from now on it is permitted to cancel the 
	// Load Full Jpeg transitions
	m_pDoc->m_bCancelLoadFullJpegTransitionAllowed = TRUE;

	// Full Load Current Picture?
	if (m_bDoFullLoad)
	{
		// Load Dib!
		if (!Dib.LoadJPEG(m_sFileName, 1, FALSE, FALSE, m_pDoc->GetView(), FALSE, this))
		{
			OnExit();
			return 0;
		}
		CDib::AutoOrientateDib(&Dib);

		// ENTER CS
		m_pDoc->m_csLoadFullJpegDib.EnterCriticalSection();
		
		if (!m_pDoc->m_pLoadFullJpegDib)
			m_pDoc->m_pLoadFullJpegDib = new CDib;
		*(m_pDoc->m_pLoadFullJpegDib) = Dib;
	}
	else
	{
		// We Want an Empty Dib!

		// ENTER CS
		m_pDoc->m_csLoadFullJpegDib.EnterCriticalSection();

		if (!m_pDoc->m_pLoadFullJpegDib)
			m_pDoc->m_pLoadFullJpegDib = new CDib;
		else
			m_pDoc->m_pLoadFullJpegDib->Free();
	}

	// Do Exit?
	if (DoExit())
	{
		m_pDoc->m_csLoadFullJpegDib.LeaveCriticalSection(); // LEAVE CS
		OnExit();
		return 0;
	}

	// Start Full Jpeg Transition
	if (m_pDoc->m_pLoadFullJpegDib				&&
		m_pDoc->m_pLoadFullJpegDib->IsValid()	&&
		m_pDoc->GetView()->m_LoadFullJpegTransitionDib.IsValid())
	{
		// Wait until the transition thread finishes.
		// It's important to wait because the transition
		// thread uses the preview of m_pDib.
		// This means that we cannot change the m_pDib
		// pointer from preview to full while the
		// transition thread is running!
		m_hEventArray[0] = GetKillEvent();
		m_hEventArray[1] = m_pDoc->m_TransitionThread.GetHandle();
		Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, INFINITE);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :	m_pDoc->m_csLoadFullJpegDib.LeaveCriticalSection(); // LEAVE CS
									OnExit();
									return 0;

			// Transition Thread Exited
			case WAIT_OBJECT_0 + 1 :break;

			case WAIT_TIMEOUT :		break;
			default:				break;
		}
		
		// Check Size Limit
		HDC hSrcMemDC = NULL;
		HBITMAP hOldSrcBitmap = NULL;
		if ((ULONGLONG)(m_pDoc->GetView()->m_ZoomRect.Width()) * (ULONGLONG)(m_pDoc->GetView()->m_ZoomRect.Height()) <=
			(ULONGLONG)nCurrentMonitorArea)
		{
			// Get DC
			CDC* pDC = m_pDoc->GetView()->GetDC();

			// Src Mem DC
			if (Dib.GetBitCount() == 8)	// Grayscale Jpeg
			{
				if (nCurrentMonitorBpp == 32)
					Dib.ConvertTo32bits();
				else
					Dib.ConvertTo24bits();
			}
			Dib.BitsToDibSection();
			if (Dib.StretchDibSection(	m_pDoc->GetView()->m_ZoomRect.Width(),
										m_pDoc->GetView()->m_ZoomRect.Height(),
										m_pDoc->m_bStretchModeHalftone ? HALFTONE : COLORONCOLOR))
			{
				hSrcMemDC = ::CreateCompatibleDC(pDC->GetSafeHdc());
				hOldSrcBitmap = (HBITMAP)::SelectObject(hSrcMemDC, Dib.GetSafeHandle());
			}

			// Dst Mem DC
			ASSERT(m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC == NULL);
			ASSERT(m_pDoc->GetView()->m_hLoadFullJpegTransitionOldBitmap == NULL);
			if (m_pDoc->GetView()->m_LoadFullJpegTransitionDib.GetBitCount() == 8) // Grayscale Jpeg
			{
				if (nCurrentMonitorBpp == 32)
					m_pDoc->GetView()->m_LoadFullJpegTransitionDib.ConvertTo32bits();
				else
					m_pDoc->GetView()->m_LoadFullJpegTransitionDib.ConvertTo24bits();
			}
			m_pDoc->GetView()->m_LoadFullJpegTransitionDib.BitsToDibSection();
			if (m_pDoc->GetView()->m_LoadFullJpegTransitionDib.StretchDibSection(	m_pDoc->GetView()->m_ZoomRect.Width(),
																					m_pDoc->GetView()->m_ZoomRect.Height(),
																					m_pDoc->m_bStretchModeHalftone ? HALFTONE : COLORONCOLOR))
			{
				m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC = ::CreateCompatibleDC(pDC->GetSafeHdc());
				m_pDoc->GetView()->m_hLoadFullJpegTransitionOldBitmap =
										(HBITMAP)::SelectObject(m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC,
																m_pDoc->GetView()->m_LoadFullJpegTransitionDib.GetSafeHandle());
			}

			// Release DC
			m_pDoc->GetView()->ReleaseDC(pDC);

			// Flush the GDI batch queue 
			::GdiFlush();
		}

		// Set m_bLoadFullJpegTransitionWorker
		m_pDoc->m_bLoadFullJpegTransitionWorker = TRUE;

		// LEAVE CS
		m_pDoc->m_csLoadFullJpegDib.LeaveCriticalSection();

		// Do Exit?
		if (DoExit())
		{
			OnExit();
			return 0;
		}

		// Set Timer
		UINT uiTimerId = ::timeSetEvent(JPEG_LOADFULL_TRANSITION_DELAY,
										DEFAULT_TIMER_RESOLUTION,
										(LPTIMECALLBACK)m_hTimerEvent, 0,
										TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);

		// Init Event Array
		m_hEventArray[0] = GetKillEvent();
		m_hEventArray[1] = m_hTimerEvent;

		// Send the Draw Messages each JPEG_LOADFULL_TRANSITION_DELAY Milliseconds
		while (	m_pDoc->m_bLoadFullJpegTransitionUI &&
				m_pDoc->m_bLoadFullJpegTransitionWorker)
		{
			Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, (uiTimerId == 0) ?
											JPEG_LOADFULL_TRANSITION_DELAY : INFINITE);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		::timeKillEvent(uiTimerId);
											if (hSrcMemDC)
											{
												if (hOldSrcBitmap)
													::SelectObject(hSrcMemDC, hOldSrcBitmap);
												::DeleteDC(hSrcMemDC);
											}
											OnExit();
											return 0;

				// Timer Event
				case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hTimerEvent);

				// Timeout
				case WAIT_TIMEOUT :			m_pDoc->m_csLoadFullJpegDib.EnterCriticalSection(); // ENTER CS

											m_pDoc->m_bLoadFullJpegTransitionWorker =
												m_pDoc->TransitionLoadFullJpeg(	hSrcMemDC,
																				0,
																				0,
																				m_pDoc->GetView()->m_hLoadFullJpegTransitionMemDC,
																				0,
																				0,
																				m_pDoc->GetView()->m_ZoomRect.Width(),
																				m_pDoc->GetView()->m_ZoomRect.Height(),
																				m_pDoc->GetView()->m_nLoadFullJpegTransitionStep++);

											// Flush the GDI batch queue 
											::GdiFlush();

											m_pDoc->m_csLoadFullJpegDib.LeaveCriticalSection(); // LEAVE CS

											::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
															WM_THREADSAFE_UPDATEWINDOWSIZES,
															(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
															(LPARAM)0);

											::Sleep(0); // Switch to UI Thread
											
											break;

				default:					break;
			}
		}

		// Reset & set done
		m_pDoc->m_csLoadFullJpegDib.EnterCriticalSection(); // ENTER CS
		m_pDoc->m_bLoadFullJpegTransitionWorker = FALSE;
		m_pDoc->m_bLoadFullJpegTransitionWorkerDone = TRUE;
		m_pDoc->m_csLoadFullJpegDib.LeaveCriticalSection(); // LEAVE CS

		// Clean-Up
		if (hSrcMemDC)
		{
			if (hOldSrcBitmap)
				::SelectObject(hSrcMemDC, hOldSrcBitmap);
			::DeleteDC(hSrcMemDC);
		}
		::timeKillEvent(uiTimerId);
	}
	else
		m_pDoc->m_csLoadFullJpegDib.LeaveCriticalSection(); // LEAVE CS

	// Now Make Sure the Load Pictures Thread is not running
	// to safe CPU for the Loading, which is more important
	// than calculating the Compression!
	m_hEventArray[0] = GetKillEvent();
	m_hEventArray[1] = m_pDoc->m_LoadPicturesThread.GetHandle();
	Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE,
									JPEG_COMPRESSION_CALCULATION_THREADWAIT_TIMEOUT);
	switch (Event)
	{
		// Shutdown Event
		case WAIT_OBJECT_0 :		OnExit();
									return 0;

		// Load Pictures Thread Exited
		case WAIT_OBJECT_0 + 1 :	break;

		// Timeout
		case WAIT_TIMEOUT :			break;
		default:					break;
	}

	// Start Loading the File
	if (!Dib.LoadJPEG(m_sFileName, 1, FALSE, FALSE, NULL, TRUE, this))
	{
		OnExit();
		return 0;
	}

	// Init Value
	int nJpegCompressionQuality = INIT_JPEG_COMPRESSION_CALCULATION;
	DWORD dwTargetSize = Dib.GetFileSize();
	int nSaveJpegSize = 0;
	int nMaxLoops = 14;
	BOOL bInc = FALSE;
	BOOL bDec = FALSE;
	do
	{
		// Calculate the Saved Size
		if (!Dib.SaveJPEG(	NULL,
							&nSaveJpegSize,
							nJpegCompressionQuality,
							Dib.IsGrayscale(),
							m_sFileName,
							FALSE,
							FALSE,
							this))
		{
			OnExit();
			return 0;
		}

		// 1% Margin Allowed
		if (!bInc && (nSaveJpegSize > Round((double)dwTargetSize + 0.01 * (double)dwTargetSize)))
		{
			nJpegCompressionQuality -= 5;
			bDec = TRUE;
			if (nJpegCompressionQuality <= 0)
			{
				nJpegCompressionQuality = 0;
				break;
			}
		}
		else if (!bDec && (nSaveJpegSize < Round((double)dwTargetSize - 0.01 * (double)dwTargetSize)))
		{
			nJpegCompressionQuality += 5;
			bInc = TRUE;
			if (nJpegCompressionQuality >= 100)
			{
				nJpegCompressionQuality = 100;
				break;
			}
		}
		else
			break;
	}
	while (--nMaxLoops);

	// Do Exit?
	if (DoExit())
	{
		OnExit();
		return 0;
	}

	// Set Compression Quality
	::InterlockedExchange(&m_lJpegCompressionQuality, nJpegCompressionQuality);

	// Update Image Info Dlg
	::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
					WM_THREADSAFE_UPDATEIMAGEINFO,
					(WPARAM)TRUE, (LPARAM)0);

	// Clean-Up
	CleanUp();

	return 0;
}
 
BOOL CPictureDoc::CLoadPicturesThread::InitFileNames() 
{
	if (m_pDoc == NULL)
		return FALSE;

	CDib TempDib;
	CDib* pTempDib = &TempDib;
	pTempDib->SetShowMessageBoxOnError(m_pDoc->IsShowMessageBoxOnError());
	int pos, size;
	BOOL res;
	CString sName;

	// Load Next Picture?
	if (!m_pDoc->m_pNextDib)
	{
		pos = m_pDoc->m_FileFind.GetFilePosition();
		size = m_pDoc->m_FileFind.GetFilesCount();
		res = FALSE;
		while (	(--size > 0) &&
				!(res = m_pDoc->LoadPicture(&pTempDib,
						sName = m_pDoc->m_FileFind.GetNextFileName(pos),
						TRUE, FALSE, TRUE)));
		if (res)
		{
			if (((CUImagerApp*)::AfxGetApp())->IsPictureSizeBig(pTempDib->GetImageSize()))
				m_pDoc->m_sNextDibName = _T("");
			else
				m_pDoc->m_sNextDibName = sName;
		}
		else
			m_pDoc->m_sNextDibName = _T("");
	}

	// Load Previous Image?
	if (!m_pDoc->m_pPrevDib)
	{
		pos = m_pDoc->m_FileFind.GetFilePosition();
		size = m_pDoc->m_FileFind.GetFilesCount();
		res = FALSE;
		while (	(--size > 0) &&
				!(res = m_pDoc->LoadPicture(&pTempDib,
						sName = m_pDoc->m_FileFind.GetPreviousFileName(pos),
						TRUE, FALSE, TRUE)));
		if (res)
		{
			if (((CUImagerApp*)::AfxGetApp())->IsPictureSizeBig(pTempDib->GetImageSize()))
				m_pDoc->m_sPrevDibName = _T("");
			else
				m_pDoc->m_sPrevDibName = sName;
		}
		else
			m_pDoc->m_sPrevDibName = _T("");
	}

	return ((m_pDoc->m_sNextDibName != _T("")) ||
			(m_pDoc->m_sPrevDibName != _T("")));
}

int CPictureDoc::CLoadPicturesThread::Work() 
{
	ASSERT(m_pDoc);

	// Do Exit?
	if (DoExit())
		return 0;

	// Load Next Picture?
	if (!m_pDoc->m_pNextDib && m_pDoc->m_sNextDibName != _T(""))
	{
		m_pDoc->m_pNextDib = new CDib;
		m_pDoc->m_pNextDib->SetShowMessageBoxOnError(m_pDoc->IsShowMessageBoxOnError());
		if (!m_pDoc->LoadPicture(&m_pDoc->m_pNextDib,
								m_pDoc->m_sNextDibName,
								TRUE, FALSE, FALSE))
		{
			m_pDoc->m_sNextDibName = _T("");
			delete m_pDoc->m_pNextDib;
			m_pDoc->m_pNextDib = NULL;
		}
	}

	// Do Exit?
	if (DoExit())
		return 0;

	// Load Previous Picture?
	if (!m_pDoc->m_pPrevDib && m_pDoc->m_sPrevDibName != _T(""))
	{
		m_pDoc->m_pPrevDib = new CDib;
		m_pDoc->m_pPrevDib->SetShowMessageBoxOnError(m_pDoc->IsShowMessageBoxOnError());
		if (!m_pDoc->LoadPicture(&m_pDoc->m_pPrevDib,
								m_pDoc->m_sPrevDibName,
								TRUE, FALSE, FALSE))
		{
			m_pDoc->m_sPrevDibName = _T("");
			delete m_pDoc->m_pPrevDib;
			m_pDoc->m_pPrevDib = NULL;
		}
	}

	return 0;
}

void CPictureDoc::CMyGifAnimationThread::OnNewFrame()
{	
	ASSERT(m_pDoc);
	CRect rcc;
	m_pDoc->GetView()->GetClientRect(&rcc);
	m_pDoc->GetView()->InvalidateRect(rcc, FALSE);
}

void CPictureDoc::CMyGifAnimationThread::OnPlayTimesDone()
{	
	ASSERT(m_pDoc);
	m_pDoc->GetView()->PostMessage(WM_COMMAND, ID_STOP_ANIMATION);
	::Sleep(100); // Make sure we are paused here!
}

CPictureDoc::CTransitionThread::CTransitionThread()
{
	m_pDoc = NULL; 	
	m_hTimerEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventArray[0] = GetKillEvent();
	m_hEventArray[1] = m_hTimerEvent;
	m_bPseudoRandomInited = FALSE;
}
			
CPictureDoc::CTransitionThread::~CTransitionThread()
{
	Kill();
	::CloseHandle(m_hTimerEvent);
	m_hTimerEvent = NULL;
}

void CPictureDoc::CTransitionThread::OnExit() 
{
	// ENTER CS
	m_pDoc->m_csTransition.EnterCriticalSection();
	
	m_pDoc->m_bTransitionWorker = FALSE;

	if (m_pDoc->GetView()->m_hTransitionMemDC)
	{
		if (m_pDoc->GetView()->m_hTransitionOldBitmap)
		{
			::SelectObject(	m_pDoc->GetView()->m_hTransitionMemDC,
							m_pDoc->GetView()->m_hTransitionOldBitmap);
			m_pDoc->GetView()->m_hTransitionOldBitmap = NULL;
		}
		::DeleteDC(m_pDoc->GetView()->m_hTransitionMemDC);
		m_pDoc->GetView()->m_hTransitionMemDC = NULL;
	}

	// LEAVE CS
	m_pDoc->m_csTransition.LeaveCriticalSection();
	
	::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
					WM_THREADSAFE_UPDATEWINDOWSIZES,
					(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
					(LPARAM)0);
}

int CPictureDoc::CTransitionThread::Work() 
{
	ASSERT(m_pDoc);
	CDib* pDib =	(m_pDoc->m_pDib->HasAlpha() && m_pDoc->m_pDib->GetBitCount() == 32) ?
					&(m_pDoc->m_AlphaRenderedDib) :
					m_pDoc->m_pDib;

	// Init pseudo-random generators
	if (!m_bPseudoRandomInited)
	{
		std::random_device TrueRandom; // non-deterministic generator implemented as crypto-secure in Visual C++
		m_PseudoRandomTransitionType.seed(TrueRandom());
		m_PseudoRandomTransitionChessX.seed(TrueRandom());
		m_PseudoRandomTransitionChessY.seed(TrueRandom());
		m_bPseudoRandomInited = TRUE;
	}

	// Current Monitor Bpp
	int nCurrentMonitorBpp = ::AfxGetMainFrame()->GetMonitorBpp();

	// Working Bpp must be 24bpp or 32bpp,
	// otherwise the stretching in HALFTONE mode
	// of 1,4 or 8bpp is not working well!
	int nWorkingBpp;

	// Get DC
	CDC* pDC = m_pDoc->GetView()->GetDC();

	// Src Mem DC
	CDib SrcDib;
	HDC hSrcMemDC = NULL;
	HBITMAP hOldSrcBitmap = NULL;
	if (pDib)
	{
		if (pDib->GetPreviewDib() &&
			pDib->GetPreviewDib()->IsValid())
			SrcDib = *pDib->GetPreviewDib();
		else
			SrcDib = *pDib;
	}
	SrcDib.SetShowMessageBoxOnError(FALSE);
	if (nCurrentMonitorBpp == 32)
	{
		nWorkingBpp = 32;
		SrcDib.ConvertTo32bits();
	}
	else
	{
		nWorkingBpp = 24;
		SrcDib.ConvertTo24bits();
	}
	SrcDib.BitsToDibSection();
	if (SrcDib.StretchDibSection(	m_pDoc->GetView()->m_ZoomRect.Width(),
									m_pDoc->GetView()->m_ZoomRect.Height(),
									m_pDoc->m_bStretchModeHalftone ? HALFTONE : COLORONCOLOR))
	{
		hSrcMemDC = ::CreateCompatibleDC(pDC->GetSafeHdc());
		hOldSrcBitmap = (HBITMAP)::SelectObject(hSrcMemDC, SrcDib.GetSafeHandle());
	}

	// ENTER CS
	m_pDoc->m_csTransition.EnterCriticalSection();

	// Dst Mem DC
	ASSERT(m_pDoc->GetView()->m_hTransitionMemDC == NULL);
	ASSERT(m_pDoc->GetView()->m_hTransitionOldBitmap == NULL);
	m_pDoc->GetView()->m_TransitionDib.SetShowMessageBoxOnError(FALSE);
	if (m_pDoc->GetView()->m_TransitionDib.AllocateDibSection(	nWorkingBpp,
																m_pDoc->GetView()->m_ZoomRect.Width(),
																m_pDoc->GetView()->m_ZoomRect.Height()))
	{
		m_pDoc->GetView()->m_hTransitionMemDC = ::CreateCompatibleDC(pDC->GetSafeHdc());
		m_pDoc->GetView()->m_hTransitionOldBitmap =
					(HBITMAP)::SelectObject(m_pDoc->GetView()->m_hTransitionMemDC,
											m_pDoc->GetView()->m_TransitionDib.GetSafeHandle());
		::BitBlt(	m_pDoc->GetView()->m_hTransitionMemDC,
					0,
					0,
					m_pDoc->GetView()->m_ZoomRect.Width(),
					m_pDoc->GetView()->m_ZoomRect.Height(),		
					pDC->GetSafeHdc(),
					m_pDoc->GetView()->m_ZoomRect.left,
					m_pDoc->GetView()->m_ZoomRect.top,
					SRCCOPY);
	}

	// Set Var
	m_pDoc->m_bTransitionWorker = TRUE;

	// LEAVE CS
	m_pDoc->m_csTransition.LeaveCriticalSection();

	// Release DC
	m_pDoc->GetView()->ReleaseDC(pDC);

	// Flush the GDI batch queue 
	::GdiFlush();

	// Init Vars
	m_pDoc->m_bNoDrawing = FALSE;
	m_pDoc->GetView()->m_nTransitionStep = 0;
	if (m_pDoc->m_nTransitionType == 1) // Random Transition
	{
		// Transitions:
		// 0: off
		// 1: random
		// 2: chessboard
		// 3: curtain-in
		// 4: roll-in
		// 5: slide-in
		// 6: blend
		std::uniform_int_distribution<int> Distribution(2, 6); // distribute results: [2, 6]
		m_pDoc->GetView()->m_nCurrentTransition = Distribution(m_PseudoRandomTransitionType);
	}
	else
		m_pDoc->GetView()->m_nCurrentTransition = m_pDoc->m_nTransitionType;

	// Set Timer				
	UINT uiTimerId = ::timeSetEvent(TRANSITION_DELAY,
									DEFAULT_TIMER_RESOLUTION,
									(LPTIMECALLBACK)m_hTimerEvent, 0,
									TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);

	// Send the Draw Messages each TRANSITION_DELAY Milliseconds
	while (	m_pDoc->m_bTransitionUI &&
			m_pDoc->m_bTransitionWorker)
	{
		DWORD Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE,
							(uiTimerId == 0) ? TRANSITION_DELAY : INFINITE);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		if (uiTimerId)
											::timeKillEvent(uiTimerId);
										if (hSrcMemDC)
										{
											if (hOldSrcBitmap)
												::SelectObject(hSrcMemDC, hOldSrcBitmap);
											::DeleteDC(hSrcMemDC);
										}
										OnExit();
										return 0;

			// Timer Event
			case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hTimerEvent);

			// Timeout
			case WAIT_TIMEOUT :			m_pDoc->m_csTransition.EnterCriticalSection(); // ENTER CS

										switch (m_pDoc->GetView()->m_nCurrentTransition)
										{
											case 2 :
												m_pDoc->m_bTransitionWorker =
													CPictureDoc::TransitionChess(	hSrcMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_hTransitionMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_ZoomRect.Width(),
																					m_pDoc->GetView()->m_ZoomRect.Height(),
																					m_pDoc->GetView()->m_nTransitionStep++,
																					32,
																					m_PseudoRandomTransitionChessX,
																					m_PseudoRandomTransitionChessY);
												break;

											case 3 :
												m_pDoc->m_bTransitionWorker =
													CPictureDoc::TransitionLRCurtain(hSrcMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_hTransitionMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_ZoomRect.Width(),
																					m_pDoc->GetView()->m_ZoomRect.Height(),
																					m_pDoc->GetView()->m_nTransitionStep++,
																					24);
												break;

											case 4 :
												m_pDoc->m_bTransitionWorker =
													CPictureDoc::TransitionLRRoll(	hSrcMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_hTransitionMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_ZoomRect.Width(),
																					m_pDoc->GetView()->m_ZoomRect.Height(),
																					m_pDoc->GetView()->m_nTransitionStep++,
																					24);
												break;

											case 5 :
												m_pDoc->m_bTransitionWorker =
													CPictureDoc::TransitionLRSlide(	hSrcMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_hTransitionMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_ZoomRect.Width(),
																					m_pDoc->GetView()->m_ZoomRect.Height(),
																					m_pDoc->GetView()->m_nTransitionStep++,
																					24);
												break;

											case 6 :
												m_pDoc->m_bTransitionWorker =
													m_pDoc->TransitionBlend(		hSrcMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_hTransitionMemDC,
																					0,
																					0,
																					m_pDoc->GetView()->m_ZoomRect.Width(),
																					m_pDoc->GetView()->m_ZoomRect.Height(),
																					m_pDoc->GetView()->m_nTransitionStep++);
												break;

											default :
												m_pDoc->m_bTransitionWorker = FALSE;
												break;
										}

										// Flush the GDI batch queue 
										::GdiFlush();

										m_pDoc->m_csTransition.LeaveCriticalSection(); // LEAVE CS

										// Note: The Last PostMessage will happen inside OnExit()
										if (m_pDoc->m_bTransitionWorker)
										{
											::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
															WM_THREADSAFE_UPDATEWINDOWSIZES,
															(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
															(LPARAM)0);
										}

										::Sleep(0); // Switch to UI Thread
										
										break;

			default:					break;
		}
	}

	// Clean-Up
	if (uiTimerId)
		::timeKillEvent(uiTimerId);
	if (hSrcMemDC)
	{
		if (hOldSrcBitmap)
			::SelectObject(hSrcMemDC, hOldSrcBitmap);
		::DeleteDC(hSrcMemDC);
	}
	OnExit();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPictureDoc Functions

CPictureDoc::CPictureDoc()
{
	// The Only View and Frame
	m_pView = NULL;
	m_pFrame = NULL;

	// Do Not Draw Flag
	m_bNoDrawing = FALSE;

	// Current Directory Name
	m_sDirName = _T("");

	// Current Copy or Move Directory Name
	m_sCopyOrMoveDirName = _T("");

	// Previous / Next Dibs
	m_pNextDib = NULL;
	m_pPrevDib = NULL;
	m_sNextDibName = _T("");
	m_sPrevDibName = _T("");

	// Load Full Jpeg Dib
	m_pLoadFullJpegDib = NULL;

	// Current Loaded Picture
	m_sFileName = _T("");

	// Strech Mode Flag
	m_bStretchModeHalftone = FALSE;

	// Reset Metadata Modified Flag
	m_bMetadataModified = FALSE;

	// Init current displayed page number to 0
	m_nPageNum = 0;

	// Printing flag
	m_bPrinting = FALSE;

	// Print Preview Flag
	m_bPrintPreviewMode = FALSE;

	// Init Threads
	m_SlideShowThread.SetDoc(this);
	m_JpegThread.SetDoc(this);
	m_LoadPicturesThread.SetDoc(this);
	m_LayeredDlgThread.SetDoc(this);
	m_TransitionThread.SetDoc(this);
	m_GifAnimationThread.SetDoc(this);
	
	// Undo / Redo
	m_nDibUndoPos = -1;
	m_bRedo = FALSE;

	// Transition
	m_bTransitionUI = FALSE;
	m_bTransitionWorker = FALSE;
	m_csTransition.EnableTimeout();

	// Load Full Jpeg Transition
	m_bLoadFullJpegTransitionUI = FALSE;
	m_bLoadFullJpegTransitionWorker = FALSE;
	m_bLoadFullJpegTransitionWorkerDone = FALSE;
	m_bFirstLoadFullJpegTransition = FALSE;
	m_bCancelLoadFullJpegTransition = FALSE;
	m_bCancelLoadFullJpegTransitionAllowed = FALSE;
	m_dwIDAfterFullLoadCommand = 0;
	m_csLoadFullJpegDib.EnableTimeout();

	// Show error boxes on dib failures,
	// reset by CSlideShowThread::SlideShow()
	m_bShowMessageBoxOnError = TRUE;

	// The Image Has no Defined Background Color
	m_bImageBackgroundColor = FALSE;
	m_crImageBackgroundColor = RGB(0,0,0);

	// Get Pixel Align, Compression and load full sized jpeg
	m_bDoJPEGGet = TRUE;				// Slideshow disables this

	// Restart Slideshow
	m_bDoRestartSlideshow = FALSE;

	// Dialogs
	m_bEnableOsd = TRUE;
	m_bOSDWasEnabled = FALSE;
	m_pOsdDlg = NULL;
	m_pImageInfoDlg = NULL;
	m_pHLSDlg = NULL;
	m_pRotationFlippingDlg = NULL;
	m_bDoRotationColorPickup = FALSE;
	m_pRedEyeDlg = NULL;
	m_bDoRedEyeColorPickup = FALSE;
	m_pMonochromeConversionDlg = NULL;
	m_pSharpenDlg = NULL;
	m_pSoftenDlg = NULL;
	m_pSoftBordersDlg = NULL;
	m_pWndPalette = NULL;
	m_pLayeredDlg = NULL;

	// Layered Dialog attributes
	m_bFirstLayeredDlgUpdate = TRUE;
	m_nLayeredDlgWidth = 0;
	m_nLayeredDlgHeight = 0;
	m_bDoUpdateLayeredDlg = TRUE;
	m_nLayeredDlgMaxsizePercent = MIN_LAYERED_DLG_MAXSIZE_PERCENT;
	m_nLayeredDlgSizePerthousand = 0;
	m_nLayeredDlgOrigin = 0;
	m_nLayeredDlgOpacity = MAX_LAYERED_DLG_OPACITY;

	// Alpha-Blend Transition
	m_hMSIMG32 = ::LoadLibraryFromSystem32(_T("msimg32.dll"));
	if (m_hMSIMG32)
		m_fpAlphaBlend = (FPALPHABLEND)::GetProcAddress(m_hMSIMG32, "AlphaBlend");
	else
		m_fpAlphaBlend = NULL;

	// Window Transparency
	m_hUser32 = ::LoadLibraryFromSystem32(_T("user32.dll"));
	if (m_hUser32)
	{
		m_pSetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)
										::GetProcAddress(m_hUser32, 
										"SetLayeredWindowAttributes");
		m_pUpdateLayeredWindow = (lpfnUpdateLayeredWindow)
										::GetProcAddress(m_hUser32, 
										"UpdateLayeredWindow");
	}
	else
	{
		m_pSetLayeredWindowAttributes = NULL;
		m_pUpdateLayeredWindow = NULL;
	}

	// Crop Vars
	m_nPixelAlignX = 1;
	m_nPixelAlignY = 1;
	m_CropDocRect = CRect(0,0,0,0);
	m_bCrop = FALSE;
	m_rcCropDelta = CRect(0,0,0,0);
	m_rcCropCenter = CRect(0,0,0,0);
	m_pCropBkgDib = NULL;
	m_bLosslessCrop = FALSE;

	// Print
	m_dPrintScale = 1.0;
	m_bPrintSizeFit = TRUE;
	m_bPrintMargin = TRUE;
	m_ptPrintOffset = CPoint(0, 0);
	m_ptLastPrintOffset = CPoint(0, 0);

	// Zoom ComboBox Index
	m_nZoomComboBoxIndex = 0; // Fit

	// Show Borders
	m_bNoBorders = FALSE;

	// Zoom Tool Flags
	m_bZoomTool = FALSE;
	m_bZoomToolMinus = FALSE;

	// Allowed Extensions
	m_FileFind.AddAllowedExtension(_T("jpg"));
	m_FileFind.AddAllowedExtension(_T("gif"));
	m_FileFind.AddAllowedExtension(_T("bmp"));
	m_FileFind.AddAllowedExtension(_T("png"));
	m_FileFind.AddAllowedExtension(_T("tif"));
	m_FileFind.AddAllowedExtension(_T("pcx"));
	m_FileFind.AddAllowedExtension(_T("emf"));
	m_FileFind.AddAllowedExtension(_T("jpeg"));
	m_FileFind.AddAllowedExtension(_T("tiff"));
	m_FileFind.AddAllowedExtension(_T("jpe"));
	m_FileFind.AddAllowedExtension(_T("jfif"));
	m_FileFind.AddAllowedExtension(_T("thm"));
	m_FileFind.AddAllowedExtension(_T("jfx"));
	m_FileFind.AddAllowedExtension(_T("dib"));
	m_FileFind.AddAllowedNumericExtensions();

	// Load the Settings
	LoadSettings();
}

CPictureDoc::~CPictureDoc()
{
	// Delete Previous / Next Dibs
	ClearPrevNextPictures();

	// Clear Undo Array
	ClearUndoArray();

	// Delete Crop Dib
	if (m_pCropBkgDib)
	{
		delete m_pCropBkgDib;
		m_pCropBkgDib = NULL;
	}

	// Delete Load Full Jpeg Dib
	if (m_pLoadFullJpegDib)
	{
		delete m_pLoadFullJpegDib;
		m_pLoadFullJpegDib = NULL;
	}

	// Free Library used for Alpha-Blend
	if (m_hMSIMG32)
	{
		::FreeLibrary(m_hMSIMG32);
		m_hMSIMG32 = NULL;
	}

	// Free Library used for Window Transparency
	if (m_hUser32)
	{
		::FreeLibrary(m_hUser32);
		m_hUser32 = NULL;
	}
}

void CPictureDoc::CloseDocument()
{
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

void CPictureDoc::CloseDocumentForce()
{
	SetModifiedFlag(FALSE);
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

BOOL CPictureDoc::IsDibReadyForCommand(DWORD dwCommand)
{
	// Cancel Transitions
	CancelTransition();
	CancelLoadFullJpegTransition();

	// Dib Not Fully Loaded?
	if (!m_pDib || !m_pDib->IsValid())
	{
		if (m_dwIDAfterFullLoadCommand == 0)
		{
			GetView()->ForceCursor();
			BeginWaitCursor();
			m_dwIDAfterFullLoadCommand = dwCommand;
		}
		::PostMessage(	GetView()->GetSafeHwnd(),
						WM_THREADSAFE_UPDATEWINDOWSIZES,
						(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
						(LPARAM)0);
		return FALSE;
	}
	else
		return TRUE;
}

BOOL CPictureDoc::DoEnableCommand()
{
	return(	m_pDib &&
			!(m_SlideShowThread.IsSlideshowRunning() ||
			m_bDoRestartSlideshow)								&&
			!m_GifAnimationThread.IsAlive()						&&
			!m_bMetadataModified								&&
			!m_pRotationFlippingDlg								&&
			!m_pWndPalette										&&
			!m_pHLSDlg											&&
			!m_pRedEyeDlg										&&
			!m_bDoRedEyeColorPickup								&&
			!m_pMonochromeConversionDlg							&&
			!m_pSharpenDlg										&&
			!m_pSoftenDlg										&&
			!m_pSoftBordersDlg									&&
			!m_bCrop);
}

void CPictureDoc::SetDocumentTitle()
{
	CString sName;
	CString strInfo;

	if (m_sFileName.IsEmpty())
	{
		sName = ML_STRING(1260, "Picture");
		m_sTabTitle = sName;
	}
	else
	{
		sName = m_sFileName;
		m_sTabTitle = ::GetShortFileName(m_sFileName);
	}

	if (m_pDib && m_pDib->GetBMI())
	{
		if (m_sFileName == _T(""))
		{
			strInfo.Format(	_T(" , %dx%d %s , %s"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							m_pDib->GetCompressionName(),
							::FormatBytes(m_pDib->GetImageSize()));
		}
		else
		{
			if (m_GifAnimationThread.IsAlive() &&
				!m_GifAnimationThread.IsRunning() &&
				m_GifAnimationThread.m_dwDibAnimationCount > 1)
			{
				strInfo.Format(	_T(" , %i/%i {%i/%i} , %dx%d %s , %s"),
								m_FileFind.GetFilePosition() + 1,
								m_FileFind.GetFilesCount(),
								m_GifAnimationThread.m_dwDibAnimationPos + 1,
								m_GifAnimationThread.m_dwDibAnimationCount,
								m_pDib->GetWidth(), 
								m_pDib->GetHeight(),
								m_pDib->GetCompressionName(), 
								::FormatBytes(m_pDib->GetFileSize()));
			}
			else if (IsMultiPageTIFF())
			{
				strInfo.Format(	_T(" , %i/%i {%i/%i} , %dx%d %s , %s"),
								m_FileFind.GetFilePosition() + 1,
								m_FileFind.GetFilesCount(),
								m_pDib->m_FileInfo.m_nImagePos + 1,
								m_pDib->m_FileInfo.m_nImageCount,
								m_pDib->GetWidth(), 
								m_pDib->GetHeight(),
								m_pDib->GetCompressionName(), 
								::FormatBytes(m_pDib->GetFileSize()));
			}
			else
			{
				strInfo.Format(	_T(" , %i/%i , %dx%d %s , %s"),
								m_FileFind.GetFilePosition() + 1,
								m_FileFind.GetFilesCount(),
								m_pDib->GetWidth(), 
								m_pDib->GetHeight(),
								m_pDib->GetCompressionName(), 
								::FormatBytes(m_pDib->GetFileSize()));
			}
		}
	}

#ifdef _DEBUG
	if (m_pDib && m_pDib->IsValid())
	{
		if (m_pDib->HasDibSection() && m_pDib->HasBits())
			strInfo += _T(" , DibSection + Bits");
		else if (m_pDib->HasDibSection())
			strInfo += _T(" , DibSection");
		else if (m_pDib->HasBits())
			strInfo += _T(" , Bits");
	}

	if (m_pDib && m_pDib->GetPreviewDib() && m_pDib->GetPreviewDib()->IsValid())
	{
		if (m_pDib->GetPreviewDib()->HasDibSection() &&
			m_pDib->GetPreviewDib()->HasBits())
			strInfo += _T(" , Preview DibSection + Bits");
		else if (m_pDib->GetPreviewDib()->HasDibSection())
			strInfo += _T(" , Preview DibSection");
		else if (m_pDib->GetPreviewDib()->HasBits())
			strInfo += _T(" , Preview Bits");
		if (m_pDib->GetPreviewDibRatio() > 0.0)
		{
			CString sRatio;
			sRatio.Format(_T(" %0.3fX"), 1.0 / m_pDib->GetPreviewDibRatio());
			strInfo += sRatio;
		}
	}

	if (m_pDib && m_pDib->IsCompressed())
		strInfo += _T(" , Compressed");
#endif

	if (IsModified())
		strInfo += _T(" *");

	// The maximum mainframe title size is 159 characters
	// (that's given by the Win32 native MDI logic, not MFC)
	// Title format for a maximized MDI child: sAppName - [SetTitle]
	CString sAppName;
	sAppName.LoadString(IDR_MAINFRAME);
	int nMaxName = 159 - sAppName.GetLength() - strInfo.GetLength() - 5; // 5 = 2x spaces [] -
	if (sName.GetLength() > nMaxName)
	{
		sName = sName.Right(nMaxName - 3); // safety: Right() with a negative parameter returns an empty string
		sName = _T("...") + sName;
	}

	CDocument::SetTitle(sName + strInfo);
}

void CPictureDoc::OnFileExtract() 
{
	if (IsMultiPageTIFF())
	{
		// Display the Save As Dialog
		CString sFirstFileName;
		TCHAR FileName[MAX_PATH];
		_tcscpy(FileName, m_sFileName);
		CNoVistaFileDlg dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY, NULL, GetView());
		dlgFile.m_ofn.lpstrFile = FileName;
		dlgFile.m_ofn.nMaxFile = MAX_PATH;
		dlgFile.m_ofn.lpstrDefExt = _T("tif");
		dlgFile.m_ofn.lpstrFilter = _T("TIFF Sequence (*.tif;*.tiff;*.jfx)\0*.tif;*.tiff;*.jfx\0");
		dlgFile.m_ofn.nFilterIndex = 1;
		GetView()->ForceCursor();
		if (dlgFile.DoModal() == IDOK)
		{
			// Extract
			BeginWaitCursor();
			sFirstFileName = CDib::TIFFExtractPages(FileName, m_sFileName, GetView(), TRUE);
			EndWaitCursor();

			// Load first extracted picture
			if (sFirstFileName != _T(""))
				::AfxGetApp()->OpenDocumentFile(sFirstFileName);
			else
				::AfxMessageBox(ML_STRING(1252, "Could Not Save The Picture."), MB_OK | MB_ICONSTOP);
		}
		GetView()->ForceCursor(FALSE);
	}
	else if (m_GifAnimationThread.IsAlive() &&
			m_GifAnimationThread.m_dwDibAnimationCount > 1)
	{
		// Display the Save As Dialog
		CString sFirstFileName;
		TCHAR FileName[MAX_PATH];
		_tcscpy(FileName, ::GetFileNameNoExt(m_sFileName) + _T(".bmp"));
		CNoVistaFileDlg dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY, NULL, GetView());
		dlgFile.m_ofn.lpstrFile = FileName;
		dlgFile.m_ofn.nMaxFile = MAX_PATH;
		dlgFile.m_ofn.lpstrDefExt = _T("bmp");
		dlgFile.m_ofn.lpstrFilter = _T("BMP Sequence (*.bmp)\0*.bmp\0");
		dlgFile.m_ofn.nFilterIndex = 1;
		GetView()->ForceCursor();
		if (dlgFile.DoModal() == IDOK)
		{
			// Extract
			BeginWaitCursor();
			sFirstFileName = ExtractFromAnimGIFToBMP(FileName);
			EndWaitCursor();

			// Load first extracted picture
			if (sFirstFileName != _T(""))
				::AfxGetApp()->OpenDocumentFile(sFirstFileName);
			else
				::AfxMessageBox(ML_STRING(1252, "Could Not Save The Picture."), MB_OK | MB_ICONSTOP);
		}
		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::OnUpdateFileExtract(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(IsMultiPageTIFF() ||
					(m_GifAnimationThread.IsAlive() &&
					m_GifAnimationThread.m_dwDibAnimationCount > 1))	&&
					m_pDib												&&
					m_dwIDAfterFullLoadCommand == 0						&&
					!IsModified()										&&
					!m_bMetadataModified								&&
					!(m_SlideShowThread.IsSlideshowRunning()			||
					m_bDoRestartSlideshow)								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop											&&
					!m_bPrintPreviewMode);
}

BOOL CPictureDoc::SaveAs(BOOL bSaveCopyAs,
						 CString sDlgTitle/*=_T("")*/) 
{
	// Wait and schedule command if dib not fully loaded!
	if (!IsDibReadyForCommand(bSaveCopyAs ? ID_FILE_SAVE_COPY_AS : ID_FILE_SAVE_AS))
		return FALSE;

	// Special Saving for Animated GIFs
	if (m_GifAnimationThread.IsAlive() &&
		m_GifAnimationThread.m_dwDibAnimationCount > 1)
		return SaveAsFromAnimGIF(bSaveCopyAs, sDlgTitle);

	// Force Cursor
	GetView()->ForceCursor();

	// Init Save As Dialog
	TCHAR FileName[MAX_PATH] = _T("");
	BOOL bNewFile = FALSE;
	CString sLastNewFileSaveAsExt(DEFAULT_SAVEAS_EXT);
	if (m_sFileName == _T(""))
	{
		sLastNewFileSaveAsExt = ::AfxGetApp()->GetProfileString(_T("PictureDoc"), _T("LastNewFileSaveAsExt"), DEFAULT_SAVEAS_EXT);
		_tcscpy(FileName, ML_STRING(1260, "Picture") + sLastNewFileSaveAsExt);
		bNewFile = TRUE;
	}
	else
		_tcscpy(FileName, m_sFileName);
	CSaveFileDlg dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, GetView());
	CString defextension(::GetFileExt(FileName));
	CString defextension_nodot(defextension);
	defextension_nodot.TrimLeft(_T('.'));
	dlgFile.m_ofn.lpstrFile = FileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrDefExt = defextension_nodot.GetBuffer();
	dlgFile.m_ofn.lpstrFilter = _T("Windows Bitmap (*.bmp)\0*.bmp\0")
								_T("Graphics Interchange Format (*.gif)\0*.gif\0")
								_T("Portable Network Graphics (*.png)\0*.png\0")
								_T("JPEG File Interchange Format (*.jpg)\0*.jpg\0")
								_T("Tag Image File Format (*.tif)\0*.tif\0")
								_T("PC Paintbrush (*.pcx)\0*.pcx\0")
								_T("Enhanced Metafile (*.emf)\0*.emf\0");
	if (sDlgTitle != _T(""))
		dlgFile.m_ofn.lpstrTitle = sDlgTitle;
	if ((defextension == _T(".bmp")) || (defextension == _T(".dib")))
	{
		dlgFile.m_ofn.nFilterIndex = 1;
	}
	else if (defextension == _T(".gif"))
	{
		dlgFile.m_ofn.nFilterIndex = 2;
	}
	else if (defextension == _T(".png"))
	{
		dlgFile.m_ofn.nFilterIndex = 3;
	}
	else if (CDib::IsJPEGExt(defextension))
	{
		dlgFile.m_ofn.nFilterIndex = 4;
	}
	else if (CDib::IsTIFFExt(defextension))
	{
		dlgFile.m_ofn.nFilterIndex = 5;
	}
	else if (defextension == _T(".pcx"))
	{
		dlgFile.m_ofn.nFilterIndex = 6;
	}
	else if (defextension == _T(".emf"))
	{
		dlgFile.m_ofn.nFilterIndex = 7;
	}

	// Show Save As Dialog
	if (dlgFile.DoModal() == IDOK)
	{
		// Store default extension for new files
		CString extension(::GetFileExt(FileName));
		if (bNewFile)
			::AfxGetApp()->WriteProfileString(_T("PictureDoc"), _T("LastNewFileSaveAsExt"), extension);

		// Save to itself?
		if (m_sFileName.CompareNoCase(FileName) == 0 && bSaveCopyAs)
		{
			::AfxMessageBox(ML_STRING(1249, "Cannot save a copy to ourselves"), MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Check whether the file is read only
		if (::IsReadonly(FileName))
		{
			CString str;
			str.Format(ML_STRING(1250, "The file %s\nis read only"), FileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Check whether we have write access to the file
		if (!::HasWriteAccess(FileName))
		{
			CString str;
			str.Format(ML_STRING(1251, "Access denied to %s"), FileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		BOOL res = FALSE;
		CDib Dib;
		if ((extension == _T(".bmp")) || (extension == _T(".dib")))
		{
			BeginWaitCursor();
			if (m_pDib->GetBitCount() == 8 || m_pDib->GetBitCount() == 4)
			{
				// RLE Encode?
				if (m_pDib->m_FileInfo.m_nType == CDib::CFileInfo::BMP	&&
					(m_pDib->m_FileInfo.m_nCompression == BI_RLE4		||
					m_pDib->m_FileInfo.m_nCompression == BI_RLE8))
				{
					// Compress
					Dib = *m_pDib;
					Dib.Compress((m_pDib->GetBitCount() == 4)
									? BI_RLE4 : BI_RLE8);

					// Save
					res = Dib.SaveBMP(	FileName,
										GetView(),
										TRUE);
				}
				else
				{
					res = m_pDib->SaveBMP(	FileName,
											GetView(),
											TRUE);
				}
			}
			// Store Alpha using the V4 Header
			else if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
			{
				Dib = *m_pDib;
				Dib.BMIToBITMAPV4HEADER();
				res = Dib.SaveBMP(	FileName,
									GetView(),
									TRUE);
			}
			else
			{
				res = m_pDib->SaveBMP(	FileName,
										GetView(),
										TRUE);
			}
			EndWaitCursor();
		}
		else if (extension == _T(".gif"))
		{
			BeginWaitCursor();
			if (m_bImageBackgroundColor)
				m_pDib->SetBackgroundColor(m_crImageBackgroundColor);
			else
				m_pDib->SetBackgroundColor(m_crBackgroundColor);
			res = SaveGIF(	FileName,
							m_pDib,
							GetView(),
							TRUE);
			EndWaitCursor();
		}
		else if (extension == _T(".png"))
		{
			BeginWaitCursor();
			if (m_bImageBackgroundColor)
				m_pDib->SetBackgroundColor(m_crImageBackgroundColor);
			else
				m_pDib->SetBackgroundColor(m_crBackgroundColor);
			BOOL bStoreBackgroundColor = m_pDib->m_FileInfo.m_bHasBackgroundColor ||
										(m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32);
			res = SavePNG(	FileName,
							m_pDib,
							bStoreBackgroundColor,
							GetView(),
							TRUE);
			EndWaitCursor();
		}
		else if (CDib::IsJPEGExt(extension))
		{
			BeginWaitCursor();
			if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
			{
				Dib = *m_pDib;
				Dib.RenderAlphaWithSrcBackground();
				Dib.SetAlpha(FALSE);
				if (!CDib::IsJPEGExt(defextension))
				{
					// Clear Orientation
					Dib.GetExifInfo()->Orientation = 1;
					res = Dib.SaveJPEG(	FileName,
										dlgFile.GetJpegCompressionQuality(),
										m_pDib->IsGrayscale(),
										_T(""),
										TRUE,
										FALSE,
										GetView(),
										TRUE);
				}
				else
				{
					// In case we are saving to ourself stop calculating compression quality
					if (m_sFileName.CompareNoCase(FileName) == 0)
						m_JpegThread.Kill();
					res = Dib.SaveJPEG(	FileName,
										dlgFile.GetJpegCompressionQuality(),
										m_pDib->IsGrayscale(),
										m_sFileName,
										FALSE,
										TRUE,
										GetView(),
										TRUE);
					// Clear Orientation,
					// this because orientation is copied from m_sFileName!
					if (res)
					{
						CDib::JPEGSetOrientationInplace(FileName,
														1,
														FALSE);
					}
				}
			}
			else
			{
				if (!CDib::IsJPEGExt(defextension))
				{
					// Clear Orientation
					int nOrientation = m_pDib->GetExifInfo()->Orientation;
					m_pDib->GetExifInfo()->Orientation = 1;
					res = m_pDib->SaveJPEG(	FileName,
											dlgFile.GetJpegCompressionQuality(),
											m_pDib->IsGrayscale(),
											_T(""),
											TRUE,
											FALSE,
											GetView(),
											TRUE);
					m_pDib->GetExifInfo()->Orientation = nOrientation;
				}
				else
				{
					// In case we are saving to ourself stop calculating compression quality
					if (m_sFileName.CompareNoCase(FileName) == 0)
						m_JpegThread.Kill();
					res = m_pDib->SaveJPEG(	FileName,
											dlgFile.GetJpegCompressionQuality(),
											m_pDib->IsGrayscale(),
											m_sFileName,
											FALSE,
											TRUE,
											GetView(),
											TRUE);
					// Clear Orientation,
					// this because orientation is copied from m_sFileName!
					if (res)
					{
						CDib::JPEGSetOrientationInplace(FileName,
														1,
														FALSE);
					}
				}
			}
			EndWaitCursor();
		}
		else if (CDib::IsTIFFExt(extension))
		{
			BeginWaitCursor();
			int nTiffCompression;
			if (CDib::IsTIFFExt(defextension))
				nTiffCompression = m_pDib->m_FileInfo.m_nCompression;
			else
			{
				if (m_pDib->GetBitCount() == 1)
					nTiffCompression = COMPRESSION_CCITTFAX4;
				else
					nTiffCompression = COMPRESSION_LZW;
			}
			int nOrientation = m_pDib->GetExifInfo()->Orientation;
			m_pDib->GetExifInfo()->Orientation = 1;
			if (IsMultiPageTIFF())
			{
				res = m_pDib->SaveMultiPageTIFF(FileName,
												m_sFileName,
												((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
												nTiffCompression,
												dlgFile.GetJpegCompressionQuality(),
												GetView(),
												TRUE);
			}
			else
			{
				res = m_pDib->SaveTIFF(	FileName,
										nTiffCompression,
										dlgFile.GetJpegCompressionQuality(),
										GetView(),
										TRUE);
			}
			m_pDib->GetExifInfo()->Orientation = nOrientation;
			EndWaitCursor();
		}
		else if (extension == _T(".pcx"))
		{
			BeginWaitCursor();
			if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
			{
				Dib = *m_pDib;
				Dib.RenderAlphaWithSrcBackground();
				Dib.SetAlpha(FALSE);
				res = Dib.SavePCX(	FileName,
									GetView(),
									TRUE);
			}
			else
				res = m_pDib->SavePCX(	FileName,
										GetView(),
										TRUE);
			EndWaitCursor();
		}
		else if (extension == _T(".emf"))
		{
			// Unfortunately with the given windows APIs we cannot choose the wanted dpi,
			// only a reference dc with a given dpi can be chosen.
			// We can choose between the printer dc which usually has 300 dpi or 600 dpi and
			// the display dc that has 96 dpi (or 72 dpi)
			BeginWaitCursor();
			int index = ((CUImagerApp*)::AfxGetApp())->GetCurrentPrinterIndex();
			HDC hPrinterDC = NULL;
			if (index >= 0							&&
				m_pDib->GetXDpi() > DEFAULT_DPI		&&
				m_pDib->GetYDpi() > DEFAULT_DPI)
			{
				CString sPrinterName = ((CUImagerApp*)::AfxGetApp())->m_PrinterControl.GetPrinterName(index);
				hPrinterDC = ::CreateDC(NULL, sPrinterName, NULL, NULL);
			}
			if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
			{
				Dib = *m_pDib;
				Dib.RenderAlphaWithSrcBackground();
				Dib.SetAlpha(FALSE);
				res = Dib.SaveEMF(FileName, hPrinterDC);
			}
			else
				res = m_pDib->SaveEMF(FileName, hPrinterDC);
			if (hPrinterDC)
				::DeleteDC(hPrinterDC);
			EndWaitCursor();
		}

		// Load
		if (res)
		{
			if (bSaveCopyAs)
				::AfxGetApp()->OpenDocumentFile(FileName);
			else
			{
				ClearPrevNextPictures();
				SetModifiedFlag(FALSE);
				if (LoadPicture(&m_pDib, FileName))
					SlideShow(); // No Recursive Slideshow in Paused State (also if it was Recursive before...)
			}
		}
		else
		{
			::DeleteFile(FileName);
			::AfxMessageBox(ML_STRING(1252, "Could Not Save The Picture."), MB_OK | MB_ICONSTOP);
		}

		GetView()->ForceCursor(FALSE);

		return res;
	}
	else
	{
		GetView()->ForceCursor(FALSE);
		return FALSE;
	}
}

BOOL CPictureDoc::SaveGIF(	const CString& sFileName,
							CDib* pDib,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/) 
{
	// Check
	if (!pDib)
		return FALSE;

	// Count the Unique Colors
	DWORD dwMaxColors = pDib->CountUniqueColors(pProgressWnd,
												bProgressSend);

	// Dither?
	BOOL bDitherColorConversion = FALSE;
	if (dwMaxColors > 256)
		bDitherColorConversion = TRUE;

	// Clip Max Colors
	if (dwMaxColors > 256)
		dwMaxColors = 256;

	// Convert to 8 bpp
	if (pDib->GetBitCount() > 8)
	{
		CDib Dib = *pDib;

		if (pDib->HasAlpha() && pDib->GetBitCount() == 32)
		{
			// Inc. by one because of Background
			if (dwMaxColors < 256)
				dwMaxColors++;

			// Convert
			CQuantizer Quantizer(dwMaxColors - 1, 8);
			Quantizer.ProcessImage(&Dib, pProgressWnd, bProgressSend);
			RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[dwMaxColors];
			Quantizer.SetColorTable(&pColors[1]);
			pColors[0] = pColors[1]; // Duplicate the first color which will be used as background!
			Dib.CreatePaletteFromColors(dwMaxColors, pColors);
			if (bDitherColorConversion)
				Dib.ConvertTo8bitsErrDiff(Dib.GetPalette(), pProgressWnd, bProgressSend);
			else
				Dib.ConvertTo8bitsPrecise(Dib.GetPalette(), pProgressWnd, bProgressSend);
			delete [] pColors;

			// Set Background
			DWORD uiDIBScanLineSize8 = DWALIGNEDWIDTHBYTES(Dib.GetWidth() * 8);
			DWORD uiDIBScanLineSize32 = DWALIGNEDWIDTHBYTES(pDib->GetWidth() * 32);
			LPBYTE lpBits = Dib.GetBits();
			LPBYTE lpAlphaBits = pDib->GetBits();
			for (unsigned int y = 0 ; y < pDib->GetHeight() ; y++)
			{
				// Do Exit?
				if (pThread && (y & 0x7) == 0)
				{
					if (pThread->DoExit())
						return FALSE;
				}

				for (unsigned int x = 0 ; x < pDib->GetWidth() ; x++)
				{
					// Index 0 changed to index 1 because index 0
					// will be used as background color
					// (Index 0 and 1 have been set to the same color,
					// so that this exchange is not a problem!))
					if (lpBits[x + y*uiDIBScanLineSize8] == 0)
						lpBits[x + y*uiDIBScanLineSize8] = 1;
					
					// If Alpha is less than 128 set the corresponding
					// pixel index to the background index 0
					BYTE A = lpAlphaBits[4*x + 3 + y*uiDIBScanLineSize32];
					if (A < 128)
						lpBits[x + y*uiDIBScanLineSize8] = 0;
				}
			}

			// First Color is the Background Color
			Dib.GetGif()->SetBackgroundColorIndex(0);
			Dib.GetColors()->rgbBlue = GetBValue(pDib->GetBackgroundColor());
			Dib.GetColors()->rgbGreen = GetGValue(pDib->GetBackgroundColor());
			Dib.GetColors()->rgbRed = GetRValue(pDib->GetBackgroundColor());
			Dib.CreatePaletteFromColors(dwMaxColors, Dib.GetColors());

			// Save
			return Dib.SaveGIF(	sFileName,
								0, // Transparency Color Index 0
								pProgressWnd,
								bProgressSend,
								pThread);
		}
		else
		{
			// Convert
			CQuantizer Quantizer(dwMaxColors, 8);
			Quantizer.ProcessImage(&Dib, pProgressWnd, bProgressSend);
			RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[dwMaxColors];
			Quantizer.SetColorTable(pColors);
			Dib.CreatePaletteFromColors(dwMaxColors, pColors);
			if (bDitherColorConversion)
				Dib.ConvertTo8bitsErrDiff(Dib.GetPalette(), pProgressWnd, bProgressSend);
			else
				Dib.ConvertTo8bits(Dib.GetPalette(), pProgressWnd, bProgressSend);
			delete [] pColors;

			// Set the Background Color Index
			Dib.GetGif()->SetBackgroundColorIndex(
					Dib.GetPalette()->GetNearestPaletteIndex(pDib->GetBackgroundColor()));

			// Save
			return Dib.SaveGIF(	sFileName,
								GIF_COLORINDEX_NOT_DEFINED,
								pProgressWnd,
								bProgressSend,
								pThread);
		}
	}
	else
	{
		// Set the Background Color Index
		pDib->GetGif()->SetBackgroundColorIndex(
				pDib->GetPalette()->GetNearestPaletteIndex(pDib->GetBackgroundColor()));

		// Save
		return pDib->SaveGIF(sFileName,
							GIF_COLORINDEX_NOT_DEFINED,
							pProgressWnd,
							bProgressSend,
							pThread);
	}
}

BOOL CPictureDoc::SavePNG(	const CString& sFileName,
							CDib* pDib,
							BOOL bStoreBackgroundColor,
							CWnd* pProgressWnd/*=NULL*/,
							BOOL bProgressSend/*=TRUE*/,
							CWorkerThread* pThread/*=NULL*/) 
{
	// Check
	if (!pDib)
		return FALSE;

	// If Alpha
	if (pDib->HasAlpha() && pDib->GetBitCount() == 32)
	{
		// Count the Unique Colors
		DWORD dwMaxColors = pDib->CountUniqueColors(pProgressWnd,
													bProgressSend);

		// Init Transparency Flag
		BOOL bUseTransparency = FALSE;
		if (dwMaxColors < 256)							// Max. of 255 colors because index 0
			bUseTransparency = pDib->IsBilevelAlpha();	// would be used for transparency!

		// If Alpha is only composed by 0s and 255s
		if (bUseTransparency)
		{
			CDib Dib = *pDib;

			// Inc. by one because of Transparency
			dwMaxColors++;

			// Convert
			CQuantizer Quantizer(dwMaxColors - 1, 8);
			Quantizer.ProcessImage(&Dib, pProgressWnd, bProgressSend);
			RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[dwMaxColors];
			Quantizer.SetColorTable(&pColors[1]);
			pColors[0] = pColors[1]; // Duplicate the first color which will be used as background!
			Dib.CreatePaletteFromColors(dwMaxColors, pColors);
			Dib.ConvertTo8bitsPrecise(Dib.GetPalette(), pProgressWnd, bProgressSend);
			delete [] pColors;

			// Set Transparency index
			DWORD uiDIBScanLineSize8 = DWALIGNEDWIDTHBYTES(Dib.GetWidth() * 8);
			DWORD uiDIBScanLineSize32 = DWALIGNEDWIDTHBYTES(pDib->GetWidth() * 32);
			LPBYTE lpBits = Dib.GetBits();
			LPBYTE lpAlphaBits = pDib->GetBits();
			for (unsigned int y = 0 ; y < pDib->GetHeight() ; y++)
			{
				// Do Exit?
				if (pThread && (y & 0x7) == 0)
				{
					if (pThread->DoExit())
						return FALSE;
				}

				for (unsigned int x = 0 ; x < pDib->GetWidth() ; x++)
				{
					// Index 0 changed to index 1 because index 0
					// will be used as background color
					// (Index 0 and 1 have been set to the same color,
					// so that this exchange is not a problem!))
					if (lpBits[x + y*uiDIBScanLineSize8] == 0)
						lpBits[x + y*uiDIBScanLineSize8] = 1;
					
					// If Alpha is less than 128 set the corresponding
					// pixel index to the background index 0
					BYTE A = lpAlphaBits[4*x + 3 + y*uiDIBScanLineSize32];
					if (A < 128)
						lpBits[x + y*uiDIBScanLineSize8] = 0;
				}
			}

			// First Color is the Background Color
			Dib.GetColors()->rgbBlue = GetBValue(pDib->GetBackgroundColor());
			Dib.GetColors()->rgbGreen = GetGValue(pDib->GetBackgroundColor());
			Dib.GetColors()->rgbRed = GetRValue(pDib->GetBackgroundColor());
			Dib.CreatePaletteFromColors(dwMaxColors, Dib.GetColors());

			// Save
			return Dib.SavePNG(	sFileName,
								TRUE, // Palette Transparency of Index 0
								bStoreBackgroundColor,
								pProgressWnd,
								bProgressSend,
								pThread);
		}
	}
	
	// Save
	return pDib->SavePNG(	sFileName,
							FALSE,
							bStoreBackgroundColor,
							pProgressWnd,
							bProgressSend,
							pThread);
}

BOOL CPictureDoc::SaveAsFromAnimGIF(BOOL bSaveCopyAs,
									CString sDlgTitle/*=_T("")*/) 
{
	// Display the Save As Gif Dialog
	TCHAR FileName[MAX_PATH] = _T("");
	_tcscpy(FileName, m_sFileName);
	CSaveFileDlg dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, GetView());
	dlgFile.m_ofn.lpstrFile = FileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrDefExt = _T("gif");
	dlgFile.m_ofn.lpstrFilter = _T("Animated GIF (*.gif)\0*.gif\0");
	dlgFile.m_ofn.nFilterIndex = 1;
	if (sDlgTitle != _T(""))
		dlgFile.m_ofn.lpstrTitle = sDlgTitle;

	// Force Cursor
	GetView()->ForceCursor();

	if (dlgFile.DoModal() == IDOK)
	{
		// Save copy to same file not allowed!
		if (m_sFileName.CompareNoCase(FileName) == 0 &&
			bSaveCopyAs)
		{
			::AfxMessageBox(ML_STRING(1249, "Cannot save a copy to ourselves"), MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Check whether the file is read only
		if (::IsReadonly(FileName))
		{
			CString str;
			str.Format(ML_STRING(1250, "The file %s\nis read only"), FileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Check whether we have write access to the file
		if (!::HasWriteAccess(FileName))
		{
			CString str;
			str.Format(ML_STRING(1251, "Access denied to %s"), FileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Save
		BOOL res = FALSE;
		CAnimGifSaveSmallDlg AnimGifSaveSmallDlg(GetView());
		int nPlayTimes = m_pDib->GetGif()->GetPlayTimes();
		if (nPlayTimes == 0)
			AnimGifSaveSmallDlg.m_nLoopInfinite = 1;
		else
		{
			AnimGifSaveSmallDlg.m_nLoopInfinite = 0;
			AnimGifSaveSmallDlg.m_uiPlayTimes = nPlayTimes;
		}
		CArray<int, int> DelaysArray;
		for (unsigned int i = 0 ; i < m_GifAnimationThread.m_dwDibAnimationCount ; i++)
		{
			CDib* pDib = m_GifAnimationThread.m_DibAnimationArray.GetAt(i);
			if (pDib)
				DelaysArray.Add(pDib->GetGif()->GetDelay());
		}
		AnimGifSaveSmallDlg.m_pDelaysArray = &DelaysArray;
		if (AnimGifSaveSmallDlg.DoModal() == IDOK)
		{	
			BeginWaitCursor();
			res = SaveAsFromAnimGIFToAnimGIF(	FileName,
												(AnimGifSaveSmallDlg.m_nLoopInfinite == 1) ?
												0 : AnimGifSaveSmallDlg.m_uiPlayTimes,
												&DelaysArray);
			EndWaitCursor();
		}
		else
		{
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Load
		if (res)
		{
			if (bSaveCopyAs)
				::AfxGetApp()->OpenDocumentFile(FileName);
			else
			{
				ClearPrevNextPictures();
				SetModifiedFlag(FALSE);
				if (LoadPicture(&m_pDib, FileName))
					SlideShow(); // No Recursive Slideshow in Paused State (also if it was Recursive before...)
			}
		}
		else
		{
			::DeleteFile(FileName);
			::AfxMessageBox(ML_STRING(1254, "Could Not Save The Animated Gif."), MB_OK | MB_ICONSTOP);
		}

		GetView()->ForceCursor(FALSE);

		return res;
	}
	else
	{
		GetView()->ForceCursor(FALSE);
		return FALSE;
	}
}

// uiPlayTimes = 0: Infinite
// uiPlayTimes = 1: Show All Frames One Time
// uiPlayTimes = 2: Show All Frames Two Times
// ...
BOOL CPictureDoc::SaveAsFromAnimGIFToAnimGIF(	const CString& sFileName,
												UINT uiPlayTimes,
												CArray<int, int>* pDelaysArray)
{
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);
	CString sDstFileName = sFileName;

	// Save to itself?
	if (m_sFileName.CompareNoCase(sFileName) == 0)
	{
		// Temporary File
		sDstFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sFileName);
	}

	// Init Progress
	DIB_INIT_PROGRESS;
	
	// Load First GIF
	if (!Dib.LoadFirstGIFRaw(m_sFileName))
		return FALSE;
	else
	{
		// Set up GIF Vars
		Dib.GetGif()->SetConfig(Dib.GetGif()->m_bHasScreenColorTable,	// Screen Color Table
								Dib.GetGif()->m_bHasImageColorTable,	// Image Color Table?
								Dib.GetGif()->m_bHasGraphicExtension,	// Graphic Extension
								(uiPlayTimes == 1) ?	FALSE :	// No Loop Count -> Show all frames 1x
														TRUE);	// See GIFSetLoopCount()

		// Set Background Color Index
		if (Dib.GetPalette())
		{
			if (m_bImageBackgroundColor)
			{
				if (m_crImageBackgroundColor != Dib.m_FileInfo.m_crBackgroundColor)
					Dib.GetGif()->SetBackgroundColorIndex(Dib.GetPalette()->
														GetNearestPaletteIndex(m_crImageBackgroundColor));
			}
			else 
			{
				if (m_crBackgroundColor != Dib.m_FileInfo.m_crBackgroundColor)
					Dib.GetGif()->SetBackgroundColorIndex(Dib.GetPalette()->
														GetNearestPaletteIndex(m_crBackgroundColor));
			}
		}

		// 0 means loop infinite
		// 1 means loop one time -> Show all frames 2x
		// 2 means loop two times -> Show all frames 3x
		if (uiPlayTimes != 1)
			Dib.GetGif()->SetLoopCount((uiPlayTimes == 0) ? 0 : uiPlayTimes - 1);
		
		// Set Delay
		if (pDelaysArray)
			Dib.GetGif()->SetDelay(pDelaysArray->GetAt(0));

		// Save First GIF
		if (!Dib.SaveFirstGIF(sDstFileName))
			return FALSE;
	}

	// Load Next GIFs
	int i = 1;
	while (Dib.LoadNextGIFRaw() > 0)
	{
		// Progress
		DIB_PROGRESS(GetView()->GetSafeHwnd(), TRUE, i, m_GifAnimationThread.m_dwDibAnimationCount);

		// Set Delay
		if (pDelaysArray)
			Dib.GetGif()->SetDelay(pDelaysArray->GetAt(i));

		// Save Next GIF
		if (!Dib.SaveNextGIF())
		{
			DIB_END_PROGRESS(GetView()->GetSafeHwnd());
			return FALSE;
		}

		// Inc.
		i++;
	}

	// Close
	Dib.GetGif()->LoadClose();
	Dib.GetGif()->SaveClose();

	// End Progress
	DIB_END_PROGRESS(GetView()->GetSafeHwnd());

	// Save to itself?
	if (m_sFileName.CompareNoCase(sFileName) == 0)
	{
		try
		{
			CFile::Remove(sFileName);
			CFile::Rename(sDstFileName, sFileName);
		}
		catch (CFileException* e)
		{
			::DeleteFile(sDstFileName);

			DWORD dwAttrib = ::GetFileAttributes(sFileName);
			if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
			{
				CString str(ML_STRING(1255, "The file is read only\n"));
				TRACE(str);
				::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			}
			else
				::ShowErrorMsg(e->m_lOsError, TRUE);
			
			e->Delete();
			return FALSE;
		}
	}

	return TRUE;
}

CString CPictureDoc::ExtractFromAnimGIFToBMP(const CString& sFileName)
{
	// Number of Digits for File Names
	int nDigits = (int)log10((double)m_GifAnimationThread.m_dwDibAnimationCount) + 1;

	// Save BMP Files
	DIB_INIT_PROGRESS;
	CDib* pDib = NULL;
	CString sFirstFileName;
	BOOL bFirst = TRUE;
	for (unsigned int i = 0 ; i < m_GifAnimationThread.m_dwDibAnimationCount ; i++)
	{
		if (pDib = m_GifAnimationThread.m_DibAnimationArray.GetAt(i))
		{
			// Progress
			DIB_PROGRESS(GetView()->GetSafeHwnd(), TRUE, i, m_GifAnimationThread.m_dwDibAnimationCount);

			// Save BMP
			CString sFormat;
			CString sCurrentFileName;
			sFormat.Format(_T("%%0%du"), nDigits);
			sCurrentFileName.Format(_T("%s") + sFormat + _T("%s"),
									::GetFileNameNoExt(sFileName),
									i + 1,
									::GetFileExt(sFileName));
			int iCopy = 0;
			while (::IsExistingFile(sCurrentFileName))
			{
				sCurrentFileName.Format(_T("%s") + sFormat + _T("(%d)%s"),
									::GetFileNameNoExt(sFileName),
									i + 1,
									++iCopy,
									::GetFileExt(sFileName));
			}
			if (pDib->HasAlpha() && pDib->GetBitCount() == 32)
			{
				CDib Dib = *pDib;
				Dib.SetShowMessageBoxOnError(FALSE);
				Dib.BMIToBITMAPV4HEADER();
				if (!Dib.SaveBMP(sCurrentFileName))
				{
					DIB_END_PROGRESS(GetView()->GetSafeHwnd());
					return _T("");
				}
			}
			else
			{
				if (!pDib->SaveBMP(sCurrentFileName))
				{
					DIB_END_PROGRESS(GetView()->GetSafeHwnd());
					return _T("");
				}
			}

			// Set First File Name
			if (bFirst)
			{
				sFirstFileName = sCurrentFileName;
				bFirst = FALSE;
			}					
		}
		else
		{
			DIB_END_PROGRESS(GetView()->GetSafeHwnd());
			return _T("");
		}
	}

	DIB_END_PROGRESS(GetView()->GetSafeHwnd());

	// OK
	return sFirstFileName;
}

BOOL CPictureDoc::Save() 
{
	if (m_sFileName == _T(""))
		return SaveAs(FALSE);
	else
	{
		BOOL res = FALSE;
		CDib Dib;

		// Force Cursor
		GetView()->ForceCursor();

		// Check whether the file is read only
		if (::IsReadonly(m_sFileName))
		{
			CString str;
			str.Format(ML_STRING(1256, "The file %s\nis read only.\nUse Save As with a different file name!"), m_sFileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Check whether we have write access to the file
		if (!::HasWriteAccess(m_sFileName))
		{
			CString str;
			str.Format(ML_STRING(1257, "Access denied to %s.\nUse Save As with a different file name!"), m_sFileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		CString extension(::GetFileExt(m_sFileName));
		if ((extension == _T(".bmp")) || (extension == _T(".dib")))
		{
			BeginWaitCursor();
			if (m_pDib->GetBitCount() == 8 || m_pDib->GetBitCount() == 4)
			{
				// RLE Encode?
				if (m_pDib->m_FileInfo.m_nCompression == BI_RLE4 ||
					m_pDib->m_FileInfo.m_nCompression == BI_RLE8)
				{
					// Compress
					Dib = *m_pDib;
					Dib.Compress((m_pDib->GetBitCount() == 4)
									? BI_RLE4 : BI_RLE8);

					// Save
					res = Dib.SaveBMP(	m_sFileName,
										GetView(),
										TRUE);
				}
				else
				{
					res = m_pDib->SaveBMP(	m_sFileName,
											GetView(),
											TRUE);
				}
			}
			// Store Alpha using the V4 Header
			else if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
			{
				Dib = *m_pDib;
				Dib.BMIToBITMAPV4HEADER();
				res = Dib.SaveBMP(	m_sFileName,
									GetView(),
									TRUE);
			}
			else
			{
				res = m_pDib->SaveBMP(	m_sFileName,
										GetView(),
										TRUE);
			}
			EndWaitCursor();
		}
		else if (extension == _T(".gif"))
		{
			BeginWaitCursor();
			if (m_bImageBackgroundColor)
				m_pDib->SetBackgroundColor(m_crImageBackgroundColor);
			else
				m_pDib->SetBackgroundColor(m_crBackgroundColor);
			res = SaveGIF(	m_sFileName,
							m_pDib,
							GetView(),
							TRUE);
			EndWaitCursor();
		}
		else if (extension == _T(".png"))
		{
			BeginWaitCursor();
			if (m_bImageBackgroundColor)
				m_pDib->SetBackgroundColor(m_crImageBackgroundColor);
			else
				m_pDib->SetBackgroundColor(m_crBackgroundColor);
			res = SavePNG(	m_sFileName,
							m_pDib,
							m_pDib->m_FileInfo.m_bHasBackgroundColor,
							GetView(),
							TRUE);
			EndWaitCursor();
		}
		else if (CDib::IsJPEGExt(extension))
		{
			BeginWaitCursor();
			res = m_pDib->SaveJPEG(	m_sFileName,
									m_JpegThread.GetJpegCompressionQualityBlocking(),
									m_pDib->IsGrayscale(),
									m_sFileName,
									FALSE,
									TRUE,
									GetView(),
									TRUE);
			// Clear Orientation,
			// this because orientation is copied from m_sFileName!
			if (res)
			{
				CDib::JPEGSetOrientationInplace(m_sFileName,
												1,
												FALSE);
			}
			EndWaitCursor();
		}
		else if (CDib::IsTIFFExt(extension))
		{
			BeginWaitCursor();
			int nOrientation = m_pDib->GetExifInfo()->Orientation;
			m_pDib->GetExifInfo()->Orientation = 1;
			if (IsMultiPageTIFF())
			{
				res = m_pDib->SaveMultiPageTIFF(m_sFileName,
												m_sFileName,
												((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
												m_pDib->m_FileInfo.m_nCompression,
												DEFAULT_JPEGCOMPRESSION,
												GetView(),
												TRUE);
			}
			else
			{
				res = m_pDib->SaveTIFF(	m_sFileName,
										m_pDib->m_FileInfo.m_nCompression,
										DEFAULT_JPEGCOMPRESSION,
										GetView(),
										TRUE);
			}
			m_pDib->GetExifInfo()->Orientation = nOrientation;
			EndWaitCursor();
		}
		else if (extension == _T(".pcx"))
		{
			BeginWaitCursor();
			if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
			{
				Dib = *m_pDib;
				Dib.RenderAlphaWithSrcBackground();
				Dib.SetAlpha(FALSE);
				res = Dib.SavePCX(	m_sFileName,
									GetView(),
									TRUE);
			}
			else
				res = m_pDib->SavePCX(	m_sFileName,
										GetView(),
										TRUE);
			EndWaitCursor();
		}
		else if (extension == _T(".emf"))
		{
			// Unfortunately with the given windows APIs we cannot choose the wanted dpi,
			// only a reference dc with a given dpi can be chosen.
			// We can choose between the printer dc which usually has 300 dpi or 600 dpi and
			// the display dc that has 96 dpi (or 72 dpi)
			BeginWaitCursor();
			int index = ((CUImagerApp*)::AfxGetApp())->GetCurrentPrinterIndex();
			HDC hPrinterDC = NULL;
			if (index >= 0							&&
				m_pDib->GetXDpi() > DEFAULT_DPI		&&
				m_pDib->GetYDpi() > DEFAULT_DPI)
			{
				CString sPrinterName = ((CUImagerApp*)::AfxGetApp())->m_PrinterControl.GetPrinterName(index);
				hPrinterDC = ::CreateDC(NULL, sPrinterName, NULL, NULL);
			}
			if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
			{
				Dib = *m_pDib;
				Dib.RenderAlphaWithSrcBackground();
				Dib.SetAlpha(FALSE);
				res = Dib.SaveEMF(m_sFileName, hPrinterDC);
			}
			else
				res = m_pDib->SaveEMF(m_sFileName, hPrinterDC);
			if (hPrinterDC)
				::DeleteDC(hPrinterDC);
			EndWaitCursor();
		}

		// Load
		if (res)
		{
			SetModifiedFlag(FALSE);
			LoadPicture(&m_pDib, m_sFileName);
		}
		else
			::AfxMessageBox(ML_STRING(1252, "Could Not Save The Picture."), MB_OK | MB_ICONSTOP);

		GetView()->ForceCursor(FALSE);

		return res;
	}
}

void CPictureDoc::OnFileCopyTo()
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocAvailable(this, TRUE))
	{
		GetView()->ForceCursor();

		// Copy To Dialog
		TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
		InitDir[0] = _T('\0');
		if (::IsExistingDir(m_sCopyOrMoveDirName))
			_tcscpy(InitDir, (LPCTSTR)m_sCopyOrMoveDirName);
		CString sExt = ::GetFileExt(m_sFileName);
		sExt.TrimLeft(_T('.'));
		CNoVistaFileDlg fd(	FALSE,
							sExt,
							::GetShortFileName(m_sFileName),
							OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT);
		fd.m_ofn.lpstrInitialDir = InitDir;
		CString sDlgTitle(ML_STRING(1258, "Copy To"));
		fd.m_ofn.lpstrTitle = sDlgTitle;
		if (fd.DoModal() == IDOK)
		{
			// Store directory for next call
			m_sCopyOrMoveDirName = ::GetDriveAndDirName(fd.GetPathName());
			::AfxGetApp()->WriteProfileString(	_T("PictureDoc"),
												_T("CopyOrMoveDirName"),
												m_sCopyOrMoveDirName);

			// Copy
			if (!::CopyFile(m_sFileName, fd.GetPathName(), FALSE))
				::ShowErrorMsg(::GetLastError(), TRUE);
		}

		// Free
		delete [] InitDir;

		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::OnFileMoveTo()
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocAvailable(this, TRUE))
	{
		GetView()->ForceCursor();

		// Be Sure We Are Not Working On This File
		m_JpegThread.Kill();
		m_GifAnimationThread.Kill();

		// Move To Dialog
		TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
		InitDir[0] = _T('\0');
		if (::IsExistingDir(m_sCopyOrMoveDirName))
			_tcscpy(InitDir, (LPCTSTR)m_sCopyOrMoveDirName);
		CString sExt = ::GetFileExt(m_sFileName);
		sExt.TrimLeft(_T('.'));
		CNoVistaFileDlg fd(	FALSE,
							sExt,
							::GetShortFileName(m_sFileName),
							OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT);
		fd.m_ofn.lpstrInitialDir = InitDir;
		CString sDlgTitle(ML_STRING(1259, "Move To"));
		fd.m_ofn.lpstrTitle = sDlgTitle;
		if (fd.DoModal() == IDOK)
		{
			// Store directory for next call
			m_sCopyOrMoveDirName = ::GetDriveAndDirName(fd.GetPathName());
			::AfxGetApp()->WriteProfileString(	_T("PictureDoc"),
												_T("CopyOrMoveDirName"),
												m_sCopyOrMoveDirName);

			// Move
			::DeleteFile(fd.GetPathName());
			if (::MoveFile(m_sFileName, fd.GetPathName()))
			{
				ClearPrevNextPictures();
				m_SlideShowThread.NextPicture();
			}
			else
				::ShowErrorMsg(::GetLastError(), TRUE);
		}

		// Free
		delete [] InitDir;

		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::OnFileReload() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocAvailable(this, TRUE))
	{
		GetView()->ForceCursor();
		LoadPicture(&m_pDib, m_sFileName);
		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::OnFileSave() 
{
	Save();
}

BOOL CPictureDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// Note:
	// OnSaveDocument() should never be called because I changed
	// the default implementation of OnFileSaveAs(),
	// OnFileSave(), OnFileSendMail() and SaveModified()
	//
	// - OnFileSaveAs() and OnFileSendMail() both call DoSave()
	// - OnFileSave() and SaveModified() both call DoFileSave()
	// - DoFileSave() calls DoSave() 
	// - DoSave() calls OnSaveDocument()

	ASSERT(FALSE);

	return FALSE;
}

BOOL CPictureDoc::SaveModified() 
{
	// If printing
	if (m_bPrinting)
		return FALSE;	// don't continue

	if (!IsModified() && !m_bMetadataModified)
		return TRUE;	// ok to continue

	// Get active view and force cursor
	CUImagerView* pActiveView = NULL;
	if (::AfxGetMainFrame()->m_bFullScreenMode)
	{
		CMDIChildWnd* pChild = ::AfxGetMainFrame()->MDIGetActive();
		if (pChild)
		{
			pActiveView = (CUImagerView*)pChild->GetActiveView();
			if (pActiveView && pActiveView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
				pActiveView->ForceCursor();
			else
				pActiveView = NULL;
		}
	}

	// Prompt for Metadata Save
	if (m_bMetadataModified && m_pImageInfoDlg)
	{
		if (!m_pImageInfoDlg->SaveModified())
		{
			if (pActiveView)
				pActiveView->ForceCursor(FALSE);
			return FALSE;	// don't continue
		}
	}

	// Prompt for Image Data Save
	if (IsModified())
	{
		CString Str;
		if (m_sFileName == _T(""))
		{
			if (m_pDib && m_pDib->GetBMI())
			{
				Str.Format(	_T("%s %dx%d %dbpp"),
							ML_STRING(1260, "Picture"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							m_pDib->GetBitCount());
			}
			else
				Str = ML_STRING(1260, "Picture");
		}
		else
			Str = m_sFileName;

		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, Str);
		switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
		{
			case IDCANCEL:
			{
				if (pActiveView)
					pActiveView->ForceCursor(FALSE);
				return FALSE;       // don't continue
			}

			case IDYES:
				// If so, either Save or Update, as appropriate
				if (!Save())
				{
					if (pActiveView)
						pActiveView->ForceCursor(FALSE);
					return FALSE;   // don't continue
				}
				break;

			case IDNO:
				// If not saving changes
				break;

			default:
				ASSERT(FALSE);
				break;
		}
	}

	if (pActiveView)
		pActiveView->ForceCursor(FALSE);

	return TRUE;    // keep going
}

void CPictureDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	IsModified()										&&
					!(m_SlideShowThread.IsSlideshowRunning()			||
					m_bDoRestartSlideshow)								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop											&&
					!m_bPrintPreviewMode);	
}

void CPictureDoc::OnFileSaveAs()
{
	SaveAs(FALSE);
}

void CPictureDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pDib												&&
					(m_dwIDAfterFullLoadCommand == 0					||
					m_dwIDAfterFullLoadCommand == ID_FILE_SAVE_AS)		&&
					!(m_SlideShowThread.IsSlideshowRunning()			||
					m_bDoRestartSlideshow)								&&
					!m_bMetadataModified								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop											&&
					!m_bPrintPreviewMode);
}

void CPictureDoc::OnFileSaveCopyAs() 
{
	SaveAs(TRUE, ML_STRING(1262, "Save Copy As"));
}

void CPictureDoc::OnUpdateFileSaveCopyAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pDib												&&
					(m_dwIDAfterFullLoadCommand == 0					||
					m_dwIDAfterFullLoadCommand == ID_FILE_SAVE_COPY_AS)	&&
					!(m_SlideShowThread.IsSlideshowRunning()			||
					m_bDoRestartSlideshow)								&&
					!m_bMetadataModified								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop											&&
					!m_bPrintPreviewMode);
}

void CPictureDoc::OnFileSaveAsPdf() 
{
	SaveAsPdf();
}

void CPictureDoc::OnUpdateFileSaveAsPdf(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pDib												&&
					(m_dwIDAfterFullLoadCommand == 0					||
					m_dwIDAfterFullLoadCommand == ID_FILE_SAVE_AS_PDF)	&&
					!(m_SlideShowThread.IsSlideshowRunning()			||
					m_bDoRestartSlideshow)								&&
					!m_bMetadataModified								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop											&&
					!m_bPrintPreviewMode);
}

BOOL CPictureDoc::SaveAsPdf()
{
	BOOL res = FALSE;

	// Wait and schedule command if dib not fully loaded!
	if (!IsDibReadyForCommand(ID_FILE_SAVE_AS_PDF))
		return FALSE;

	// Force Cursor
	GetView()->ForceCursor();

	// Check
	if (IsModified() && IsMultiPageTIFF())
	{
		::AfxMessageBox(ML_STRING(1180, "Try again after saving the picture file."), MB_OK | MB_ICONINFORMATION);
		GetView()->ForceCursor(FALSE);
		return FALSE;
	}

	// Display the Save As Pdf Dialog
	CSaveFileDlg dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, GetView());
	TCHAR FileName[MAX_PATH] = _T("");
	if (m_sFileName == _T(""))
		_tcscpy(FileName, ML_STRING(1260, "Picture") + _T(".pdf"));
	else
		_tcscpy(FileName, ::GetFileNameNoExt(m_sFileName) + _T(".pdf"));
	dlgFile.m_ofn.lpstrFile = FileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrCustomFilter = NULL;
	CString sDlgTitle(ML_STRING(1263, "Save As Pdf"));
	dlgFile.m_ofn.lpstrTitle = sDlgTitle;
	dlgFile.m_ofn.lpstrFilter = _T("Pdf File (*.pdf)\0*.pdf\0");
	dlgFile.m_ofn.lpstrDefExt = _T("pdf");
	if (dlgFile.DoModal() == IDOK)
	{
		// Check whether the file is read only
		if (::IsReadonly(FileName))
		{
			CString str;
			str.Format(ML_STRING(1250, "The file %s\nis read only"), FileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Check whether we have write access to the file
		if (!::HasWriteAccess(FileName))
		{
			CString str;
			str.Format(ML_STRING(1251, "Access denied to %s"), FileName);
			::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		// Begin Wait Cursor
		BeginWaitCursor();
		
		// Save as tiff
		CString sTiffFileName = ::MakeTempFileName(	((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
													::GetFileNameNoExt(FileName) + _T(".tif"));
		if (IsMultiPageTIFF())
		{
			TIFF* TiffDst = TIFFOpenW(sTiffFileName, "w");   
			if (TiffDst)
			{
				res = CDib::TIFFCopyAllPages(	m_sFileName,					// Source file name
												TiffDst,						// Destination file
												0,								// Start page number
												m_pDib->m_FileInfo.m_nImageCount,// Total pages count
												-1,								// Work on all pages
												COMPRESSION_JPEG,				// Use Jpeg compression
												dlgFile.GetJpegCompressionQuality(), // Jpeg compression quality
												TRUE,							// Flatten all pages with a alpha channel
												RGB(255,255,255),				// White background for PDFs is ok
												TRUE,	// Limit to 1,2,4 or 8 Bits per Sample because PDF is not supporting others!
												TRUE);	// YCbCr Jpegs inside Tiff are not supported by Tiff2Pdf -> re-encode
				::TIFFClose(TiffDst);
			}
		}
		else
		{
			// The Animation has a separate array of dibs, sync the document's
			// one with the current one of the animation array
			if (m_GifAnimationThread.IsAlive())
			{
				::EnterCriticalSection(&m_csDib);
				*m_pDib = *(m_GifAnimationThread.m_DibAnimationArray.GetAt
									(m_GifAnimationThread.m_dwDibAnimationPos));
				::LeaveCriticalSection(&m_csDib);
				if (m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetSize() > 0)
					m_AlphaRenderedDib = *(m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetAt
										(m_GifAnimationThread.m_dwDibAnimationPos));
				else
					UpdateAlphaRenderedDib();
			}

			// Flatten
			CDib Dib(*m_pDib);
			if (Dib.HasAlpha() && Dib.GetBitCount() == 32)
			{
				Dib.SetBackgroundColor(RGB(255,255,255)); // White background for PDFs is ok
				Dib.RenderAlphaWithSrcBackground();
				Dib.SetAlpha(FALSE);
			}
			
			// Save
			int nOrientation = Dib.GetExifInfo()->Orientation;
			Dib.GetExifInfo()->Orientation = 1;
			res = Dib.SaveTIFF(	sTiffFileName,
								COMPRESSION_JPEG,
								dlgFile.GetJpegCompressionQuality(),
								GetView(),
								TRUE);
			Dib.GetExifInfo()->Orientation = nOrientation;
		}

		// To pdf
		if (res)
		{
			// Compression conversion from tiff to pdf
			//
			// Note: Jpegs in tiff are re-encoded to avoid all the troubles
			//       with nonstandard compliant pdf viewers!
			//
			// COMPRESSION_CCITTRLE, COMPRESSION_CCITTFAX3, COMPRESSION_CCITTFAX4
			//   -> T2P_COMPRESS_G4
			// COMPRESSION_OJPEG, COMPRESSION_JPEG
			//   -> T2P_COMPRESS_JPEG
			// All Other
			//   -> T2P_COMPRESS_ZIP
			res = ::Tiff2Pdf(	sTiffFileName,	// Tiff
								FileName,		// Pdf
								_T("Fit"),
								TRUE,			// Fit Window
								TRUE,			// Interpolate
								dlgFile.GetJpegCompressionQuality());
		}

		// Delete temp tiff
		::DeleteFile(sTiffFileName);

		// End Wait Cursor
		EndWaitCursor();

		// Show message
		if (res)
			::AfxGetMainFrame()->PopupNotificationWnd(CString(APPNAME_NOEXT) + _T(" ") + ML_STRING(1849, "Saved"), FileName);
		else
		{
			::DeleteFile(FileName);
			::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, ML_STRING(1850, "Save Failed!"), 0);
		}
	}

	GetView()->ForceCursor(FALSE);
	
	return res;
}

void CPictureDoc::OnEditDelete() 
{
	EditDelete(TRUE);
}

void CPictureDoc::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_dwIDAfterFullLoadCommand == 0						&&
					!IsModified()										&&
					!m_bMetadataModified								&&
					!(m_SlideShowThread.IsSlideshowRunning()			||
					m_bDoRestartSlideshow)								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop											&&
					!m_bPrintPreviewMode);
}

void CPictureDoc::EditDelete(BOOL bPrompt)
{
	if (IsMultiPageTIFF())
	{
		CDeletePageDlg dlg(GetView());
		GetView()->ForceCursor();
		int nRes = dlg.DoModal();
		if (nRes == IDOK)
		{
			// Delete Current Page
			if (!CDib::TIFFDeletePage(	m_nPageNum,	
										m_sFileName,
										((CUImagerApp*)::AfxGetApp())->GetAppTempDir()))
			{
				CString str;
				str.Format(ML_STRING(1265, "Cannot delete page %d"), m_nPageNum + 1);
				::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
				GetView()->ForceCursor(FALSE);
				return;
			}
			else
				GetView()->ForceCursor(FALSE);

			// Load Next Page
			if (m_nPageNum == m_pDib->m_FileInfo.m_nImageCount - 1) // If last has been delete dec. by one
				m_nPageNum--;
			LoadPicture(&m_pDib, m_sFileName);
		}
		else if (nRes == IDCANCEL)
			GetView()->ForceCursor(FALSE);
		else // IDNO
		{
			GetView()->ForceCursor(FALSE);
			DeleteDocFile();
		}
	}
	else
	{
		// Prompt for Deleting
		if (bPrompt)
		{
			CString sMsg;
			sMsg.Format(ML_STRING(1266, "Are you sure you want to delete %s ?"), ::GetShortFileName(m_sFileName));
			GetView()->ForceCursor();
			if (::AfxMessageBox(sMsg, MB_YESNO) == IDYES)
			{	
				GetView()->ForceCursor(FALSE);
				DeleteDocFile();
			}
			else
				GetView()->ForceCursor(FALSE);
		}
		else
			DeleteDocFile();
	}
}

BOOL CPictureDoc::DeleteDocFile()
{
	BOOL res;

	// Store File Name
	CString sFileNameToDelete = m_sFileName;

	// Check whether it is Read-Only
	DWORD dwAttrib = ::GetFileAttributes(sFileNameToDelete);
	if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
	{
		CString str;
		str.Format(ML_STRING(1250, "The file %s\nis read only"), sFileNameToDelete);
		GetView()->ForceCursor();
		::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
		GetView()->ForceCursor(FALSE);
		return FALSE;
	}

	// Be Sure We Are Not Working On This File
	m_JpegThread.Kill();
	m_GifAnimationThread.Kill();

	// Delete It
	if (!::DeleteToRecycleBin(sFileNameToDelete))
	{
		CString str;
		str.Format(ML_STRING(1267, "Cannot delete the %s file"), sFileNameToDelete);
		GetView()->ForceCursor();
		::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
		GetView()->ForceCursor(FALSE);
		res = FALSE;
	}
	else
	{
		ClearPrevNextPictures();
		m_SlideShowThread.NextPicture();
		res = TRUE;
	}

	return res;
}

void CPictureDoc::OnEditRename() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocAvailable(this, TRUE))
	{
		GetView()->ForceCursor();

		CRenameDlg dlg;
		dlg.m_sFileName = ::GetShortFileNameNoExt(m_sFileName);
		if (dlg.DoModal() == IDOK && ::IsValidFileName(dlg.m_sFileName, TRUE))
		{	
			// New file name
			CString sNewFileName =	::GetDriveAndDirName(m_sFileName) +
									dlg.m_sFileName +
									::GetFileExt(m_sFileName);

			// Be Sure We Are Not Working On This File
			m_JpegThread.Kill();
			m_GifAnimationThread.Kill();

			// Rename
			if (!::MoveFile(m_sFileName, sNewFileName))
			{
				::ShowErrorMsg(::GetLastError(), TRUE);
				sNewFileName = m_sFileName;
			}
			
			// Reload
			ClearPrevNextPictures();
			if (LoadPicture(&m_pDib, sNewFileName))
				SlideShow(); // No Recursive Slideshow in Paused State (also if it was Recursive before...)
		}

		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::LoadSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("PictureDoc"));

	m_SlideShowThread.SetMilliSecondsDelay((int)pApp->GetProfileInt(sSection, _T("SlideShowDelay"), DEFAULT_SLIDESHOW_DELAY));
	m_SlideShowThread.SetLoop((BOOL)pApp->GetProfileInt(sSection, _T("SlideShowInfiniteLoop"), FALSE));
	m_SlideShowThread.SetRandom((BOOL)pApp->GetProfileInt(sSection, _T("SlideShowRandomPlay"), FALSE));
	m_nTransitionType = (int)pApp->GetProfileInt(sSection, _T("TransitionType"), 0);
	m_bDitherColorConversion = (BOOL)pApp->GetProfileInt(sSection, _T("DitherColorConversion"), TRUE);
	m_uiMaxColors16 = (BOOL)pApp->GetProfileInt(sSection, _T("MaxColors16"), 16);
	m_uiMaxColors256 = (BOOL)pApp->GetProfileInt(sSection, _T("MaxColors256"), 256);
	m_crBackgroundColor = (COLORREF)pApp->GetProfileInt(sSection, _T("BackgroundColor"), ::GetSysColor(COLOR_WINDOW));
	m_bForceLossyTrafo = (BOOL)pApp->GetProfileInt(sSection, _T("ForceLossyTrafo"), FALSE);
	m_bStretchModeHalftone = (BOOL)pApp->GetProfileInt(sSection, _T("StretchModeHalftone"), TRUE);
	m_bEnableOsd = (BOOL)pApp->GetProfileInt(sSection, _T("EnableOSD"), TRUE);
	m_bNoBorders = (BOOL)pApp->GetProfileInt(sSection, _T("NoBorders"), FALSE);
	m_bPrintMargin = (BOOL)pApp->GetProfileInt(sSection, _T("PrintMargin"), TRUE);
	m_nZoomComboBoxIndex = (int)pApp->GetProfileInt(sSection, _T("ZoomComboBoxIndex"), 0); // Default Fit
	if (m_nZoomComboBoxIndex < MIN_ZOOM_COMBOBOX_INDEX ||
		m_nZoomComboBoxIndex > MAX_ZOOM_COMBOBOX_INDEX)
		m_nZoomComboBoxIndex = 0; // Fit
	m_nLayeredDlgMaxsizePercent = (int)pApp->GetProfileInt(sSection, _T("LayeredDlgMaxsizePercent"), MIN_LAYERED_DLG_MAXSIZE_PERCENT);
	m_nLayeredDlgSizePerthousand = (int)pApp->GetProfileInt(sSection, _T("LayeredDlgSizePerthousand"), 0);
	m_nLayeredDlgOrigin = (int)pApp->GetProfileInt(sSection, _T("LayeredDlgOrigin"), 0);
	m_nLayeredDlgOpacity = (int)pApp->GetProfileInt(sSection, _T("LayeredDlgOpacity"), MAX_LAYERED_DLG_OPACITY);
	if (m_nLayeredDlgOpacity < MIN_LAYERED_DLG_OPACITY)
		m_nLayeredDlgOpacity = MIN_LAYERED_DLG_OPACITY;
	else if (m_nLayeredDlgOpacity > MAX_LAYERED_DLG_OPACITY)
		m_nLayeredDlgOpacity = MAX_LAYERED_DLG_OPACITY;
	m_sCopyOrMoveDirName = pApp->GetProfileString(sSection, _T("CopyOrMoveDirName"), _T(""));
	m_bZoomTool = pApp->GetProfileInt(sSection, _T("ZoomTool"), FALSE);
	if (m_bZoomTool)
		::AfxGetMainFrame()->StatusText(ML_STRING(1226, "*** Right Click to Zoom Out ***"));
}

CDib* CPictureDoc::AddUndo(CDib* pDib/*=NULL*/)
{
	if (pDib == NULL)
		pDib = m_pDib;

	// Remove Redo
	if (m_DibUndoArray.GetSize() > 0)
	{
		while (m_nDibUndoPos < m_DibUndoArray.GetUpperBound())
		{
			delete m_DibUndoArray.GetAt(m_DibUndoArray.GetUpperBound());
			m_DibUndoArray.RemoveAt(m_DibUndoArray.GetUpperBound());	
		}
	}

	// Add
	CDib* pNewDib = (CDib*)new CDib(*pDib);
	m_DibUndoArray.Add(pNewDib);
	m_nDibUndoPos++;

	// No Redo
	m_bRedo = FALSE;

	return pNewDib;
}

void CPictureDoc::OnEditUndo() 
{
	Undo();
}

void CPictureDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	if (m_nDibUndoPos >= 0									&&
		!(m_SlideShowThread.IsSlideshowRunning() ||
		m_bDoRestartSlideshow)								&&
		!m_bMetadataModified								&&
		!m_pRotationFlippingDlg								&&
		!m_pWndPalette										&&
		!m_pHLSDlg											&&
		!m_pRedEyeDlg										&&
		!m_bDoRedEyeColorPickup								&&
		!m_pMonochromeConversionDlg							&&
		!m_pSharpenDlg										&&
		!m_pSoftenDlg										&&
		!m_pSoftBordersDlg									&&
		!m_bCrop)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);

	if (m_nDibUndoPos >= 0)
	{
		CString str;
		str.Format(ML_STRING(1268, "&Undo %d\tCtrl+Z"), m_nDibUndoPos+1);
		pCmdUI->SetText(str);
	}
	else
		pCmdUI->SetText(ML_STRING(1269, "&Undo\tCtrl+Z"));
}

void CPictureDoc::Undo(BOOL bUpdate/*=TRUE*/)
{
	if (m_nDibUndoPos >= 0)
	{
		CDib* pDib = m_DibUndoArray.GetAt(m_nDibUndoPos);
		m_DibUndoArray.SetAt(m_nDibUndoPos--, m_pDib);
		m_pDib = pDib;
		UpdateAlphaRenderedDib();

		if (m_sFileName != _T("") && bUpdate && m_nDibUndoPos == -1)
			SetModifiedFlag(FALSE);

		m_bRedo = TRUE;

		m_DocRect.bottom = m_pDib->GetHeight();
		m_DocRect.right = m_pDib->GetWidth();
		if (bUpdate)
		{
			SetDocumentTitle();
			UpdateImageInfo();
			UpdateAllViews(NULL);
		}
	}
}

void CPictureDoc::OnEditRedo()
{
	Redo();
}

void CPictureDoc::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	if (m_DibUndoArray.GetSize() > 0						&&
		m_nDibUndoPos < m_DibUndoArray.GetUpperBound()		&&
		!(m_SlideShowThread.IsSlideshowRunning() ||
		m_bDoRestartSlideshow)								&&
		!m_bMetadataModified								&&
		!m_pRotationFlippingDlg								&&
		!m_pWndPalette										&&
		!m_pHLSDlg											&&
		!m_pRedEyeDlg										&&
		!m_bDoRedEyeColorPickup								&&
		!m_pMonochromeConversionDlg							&&
		!m_pSharpenDlg										&&
		!m_pSoftenDlg										&&
		!m_pSoftBordersDlg									&&
		!m_bCrop)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CPictureDoc::Redo(BOOL bUpdate/*=TRUE*/)
{
	if (m_DibUndoArray.GetSize() > 0)
	{
		if (m_nDibUndoPos < m_DibUndoArray.GetUpperBound())
		{
			CDib* pDib = m_DibUndoArray.GetAt(++m_nDibUndoPos);
			SetModifiedFlag(TRUE);
			m_DibUndoArray.SetAt(m_nDibUndoPos, m_pDib);
			m_pDib = pDib;
			UpdateAlphaRenderedDib();
		
			if (m_nDibUndoPos == m_DibUndoArray.GetUpperBound())
				m_bRedo = FALSE;
		
			m_DocRect.bottom = m_pDib->GetHeight();
			m_DocRect.right = m_pDib->GetWidth();
			if (bUpdate)
			{
				SetDocumentTitle();
				UpdateImageInfo();
				UpdateAllViews(NULL);
			}
		}
	}
}

void CPictureDoc::ClearUndoArray()
{	
	while (m_DibUndoArray.GetSize() > 0)
	{
		delete m_DibUndoArray.GetAt(m_DibUndoArray.GetUpperBound());
		m_DibUndoArray.RemoveAt(m_DibUndoArray.GetUpperBound());
	}
	m_nDibUndoPos = -1;
}

void CPictureDoc::OnViewNextPicture() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(this, TRUE))
		m_SlideShowThread.NextPicture();
}

void CPictureDoc::OnViewPreviousPicture() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(this, TRUE))
		m_SlideShowThread.PreviousPicture();
}

void CPictureDoc::ClearPrevNextPictures()
{
	m_LoadPicturesThread.WaitDone_Blocking();

	m_sPrevDibName = _T("");
	if (m_pPrevDib)
	{
		delete m_pPrevDib;
		m_pPrevDib = NULL;
	}
	m_sNextDibName = _T("");
	if (m_pNextDib)
	{
		delete m_pNextDib;
		m_pNextDib = NULL;
	}
}

void CPictureDoc::ViewZoomTool() 
{
	if (!GetView()->m_bFullScreenMode)
	{
		m_bZoomTool = TRUE;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("ZoomTool"),
										TRUE);
		::AfxGetMainFrame()->StatusText(ML_STRING(1226, "*** Right Click to Zoom Out ***"));
	}
}

void CPictureDoc::CancelZoomTool()
{
	m_bZoomTool = FALSE;
	m_bZoomToolMinus = FALSE;
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("ZoomTool"),
									FALSE);
	::AfxGetMainFrame()->StatusText();
}

void CPictureDoc::StretchHalftone() 
{
	m_bStretchModeHalftone = !m_bStretchModeHalftone;
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("StretchModeHalftone"),
									m_bStretchModeHalftone);
	if (m_pLayeredDlg)
		m_pLayeredDlg->m_CurrentLayeredDib.Free();
	GetView()->Draw();
	InvalidateAllViews(FALSE);
}

void CPictureDoc::OnViewStretchHalftone() 
{
	StretchHalftone();
}

void CPictureDoc::OnUpdateViewStretchHalftone(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_bPrintPreviewMode);
	pCmdUI->SetCheck(m_bStretchModeHalftone ? 1 : 0);
}

BOOL CPictureDoc::CreatePreviewDib(CDib* pDib)
{
	// Check
	if (!pDib)
		return FALSE;

	// Current Monitor's Max Edge
	CSize szMonitor = ::AfxGetMainFrame()->GetMonitorSize();
	int nMaxSizeX = szMonitor.cx;
	int nMaxSizeY = szMonitor.cy;

	if ((int)pDib->GetWidth() < nMaxSizeX &&
		(int)pDib->GetHeight() < nMaxSizeY)
		return pDib->CreatePreviewDib(pDib->GetWidth(), pDib->GetHeight());
	else
		return pDib->CreatePreviewDib(nMaxSizeX, nMaxSizeY);
}

BOOL CPictureDoc::LoadPicture(CDib *volatile *ppDib,
							  CString sFileName,
							  BOOL bLoadOnly/*=FALSE*/,
							  BOOL bDoPrevNextPreload/*=TRUE*/,
							  BOOL bOnlyHeader/*=FALSE*/)
{
	BOOL res;

	// Is Exiting?
	if (!::IsExistingFile(sFileName))
	{
		if (m_bShowMessageBoxOnError)
		{
			CString sError;
			sError.Format(ML_STRING(1270, "The file %s\nis not existing!"), sFileName);
			::AfxMessageBox(sError, MB_OK | MB_ICONSTOP);
		}
		return FALSE;
	}

	// If the client edges are smaller than the picture edges,
	// the picture is loaded as a shrinked image to the
	// Preview Bits and not to the main Bits of the **ppDib Object!
	// Full size load will be performed later on by the CJpegThread.
	CRect rcClient;
	GetView()->GetClientRect(&rcClient);
	int nMaxSizeX = rcClient.Width();
	int nMaxSizeY = rcClient.Height();

	// Just Load Picture
	if (bLoadOnly)
	{
		res = (*ppDib)->LoadImage(	sFileName,
									((CUImagerApp*)::AfxGetApp())->m_bUseLoadPreviewDib ? nMaxSizeX : 0,
									((CUImagerApp*)::AfxGetApp())->m_bUseLoadPreviewDib ? nMaxSizeY : 0,
									0,				// Page Num 0
									TRUE,			// Decompress Bmp
									bOnlyHeader); 
		if (res)
			CDib::AutoOrientateDib(*ppDib);
		return res;
	}

	// Kill Gif Animation Thread
	if (m_GifAnimationThread.IsAlive())
		m_GifAnimationThread.Kill_NoBlocking();

	// Stop Jpeg Full Load Transition and start killing Jpeg thread
	m_bLoadFullJpegTransitionUI = FALSE;
	m_bCancelLoadFullJpegTransition = FALSE;
	m_JpegThread.Kill_NoBlocking();

	// Do Not Draw Now!
	m_bNoDrawing = TRUE;

	// Cancel Transition Thread and wait Until Thread Exits
	CancelTransition();
	while (m_TransitionThread.IsRunning())
	{	
		// Still Running?
		if (m_TransitionThread.IsRunning())
			::Sleep(0); // Switch to Worker Thread
		else
			break; // Go Ahead!
	}

	// Wait Until the Pictures Load Finishes
	while (m_LoadPicturesThread.IsRunning())
	{	
		// Still Running?
		if (m_LoadPicturesThread.IsRunning())
			::Sleep(0); // Switch to Worker Thread
		else
			break; // Go Ahead!
	}

	// Reload Picture (after Save command or when
	// loading next / prev. page in multi-page tiff)
	if (sFileName.CompareNoCase(m_sFileName) == 0)
	{
		// Load Picture
		res = (*ppDib)->LoadImage(	sFileName,
									((CUImagerApp*)::AfxGetApp())->m_bUseLoadPreviewDib ? nMaxSizeX : 0,
									((CUImagerApp*)::AfxGetApp())->m_bUseLoadPreviewDib ? nMaxSizeY : 0,
									m_nPageNum,	// Page Num
									TRUE,		// Decompress Bmp
									bOnlyHeader,
									GetView(),
									TRUE);
		if (res)
			CDib::AutoOrientateDib(*ppDib);

		// Do Not Start Thread because
		// Previous & Next Pictures Are already Loaded!
		bDoPrevNextPreload = FALSE;
	}
	else if (sFileName == m_sNextDibName) // Next Picture Load
	{
		// Throw Previous
		if (m_pPrevDib)
		{
			delete m_pPrevDib;
			m_pPrevDib = NULL;
		}
		m_sPrevDibName = _T("");

		// Previous from Current if Valid
		// Note: OnThreadSafeSlideshowLoadPicture() skips failed loads, but leaves
		//       the current dib invalid, so we must check its validity here!
		if ((*ppDib)->IsValid() ||
			((*ppDib)->GetPreviewDib() && (*ppDib)->GetPreviewDib()->IsValid()))
		{
			m_pPrevDib = (*ppDib);
			m_sPrevDibName = m_sFileName;
		}
		else
			delete (*ppDib);

		// Current from Next
		(*ppDib) = m_pNextDib;

		// Next to be Loaded by Thread
		m_pNextDib = NULL;
		m_sNextDibName = _T("");

		// Check
		if (*ppDib)
		{
			m_nPageNum = (*ppDib)->m_FileInfo.m_nImagePos;
			res = TRUE;
		}
		else
			res = FALSE;
	}
	else if (sFileName == m_sPrevDibName) // Previous Picture Load
	{
		// Throw Next
		if (m_pNextDib)
		{
			delete m_pNextDib;
			m_pNextDib = NULL;
		}
		m_sNextDibName = _T("");

		// Next from Current if Valid
		// Note: OnThreadSafeSlideshowLoadPicture() skips failed loads, but leaves
		//       the current dib invalid, so we must check its validity here!
		if ((*ppDib)->IsValid() ||
			((*ppDib)->GetPreviewDib() && (*ppDib)->GetPreviewDib()->IsValid()))
		{
			m_pNextDib = (*ppDib);
			m_sNextDibName = m_sFileName;
		}
		else
			delete (*ppDib);

		// Current from Previous
		(*ppDib) = m_pPrevDib;

		// Previous to be Loaded by Thread
		m_pPrevDib = NULL;
		m_sPrevDibName = _T("");

		// Check
		if (*ppDib)
		{
			m_nPageNum = (*ppDib)->m_FileInfo.m_nImagePos;
			res = TRUE;
		}
		else
			res = FALSE;
	}
	else // First Time Load
	{
		CSize szMonitor;
		if (((CUImagerApp*)::AfxGetApp())->m_bUseLoadPreviewDib)
			szMonitor = ::AfxGetMainFrame()->GetMonitorSize();

		// Load Picture
		res = (*ppDib)->LoadImage(	sFileName,
									((CUImagerApp*)::AfxGetApp())->m_bUseLoadPreviewDib ? szMonitor.cx : 0,
									((CUImagerApp*)::AfxGetApp())->m_bUseLoadPreviewDib ? szMonitor.cy : 0,
									m_nPageNum = 0,	// Page Num
									TRUE,			// Decompress Bmp
									bOnlyHeader,
									GetView(),
									TRUE);
		if (res)
			CDib::AutoOrientateDib(*ppDib);
		
		// Previous & Next to be Loaded by Thread
		if (res)
		{
			if (m_pPrevDib)
			{
				delete m_pPrevDib;
				m_pPrevDib = NULL;
			}
			m_sPrevDibName = _T("");
			if (m_pNextDib)
			{
				delete m_pNextDib;
				m_pNextDib = NULL;
			}
			m_sNextDibName = _T("");
		}
	}
	
	// Initializations for Current Picture & Thread Start
	if (res)
	{
		// Clear Undo / Redo
		ClearUndoArray();

		// Cancel Crop
		CancelCrop();

		// Reset Metadata Modified Flag
		m_bMetadataModified = FALSE;

		// Document Rectangle
		m_DocRect.top = 0;
		m_DocRect.left = 0;
		m_DocRect.right = (*ppDib)->GetWidth();
		m_DocRect.bottom = (*ppDib)->GetHeight();
		m_nPixelAlignX = 1; // This is modified by JPEGGet() for jpegs
		m_nPixelAlignY = 1; // This is modified by JPEGGet() for jpegs

		// Set File Name
		m_sFileName = sFileName;
		SetPathName(sFileName, TRUE);

		// Make Sure Jpeg Thread Is Not Running!
		if (m_bDoJPEGGet)
			m_JpegThread.Kill();

		// Wait until the Gif Animation Thread Stops
		if (m_GifAnimationThread.IsAlive())
			m_GifAnimationThread.WaitDone_Blocking();
		
		// Clear all Animation Frames
		m_GifAnimationThread.ClearAnimationArrays();

		// Load All Frames of the Animated GIF File to m_DibAnimationArray
		if (CDib::IsAnimatedGIF(sFileName, FALSE))
		{
			m_GifAnimationThread.SetFileName(sFileName);
			if (m_GifAnimationThread.Load(TRUE) > 1)
			{
				// Start Thread
				m_GifAnimationThread.Start();
			}
		}

		// Fit zoom factor (if fit selected) and calc.
		// the new zoom rect. Do the calculation here
		// because the JPEGGet Thread needs m_ZoomRect.
		// This function updates also the buttons of the
		// toolbar.
		GetView()->UpdateWindowSizes(FALSE, FALSE, FALSE);

		// Set Title
		SetDocumentTitle();

		// Update Flag
		m_bImageBackgroundColor = (*ppDib)->m_FileInfo.m_bHasBackgroundColor;

		// Update Control
		if (m_bImageBackgroundColor)
		{
			m_crImageBackgroundColor = (*ppDib)->m_FileInfo.m_crBackgroundColor;
			CColorButtonPicker* pBkgColorButtonPicker =
				&(((CPictureToolBar*)((CToolBarChildFrame*)
				(GetView()->GetParentFrame()))->GetToolBar())->m_BkgColorButtonPicker);
			pBkgColorButtonPicker->SetDefaultText(ML_STRING(1272, "Image Default Backgnd"));
			pBkgColorButtonPicker->SetColor(m_crImageBackgroundColor);
			pBkgColorButtonPicker->SetDefaultColor(m_crImageBackgroundColor);
		}
		// Restore Control
		else
		{
			(*ppDib)->SetBackgroundColor(m_crBackgroundColor);
			CColorButtonPicker* pBkgColorButtonPicker =
						&(((CPictureToolBar*)((CToolBarChildFrame*)
						(GetView()->GetParentFrame()))->GetToolBar())->m_BkgColorButtonPicker);
			pBkgColorButtonPicker->SetDefaultText(ML_STRING(1273, "Default Background"));
			pBkgColorButtonPicker->SetColor(m_crBackgroundColor);
			pBkgColorButtonPicker->SetDefaultColor(::GetSysColor(COLOR_WINDOW));
		}

		// Make Alpha Renderer Dib from Background Color
		UpdateAlphaRenderedDib();

		// Start Load Pictures Thread
		if (bDoPrevNextPreload)
		{
			if (m_LoadPicturesThread.InitFileNames())
				m_LoadPicturesThread.Start();
		}

		// Start Transition (only if it's not a animated gif)
		if (m_nTransitionType &&
			!((m_GifAnimationThread.m_dwDibAnimationCount > 1) &&
			m_GifAnimationThread.IsAlive()))
		{
			// Set m_bTransitionUI 
			m_bTransitionUI = TRUE;

			// Flush the GDI batch queue 
			::GdiFlush();

			// Start Thread
			// (m_bNoDrawing will be cleared inside the Thread)
			m_TransitionThread.Start();
		}
		else // or Display Picture
		{
			// Draw
			m_bNoDrawing = FALSE;
			GetView()->InvalidateRect(rcClient, FALSE);
		}

		// Start Calc. Jpeg Compression,
		// get pixel align (for cropping)
		// and load the full-sized jpeg 
		// if not already done.
		if (m_bDoJPEGGet)
			JPEGGet();

		// Set flag which would be otherwise set
		// in CJpegThread started by JPEGGet()
		if (!m_bDoJPEGGet || !CDib::IsJPEG(m_sFileName))
			m_bCancelLoadFullJpegTransitionAllowed = TRUE;

		// Update Info if dialog is open
		UpdateImageInfo();

		return TRUE;
	}
	else
	{
		m_bNoDrawing = FALSE;
		return FALSE;
	}
}

void CPictureDoc::SlideShow()
{
	// Stop Thread
	m_SlideShowThread.Kill();

	// (Re)Start Thread
	m_SlideShowThread.SetStartName(m_sFileName);
	m_SlideShowThread.SetRecursive(FALSE);
	m_SlideShowThread.PauseSlideshow();
}

void CPictureDoc::OnSlideshowDelay2() 
{
	m_SlideShowThread.SetMilliSecondsDelay(2000);
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowDelay"),
									2000);
}

void CPictureDoc::OnUpdateSlideshowDelay2(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.GetMilliSecondsDelay() == 2000 ? 1 : 0);
}

void CPictureDoc::OnSlideshowDelay3() 
{
	m_SlideShowThread.SetMilliSecondsDelay(3000);
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowDelay"),
									3000);
}

void CPictureDoc::OnUpdateSlideshowDelay3(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.GetMilliSecondsDelay() == 3000 ? 1 : 0);
}

void CPictureDoc::OnSlideshowDelay4() 
{
	m_SlideShowThread.SetMilliSecondsDelay(4000);
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowDelay"),
									4000);
}

void CPictureDoc::OnUpdateSlideshowDelay4(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.GetMilliSecondsDelay() == 4000 ? 1 : 0);
}

void CPictureDoc::OnSlideshowDelay5() 
{
	m_SlideShowThread.SetMilliSecondsDelay(5000);
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowDelay"),
									5000);
}

void CPictureDoc::OnUpdateSlideshowDelay5(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.GetMilliSecondsDelay() == 5000 ? 1 : 0);
}

void CPictureDoc::OnSlideshowDelay10() 
{
	m_SlideShowThread.SetMilliSecondsDelay(10000);
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowDelay"),
									10000);
}

void CPictureDoc::OnUpdateSlideshowDelay10(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.GetMilliSecondsDelay() == 10000 ? 1 : 0);
}

void CPictureDoc::OnSlideshowDelay30() 
{
	m_SlideShowThread.SetMilliSecondsDelay(30000);
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowDelay"),
									30000);
}

void CPictureDoc::OnUpdateSlideshowDelay30(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.GetMilliSecondsDelay() == 30000 ? 1 : 0);
}

void CPictureDoc::OnSlideshowDelay60() 
{
	m_SlideShowThread.SetMilliSecondsDelay(60000);
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowDelay"),
									60000);
}

void CPictureDoc::OnUpdateSlideshowDelay60(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.GetMilliSecondsDelay() == 60000 ? 1 : 0);
}

void CPictureDoc::OnEditRotate90cw()
{
	Rotate90cw();
}

void CPictureDoc::OnUpdateEditRotate90cw(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0					||
					m_dwIDAfterFullLoadCommand == ID_EDIT_ROTATE_90CW)	&&
					DoEnableCommand());
}

void CPictureDoc::OnEditRotate90ccw() 
{
	Rotate90ccw();
}

void CPictureDoc::OnUpdateEditRotate90ccw(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0					||
					m_dwIDAfterFullLoadCommand == ID_EDIT_ROTATE_90CCW) &&
					DoEnableCommand());
}

void CPictureDoc::OnEditRotate180() 
{
	Rotate180();
}

void CPictureDoc::OnUpdateEditRotate180(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0					||
					m_dwIDAfterFullLoadCommand == ID_EDIT_ROTATE_180)	&&
					DoEnableCommand());
}

BOOL CPictureDoc::Rotate90cw()
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_ROTATE_90CW))
			return FALSE;

		// Show Cursor & Begin Wait Cursor
		GetView()->ForceCursor();
		BeginWaitCursor();

		// Check for JPEG Extensions and make sure the file has not been modified
		BOOL bLosslessDone = FALSE;
		if (CDib::IsJPEG(m_sFileName) && !IsModified() && !m_bPrintPreviewMode)
		{
			// Kill Jpeg Thread
			m_JpegThread.Kill();

			// Make Sure The File has the right Orientation
			CDib::JPEGAutoOrientate(m_sFileName,
									((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
									FALSE,
									GetView(),
									TRUE);

			// Temporary File
			CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

			// Do Transformation
			if (m_pDib->LossLessJPEGTrans(	m_sFileName,
											sTempFileName,
											JXFORM_ROT_90,
											TRUE,
											FALSE,
											TRUE,
											"",
											FALSE,
											0, 0, 0, 0,
											FALSE,
											GetView(),
											TRUE))
			{
				// Remove and Rename Files
				try
				{
					// Get Last Write File Time
					FILETIME LastWriteTime;
					::GetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);

					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);

					// Set Last Write File Time
					::SetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);

					// OK
					bLosslessDone = TRUE;
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);
					e->Delete();
				}
			}
		}

		if (bLosslessDone)
			LoadPicture(&m_pDib, m_sFileName);
		else
		{
			// A Duplicated Dib is created
			AddUndo();
			m_pDib->Rotate90CW();
			SetModifiedFlag();
			m_DocRect.bottom = m_pDib->GetHeight();
			m_DocRect.right = m_pDib->GetWidth();
			UpdateAlphaRenderedDib();
			SetDocumentTitle();
			UpdateAllViews(NULL);
			UpdateImageInfo();
		}

		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		return TRUE;
	}
	else
		return FALSE;
}

BOOL CPictureDoc::Rotate90ccw() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_ROTATE_90CCW))
			return FALSE;

		// Show Cursor & Begin Wait Cursor
		GetView()->ForceCursor();
		BeginWaitCursor();

		// Check for JPEG Extensions and make sure the file has not been modified
		BOOL bLosslessDone = FALSE;
		if (CDib::IsJPEG(m_sFileName) && !IsModified() && !m_bPrintPreviewMode)
		{
			// Kill Jpeg Thread
			m_JpegThread.Kill();

			// Make Sure The File has the right Orientation
			CDib::JPEGAutoOrientate(m_sFileName,
									((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
									FALSE,
									GetView(),
									TRUE);

			// Temporary File
			CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

			// Do Transformation
			if (m_pDib->LossLessJPEGTrans(	m_sFileName,
											sTempFileName,
											JXFORM_ROT_270,
											TRUE,
											FALSE,
											TRUE,
											"",
											FALSE,
											0, 0, 0, 0,
											FALSE,
											GetView(),
											TRUE))
			{
				// Remove and Rename Files
				try
				{
					// Get Last Write File Time
					FILETIME LastWriteTime;
					::GetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);

					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);

					// Set Last Write File Time
					::SetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);

					// OK
					bLosslessDone = TRUE;
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);
					e->Delete();
				}
			}
		}

		if (bLosslessDone)
			LoadPicture(&m_pDib, m_sFileName);
		else
		{
			// A Duplicated Dib is created
			AddUndo();
			m_pDib->Rotate90CCW();
			SetModifiedFlag();
			m_DocRect.bottom = m_pDib->GetHeight();
			m_DocRect.right = m_pDib->GetWidth();
			UpdateAlphaRenderedDib();
			SetDocumentTitle();
			UpdateAllViews(NULL);
			UpdateImageInfo();
		}

		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		return TRUE;
	}
	else
		return FALSE;
}

BOOL CPictureDoc::Rotate180() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_ROTATE_180))
			return FALSE;

		// Show Cursor & Begin Wait Cursor
		GetView()->ForceCursor();
		BeginWaitCursor();

		// Check for JPEG Extensions and make sure the file has not been modified
		BOOL bLosslessDone = FALSE;
		if (CDib::IsJPEG(m_sFileName) && !IsModified() && !m_bPrintPreviewMode)
		{
			// Kill Jpeg Thread
			m_JpegThread.Kill();

			// Make Sure The File has the right Orientation
			CDib::JPEGAutoOrientate(m_sFileName,
									((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
									FALSE,
									GetView(),
									TRUE);

			// Temporary File
			CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

			// Do Transformation
			if (m_pDib->LossLessJPEGTrans(	m_sFileName,
											sTempFileName,
											JXFORM_ROT_180,
											TRUE,
											FALSE,
											TRUE,
											"",
											FALSE,
											0, 0, 0, 0,
											FALSE,
											GetView(),
											TRUE))
			{
				// Remove and Rename Files
				try
				{
					// Get Last Write File Time
					FILETIME LastWriteTime;
					::GetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);

					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);

					// Set Last Write File Time
					::SetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);

					// OK
					bLosslessDone = TRUE;
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);
					e->Delete();
				}
			}
		}

		if (bLosslessDone)
			LoadPicture(&m_pDib, m_sFileName);
		else
		{
			// A Duplicated Dib is created
			AddUndo();
			m_pDib->Rotate180();
			SetModifiedFlag();
			m_DocRect.bottom = m_pDib->GetHeight();
			m_DocRect.right = m_pDib->GetWidth();
			UpdateAlphaRenderedDib();
			SetDocumentTitle();
			UpdateAllViews(NULL);
			UpdateImageInfo();
		}

		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditRotateFlip() 
{
	if (m_pRotationFlippingDlg)
	{
		// m_pRotationFlippingDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pRotationFlippingDlg->Close();
	}
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_ROTATE_FLIP))
			return;

		// Show dialog
		m_pRotationFlippingDlg = new CRotationFlippingDlg(GetView());
		m_pRotationFlippingDlg->ShowWindow(SW_RESTORE);
	}
}

// Return values:
// 0 = save before doing a lossless transformation
// 1 = ok done
// 2 = cannot do a lossless transformation
int CPictureDoc::LossLessRotateFlip(BOOL bShowMessageBoxOnError, CRotationFlippingDlg* pDlg) 
{
	// Check for JPEG Extensions
	if (CDib::IsJPEG(m_sFileName))
	{
		// Make sure the File is saved before doing a lossless jpeg transformations
		switch (pDlg->m_TransformationType)
		{
			case 0 :
			case 1 :
			case 2 :
			case 3 :
			case 4 :
				if (IsModified() || m_bForceLossyTrafo || m_bPrintPreviewMode)
					return 2;	// Do Lossy
				else
					break;
			case 5 : 
				if (((pDlg->m_uiAngle == 90) || (pDlg->m_uiAngle == 180) || (pDlg->m_uiAngle == 270)) &&
					pDlg->m_uiAngleMinutes == 0)
				{
					if (IsModified() || m_bForceLossyTrafo || m_bPrintPreviewMode)
						return 2; // Do Lossy
					else
						break;
				}
				else
					return 2;	// Only Lossy Trafo Available
			default: return 2;
		}

		// Kill Jpeg Thread
		m_JpegThread.Kill();

		// Make Sure The File has the right Orientation
		if (!CDib::JPEGAutoOrientate(m_sFileName,
									((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
									bShowMessageBoxOnError,
									GetView(),
									TRUE))
			return 2;

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Do Transformation
		BOOL res = FALSE;
		switch (pDlg->m_TransformationType)
		{
			case 0 : if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_ROT_90,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;
					else
						break;
			case 1 : if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_ROT_270,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;				
					else
						break;
			case 2 : if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_ROT_180,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;
					else
						break;
			case 3 : if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_FLIP_H,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;
					else
						break;
			case 4 : if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_FLIP_V,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;
					else
						break;
			case 5 : 
				if (pDlg->m_uiAngle == 90)
				{
					if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_ROT_90,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;
					else
						break;
				}
				else if (pDlg->m_uiAngle == 180)
				{
					if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_ROT_180,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;
					else
						break;
				}
				else if (pDlg->m_uiAngle == 270)
				{
					if (!(res = m_pDib->LossLessJPEGTrans(m_sFileName,
													sTempFileName,
													JXFORM_ROT_270,
													TRUE,
													FALSE,
													TRUE,
													"",
													FALSE,
													0, 0, 0, 0,
													bShowMessageBoxOnError,
													GetView(),
													TRUE)))
						return 2;
					else
						break;
				}
				else
					return 2;
			default:
				return 2;
		}

		if (res)
		{
			// Remove and Rename Files
			try
			{
				// Get Last Write File Time
				FILETIME LastWriteTime;
				::GetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);

				CFile::Remove(m_sFileName);
				CFile::Rename(sTempFileName, m_sFileName);

				// Set Last Write File Time
				::SetFileTime(m_sFileName, NULL, NULL, &LastWriteTime);
			}
			catch (CFileException* e)
			{
				::DeleteFile(sTempFileName);

				DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
				if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
				{
					CString str(ML_STRING(1255, "The file is read only\n"));
					TRACE(str);
					if (bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
				}
				else
					::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

				e->Delete();
				return 2;
			}
		}

		return 1;
	}
	else
		return 2;
}

void CPictureDoc::OnUpdateEditRotateFlip(CCmdUI* pCmdUI) 
{	
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_ROTATE_FLIP)	&&
					m_pDib												&&
					!(m_SlideShowThread.IsSlideshowRunning() ||
					m_bDoRestartSlideshow)								&&
					!m_GifAnimationThread.IsAlive()						&&
					!m_bMetadataModified								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop);
	pCmdUI->SetCheck(m_pRotationFlippingDlg != NULL ? 1 : 0);
}

BOOL CPictureDoc::EditPalette()
{
	if (m_pWndPalette)
	{	
		m_pWndPalette->Close();
		return TRUE;
	}
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_PALETTE))
			return FALSE;

		if (m_pDib && m_pDib->GetColors())
		{
			m_pWndPalette = (CPaletteWnd*)new CPaletteWnd;
			if (!m_pWndPalette)
				return FALSE;

			CRect rcClient;
			GetView()->GetClientRect(&rcClient);
			GetView()->ClientToScreen(&rcClient);
			m_pWndPalette->Create(	rcClient.TopLeft(),
									GetView(),
									m_pDib->GetPalette());
			GetView()->ForceCursor();
			m_pWndPalette->ShowWindow(SW_SHOW);

			return TRUE;
		}
		else
			return FALSE;
	}
}

void CPictureDoc::OnEditPalette() 
{
	EditPalette();
}

void CPictureDoc::OnUpdateEditPalette(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_PALETTE)		&&
					m_pDib && m_pDib->GetColors()						&&
					!(m_SlideShowThread.IsSlideshowRunning() ||
					m_bDoRestartSlideshow)								&&
					!m_GifAnimationThread.IsAlive()						&&
					!m_bMetadataModified								&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_pRotationFlippingDlg								&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop);
	pCmdUI->SetCheck(m_pWndPalette ? 1 : 0);
}

BOOL CPictureDoc::EditColorsCount()
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_COLORS_COUNT))
			return FALSE;

		// Show Cursor & Begin Wait Cursor
		GetView()->ForceCursor();
		BeginWaitCursor();

		// Count
		DWORD dwUniqueColorsCount = m_pDib->CountUniqueColors(GetView(), TRUE);
		CString s;
		s.Format(ML_STRING(1277, "Unique Colors Count: %u\r\n"), dwUniqueColorsCount);
		
		// End Wait Cursor
		EndWaitCursor();

		// Display Message
		::AfxMessageBox(s, MB_OK | MB_ICONINFORMATION);

		// End Force Cursor
		GetView()->ForceCursor(FALSE);

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditColorsCount() 
{
	EditColorsCount();
}

void CPictureDoc::OnUpdateEditColorsCount(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_COLORS_COUNT)	&&
					DoEnableCommand());
}

BOOL CPictureDoc::EditResize(BOOL bShowMessageBoxOnError) 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_RESIZE))
			return FALSE;

		// Force Cursor
		GetView()->ForceCursor();

		// Resize Dialog
		int nXDpi = m_pDib->GetXDpi();
		int nYDpi = m_pDib->GetYDpi();
		if (nXDpi == 0 || nYDpi == 0)
		{
			nXDpi = DEFAULT_DPI;
			nYDpi = DEFAULT_DPI;
		}
		CResizingDpiDlg dlg(m_DocRect.Width(), m_DocRect.Height(),
							nXDpi, nYDpi,
							m_pDib->GetBitCount() <= 8 ?
							CResizingDpiDlg::NEARESTNEIGHBOR :
							CResizingDpiDlg::BESTQUALITY,
							GetView());
		if (dlg.DoModal() == IDOK)
		{
			// Resize?
			if (dlg.m_nPixelsWidth == m_DocRect.Width()		&&
				dlg.m_nPixelsHeight == m_DocRect.Height()	&&
				dlg.m_nXDpi == nXDpi						&&
				dlg.m_nYDpi == nYDpi)
			{
				GetView()->ForceCursor(FALSE);
				return TRUE;
			}

			BeginWaitCursor();
			
			CDib* pSrcDib = AddUndo();

			// Do Resize
			if (!m_pDib->StretchBits(	(DWORD)(dlg.m_nPixelsWidth),
										(DWORD)(dlg.m_nPixelsHeight),
										pSrcDib,
										GetView(),
										TRUE,
										NULL,
										dlg.m_ResizingMethod == CResizingDpiDlg::NEARESTNEIGHBOR))
			{
				Undo();
				EndWaitCursor();
				GetView()->ForceCursor(FALSE);
				return FALSE;
			}

			m_pDib->SetXDpi(dlg.m_nXDpi);
			m_pDib->SetYDpi(dlg.m_nYDpi);

			EndWaitCursor();
		}
		else
		{
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		m_DocRect.bottom = m_pDib->GetHeight();
		m_DocRect.right = m_pDib->GetWidth();

		GetView()->ForceCursor(FALSE);

		UpdateAlphaRenderedDib();
		SetModifiedFlag();
		SetDocumentTitle();
		UpdateAllViews(NULL);
		UpdateImageInfo();

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditResize() 
{
	EditResize(TRUE);
}

void CPictureDoc::OnUpdateEditResize(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0				||
					m_dwIDAfterFullLoadCommand == ID_EDIT_RESIZE)	&&
					DoEnableCommand()								&&
					!m_bPrintPreviewMode);
}

void CPictureDoc::OnEditFilterSharpen() 
{
	if (m_pSharpenDlg)
	{
		// m_pSharpenDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pSharpenDlg->Close();
	}
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_FILTER_SHARPEN))
			return;

		m_pSharpenDlg = new CSharpenDlg(GetView());
		m_pSharpenDlg->ShowWindow(SW_RESTORE);
	}
}

void CPictureDoc::OnUpdateEditFilterSharpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_FILTER_SHARPEN) &&
					DoEnableCommand());
}

void CPictureDoc::OnEditFilterSoften() 
{
	if (m_pSoftenDlg)
	{
		// m_pSoftenDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pSoftenDlg->Close();
	}
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_FILTER_SOFTEN))
			return;

		m_pSoftenDlg = new CSoftenDlg(GetView());
		m_pSoftenDlg->ShowWindow(SW_RESTORE);
	}
}

void CPictureDoc::OnUpdateEditFilterSoften(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_FILTER_SOFTEN) &&
					DoEnableCommand());
}

void CPictureDoc::OnEditHls()
{
	if (m_pHLSDlg)
	{
		// m_pHLSDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pHLSDlg->Close();
	}
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_HLS))
			return;

		m_pHLSDlg = new CHLSDlgModeless(GetView());
		m_pHLSDlg->ShowWindow(SW_RESTORE);
	}
}

void CPictureDoc::OnUpdateEditHls(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_HLS)			&&
					m_pDib												&&
					!(m_SlideShowThread.IsSlideshowRunning() ||
					m_bDoRestartSlideshow)								&&
					!m_GifAnimationThread.IsAlive()						&&
					!m_bMetadataModified								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop);
	pCmdUI->SetCheck(m_pHLSDlg != NULL ? 1 : 0);
}

// nCompression may be:
// BI_RGB  : Decompress
// BI_RLE8 : Compress to RLE8
// BI_RLE4 : Compress to RLE4
BOOL CPictureDoc::CompressRLE(int nCompression)
{
	BOOL res = FALSE;

	if (m_pDib && m_pDib->IsValid())
	{
		GetView()->ForceCursor();
		BeginWaitCursor();

		// A Duplicated Dib is created
		AddUndo();
		if (nCompression == BI_RGB)
			res = m_pDib->Decompress(m_pDib->GetBitCount());
		else
			res = m_pDib->Compress(nCompression);

		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		SetModifiedFlag();
		SetDocumentTitle();

		InvalidateAllViews(FALSE);

		UpdateImageInfo();
	}

	return res;
}

BOOL CPictureDoc::EditPasteInto(int nCorner, CPoint pt, int nAlphaOffset/*=0*/, BOOL bClientCoordinates/*=TRUE*/)
{
	if (m_pDib)
	{
		// Show Cursor & Begin Wait Cursor
		GetView()->ForceCursor();
		BeginWaitCursor();
			
		// A Duplicated Dib is created
		AddUndo();

		// Paste Dib
		CDib PasteDib;
		if (m_bImageBackgroundColor)
			PasteDib.SetBackgroundColor(m_crImageBackgroundColor);
		else
			PasteDib.SetBackgroundColor(m_crBackgroundColor);
		PasteDib.EditPaste(m_pDib->GetXDpi(), m_pDib->GetYDpi());

		// Background Color
		COLORREF crBackground;
		if (m_pDib->GetBitCount() == 32 && m_pDib->HasAlpha())
			crBackground = RGBA(0,0,0,0); // Transparent
		else
		{
			if (m_bImageBackgroundColor)
				crBackground = m_crImageBackgroundColor;
			else
				crBackground = m_crBackgroundColor;
		}

		// Alpha offset
		if (nAlphaOffset != 0)
		{
			if (PasteDib.GetBitCount() != 32 || !PasteDib.HasAlpha())
				PasteDib.ConvertTo32bitsAlpha();
			PasteDib.AlphaOffset(nAlphaOffset);
		}

		// From top-down client coordinates to top-down dib coordinates
		if (bClientCoordinates)
		{
			pt += GetView()->GetScrollPosition();
			pt -= GetView()->m_ZoomRect.TopLeft();
			pt.x = (int)floor((double)pt.x / m_dZoomFactor);
			pt.y = (int)floor((double)pt.y / m_dZoomFactor);
		}

		// To bottom-up dib offsets
		int nOffsetX, nOffsetY;
		switch (nCorner)
		{
			// Top-Left
			case 0 :
				nOffsetX = pt.x;
				nOffsetY = (int)m_pDib->GetHeight() - pt.y - (int)PasteDib.GetHeight();
				break;
			// Top-Right
			case 1 :
				nOffsetX = pt.x - (int)PasteDib.GetWidth() + 1;
				nOffsetY = (int)m_pDib->GetHeight() - pt.y - (int)PasteDib.GetHeight();
				break;
			// Bottom-Left
			case 2 :
				nOffsetX = pt.x;
				nOffsetY = (int)m_pDib->GetHeight() - pt.y - 1;
				break;
			// Bottom-Right
			default :
				nOffsetX = pt.x - (int)PasteDib.GetWidth() + 1;
				nOffsetY = (int)m_pDib->GetHeight() - pt.y - 1;
				break;
		}
		
		// Add borders if necessary
		int nLeftBorder = -nOffsetX;
		int nBottomBorder = -nOffsetY;
		int nRightBorder = (int)PasteDib.GetWidth() - m_pDib->GetWidth() + nOffsetX;
		int nTopBorder = (int)PasteDib.GetHeight() - m_pDib->GetHeight() + nOffsetY;
		m_pDib->AddBorders(	nLeftBorder > 0   ? nLeftBorder   : 0,
							nTopBorder > 0    ? nTopBorder    : 0,
							nRightBorder > 0  ? nRightBorder  : 0,
							nBottomBorder > 0 ? nBottomBorder : 0,
							m_pDib->GetBitCount() <= 8 ?
							m_pDib->GetPalette()->GetNearestPaletteIndex(crBackground) :
							crBackground);

		// Reset negative offsets because we added the borders
		if (nOffsetX < 0)
			nOffsetX = 0;
		if (nOffsetY < 0)
			nOffsetY = 0;

		// Copy
		if (m_pDib->GetBitCount() <= 8)
		{
			if (PasteDib.GetBitCount() == 32 && PasteDib.HasAlpha())
			{
				for (int y = 0 ; y < (int)PasteDib.GetHeight() ; y++)
				{
					for (int x = 0 ; x < (int)PasteDib.GetWidth() ; x++)
					{
						COLORREF crSource = PasteDib.GetPixelColor32Alpha(x, y);
						int Src_R = GetRValue(crSource);
						int Src_G = GetGValue(crSource);
						int Src_B = GetBValue(crSource);
						int Src_A = GetAValue(crSource);

						COLORREF crDestination = m_pDib->GetPixelColor(x + nOffsetX, y + nOffsetY);
						int Dst_R = GetRValue(crDestination);
						int Dst_G = GetGValue(crDestination);
						int Dst_B = GetBValue(crDestination);
						
						int Mix_B = (Src_B * Src_A + Dst_B * (255 - Src_A)) / 255;
						int Mix_G = (Src_G * Src_A + Dst_G * (255 - Src_A)) / 255;
						int Mix_R = (Src_R * Src_A + Dst_R * (255 - Src_A)) / 255;
						COLORREF crMix = RGB(Mix_R, Mix_G, Mix_B);
						m_pDib->SetPixelIndex(x + nOffsetX, y + nOffsetY,
								m_pDib->GetPalette()->GetNearestPaletteIndex(crMix));
					}
				}
			}
			else
			{
				for (int y = 0 ; y < (int)PasteDib.GetHeight() ; y++)
				{
					for (int x = 0 ; x < (int)PasteDib.GetWidth() ; x++)
					{
						COLORREF crSource = PasteDib.GetPixelColor(x, y);
						m_pDib->SetPixelIndex(x + nOffsetX, y + nOffsetY,
								m_pDib->GetPalette()->GetNearestPaletteIndex(crSource));
					}
				}
			}
		}
		else
		{
			if (PasteDib.GetBitCount() == 32 && PasteDib.HasAlpha())
			{
				if (m_pDib->GetBitCount() == 32 && m_pDib->HasAlpha())
				{
					for (int y = 0 ; y < (int)PasteDib.GetHeight() ; y++)
					{
						for (int x = 0 ; x < (int)PasteDib.GetWidth() ; x++)
						{
							COLORREF crSource = PasteDib.GetPixelColor32Alpha(x, y);
							int Src_R = GetRValue(crSource);
							int Src_G = GetGValue(crSource);
							int Src_B = GetBValue(crSource);
							int Src_A = GetAValue(crSource);

							COLORREF crDestination = m_pDib->GetPixelColor32Alpha(x + nOffsetX, y + nOffsetY);
							int Dst_R = GetRValue(crDestination);
							int Dst_G = GetGValue(crDestination);
							int Dst_B = GetBValue(crDestination);
							int Dst_A = GetAValue(crDestination);
							
							int Mix_R, Mix_G, Mix_B, Mix_A, Mix_Term;
							Mix_Term = (255 - Src_A) * Dst_A; 
							Mix_A = Mix_Term / 255 + Src_A;
							if (Mix_A > 0)
							{
								Mix_R = MIN(255, (Mix_Term * Dst_R / 255 + Src_R * Src_A) / Mix_A);
								Mix_G = MIN(255, (Mix_Term * Dst_G / 255 + Src_G * Src_A) / Mix_A);
								Mix_B = MIN(255, (Mix_Term * Dst_B / 255 + Src_B * Src_A) / Mix_A);
							}
							else
							{
								Mix_R = 0;
								Mix_G = 0;
								Mix_B = 0;
							}
							COLORREF crMix = RGBA(Mix_R, Mix_G, Mix_B, Mix_A);
							m_pDib->SetPixelColor32Alpha(x + nOffsetX, y + nOffsetY, crMix);
						}
					}
				}
				else
				{
					for (int y = 0 ; y < (int)PasteDib.GetHeight() ; y++)
					{
						for (int x = 0 ; x < (int)PasteDib.GetWidth() ; x++)
						{
							COLORREF crSource = PasteDib.GetPixelColor32Alpha(x, y);
							int Src_R = GetRValue(crSource);
							int Src_G = GetGValue(crSource);
							int Src_B = GetBValue(crSource);
							int Src_A = GetAValue(crSource);

							COLORREF crDestination = m_pDib->GetPixelColor(x + nOffsetX, y + nOffsetY);
							int Dst_R = GetRValue(crDestination);
							int Dst_G = GetGValue(crDestination);
							int Dst_B = GetBValue(crDestination);
							
							int Mix_B = (Src_B * Src_A + Dst_B * (255 - Src_A)) / 255;
							int Mix_G = (Src_G * Src_A + Dst_G * (255 - Src_A)) / 255;
							int Mix_R = (Src_R * Src_A + Dst_R * (255 - Src_A)) / 255;
							COLORREF crMix = RGB(Mix_R, Mix_G, Mix_B);
							m_pDib->SetPixelColor(x + nOffsetX, y + nOffsetY, crMix);
						}
					}
				}
			}
			else
			{
				if (m_pDib->GetBitCount() == 32 && m_pDib->HasAlpha())
				{
					for (int y = 0 ; y < (int)PasteDib.GetHeight() ; y++)
					{
						for (int x = 0 ; x < (int)PasteDib.GetWidth() ; x++)
						{
							COLORREF crSource = PasteDib.GetPixelColor(x, y);
							crSource |= 0xFF000000;
							m_pDib->SetPixelColor32Alpha(x + nOffsetX, y + nOffsetY, crSource);
						}
					}
				}
				else
				{
					for (int y = 0 ; y < (int)PasteDib.GetHeight() ; y++)
					{
						for (int x = 0 ; x < (int)PasteDib.GetWidth() ; x++)
						{
							COLORREF crSource = PasteDib.GetPixelColor(x, y);
							m_pDib->SetPixelColor(x + nOffsetX, y + nOffsetY, crSource);
						}
					}
				}
			}
		}
		
		// End Wait Cursor and End Force Cursor
		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		// Update
		m_DocRect.bottom = m_pDib->GetHeight();
		m_DocRect.right = m_pDib->GetWidth();
		UpdateAlphaRenderedDib();
		SetModifiedFlag();
		SetDocumentTitle();
		UpdateAllViews(NULL);
		UpdateImageInfo();

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditPasteIntoTopleft() 
{
	if (IsDibReadyForCommand(ID_EDIT_PASTE_INTO_TOPLEFT))
		EditPasteInto(0, GetView()->m_ptLastRightClick);
}

void CPictureDoc::OnUpdateEditPasteIntoTopleft(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_PASTE_INTO_TOPLEFT)		&&
					DoEnableCommand()												&&
					(::IsClipboardFormatAvailable(CF_DIB)							||
					::IsClipboardFormatAvailable(CF_ENHMETAFILE)));
}

void CPictureDoc::OnEditPasteIntoTopright() 
{
	if (IsDibReadyForCommand(ID_EDIT_PASTE_INTO_TOPRIGHT))
		EditPasteInto(1, GetView()->m_ptLastRightClick);
}

void CPictureDoc::OnUpdateEditPasteIntoTopright(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_PASTE_INTO_TOPRIGHT)		&&
					DoEnableCommand()												&&
					(::IsClipboardFormatAvailable(CF_DIB)							||
					::IsClipboardFormatAvailable(CF_ENHMETAFILE)));
}

void CPictureDoc::OnEditPasteIntoBottomleft() 
{
	if (IsDibReadyForCommand(ID_EDIT_PASTE_INTO_BOTTOMLEFT))
		EditPasteInto(2, GetView()->m_ptLastRightClick);
}

void CPictureDoc::OnUpdateEditPasteIntoBottomleft(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_PASTE_INTO_BOTTOMLEFT)	&&
					DoEnableCommand()												&&
					(::IsClipboardFormatAvailable(CF_DIB)							||
					::IsClipboardFormatAvailable(CF_ENHMETAFILE)));
}

void CPictureDoc::OnEditPasteIntoBottomright() 
{
	if (IsDibReadyForCommand(ID_EDIT_PASTE_INTO_BOTTOMRIGHT))
		EditPasteInto(3, GetView()->m_ptLastRightClick);
}

void CPictureDoc::OnUpdateEditPasteIntoBottomright(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_PASTE_INTO_BOTTOMRIGHT)	&&
					DoEnableCommand()												&&
					(::IsClipboardFormatAvailable(CF_DIB)							||
					::IsClipboardFormatAvailable(CF_ENHMETAFILE)));
}

void CPictureDoc::OnEditPasteIntoFile() 
{
	if (IsDibReadyForCommand(ID_EDIT_PASTE_INTO_FILE))
	{
		GetView()->ForceCursor();
		CNoVistaFileDlg fd(	TRUE,
						_T("txt"),
						_T(""),
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, // file must exist and hide the read-only check box
						_T("Text Files (*.txt)|*.txt||"));
		if (fd.DoModal() == IDOK)
		{	
			BeginWaitCursor();
			LPBYTE pData = NULL;
			try
			{
				// BOM
				const unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};

				// Open File
				CFile f(fd.GetPathName(),
						CFile::modeRead |
						CFile::shareDenyWrite);
				DWORD dwLength = (DWORD)f.GetLength();
				if (dwLength == 0)
				{
					EndWaitCursor();
					GetView()->ForceCursor(FALSE);
					return;
				}

				// Allocate Buffer
				pData = new BYTE [dwLength];
				if (!pData)
				{
					EndWaitCursor();
					GetView()->ForceCursor(FALSE);
					return;
				}
				
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

				// Get Lines
				CString sLine;
				CStringArray Lines;
				int nIndex;
				s.TrimLeft(); // Remove leading spaces (including '\r' or '\n')
				while ((nIndex = s.FindOneOf(_T("\r\n"))) >= 0)
				{
					sLine = s.Left(nIndex);
					Lines.Add(sLine); 
					s.Delete(0, nIndex);
					s.TrimLeft(); // Remove leading spaces (including '\r' or '\n')
				}

				// Last Line
				if (s != _T(""))
					Lines.Add(s);

				// Process Lines
				for (int i = 0 ; i <= Lines.GetUpperBound() ; i++)
				{
					// Reference corner
					sLine = Lines[i];
					sLine.MakeLower();
					sLine.Replace(_T(","), _T("."));
					int nCorner = 0; // Top-Left is the default
					if (sLine.Find(_T("top")) >= 0 && sLine.Find(_T("right")) >= 0)
						nCorner = 1;
					else if (sLine.Find(_T("bottom")) >= 0 && sLine.Find(_T("left")) >= 0)
						nCorner = 2;
					else if (sLine.Find(_T("bottom")) >= 0 && sLine.Find(_T("right")) >= 0)
						nCorner = 3;

					// Units
					int nCoordinateUnit = COORDINATES_PIX; // Pixels is the default
					if (sLine.Find(_T("in")) >= 0)
						nCoordinateUnit = COORDINATES_INCH;
					else if (sLine.Find(_T("cm")) >= 0)
						nCoordinateUnit = COORDINATES_CM;
					else if (sLine.Find(_T("mm")) >= 0)
						nCoordinateUnit = COORDINATES_MM;

					// To pixel coordinates
					int nXDpi = 0;
					if (m_pDib)
						nXDpi = m_pDib->GetXDpi();
					if (nXDpi <= 0)
						nXDpi = DEFAULT_DPI;
					int nYDpi = 0;
					if (m_pDib)
						nYDpi = m_pDib->GetYDpi();
					if (nYDpi <= 0)
						nYDpi = DEFAULT_DPI;
					CPoint pt(0, 0);
					double dNum;
					CString sNumber;
					int nStartIndex = sLine.FindOneOf(_T("+-.0123456789"));
					if (nStartIndex >= 0)
					{
						// X
						s = sLine.Mid(nStartIndex);
						sNumber = s.SpanIncluding(_T("+-.0123456789"));
						if (_stscanf(sNumber, _T("%lf"), &dNum) > 0)
						{
							switch (nCoordinateUnit)
							{
								case COORDINATES_INCH :
									pt.x = Round(dNum * (double)nXDpi);
									break;
								case COORDINATES_CM :
									pt.x = Round(dNum * (double)nXDpi / 2.54);
									break;
								case COORDINATES_MM :
									pt.x = Round(dNum * (double)nXDpi / 25.4);
									break;
								default :
									pt.x = Round(dNum);
									break;
							}	
						}

						// Y
						s = s.Mid(sNumber.GetLength());
						nStartIndex = s.FindOneOf(_T("+-.0123456789"));
						if (nStartIndex >= 0)
						{
							s = s.Mid(nStartIndex);
							sNumber = s.SpanIncluding(_T("+-.0123456789"));
							if (_stscanf(sNumber, _T("%lf"), &dNum) > 0)
							{
								switch (nCoordinateUnit)
								{
									case COORDINATES_INCH :
										pt.y = Round(dNum * (double)nYDpi);
										break;
									case COORDINATES_CM :
										pt.y = Round(dNum * (double)nYDpi / 2.54);
										break;
									case COORDINATES_MM :
										pt.y = Round(dNum * (double)nYDpi / 25.4);
										break;
									default :
										pt.y = Round(dNum);
										break;
								}	
							}
						}
					}

					// Adjust m_pDib coordinates origin
					switch (nCorner)
					{
						case 1 : // To top-right origin
							if (m_pDib)
								pt.x = m_pDib->GetWidth() - 1 - pt.x;
							break;
						case 2 : // To bottom-left origin
							if (m_pDib)
								pt.y = m_pDib->GetHeight() - 1 - pt.y;
							break;
						case 3 : // To bottom-right origin
							if (m_pDib)
							{
								pt.x = m_pDib->GetWidth() - 1 - pt.x;
								pt.y = m_pDib->GetHeight() - 1 - pt.y;
							}
							break;
						default :
							break;
					}

					// Paste Into
					pt.x = MIN(MAX(pt.x,-SHRT_MAX),SHRT_MAX);
					pt.y = MIN(MAX(pt.y,-SHRT_MAX),SHRT_MAX);
					EditPasteInto(nCorner, pt, 0, FALSE);
				}

				// Free
				delete [] pData;
			}
			catch (CFileException* e)
			{
				if (pData)
					delete [] pData;
				e->ReportError();
				e->Delete();
			}
			EndWaitCursor();
		}
		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::OnUpdateEditPasteIntoFile(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_PASTE_INTO_FILE)	&&
					DoEnableCommand()										&&
					(::IsClipboardFormatAvailable(CF_DIB)					||
					::IsClipboardFormatAvailable(CF_ENHMETAFILE)));
}

void CPictureDoc::OnEditPasteIntoFileHelp() 
{
	GetView()->ForceCursor();
	::AfxMessageBox(ML_STRING(1816, "Create a text file where each line represents a Paste\ncommand at the given position:\n  1. Origin: top left, top right, bottom left, bottom right\n  2. horizontal and vertical positions with sign and unit\n      (px, in, cm, mm)\n\nExample:\n\ntop left 1.57cm -1.32cm\nbottom right 12.1mm 5.5mm"), MB_OK | MB_ICONINFORMATION);
	GetView()->ForceCursor(FALSE);
}

BOOL CPictureDoc::EditAddBorders() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_ADD_BORDERS))
			return FALSE;

		// Force Cursor
		GetView()->ForceCursor();

		// Add Borders Dialog
		CAddBordersDlg dlg(GetView());
		if (m_pDib->GetBitCount() <= 8)
		{
			dlg.m_uiBorderIndex = 0;
			dlg.m_crBorder = RGB(	m_pDib->GetColors()->rgbRed,
									m_pDib->GetColors()->rgbGreen,
									m_pDib->GetColors()->rgbBlue);
		}
		else
			dlg.m_crBorder = 0;
		if (dlg.DoModal() == IDOK)
		{
			if (dlg.m_uiBottomBorder == 0	&&
				dlg.m_uiTopBorder == 0		&&
				dlg.m_uiLeftBorder == 0		&&
				dlg.m_uiRightBorder == 0)
			{
				GetView()->ForceCursor(FALSE);
				return FALSE;
			}

			BeginWaitCursor();
			
			AddUndo();

			// Do Add Borders
			if (!m_pDib->AddBorders(dlg.m_uiLeftBorder,
									dlg.m_uiTopBorder,
									dlg.m_uiRightBorder,
									dlg.m_uiBottomBorder,
									m_pDib->GetBitCount() <= 8 ?
										dlg.m_uiBorderIndex :
										dlg.m_crBorder,
									NULL,
									GetView(),
									TRUE))
			{
				Undo();
				EndWaitCursor();
				GetView()->ForceCursor(FALSE);
				return FALSE;
			}
			else
				EndWaitCursor();
		}
		else
		{
			GetView()->ForceCursor(FALSE);
			return FALSE;
		}

		m_DocRect.bottom = m_pDib->GetHeight();
		m_DocRect.right = m_pDib->GetWidth();

		GetView()->ForceCursor(FALSE);

		UpdateAlphaRenderedDib();
		SetModifiedFlag();
		SetDocumentTitle();
		UpdateAllViews(NULL);
		UpdateImageInfo();

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditAddBorders() 
{
	EditAddBorders();
}

void CPictureDoc::OnUpdateEditAddBorders(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_ADD_BORDERS)	&&
					DoEnableCommand());
}

void CPictureDoc::OnEditSoftBorders() 
{
	if (m_pSoftBordersDlg)
	{
		// m_pSoftBordersDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pSoftBordersDlg->Close();
	}
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_SOFT_BORDERS))
			return;

		m_pSoftBordersDlg = new CSoftBordersDlg(GetView());
		m_pSoftBordersDlg->ShowWindow(SW_RESTORE);
	}
}

void CPictureDoc::OnUpdateEditSoftBorders(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_SOFT_BORDERS)	&&
					m_pDib												&&
					!(m_SlideShowThread.IsSlideshowRunning() ||
					m_bDoRestartSlideshow)								&&
					!m_GifAnimationThread.IsAlive()						&&
					!m_bMetadataModified								&&
					!m_pHLSDlg											&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_bCrop);
	pCmdUI->SetCheck(m_pSoftBordersDlg != NULL ? 1 : 0);
}

void CPictureDoc::OnEditTextToAlpha() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TEXT_TO_ALPHA))
			return;

		// Begin Wait Cursor
		BeginWaitCursor();

		// A Duplicated Dib is created
		AddUndo();

		// Make transparent black text
		m_pDib->ConvertTo32bits(GetView(), TRUE);
		m_pDib->Negative(GetView(), TRUE);
		m_pDib->GrayToAlphaChannel(RGB(255, 0, 0), NULL, GetView(), TRUE);

		// Update Alpha Rendered Dib
		UpdateAlphaRenderedDib();

		// End Wait Cursor
		EndWaitCursor();

		// Set Modified Flag
		SetModifiedFlag();
		
		// Set Document Title
		SetDocumentTitle();

		// Invalidate
		InvalidateAllViews(FALSE);

		// Update Image Info
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTextToAlpha(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TEXT_TO_ALPHA) &&
					DoEnableCommand());
}

BOOL CPictureDoc::EditGrayscale(BOOL bShowMessageBoxOnError) 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_GRAYSCALE))
			return FALSE;

		// Show Cursor & Begin Wait Cursor
		GetView()->ForceCursor();
		BeginWaitCursor();

		// A Duplicated Dib is created
		AddUndo();

		// Calc. Grayscale
		m_pDib->Grayscale(GetView(), TRUE);

		// To 8bpp
		if (m_pDib->GetBitCount() > 8 && !(m_pDib->GetBitCount() == 32 && m_pDib->HasAlpha()))
		{
			RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[256];
			CDib::FillGrayscaleColors(pColors, 256);
			m_pDib->CreatePaletteFromColors(256, pColors);
			m_pDib->ConvertTo8bits(m_pDib->GetPalette(), GetView(), TRUE);
			delete [] pColors;
		}

		// End Wait Cursor and End Force Cursor
		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		// Update Alpha Rendered Dib
		UpdateAlphaRenderedDib();

		// Set Modified Flag
		SetModifiedFlag();
		
		// Set Document Title
		SetDocumentTitle();

		// Invalidate
		InvalidateAllViews(FALSE);

		// Update Image Info
		UpdateImageInfo();

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditGrayscale() 
{
	EditGrayscale(TRUE);
}

void CPictureDoc::OnUpdateEditGrayscale(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_GRAYSCALE) &&
					DoEnableCommand());
}

BOOL CPictureDoc::EditNegative(BOOL bShowMessageBoxOnError) 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_NEGATIVE))
			return FALSE;

		// Show Cursor & Begin Wait Cursor
		GetView()->ForceCursor();
		BeginWaitCursor();

		// A Duplicated Dib is created
		AddUndo();

		// Calc. Negative
		m_pDib->Negative(GetView(), TRUE);

		// End Wait Cursor and End Force Cursor
		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		// Update Alpha Rendered Dib
		UpdateAlphaRenderedDib();

		// Set Modified Flag
		SetModifiedFlag();
		
		// Set Document Title
		SetDocumentTitle();

		// Invalidate
		InvalidateAllViews(FALSE);

		// Update Image Info
		UpdateImageInfo();

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditNegative() 
{
	EditNegative(TRUE);
}

void CPictureDoc::OnUpdateEditNegative(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_NEGATIVE) &&
					DoEnableCommand());
}

void CPictureDoc::OnEditUpdateExifthumb() 
{
	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_UPDATE_EXIFTHUMB))
			return;

		BeginWaitCursor();

		// Be Sure We Are Not Working On This File
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill();
		}

		// Update Thumb
		if (CDib::UpdateExifWidthHeightThumb(m_sFileName, TRUE))
		{
			EndWaitCursor();
			m_pDib->JPEGLoadMetadata(m_sFileName);
			CDib::AutoOrientateThumbnailDib(m_pDib);
			SetDocumentTitle();
			UpdateImageInfo();
			::AfxMessageBox(ML_STRING(1287, "EXIF Thumbnail Was Successfully Updated"), MB_OK | MB_ICONINFORMATION);
		}
		else
			EndWaitCursor();

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditUpdateExifthumb(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_UPDATE_EXIFTHUMB) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					m_pDib->GetExifInfo()->bHasExif &&
					m_pDib->GetExifInfo()->ThumbnailPointer);
}

void CPictureDoc::OnEditAddExifthumb() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_ADD_EXIFTHUMB))
			return;
		
		BeginWaitCursor();

		// Kill Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Add Thumb
		BOOL res = CDib::AddEXIFThumbnail(	m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		EndWaitCursor();

		if (res)
		{
			// Remove and Rename Files
			try
			{
				CFile::Remove(m_sFileName);
				CFile::Rename(sTempFileName, m_sFileName);
				m_pDib->JPEGLoadMetadata(m_sFileName);
				CDib::AutoOrientateThumbnailDib(m_pDib);
				SetDocumentTitle();
				UpdateImageInfo();
				::AfxMessageBox(ML_STRING(1288, "EXIF Thumbnail Was Successfully Added"), MB_OK | MB_ICONINFORMATION);
			}
			catch (CFileException* e)
			{
				::DeleteFile(sTempFileName);

				DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
				if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
				{
					CString str(ML_STRING(1255, "The file is read only\n"));
					TRACE(str);
					if (bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
				}
				else
					::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

				e->Delete();
			}
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditAddExifthumb(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_ADD_EXIFTHUMB) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(!m_pDib->GetExifInfo()->bHasExif ||
					!m_pDib->GetExifInfo()->ThumbnailPointer));
}

void CPictureDoc::OnEditRemoveExifthumb() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_EXIFTHUMB))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Remove Thumb
		BOOL res = CDib::RemoveEXIFThumbnail(m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (res)
		{
			// Remove and Rename Files
			try
			{
				CFile::Remove(m_sFileName);
				CFile::Rename(sTempFileName, m_sFileName);
				m_pDib->JPEGLoadMetadata(m_sFileName);
				SetDocumentTitle();
				UpdateImageInfo();
				::AfxMessageBox(ML_STRING(1289, "EXIF Thumbnail Was Successfully Removed"), MB_OK | MB_ICONINFORMATION);
			}
			catch (CFileException* e)
			{
				::DeleteFile(sTempFileName);

				DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
				if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
				{
					CString str(ML_STRING(1255, "The file is read only\n"));
					TRACE(str);
					if (bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
				}
				else
					::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

				e->Delete();
			}
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveExifthumb(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_EXIFTHUMB) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					m_pDib->GetExifInfo()->bHasExif &&
					m_pDib->GetExifInfo()->ThumbnailPointer);
}

void CPictureDoc::OnEditClearExifOrientate() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_CLEAR_EXIF_ORIENTATE))
			return;

		BeginWaitCursor();

		// Be Sure We Are Not Working On This File
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill();
		}

		// Auto Orientate
		if (CDib::JPEGAutoOrientate(m_sFileName,
									((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
									bShowMessageBoxOnError,
									GetView(),
									TRUE))
		{
			EndWaitCursor();
			m_pDib->JPEGLoadMetadata(m_sFileName);
			SetDocumentTitle();
			UpdateImageInfo();
			::AfxMessageBox(ML_STRING(1290, "EXIF Orientation Has Been Successfully Cleared"), MB_OK | MB_ICONINFORMATION);
		}
		else
			EndWaitCursor();

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditClearExifOrientate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_CLEAR_EXIF_ORIENTATE) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					CDib::DoAutoOrientate(m_pDib));
}

void CPictureDoc::OnEditRemoveExif() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_EXIF))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Remove EXIF Section
		BOOL res = CDib::JPEGRemoveSection(	M_EXIF_XMP,
											CMetadata::m_ExifHeader,
											EXIF_HEADER_SIZE,
											m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (!res)
		{
			::AfxMessageBox(ML_STRING(1291, "No EXIF Section Found."), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (::AfxMessageBox(ML_STRING(1292, "Do You Really Want To Remove the EXIF Section?"), MB_YESNO) == IDYES)
			{
				// Remove and Rename Files
				try
				{
					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);
					m_pDib->JPEGLoadMetadata(m_sFileName);
					SetDocumentTitle();
					UpdateImageInfo();
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);

					DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
					}
					else
						::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

					e->Delete();
				}
			}
			else
				::DeleteFile(sTempFileName);
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveExif(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_EXIF) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(m_pDib && m_pDib->GetExifInfo()->bHasExifSection));
}

void CPictureDoc::OnEditRemoveIcc() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_ICC))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Remove ICC Section
		BOOL res = CDib::JPEGRemoveSection(	M_ICC,
											CMetadata::m_IccHeader,
											ICC_HEADER_SIZE,
											m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (!res)
		{
			::AfxMessageBox(ML_STRING(1293, "No ICC Section Found."), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (::AfxMessageBox(ML_STRING(1294, "Do You Really Want To Remove the ICC Section?"), MB_YESNO) == IDYES)
			{
				// Remove and Rename Files
				try
				{
					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);
					m_pDib->JPEGLoadMetadata(m_sFileName);
					SetDocumentTitle();
					UpdateImageInfo();
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);

					DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
					}
					else
						::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

					e->Delete();
				}
			}
			else
				::DeleteFile(sTempFileName);
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveIcc(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_ICC) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(m_pDib && m_pDib->GetMetadata()->HasIcc()));	
}

void CPictureDoc::OnEditRemoveXmp() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_XMP))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Remove XMP Section
		BOOL res = CDib::JPEGRemoveSection(	M_EXIF_XMP,
											CMetadata::m_XmpHeader,
											XMP_HEADER_SIZE,
											m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (!res)
		{
			::AfxMessageBox(ML_STRING(1295, "No XMP Section Found."), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (::AfxMessageBox(ML_STRING(1296, "Do You Really Want To Remove the XMP Section?"), MB_YESNO) == IDYES)
			{
				// Remove and Rename Files
				try
				{
					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);
					m_pDib->JPEGLoadMetadata(m_sFileName);
					SetDocumentTitle();
					UpdateImageInfo();
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);

					DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
					}
					else
						::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

					e->Delete();
				}
			}
			else
				::DeleteFile(sTempFileName);
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveXmp(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_XMP) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(m_pDib && m_pDib->GetMetadata()->HasXmp()));
}

void CPictureDoc::OnEditRemoveJfif() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_JFIF))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Remove JFIF Section
		BOOL res = CDib::JPEGRemoveSection(	M_JFIF,
											NULL,
											0,
											m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (!res)
		{
			::AfxMessageBox(ML_STRING(1297, "No JFIF Section Found."), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (::AfxMessageBox(ML_STRING(1298, "Do You Really Want To Remove the JFIF Section?"), MB_YESNO) == IDYES)
			{
				// Remove and Rename Files
				try
				{
					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);
					m_pDib->JPEGLoadMetadata(m_sFileName);
					SetDocumentTitle();
					UpdateImageInfo();
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);

					DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
					}
					else
						::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

					e->Delete();
				}
			}
			else
				::DeleteFile(sTempFileName);
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveJfif(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_JFIF) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(m_pDib && m_pDib->GetMetadata()->HasJfif()));
}

void CPictureDoc::OnEditRemoveIptc() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_IPTC))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Remove IPTC Section
		BOOL res = CDib::JPEGRemoveSection(	M_IPTC,
											NULL,
											0,
											m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (!res)
		{
			::AfxMessageBox(ML_STRING(1299, "No IPTC Section Found."), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (::AfxMessageBox(ML_STRING(1300, "Do You Really Want To Remove the IPTC Section?"), MB_YESNO) == IDYES)
			{
				// Remove and Rename Files
				try
				{
					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);
					m_pDib->JPEGLoadMetadata(m_sFileName);
					SetDocumentTitle();
					UpdateImageInfo();
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);

					DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
					}
					else
						::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

					e->Delete();
				}
			}
			else
				::DeleteFile(sTempFileName);
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveIptc(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_IPTC) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(m_pDib && m_pDib->GetMetadata()->HasIptcLegacy()));	
}

void CPictureDoc::OnEditRemoveOtherApp()
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_OTHERAPP))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName1 = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);
		CString sTempFileName2 = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sTempFileName1);

		BOOL res1, res2;

		// Remove APP3 - APP12 Sections
		res1 = CDib::JPEGRemoveSections(M_APP3,
										M_APP12,
										m_sFileName,
										sTempFileName1,
										bShowMessageBoxOnError);

		// Remove APP14 - APP15 Sections
		if (res1)
		{
			res2 = CDib::JPEGRemoveSections(M_APP14,
											M_APP15,
											sTempFileName1,
											sTempFileName2,
											bShowMessageBoxOnError);
		}
		else
		{
			res2 = CDib::JPEGRemoveSections(M_APP14,
											M_APP15,
											m_sFileName,
											sTempFileName1,
											bShowMessageBoxOnError);
		}

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (!res1 && !res2)
		{
			::AfxMessageBox(ML_STRING(1301, "No Other APP Sections Found."), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (::AfxMessageBox(ML_STRING(1302, "Do You Really Want To Remove the Other APP Sections?"), MB_YESNO) == IDYES)
			{
				// Remove and Rename Files
				try
				{
					CFile::Remove(m_sFileName);
					if (res1 && res2)
					{
						CFile::Rename(sTempFileName2, m_sFileName);
						CFile::Remove(sTempFileName1);
					}
					else
						CFile::Rename(sTempFileName1, m_sFileName);
					m_pDib->JPEGLoadMetadata(m_sFileName);
					SetDocumentTitle();
					UpdateImageInfo();
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName1);
					::DeleteFile(sTempFileName2);

					DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
					}
					else
						::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

					e->Delete();
				}
			}
			else
			{
				::DeleteFile(sTempFileName1);
				::DeleteFile(sTempFileName2);
			}
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveOtherApp(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_OTHERAPP) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(m_pDib && m_pDib->GetMetadata()->HasOtherAppSections()));
}

void CPictureDoc::OnEditRemoveCom() 
{
	const BOOL bShowMessageBoxOnError = TRUE;

	if (CDib::IsJPEG(m_sFileName))
	{
		if (IsModified())
		{
			::AfxMessageBox(ML_STRING(1286, "Please Save Before Performing This Operation."));
			return;
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_REMOVE_COM))
			return;

		BeginWaitCursor();

		// Start Killing Jpeg Thread
		BOOL bWasRunning = FALSE;
		if (m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_JpegThread.Kill_NoBlocking();
		}

		// Temporary File
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);

		// Remove COM Section
		BOOL res = CDib::JPEGRemoveSection(	M_COM,
											NULL,
											0,
											m_sFileName,
											sTempFileName,
											bShowMessageBoxOnError);

		// Be Sure We Are Not Working On This File
		m_JpegThread.WaitDone_Blocking();

		EndWaitCursor();

		if (!res)
		{
			::AfxMessageBox(ML_STRING(1303, "No COM Section Found."), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			if (::AfxMessageBox(ML_STRING(1304, "Do You Really Want To Remove the COM Section?"), MB_YESNO) == IDYES)
			{
				// Remove and Rename Files
				try
				{
					CFile::Remove(m_sFileName);
					CFile::Rename(sTempFileName, m_sFileName);
					m_pDib->JPEGLoadMetadata(m_sFileName);
					SetDocumentTitle();
					UpdateImageInfo();
				}
				catch (CFileException* e)
				{
					::DeleteFile(sTempFileName);

					DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
					}
					else
						::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

					e->Delete();
				}
			}
			else
				::DeleteFile(sTempFileName);
		}

		if (bWasRunning && m_bDoJPEGGet)
			JPEGGet();
	}
}

void CPictureDoc::OnUpdateEditRemoveCom(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REMOVE_COM) &&
					DoEnableCommand() &&
					CDib::IsJPEG(m_sFileName) &&
					(m_pDib && m_pDib->GetMetadata()->HasCom()));
}

void CPictureDoc::OnEditRedeye() 
{
	if (m_pRedEyeDlg)
	{
		// m_pRedEyeDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pRedEyeDlg->Close();
	}
	else
	{
		if (m_bDoRedEyeColorPickup)
		{
			m_bDoRedEyeColorPickup = FALSE;
			GetView()->ForceCursor(FALSE);
			::AfxGetMainFrame()->StatusText();
		}
		else
		{
			// Wait and schedule command if dib not fully loaded!
			if (!IsDibReadyForCommand(ID_EDIT_REDEYE))
				return;

			// Cancel Zoom
			CancelZoomTool();

			// Invalidate
			InvalidateAllViews(FALSE);

			if (m_pDib && m_pDib->IsValid())
			{
				GetView()->ForceCursor();
				m_bDoRedEyeColorPickup = TRUE;
				::AfxGetMainFrame()->StatusText(ML_STRING(1228, "*** Click In The Middle Of A Red-Eye ***"));
				GetView()->UpdateCursor();
			}
		}
	}
}

void CPictureDoc::OnUpdateEditRedeye(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_REDEYE)		&&
					m_pDib												&&
					!(m_SlideShowThread.IsSlideshowRunning() ||
					m_bDoRestartSlideshow)								&&
					!m_GifAnimationThread.IsAlive()						&&
					!m_bMetadataModified								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pHLSDlg											&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_bCrop											&&
					!m_bPrintPreviewMode);
	pCmdUI->SetCheck((m_pRedEyeDlg != NULL || m_bDoRedEyeColorPickup) ? 1 : 0);
}

BOOL CPictureDoc::IsClickPointRedEye(CPoint point, CDib* pSrcDib) 
{
	// Adjust Point Offset
	point += GetView()->GetScrollPosition();
	point -= GetView()->m_ZoomRect.TopLeft();
	point.x = (int)((double)point.x / m_dZoomFactor);
	point.y = (int)((double)point.y / m_dZoomFactor);
	point.y = pSrcDib->GetHeight() - point.y - 1;

	// Clicked point is supposed to be red-eye or bright white reflection.
	// If it's not the case do not give up, check the neighbours!
	if (!CRedEyeDlg::IsRedEyePixel(	pSrcDib->GetPixelColor(point.x, point.y),
									REDEYE_HUERANGE_START,
									REDEYE_HUERANGE_END,
									REDEYE_PICK_REDRATIO) &&		
		!CRedEyeDlg::IsBrightWhite(pSrcDib->GetPixelColor(point.x, point.y)))
	{
		int nRedEyeCount = 0;
		int nBrightWhiteCount = 0;
		CRedEyeDlg::CheckNeighbours(0,
									0,
									pSrcDib,
									&nRedEyeCount,
									&nBrightWhiteCount,
									point.x,
									point.y,
									REDEYE_HUERANGE_START,
									REDEYE_HUERANGE_END,
									REDEYE_PICK_REDRATIO);
		if ((nRedEyeCount + nBrightWhiteCount) >= 3 || nBrightWhiteCount >= 2)
			return TRUE;
		else
			return FALSE;
	}
	else
		return TRUE;
}

void CPictureDoc::OnEditTo1bit() 
{
	if (m_pMonochromeConversionDlg)
	{
		// m_pMonochromeConversionDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pMonochromeConversionDlg->Close();
	}
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO1BIT))
			return;

		m_pMonochromeConversionDlg = new CMonochromeConversionDlg(GetView());
		m_pMonochromeConversionDlg->ShowWindow(SW_RESTORE);
	}
}

void CPictureDoc::OnUpdateEditTo1bit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO1BIT) &&
					DoEnableCommand() &&
					(m_pDib && m_pDib->GetBitCount() != 1));
}

void CPictureDoc::OnEditTo1bitDitherErrDiff() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO1BIT_DITHER_ERRDIFF))
			return;

		GetView()->ForceCursor();
		BeginWaitCursor();
		AddUndo();
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			m_pDib->RenderAlphaWithSrcBackground();
			m_pDib->SetAlpha(FALSE);
		}
		m_pDib->ConvertTo1bitDitherErrDiff(0, GetView(), TRUE); // 0 = Floyd-Steinberg
		EndWaitCursor();
		
		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo1bitDitherErrDiff(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO1BIT_DITHER_ERRDIFF) &&
					DoEnableCommand() &&
					(m_pDib && m_pDib->GetBitCount() != 1));
}

void CPictureDoc::OnEditTo4bits() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO4BITS))
			return;

		GetView()->ForceCursor();

		// Popup Dialog
		CDecreaseBppDlg dlg(2, 16, GetView());
		if (m_pDib->GetNumColors() > 0 &&
			m_pDib->GetNumColors() <= 16)
			dlg.m_uiMaxColors = m_pDib->GetNumColors();
		else
			dlg.m_uiMaxColors = m_uiMaxColors16;
		dlg.m_bDitherColorConversion = m_bDitherColorConversion;
		if (dlg.DoModal() != IDOK)
		{
			GetView()->ForceCursor(FALSE);
			return;
		}
		else
		{
			m_uiMaxColors16 = dlg.m_uiMaxColors;
			m_bDitherColorConversion = dlg.m_bDitherColorConversion;
			::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
											_T("MaxColors16"),
											m_uiMaxColors16);
			::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
											_T("DitherColorConversion"),
											m_bDitherColorConversion);
		}

		BeginWaitCursor();
		AddUndo();

		// Remove Alpha
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			m_pDib->RenderAlphaWithSrcBackground();
			m_pDib->SetAlpha(FALSE);
		}

		// Convert
		CQuantizer Quantizer(m_uiMaxColors16, 8);
		Quantizer.ProcessImage(m_pDib, GetView(), TRUE);
		RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[m_uiMaxColors16];
		Quantizer.SetColorTable(pColors);
		m_pDib->CreatePaletteFromColors(m_uiMaxColors16, pColors);
		if (m_bDitherColorConversion)
			m_pDib->ConvertTo4bitsErrDiff(m_pDib->GetPalette(), GetView(), TRUE);
		else
			m_pDib->ConvertTo4bits(m_pDib->GetPalette(), GetView(), TRUE);
		delete [] pColors;
		
		EndWaitCursor();

		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo4bits(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO4BITS) &&
					DoEnableCommand());
}

void CPictureDoc::OnEditTo8bits() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO8BITS))
			return;

		GetView()->ForceCursor();

		// Popup Dialog
		CDecreaseBppDlg dlg(2, 256, GetView());
		if (m_pDib->GetNumColors() > 0)
			dlg.m_uiMaxColors = m_pDib->GetNumColors();
		else
			dlg.m_uiMaxColors = m_uiMaxColors256;
		dlg.m_bDitherColorConversion = m_bDitherColorConversion;
		if (dlg.DoModal() != IDOK)
		{
			GetView()->ForceCursor(FALSE);
			return;
		}
		else
		{
			m_uiMaxColors256 = dlg.m_uiMaxColors;
			m_bDitherColorConversion = dlg.m_bDitherColorConversion;
			::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
											_T("MaxColors256"),
											m_uiMaxColors256);
			::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
											_T("DitherColorConversion"),
											m_bDitherColorConversion);
		}

		BeginWaitCursor();
		AddUndo();

		// Remove Alpha
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			m_pDib->RenderAlphaWithSrcBackground();
			m_pDib->SetAlpha(FALSE);
		}

		// Convert
		CQuantizer Quantizer(m_uiMaxColors256, 8);
		Quantizer.ProcessImage(m_pDib, GetView(), TRUE);
		RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[m_uiMaxColors256];
		Quantizer.SetColorTable(pColors);
		m_pDib->CreatePaletteFromColors(m_uiMaxColors256, pColors);
		if (m_bDitherColorConversion)
			m_pDib->ConvertTo8bitsErrDiff(m_pDib->GetPalette(), GetView(), TRUE);
		else
			m_pDib->ConvertTo8bits(m_pDib->GetPalette(), GetView(), TRUE);
		delete [] pColors;
		
		EndWaitCursor();

		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo8bits(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO8BITS) &&
					DoEnableCommand());
}

void CPictureDoc::OnEditTo15bits() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO16BITS))
			return;

		GetView()->ForceCursor();
		BeginWaitCursor();
		AddUndo();
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			m_pDib->RenderAlphaWithSrcBackground();
			m_pDib->SetAlpha(FALSE);
		}
		m_pDib->ConvertTo15bits(GetView(), TRUE);
		EndWaitCursor();

		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo15bits(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO15BITS) &&
					DoEnableCommand() &&
					(m_pDib && !m_pDib->IsRgb16_555()));
}

void CPictureDoc::OnEditTo16bits()
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO16BITS))
			return;

		GetView()->ForceCursor();
		BeginWaitCursor();
		AddUndo();
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			m_pDib->RenderAlphaWithSrcBackground();
			m_pDib->SetAlpha(FALSE);
		}
		m_pDib->ConvertTo16bitsMasks(	0xF800,
										0x07E0,
										0x001F,
										GetView(),
										TRUE);
		EndWaitCursor();

		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo16bits(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO16BITS) &&
					DoEnableCommand() &&
					(m_pDib && !m_pDib->IsRgb16_565()));
}

void CPictureDoc::OnEditTo24bits()
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO24BITS))
			return;

		GetView()->ForceCursor();
		BeginWaitCursor();
		AddUndo();
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			m_pDib->RenderAlphaWithSrcBackground();
			m_pDib->SetAlpha(FALSE);
		}
		m_pDib->ConvertTo24bits(GetView(), TRUE);
		EndWaitCursor();

		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo24bits(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO24BITS) &&
					DoEnableCommand() &&
					(m_pDib && m_pDib->GetBitCount() != 24));
}

void CPictureDoc::OnEditTo32bits() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO32BITS))
			return;

		GetView()->ForceCursor();
		BeginWaitCursor();
		AddUndo();
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			m_pDib->RenderAlphaWithSrcBackground(); // flatten with background
			m_pDib->AlphaOffset(255);				// remove alpha information by making it fully opaque
			m_pDib->SetAlpha(FALSE);				// clear alpha flag
			if (m_pDib->GetBMIH()->biSize == sizeof(BITMAPV4HEADER) ||
				m_pDib->GetBMIH()->biSize == sizeof(BITMAPV5HEADER))
			{
				LPBITMAPV4HEADER pBV4 = (LPBITMAPV4HEADER)m_pDib->GetBMI();
				pBV4->bV4AlphaMask = 0;				// clear alpha mask
			}
		}
		else
			m_pDib->ConvertTo32bits(GetView(), TRUE);
		EndWaitCursor();

		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo32bits(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO32BITS) &&
					DoEnableCommand() &&
					m_pDib &&
					((m_pDib->HasAlpha() &&
					m_pDib->GetBitCount() == 32) ||
					(m_pDib->GetBitCount() != 32)));
}

void CPictureDoc::OnEditTo32bitsAlpha() 
{
	if (m_pDib)
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_TO32BITS_ALPHA))
			return;

		GetView()->ForceCursor();
		BeginWaitCursor();
		AddUndo();
		m_pDib->ConvertTo32bitsAlpha(255, GetView(), TRUE);
		UpdateAlphaRenderedDib();
		EndWaitCursor();

		SetModifiedFlag();
		SetDocumentTitle();
		InvalidateAllViews(FALSE);
		GetView()->ForceCursor(FALSE);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdateEditTo32bitsAlpha(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_TO32BITS_ALPHA) &&
					DoEnableCommand() &&
					m_pDib &&
					((!m_pDib->HasAlpha() &&
					m_pDib->GetBitCount() == 32) ||
					(m_pDib->GetBitCount() != 32)));
}

void CPictureDoc::OnPlayPlay() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(this, TRUE))
		m_SlideShowThread.RunSlideshow();
}

void CPictureDoc::OnUpdatePlayPlay(CCmdUI* pCmdUI) 
{
	// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
	pCmdUI->SetCheck((m_SlideShowThread.IsSlideshowRunning() || m_bDoRestartSlideshow) ? 1 : 0);
}

void CPictureDoc::OnPlayStop() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(this, TRUE))
	{
		// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
		m_bDoRestartSlideshow = FALSE;
		m_SlideShowThread.PauseSlideshow();
	}
}

void CPictureDoc::OnUpdatePlayStop(CCmdUI* pCmdUI) 
{
	// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
	pCmdUI->SetCheck((!m_SlideShowThread.IsSlideshowRunning() && !m_bDoRestartSlideshow) ? 1 : 0);
}

void CPictureDoc::OnPlayLoop() 
{
	m_SlideShowThread.SetLoop(!m_SlideShowThread.IsLoop());
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowInfiniteLoop"),
									m_SlideShowThread.IsLoop());
}

void CPictureDoc::OnUpdatePlayLoop(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.IsLoop() ? 1 : 0);
}

void CPictureDoc::OnPlayRandom() 
{
	m_SlideShowThread.SetRandom(!m_SlideShowThread.IsRandom());
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("SlideShowRandomPlay"),
									m_SlideShowThread.IsRandom());
}

void CPictureDoc::OnUpdatePlayRandom(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_SlideShowThread.IsRandom() ? 1 : 0);
}

void CPictureDoc::JPEGGet()
{
	if (CDib::IsJPEG(m_sFileName))
	{
		// Get JPEG Pixel Alignment
		CDib::JPEGGetPixelAlignment(	m_sFileName,
										&m_nPixelAlignX,
										&m_nPixelAlignY,
										FALSE);

		// Make Sure Jpeg Thread Is Not Running!
		m_JpegThread.Kill();

		// Reset Vars
		m_JpegThread.ResetJpegCompressionQuality();
		m_bLoadFullJpegTransitionWorkerDone = FALSE;

		// ENTER CS
		m_csLoadFullJpegDib.EnterCriticalSection();

		if (m_pDib &&
			!m_pDib->IsValid() &&
			m_pDib->GetPreviewDib() &&
			m_pDib->GetPreviewDib()->IsValid())
		{
			m_bLoadFullJpegTransitionUI = TRUE;
			m_bFirstLoadFullJpegTransition = TRUE;
			GetView()->m_nLoadFullJpegTransitionStep = 0;
			GetView()->m_LoadFullJpegTransitionDib = *m_pDib->GetPreviewDib();
			GetView()->m_LoadFullJpegTransitionDib.SetShowMessageBoxOnError(FALSE);
		}
		else
			GetView()->m_LoadFullJpegTransitionDib.Free();

		// LEAVE CS
		m_csLoadFullJpegDib.LeaveCriticalSection();
		
		// Do a Full Load?
		m_JpegThread.SetDoFullLoad(m_pDib && !m_pDib->IsValid());

		// Set File Name
		m_JpegThread.SetFileName(m_sFileName);
		
		// Flush the GDI batch queue 
		::GdiFlush();
		
		// Start Jpeg Thread
		m_JpegThread.Start(THREAD_PRIORITY_BELOW_NORMAL);
	}
}

void CPictureDoc::UpdateAlphaRenderedDib()
{
	if (m_pDib)
	{
		if (m_pDib->GetPreviewDib()					&&
			m_pDib->GetPreviewDib()->IsValid()		&&
			m_pDib->HasAlpha()						&&
			m_pDib->GetBitCount() == 32				&&
			m_pDib->GetPreviewDib()->HasAlpha()		&&
			m_pDib->GetPreviewDib()->GetBitCount() == 32)
		{
			// Set Background Color
			if (m_bImageBackgroundColor)
			{
				m_pDib->SetBackgroundColor(m_crImageBackgroundColor);
				m_pDib->GetPreviewDib()->SetBackgroundColor(m_crImageBackgroundColor);
			}
			else
			{
				m_pDib->SetBackgroundColor(m_crBackgroundColor);
				m_pDib->GetPreviewDib()->SetBackgroundColor(m_crBackgroundColor);
			}

			// Render Alpha
			m_AlphaRenderedDib.RenderAlphaWithSrcBackground(m_pDib->GetPreviewDib());

			// Create Preview Dib of Alpha Rendered Dib,
			// this is not efficient (two times the same dib...)
			// but necessary to simplify the code!
			if (!m_AlphaRenderedDib.GetPreviewDib())
				CreatePreviewDib(&m_AlphaRenderedDib);
			m_AlphaRenderedDib.GetPreviewDib()->RenderAlphaWithSrcBackground(m_pDib->GetPreviewDib());
		}
		else if (m_pDib->IsValid())
		{
			if (m_pDib->HasAlpha() &&
				m_pDib->GetBitCount() == 32)
			{
				if (m_bImageBackgroundColor)
					m_pDib->SetBackgroundColor(m_crImageBackgroundColor);
				else
					m_pDib->SetBackgroundColor(m_crBackgroundColor);
				m_AlphaRenderedDib.RenderAlphaWithSrcBackground(m_pDib);
			}
		}
	}
}

void CPictureDoc::UpdateImageInfo(BOOL bUpdateFileInfoOnly/*=FALSE*/)
{
	CString s(_T(""));
	CString t;

	// Checks
	if (!m_pDib)
		return;
	if (!m_pDib->GetBMI())
		return;

	// Store Current Focus (that could be an open dialog)
	CWnd* pOldActiveWnd = CWnd::GetActiveWindow();

	// Dpi
	int nXDpi = m_pDib->GetXDpi();
	int nYDpi = m_pDib->GetYDpi();
	BOOL bDpi = TRUE;
	if (nXDpi == 0 || nYDpi == 0)
		bDpi = FALSE;

	// OSD
	if (m_pOsdDlg)
		m_pOsdDlg->UpdateDisplay(); // this function also changes the active window

	// Image Info Dialog
	if (m_pImageInfoDlg)
	{
		// If new doc, like from paste or scan
		if (m_sFileName == _T(""))
		{
			if (bDpi)
			{
				t.Format(ML_STRING(1305, "Dim: %dx%d\r\nDpi: %dx%d\r\nCurrent Depth: %s, %s\r\nImage Size: %d %s"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							nXDpi,
							nYDpi,
							m_pDib->GetCompressionName(), 
							m_pDib->GetNumColorsName(),
							(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
							(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
			}
			else
			{
				t.Format(ML_STRING(1306, "Dim: %dx%d\r\nCurrent Depth: %s, %s\r\nImage Size: %d %s"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							m_pDib->GetCompressionName(), 
							m_pDib->GetNumColorsName(),
							(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
							(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
			}
			s+=t;
		}
		else
		{
			// File Name
			t.Format(ML_STRING(1308, "File %i of %i: %s\r\n"),	m_FileFind.GetFilePosition() + 1,
																m_FileFind.GetFilesCount(),
																::GetShortFileName(m_sFileName));
			s+=t;

			// Path
			t.Format(ML_STRING(1309, "Path: %s\r\n"), ::GetDriveAndDirName(m_sFileName));
			s+=t;

			// Get File Time
			CFileStatus FileStatus;
			BOOL bFileTimesOk = CFile::GetStatus(m_sFileName, FileStatus);
			if (bFileTimesOk)
			{
				t =	ML_STRING(1310, "Date: ") +
					::MakeDateLocalFormat(FileStatus.m_mtime) +
					_T(" ") +
					::MakeTimeLocalFormat(FileStatus.m_mtime, TRUE) +
					_T("\r\n");
			}
			else
			{
				t = ML_STRING(1311, "Error Reading Timestamp\r\n");
			}
			s+=t;

			if (CDib::IsJPEG(m_sFileName) && m_bDoJPEGGet)
			{
				if (bDpi)
				{
					if (m_JpegThread.GetJpegCompressionQuality() < 0)
						t.Format(ML_STRING(1312, "Dim: %dx%d , Dpi: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nCompression Quality: calc...\r\nUnc. Image Size: %d %s , File Size: %d %s"),
								m_pDib->GetWidth(), 
								m_pDib->GetHeight(),
								nXDpi,
								nYDpi,
								m_pDib->GetCompressionName(),
								m_pDib->GetNumColorsName(),
								m_pDib->m_FileInfo.GetDepthName(),
								(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
								(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
								(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
								(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
					else
						t.Format(ML_STRING(1313, "Dim: %dx%d , Dpi: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nCompression Quality: %d\r\nUnc. Image Size: %d %s , File Size: %d %s"),
								m_pDib->GetWidth(), 
								m_pDib->GetHeight(),
								nXDpi,
								nYDpi,
								m_pDib->GetCompressionName(),
								m_pDib->GetNumColorsName(),
								m_pDib->m_FileInfo.GetDepthName(),
								m_JpegThread.GetJpegCompressionQuality(),
								(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
								(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
								(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
								(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				}
				else
				{
					if (m_JpegThread.GetJpegCompressionQuality() < 0)
						t.Format(ML_STRING(1314, "Dim: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nCompression Quality: calc...\r\nUnc. Image Size: %d %s , File Size: %d %s"),
								m_pDib->GetWidth(), 
								m_pDib->GetHeight(),
								m_pDib->GetCompressionName(),
								m_pDib->GetNumColorsName(),
								m_pDib->m_FileInfo.GetDepthName(),
								(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
								(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
								(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
								(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
					else
						t.Format(ML_STRING(1315, "Dim: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nCompression Quality: %d\r\nUnc. Image Size: %d %s , File Size: %d %s"),
								m_pDib->GetWidth(), 
								m_pDib->GetHeight(),
								m_pDib->GetCompressionName(),
								m_pDib->GetNumColorsName(),
								m_pDib->m_FileInfo.GetDepthName(),
								m_JpegThread.GetJpegCompressionQuality(),
								(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
								(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
								(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
								(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				}
			}
			// Note: The Gif File Format Doesn't Support Dpi Storing
			else if (::GetFileExt(m_sFileName) == _T(".gif"))
			{
				// Animated GIF
				if (m_GifAnimationThread.IsAlive() && m_GifAnimationThread.m_dwDibAnimationCount > 1)
				{
					// Play Times
					CString sPlayTimes;
					int nPlayTimes = m_pDib->GetGif()->GetPlayTimes();
					if (nPlayTimes == 0)
						sPlayTimes = ML_STRING(1316, "Play: infinite times");
					else if (nPlayTimes == 1)
						sPlayTimes = ML_STRING(1317, "Play: 1 time");
					else
						sPlayTimes.Format(ML_STRING(1318, "Play: %d times"), nPlayTimes);

					// Running
					if (m_GifAnimationThread.IsRunning())
					{
						CDib* pDib = m_GifAnimationThread.m_DibAnimationArray.GetAt(0);
						t.Format(ML_STRING(1319, "Dim: %dx%d , Ver: %s\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nFrames Count: %d , %s\r\nFile Size: %d %s"),
							pDib->GetWidth(), 
							pDib->GetHeight(),
							CDib::GIFGetVersion(m_sFileName, FALSE),
							pDib->GetCompressionName(),
							pDib->GetNumColorsName(),
							m_pDib->m_FileInfo.GetDepthName(),
							m_GifAnimationThread.m_dwDibAnimationCount > 1 ? m_GifAnimationThread.m_dwDibAnimationCount : 1,
							sPlayTimes,
							(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
							(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
					}
					// Stopped
					else
					{
						CDib* pDib = m_GifAnimationThread.m_DibAnimationArray.GetAt(m_GifAnimationThread.m_dwDibAnimationPos);
						t.Format(ML_STRING(1320, "Dim: %dx%d , Ver: %s\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nFrame %d of %d , Delay: %dms , %s\r\nFile Size: %d %s"),
							pDib->GetWidth(), 
							pDib->GetHeight(),
							CDib::GIFGetVersion(m_sFileName, FALSE),
							pDib->GetCompressionName(),
							pDib->GetNumColorsName(),
							m_pDib->m_FileInfo.GetDepthName(),
							m_GifAnimationThread.m_dwDibAnimationPos + 1,
							m_GifAnimationThread.m_dwDibAnimationCount,
							pDib->GetGif()->GetDelay(),
							sPlayTimes,
							(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
							(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
					}
				}
				// Normal Single Image GIF
				else
				{
					t.Format(ML_STRING(1321, "Dim: %dx%d , Ver: %s\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nFrames Count: 1\r\nUnc. Image Size: %d %s , File Size: %d %s"),
						m_pDib->GetWidth(), 
						m_pDib->GetHeight(),
						CDib::GIFGetVersion(m_sFileName, FALSE),
						m_pDib->GetCompressionName(),
						m_pDib->GetNumColorsName(),
						m_pDib->m_FileInfo.GetDepthName(),
						(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
						(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
						(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
						(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				}
			}
			// Multi-Page TIFF
			else if (IsMultiPageTIFF())
			{
				if (bDpi)
				{
					t.Format(ML_STRING(1323, "Dim: %dx%d , Dpi: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nPage %d of %d\r\nUnc. Image Size: %d %s , File Size: %d %s"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							nXDpi,
							nYDpi,
							m_pDib->GetCompressionName(), 
							m_pDib->GetNumColorsName(),
							m_pDib->m_FileInfo.GetDepthName(),
							m_pDib->m_FileInfo.m_nImagePos + 1,
							m_pDib->m_FileInfo.m_nImageCount,
							(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
							(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
							(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
							(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				}
				else
				{
					t.Format(ML_STRING(1324, "Dim: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nPage %d of %d\r\nUnc. Image Size: %d %s , File Size: %d %s"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							m_pDib->GetCompressionName(), 
							m_pDib->GetNumColorsName(),
							m_pDib->m_FileInfo.GetDepthName(),
							m_pDib->m_FileInfo.m_nImagePos + 1,
							m_pDib->m_FileInfo.m_nImageCount,
							(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
							(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
							(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
							(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				}
			}
			else
			{
				if (bDpi)
				{
					t.Format(ML_STRING(1325, "Dim: %dx%d , Dpi: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nUnc. Image Size: %d %s\r\nFile Size: %d %s"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							nXDpi,
							nYDpi,
							m_pDib->GetCompressionName(), 
							m_pDib->GetNumColorsName(),
							m_pDib->m_FileInfo.GetDepthName(),
							(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
							(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
							(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
							(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				}
				else
				{
					t.Format(ML_STRING(1326, "Dim: %dx%d\r\nCurrent Depth: %s, %s\r\nFile Depth: %s\r\nUnc. Image Size: %d %s\r\nFile Size: %d %s"),
							m_pDib->GetWidth(), 
							m_pDib->GetHeight(),
							m_pDib->GetCompressionName(), 
							m_pDib->GetNumColorsName(),
							m_pDib->m_FileInfo.GetDepthName(),
							(m_pDib->GetImageSize() >= 1024) ? m_pDib->GetImageSize() >> 10 : m_pDib->GetImageSize(),
							(m_pDib->GetImageSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"),
							(m_pDib->GetFileSize() >= 1024) ? m_pDib->GetFileSize() >> 10 : m_pDib->GetFileSize(),
							(m_pDib->GetFileSize() >= 1024) ? ML_STRING(1243, "KB") : ML_STRING(1244, "Bytes"));
				}
			}
			s+=t;
		}

		CEdit* pFileInfoBox = (CEdit*)m_pImageInfoDlg->GetDlgItem(IDC_FILEINFO);
		pFileInfoBox->SetWindowText(s);

		// Update Dialog Title
		m_pImageInfoDlg->UpdateDlgTitle();

		// Skip Metadata Update
		if (bUpdateFileInfoOnly)
		{
			// Restore Focus
			if (pOldActiveWnd &&
				(pOldActiveWnd != CWnd::GetActiveWindow()))
				pOldActiveWnd->SetActiveWindow();
			return;
		}

		// Update Metadata Edit Buttons
		BOOL bFileMayHaveMetadata = CDib::IsJPEG(m_sFileName) || CDib::IsTIFF(m_sFileName);
		CButton* pButton = (CButton*)m_pImageInfoDlg->GetDlgItem(IDC_BUTTON_EXPORT_METADATA);
		pButton->EnableWindow(bFileMayHaveMetadata);
		pButton = (CButton*)m_pImageInfoDlg->GetDlgItem(IDC_BUTTON_IMPORT_METADATA);
		pButton->EnableWindow(bFileMayHaveMetadata);
		pButton = (CButton*)m_pImageInfoDlg->GetDlgItem(IDC_BUTTON_SAVE_METADATA);
		pButton->EnableWindow(bFileMayHaveMetadata);

		// Update Metadata
		m_pImageInfoDlg->UpdateMetadata();
	}

	// Restore Focus
	if (pOldActiveWnd &&
		(pOldActiveWnd != CWnd::GetActiveWindow()))
		pOldActiveWnd->SetActiveWindow();
}

void CPictureDoc::FileInfo() 
{
	if (m_pImageInfoDlg)
	{
		// m_pImageInfoDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pImageInfoDlg->Close();
	}
	else
	{
		m_pImageInfoDlg = new CImageInfoDlg(this);
		::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
		m_pImageInfoDlg->ShowWindow(SW_SHOWNA);
		UpdateImageInfo();
	}
}

void CPictureDoc::OnFileInfo() 
{
	FileInfo();
}

void CPictureDoc::OnUpdateFileInfo(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_pImageInfoDlg != NULL ? 1 : 0);	
}

BOOL CPictureDoc::TransitionChess(	HDC hSrcDC, int xs, int ys,
									HDC hDestDC, int xd, int yd,
									int width, int height,
									int nStep, int nMaxSteps,
									std::mt19937& PseudoRandomX,
									std::mt19937& PseudoRandomY)
{
	if ((nStep >= nMaxSteps) || (nStep < 0))
		return FALSE;
	else
	{
		int nSquareSize = (int)sqrt((double)(width * height) / (16 * nMaxSteps));
		if (nSquareSize == 0)
			nSquareSize = 1;
		int nXNum = width / nSquareSize;
		int nYNum = height / nSquareSize;
		std::uniform_int_distribution<int> DistributionX(0, nXNum); // distribute results: [0, nXNum]
		std::uniform_int_distribution<int> DistributionY(0, nYNum); // distribute results: [0, nYNum]
		if (hSrcDC && hDestDC)
		{
			for (int i = 0 ; i < 64 ; i++)
			{
				int nX = DistributionX(PseudoRandomX);
				int nY = DistributionY(PseudoRandomY);
				::BitBlt(	hDestDC,
							xd + nX*nSquareSize, yd + nY*nSquareSize,
							nSquareSize, nSquareSize,
							hSrcDC,
							xs + nX*nSquareSize, ys + nY*nSquareSize,
							SRCCOPY);
			}
		}

		return TRUE;
	}
}

BOOL CPictureDoc::TransitionLRCurtain(	HDC hSrcDC, int xs, int ys,
										HDC hDestDC, int xd, int yd,
										int width, int height,
										int nStep, int nMaxSteps)
{
	if ((nStep >= nMaxSteps) || (nStep < 0))
		return FALSE;
	else
	{
		int pos = width / nMaxSteps;

		if (hSrcDC && hDestDC)
		{
			for (int j = 0 ; j < pos ; j++)
			{
				::BitBlt(	hDestDC,
							xd + j*nMaxSteps + nStep, yd,
							1, height,
							hSrcDC,
							xs + j*nMaxSteps + nStep, ys,
							SRCCOPY);
			}
		}

		return TRUE;
	}
}

BOOL CPictureDoc::TransitionLRRoll(	HDC hSrcDC, int xs, int ys,
									HDC hDestDC, int xd, int yd,
									int width, int height,
									int nStep, int nMaxSteps)
{
	if ((nStep >= nMaxSteps) || (nStep < 0))
		return FALSE;
	else
	{
		int pos = width / nMaxSteps;

		if (hSrcDC && hDestDC)
		{
			::BitBlt(	hDestDC,
						xd + nStep*pos, yd,
						pos, height,
						hSrcDC,
						xs + nStep*pos, ys,
						SRCCOPY);
		}

		return TRUE;
	}
}

BOOL CPictureDoc::TransitionLRSlide(HDC hSrcDC, int xs, int ys,
									HDC hDestDC, int xd, int yd,
									int width, int height,
									int nStep, int nMaxSteps)
{
	if ((nStep >= nMaxSteps) || (nStep < 0))
		return FALSE;
	else
	{
		int pos = width / nMaxSteps;

		if (hSrcDC && hDestDC)
		{
			::BitBlt(	hDestDC,
						xd, yd,
						nStep*pos, height,
						hSrcDC,
						xs + width - (nStep+1)*pos, ys,
						SRCCOPY);
		}

		return TRUE;
	}
}

const BYTE AlphaLoadFullJpegTransitionValues[] = {48, 84, 128, 180};
//                                                  +36  +44  +52
BOOL CPictureDoc::TransitionLoadFullJpeg(	HDC hSrcDC, int xs, int ys,
											HDC hDestDC, int xd, int yd,
											int width, int height,
											int nStep)
{
	const int nSteps = sizeof(AlphaLoadFullJpegTransitionValues) / sizeof(AlphaLoadFullJpegTransitionValues[0]);
	if ((nStep >= nSteps) || (nStep < 0))
		return FALSE;
	else
	{
		int alpha = AlphaLoadFullJpegTransitionValues[nStep];
		
		if (hSrcDC && hDestDC)
		{
			if (m_fpAlphaBlend)
			{
				BLENDFUNCTION BlendFunction;
				BlendFunction.BlendOp = AC_SRC_OVER;
				BlendFunction.BlendFlags = 0;
				BlendFunction.SourceConstantAlpha = alpha;
				BlendFunction.AlphaFormat = 0;
				m_fpAlphaBlend(	hDestDC,
								xd,
								yd,
								width,
								height,
								hSrcDC,
								xs,
								ys,
								width,
								height,
								BlendFunction);
			}
			else
			{
				::AlphaBlendU(	hDestDC,
								xd,
								yd,
								width,
								height,
								hSrcDC,
								xs,
								ys,
								width,
								height,
								alpha);
			}
		}

		return TRUE;
	}
}

const BYTE AlphaTransitionValues[] = {32, 56, 80, 104, 128, 152, 188, 240};
//                                      +24 +24 +24  +24  +24  +36  +52
BOOL CPictureDoc::TransitionBlend(	HDC hSrcDC, int xs, int ys,
									HDC hDestDC, int xd, int yd,
									int width, int height,
									int nStep)
{
	const int nSteps = sizeof(AlphaTransitionValues) / sizeof(AlphaTransitionValues[0]);
	if ((nStep >= nSteps) || (nStep < 0))
		return FALSE;
	else
	{
		int alpha = AlphaTransitionValues[nStep];

		if (hSrcDC && hDestDC)
		{
			if (m_fpAlphaBlend)
			{
				BLENDFUNCTION BlendFunction;
				BlendFunction.BlendOp = AC_SRC_OVER;
				BlendFunction.BlendFlags = 0;
				BlendFunction.SourceConstantAlpha = alpha;
				BlendFunction.AlphaFormat = 0;
				m_fpAlphaBlend(	hDestDC,
								xd,
								yd,
								width,
								height,
								hSrcDC,
								xs,
								ys,
								width,
								height,
								BlendFunction);
			}
			else
			{
				::AlphaBlendU(	hDestDC,
								xd,
								yd,
								width,
								height,
								hSrcDC,
								xs,
								ys,
								width,
								height,
								alpha);
			}
		}

		return TRUE;
	}
}

void CPictureDoc::OnPlayTransitionOff() 
{
	if (m_nTransitionType != 0)
	{
		CancelTransition();
		m_nTransitionType = 0;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("TransitionType"),
										0);
	}
}

void CPictureDoc::OnUpdatePlayTransitionOff(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nTransitionType == 0 ? 1 : 0);
}

void CPictureDoc::OnPlayTransitionRandom() 
{
	if (m_nTransitionType != 1)
	{
		CancelTransition();
		m_nTransitionType = 1;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("TransitionType"),
										1);
	}
}

void CPictureDoc::OnUpdatePlayTransitionRandom(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nTransitionType == 1 ? 1 : 0);
}

void CPictureDoc::OnPlayTransitionChessboard() 
{
	if (m_nTransitionType != 2)
	{
		CancelTransition();
		m_nTransitionType = 2;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("TransitionType"),
										2);
	}
}

void CPictureDoc::OnUpdatePlayTransitionChessboard(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nTransitionType == 2 ? 1 : 0);
}

void CPictureDoc::OnPlayTransitionCurtainin() 
{
	if (m_nTransitionType != 3)
	{
		CancelTransition();
		m_nTransitionType = 3;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("TransitionType"),
										3);
	}
}

void CPictureDoc::OnUpdatePlayTransitionCurtainin(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nTransitionType == 3 ? 1 : 0);
}

void CPictureDoc::OnPlayTransitionRoolin() 
{
	if (m_nTransitionType != 4)
	{
		CancelTransition();
		m_nTransitionType = 4;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("TransitionType"),
										4);
	}
}

void CPictureDoc::OnUpdatePlayTransitionRoolin(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nTransitionType == 4 ? 1 : 0);
}

void CPictureDoc::OnPlayTransitionSlidein() 
{
	if (m_nTransitionType != 5)
	{
		CancelTransition();
		m_nTransitionType = 5;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("TransitionType"),
										5);
	}
}

void CPictureDoc::OnUpdatePlayTransitionSlidein(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nTransitionType == 5 ? 1 : 0);
}

void CPictureDoc::OnPlayTransitionBlend() 
{
	if (m_nTransitionType != 6)
	{
		CancelTransition();
		m_nTransitionType = 6;
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("TransitionType"),
										6);
	}
}

void CPictureDoc::OnUpdatePlayTransitionBlend(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nTransitionType == 6 ? 1 : 0);
}

void CPictureDoc::EditCrop() 
{
	if (m_bCrop)
		CancelCrop();
	else
	{
		// Cancel Zoom
		CancelZoomTool();

		// Lossless crop needs integer zoom factors!
		// -> Set the closest integer zoom factor
		if (m_bLosslessCrop)
		{
			// (6.25%, 12.5%, 25%, 50%, 100%, 200%, 400%, 800% or 1600%)
			CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(GetFrame()))->GetToolBar())->m_ZoomComboBox);
			if (pZoomCB->GetCurSel() == 0 ||	// Fit
				pZoomCB->GetCurSel() == 1)		// Fit Big
			{
				void* pData = NULL;
				int nIndex = pZoomCB->GetCount() - 1;	// Start with highest zoom factor,
				while (nIndex >= 2)						// down to Zoom Factor 6.25%, which has nIndex == 2
				{
					pData = pZoomCB->GetItemDataPtr(nIndex--);
					if (*((double*)pData) <= m_dZoomFactor)
						break;
				}
				if ((int)pData != -1)
				{
					pZoomCB->SetCurSel(nIndex+1);
					pZoomCB->OnChangeZoomFactor(*((double*)pData));
					GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
				}
			}
		}

		// Set Vars
		m_CropDocRect = m_DocRect;
		GetView()->m_CropZoomRect = GetView()->m_ZoomRect;
		GetView()->m_bCropTopLeft = FALSE;
		GetView()->m_bCropBottomRight = FALSE;
		GetView()->m_bCropBottomLeft = FALSE;
		GetView()->m_bCropTopRight = FALSE;
		GetView()->m_bCropTop = FALSE;
		GetView()->m_bCropBottom = FALSE;
		GetView()->m_bCropLeft = FALSE;
		GetView()->m_bCropRight = FALSE;
		GetView()->m_bCropCenter = FALSE;
		GetView()->m_bCropDrag = FALSE;
		GetView()->m_ptCropClick = CPoint(0,0);
		GetView()->m_dCropAspectRatio = 1.0;
		GetView()->m_bCropMaintainAspectRatio = FALSE;
		GetView()->m_nAspectRatioPos = 0;
		m_rcCropDelta = CRect(0,0,0,0);
		m_rcCropCenter = CRect(0,0,0,0);
		GetView()->ForceCursor();
		m_bCrop = TRUE;

		BeginWaitCursor();

		// Alpha
		CDib* pDib =	(m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32) ?
						&m_AlphaRenderedDib :
						m_pDib;

		// Create Crop Darker Background Image
		if (m_pCropBkgDib)
			delete m_pCropBkgDib;
		m_pCropBkgDib = new CDib;
		if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
			*m_pCropBkgDib = *pDib->GetPreviewDib();
		else
			*m_pCropBkgDib = *pDib;
		m_pCropBkgDib->AdjustImage(	CROP_BKG_BRIGHTNESS,
									CROP_BKG_CONTRAST,
									0, 0,
									FALSE);	// Disable Undo

		EndWaitCursor();

		GetView()->UpdateWindowSizes(TRUE, FALSE, FALSE);
	}
}

void CPictureDoc::OnEditCropLossless() 
{
	if (CDib::IsJPEG(m_sFileName))
	{
		if (m_bCrop)
			DoCropRect();
		else
		{
			// Wait and schedule command if dib not fully loaded!
			if (!IsDibReadyForCommand(ID_EDIT_CROP_LOSSLESS))
				return;

			// Lossless Crop if not modified
			m_bLosslessCrop = !IsModified();
			EditCrop();
		}
	}
}

void CPictureDoc::OnUpdateEditCropLossless(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0			||
					m_dwIDAfterFullLoadCommand == ID_EDIT_CROP_LOSSLESS)&&
					m_pDib												&&
					!(m_SlideShowThread.IsSlideshowRunning()	||
					m_bDoRestartSlideshow)								&&
					!m_GifAnimationThread.IsAlive()						&&
					!m_bMetadataModified								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_pHLSDlg											&&
					CDib::IsJPEG(m_sFileName)							&&
					m_DocRect.Width() >= m_nPixelAlignX					&&
					m_DocRect.Height() >= m_nPixelAlignY				&&
					!m_bPrintPreviewMode);
}

void CPictureDoc::OnEditCrop() 
{
	if (m_bCrop)
		DoCropRect();
	else
	{
		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_CROP))
			return;

		// No Lossless Crop
		m_bLosslessCrop = FALSE;
		EditCrop();
	}
}

void CPictureDoc::OnUpdateEditCrop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0			||
					m_dwIDAfterFullLoadCommand == ID_EDIT_CROP)			&&
					m_pDib												&&
					!(m_SlideShowThread.IsSlideshowRunning()	||
					m_bDoRestartSlideshow)								&&
					!m_GifAnimationThread.IsAlive()						&&
					!m_bMetadataModified								&&
					!m_pRotationFlippingDlg								&&
					!m_pWndPalette										&&
					!m_pRedEyeDlg										&&
					!m_bDoRedEyeColorPickup								&&
					!m_pMonochromeConversionDlg							&&
					!m_pSharpenDlg										&&
					!m_pSoftenDlg										&&
					!m_pSoftBordersDlg									&&
					!m_pHLSDlg											&&
					!m_bPrintPreviewMode);
}

void CPictureDoc::DoCropRect()
{
	CopyDelCrop(TRUE, FALSE, FALSE, TRUE);
	m_bCrop = FALSE;
	GetView()->FreeCropMemDCDrawing();
	if (GetView()->m_nAutoScroll)
		GetView()->StopAutoScroll();
	GetView()->UpdateWindowSizes(TRUE, FALSE, FALSE);
	::AfxGetMainFrame()->StatusText();
	GetView()->ForceCursor(FALSE);
}

void CPictureDoc::DoCopyRect()
{
	CopyDelCrop(TRUE, TRUE, FALSE, FALSE);
	m_bCrop = FALSE;
	GetView()->FreeCropMemDCDrawing();
	if (GetView()->m_nAutoScroll)
		GetView()->StopAutoScroll();
	GetView()->UpdateWindowSizes(TRUE, FALSE, FALSE);
	::AfxGetMainFrame()->StatusText();
	GetView()->ForceCursor(FALSE);
}

void CPictureDoc::DoCutRect()
{
	CopyDelCrop(TRUE, TRUE, TRUE, FALSE);
	m_bCrop = FALSE;
	GetView()->FreeCropMemDCDrawing();
	if (GetView()->m_nAutoScroll)
		GetView()->StopAutoScroll();
	GetView()->UpdateWindowSizes(TRUE, FALSE, FALSE);
	::AfxGetMainFrame()->StatusText();
	GetView()->ForceCursor(FALSE);
}

void CPictureDoc::CancelCrop()
{
	if (m_bCrop)
	{
		m_bCrop = FALSE;
		GetView()->FreeCropMemDCDrawing();
		GetView()->m_CropZoomRect = GetView()->m_ZoomRect;
		m_CropDocRect = m_DocRect;
		m_rcCropDelta = CRect(0,0,0,0);
		GetView()->UpdateWindowSizes(TRUE, FALSE, FALSE);
	
		if (m_pCropBkgDib)
		{
			delete m_pCropBkgDib;
			m_pCropBkgDib = NULL;
		}
		if (GetView()->m_nAutoScroll)
			GetView()->StopAutoScroll();
		::AfxGetMainFrame()->StatusText();
		GetView()->ForceCursor(FALSE);
	}
}

BOOL CPictureDoc::CopyDelCrop(BOOL bShowMessageBoxOnError, BOOL bCopy, BOOL bDel, BOOL bCrop)
{
	if (m_pDib && m_pDib->IsValid())
	{
		// Force Cursor
		GetView()->ForceCursor();

		// Lossless crop,
		// check for JPEG Extensions,
		// make sure the file has not been modified
		// and we are not copying and not deleting
		if (m_bLosslessCrop				&&
			CDib::IsJPEG(m_sFileName)	&&
			!IsModified()				&&
			!bCopy						&&
			!bDel)
		{	
			CString sCroppedFileName;
			int nID;
			if (!::HasWriteAccess(m_sFileName))
				nID = IDYES;
			else
				nID = ::AfxMessageBox(ML_STRING(1327, "Do You Want To Save The Cropped Image To A New File?"), MB_YESNOCANCEL);
			if (nID == IDYES)
			{
				// Display the Save As Dialog
				// Note: the OFN_OVERWRITEPROMPT flag makes no sense here
				//       because below we fail when attempting to overwrite
				TCHAR szFileName[MAX_PATH];
				CNoVistaFileDlg dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY);
				sCroppedFileName = m_sFileName;
				int index = sCroppedFileName.ReverseFind(_T('.'));	
				if (index > 0)
				{
					CString sSize;
					sSize.Format(_T("_%ux%u"), m_CropDocRect.Width(), m_CropDocRect.Height());
					sCroppedFileName.Insert(index, sSize);
				}
				else
				{
					GetView()->ForceCursor(FALSE);
					return FALSE;
				}
				_tcscpy(szFileName, sCroppedFileName);
				dlgFile.m_ofn.lpstrFile = szFileName;
				dlgFile.m_ofn.nMaxFile = MAX_PATH;
				dlgFile.m_ofn.lpstrCustomFilter = NULL;
				dlgFile.m_ofn.lpstrFilter = _T("JPEG File Interchange Format (*.jpg;*.jpeg;*.jpe;*.jfif;*.thm)\0*.jpg;*.jpeg;*.jpe;*.jfif;*.thm\0");
				dlgFile.m_ofn.lpstrDefExt = _T("jpg");
				if (dlgFile.DoModal() == IDOK)
				{
					sCroppedFileName = szFileName;
					if (sCroppedFileName.CompareNoCase(m_sFileName) == 0)
					{
						::AfxMessageBox(ML_STRING(1275, "Cannot save to ourself"), MB_OK | MB_ICONSTOP);
						GetView()->ForceCursor(FALSE);
						return FALSE;
					}
				}
				else
				{
					GetView()->ForceCursor(FALSE);
					return FALSE;
				}
			}
			else if (nID == IDNO)
			{
				// Temporary File
				sCroppedFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);
			}
			else // Cancel
			{
				GetView()->ForceCursor(FALSE);
				return FALSE;
			}

			// Begin Wait Cursor
			BeginWaitCursor();

			// Kill Jpeg Thread
			m_JpegThread.Kill();

			// Make Sure The File has the right Orientation
			if (!CDib::JPEGAutoOrientate(m_sFileName,
										((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
										bShowMessageBoxOnError,
										GetView(),
										TRUE))
			{
				EndWaitCursor();
				GetView()->ForceCursor(FALSE);
				return FALSE;
			}

			// Do Transformation
			BOOL res = m_pDib->LossLessJPEGTrans(m_sFileName,		// Src File
												sCroppedFileName,	// Dst File
												JXFORM_NONE,		// No Transformation (cropping has separate params)
												TRUE,				// Do Trim Edges
												FALSE,				// Do Not Force to Grayscale
												TRUE,				// Do Update Exif Data After Cropping
												"",					// Do not add comments
												TRUE,				// Do Cropping!
												m_CropDocRect.left, m_CropDocRect.top,			// Cropping (left,top) point
												m_CropDocRect.Width(), m_CropDocRect.Height(),	// Cropping size
												bShowMessageBoxOnError,
												GetView(),
												TRUE);				// Do Show a MessageBox in case of error

			// If Ok
			if (res)
			{
				// Remove and Rename File
				if (nID == IDNO)
				{
					try
					{
						CFile::Remove(m_sFileName);
						CFile::Rename(sCroppedFileName, m_sFileName);
					}
					catch (CFileException* e)
					{
						EndWaitCursor();
						::DeleteFile(sCroppedFileName);

						DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
						if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
						{
							CString str(ML_STRING(1255, "The file is read only\n"));
							TRACE(str);
							if (bShowMessageBoxOnError)
								::AfxMessageBox(str, MB_OK | MB_ICONSTOP);
						}
						else
							::ShowErrorMsg(e->m_lOsError, bShowMessageBoxOnError);

						e->Delete();
						GetView()->ForceCursor(FALSE);
						return FALSE;
					}
				}
			}
			else
			{
				EndWaitCursor();
				GetView()->ForceCursor(FALSE);
				return FALSE;
			}

			// Load
			if (nID == IDNO)
				LoadPicture(&m_pDib, m_sFileName); // Reload Picture
			else
			{
				ClearPrevNextPictures();
				if (LoadPicture(&m_pDib, sCroppedFileName)) // Load Picture
					SlideShow(); // No Recursive Slideshow in Paused State (also if it was Recursive before...)
			}
			
			EndWaitCursor();
			GetView()->ForceCursor(FALSE);

			return TRUE;
		}

		// Begin Wait Cursor
		BeginWaitCursor();

		// Copy
		if (bCopy)
		{
			CDib Dib(*m_pDib);
			if (Dib.Crop(m_CropDocRect.left, m_CropDocRect.top,
						m_CropDocRect.Width(), m_CropDocRect.Height(),
						GetView(), TRUE))
			{
				// Has Alpha?
				if (Dib.HasAlpha() && Dib.GetBitCount() == 32)
					Dib.BMIToBITMAPV4HEADER();
				Dib.EditCopy();
			}
		}

		// Delete with current background color or
		// make transparent if image has alpha channel
		if (bDel)
		{
			// From top-down to bottom-up Coordinates
			int xStart = m_CropDocRect.left;
			int xEnd = m_CropDocRect.right;
			int yStart = m_pDib->GetHeight() - m_CropDocRect.bottom;
			int yEnd = m_pDib->GetHeight() - m_CropDocRect.top;

			// A Duplicated Dib is created
			AddUndo();
			
			// Set pixels
			if (m_pDib->GetBitCount() <= 8)
			{
				int nBackgroundIndex =	m_bImageBackgroundColor ?
										m_pDib->GetPalette()->GetNearestPaletteIndex(m_crImageBackgroundColor) :
										m_pDib->GetPalette()->GetNearestPaletteIndex(m_crBackgroundColor);
				for (int y = yStart ; y < yEnd ; y++)
				{
					for (int x = xStart ; x < xEnd ; x++)
					{
						m_pDib->SetPixelIndex(x, y, nBackgroundIndex);
					}
				}
			}
			else if (m_pDib->GetBitCount() == 32 && m_pDib->HasAlpha())
			{
				COLORREF crTransparentBackgroundColor = m_bImageBackgroundColor ?
														m_crImageBackgroundColor :
														m_crBackgroundColor;
				crTransparentBackgroundColor &= 0x00FFFFFF;
				for (int y = yStart ; y < yEnd ; y++)
				{
					for (int x = xStart ; x < xEnd ; x++)
					{
						m_pDib->SetPixelColor32Alpha(x, y, crTransparentBackgroundColor);
					}
				}
			}
			else
			{
				COLORREF crBackgroundColor =	m_bImageBackgroundColor ?
												m_crImageBackgroundColor :
												m_crBackgroundColor;
				for (int y = yStart ; y < yEnd ; y++)
				{
					for (int x = xStart ; x < xEnd ; x++)
					{
						m_pDib->SetPixelColor(x, y, crBackgroundColor);
					}
				}
			}

			// Update
			UpdateAlphaRenderedDib();
			SetModifiedFlag();
			SetDocumentTitle();
			UpdateAllViews(NULL);
			UpdateImageInfo();
		}

		// Crop
		if (bCrop)
		{
			// A Duplicated Dib is created
			AddUndo();
			m_pDib->Crop(	m_CropDocRect.left, m_CropDocRect.top,
							m_CropDocRect.Width(), m_CropDocRect.Height(),
							GetView(), TRUE);
			UpdateAlphaRenderedDib();
			SetModifiedFlag();
			m_DocRect.bottom = m_pDib->GetHeight();
			m_DocRect.right = m_pDib->GetWidth();
			SetDocumentTitle();
			UpdateAllViews(NULL);
			UpdateImageInfo();
		}

		// End Wait Cursor and Disable Force Cursor
		EndWaitCursor();
		GetView()->ForceCursor(FALSE);

		return TRUE;
	}
	else
		return FALSE;
}

void CPictureDoc::OnEditCropApply() 
{
	DoCropRect();
}

void CPictureDoc::OnUpdateEditCropApply(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bCrop);
}

void CPictureDoc::OnEditCopy()
{
	if (m_bCrop)
	{
		DoCopyRect();
	}
	else
	{
		// Stop Playing
		if (m_SlideShowThread.IsSlideshowRunning() ||
								m_bDoRestartSlideshow)
		{
			// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
			m_bDoRestartSlideshow = FALSE;
			m_SlideShowThread.PauseSlideshow();
		}

		// Wait and schedule command if dib not fully loaded!
		if (!IsDibReadyForCommand(ID_EDIT_COPY))
			return;

		// The Animation has a separate array of dibs, sync the document's
		// one with the current one of the animation array
		if (m_GifAnimationThread.IsAlive())
		{
			::EnterCriticalSection(&m_csDib);
			*m_pDib = *(m_GifAnimationThread.m_DibAnimationArray.GetAt
								(m_GifAnimationThread.m_dwDibAnimationPos));
			::LeaveCriticalSection(&m_csDib);
			if (m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetSize() > 0)
				m_AlphaRenderedDib = *(m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetAt
									(m_GifAnimationThread.m_dwDibAnimationPos));
			else
				UpdateAlphaRenderedDib();
		}

		// Has Alpha?
		if (m_pDib->HasAlpha() && m_pDib->GetBitCount() == 32)
		{
			CDib Dib;
			::EnterCriticalSection(&m_csDib);
			Dib = *m_pDib;
			::LeaveCriticalSection(&m_csDib);
			Dib.BMIToBITMAPV4HEADER();
			Dib.EditCopy();
		}
		else
			CUImagerDoc::OnEditCopy();
	}
}

void CPictureDoc::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	(m_dwIDAfterFullLoadCommand == 0 ||
					m_dwIDAfterFullLoadCommand == ID_EDIT_COPY) &&
					m_pDib										&&
					!m_pRotationFlippingDlg						&&
					!m_pWndPalette								&&
					!m_pHLSDlg									&&
					!m_pRedEyeDlg								&&
					!m_bDoRedEyeColorPickup						&&
					!m_pMonochromeConversionDlg					&&
					!m_pSharpenDlg								&&
					!m_pSoftenDlg								&&
					!m_pSoftBordersDlg);
}

void CPictureDoc::OnEditCut() 
{
	DoCutRect();
}

void CPictureDoc::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bCrop);
}

void CPictureDoc::OnEditCropCancel() 
{
	CancelCrop();
}

void CPictureDoc::OnUpdateEditCropCancel(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bCrop);
}

void CPictureDoc::OnPlayAnimation() 
{
	if (m_GifAnimationThread.IsAlive() && !m_GifAnimationThread.IsRunning())
	{
		m_GifAnimationThread.Start();
		SetDocumentTitle();
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdatePlayAnimation(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_GifAnimationThread.IsAlive());
	if (m_GifAnimationThread.IsAlive())
		pCmdUI->SetCheck(m_GifAnimationThread.IsRunning() ? 1 : 0);
	else
		pCmdUI->SetCheck(0);
}

void CPictureDoc::OnPlayStopAnimation() 
{
	if (m_GifAnimationThread.IsRunning())
	{
		m_GifAnimationThread.Pause();
		CRect rcc;
		GetView()->GetClientRect(&rcc);
		GetView()->InvalidateRect(rcc, FALSE);
		SetDocumentTitle();
		UpdateImageInfo();
	}
}

void CPictureDoc::OnUpdatePlayStopAnimation(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_GifAnimationThread.IsAlive());
	if (m_GifAnimationThread.IsAlive())
		pCmdUI->SetCheck((!m_GifAnimationThread.IsRunning()) ? 1 : 0);
	else
		pCmdUI->SetCheck(0);
}

BOOL CPictureDoc::ViewNextPageFrame() 
{
	if (::GetFileExt(m_sFileName) == _T(".gif"))
	{
		if (m_GifAnimationThread.IsAlive()		&&
			!m_GifAnimationThread.IsRunning()	&&
			m_GifAnimationThread.m_dwDibAnimationCount > 1)
		{
			m_GifAnimationThread.ShowNext();
			CRect rcClient;
			GetView()->GetClientRect(&rcClient);
			GetView()->InvalidateRect(rcClient, FALSE);
			SetDocumentTitle();
			UpdateImageInfo();
			return TRUE;
		}
	}
	else if (IsMultiPageTIFF())
	{
		if (m_nPageNum == m_pDib->m_FileInfo.m_nImageCount - 1)
			m_nPageNum = 0;
		else
			m_nPageNum++;
		LoadPicture(&m_pDib, m_sFileName);
		return TRUE;
	}

	return FALSE;
}

void CPictureDoc::OnViewNextPageFrame() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(this, TRUE))
	{
		if (::GetFileExt(m_sFileName) == _T(".gif"))
		{
			if (m_GifAnimationThread.IsAlive()		&&
				m_GifAnimationThread.IsRunning()	&&
				m_GifAnimationThread.m_dwDibAnimationCount > 1)
			{
				::AfxMessageBox(ML_STRING(1329, "Stop playing the Animated GIF before viewing it frame by frame."));
				return;
			}
		}
		ViewNextPageFrame();
	}
}

void CPictureDoc::OnUpdateViewNextPageFrame(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsMultiPageTIFF() || m_GifAnimationThread.IsAlive());
}

BOOL CPictureDoc::ViewPreviousPageFrame() 
{
	if (::GetFileExt(m_sFileName) == _T(".gif"))
	{
		if (m_GifAnimationThread.IsAlive()		&&
			!m_GifAnimationThread.IsRunning()	&&
			m_GifAnimationThread.m_dwDibAnimationCount > 1)
		{
			m_GifAnimationThread.ShowPrevious();
			CRect rcClient;
			GetView()->GetClientRect(&rcClient);
			GetView()->InvalidateRect(rcClient, FALSE);
			SetDocumentTitle();
			UpdateImageInfo();
			return TRUE;
		}
	}
	else if (IsMultiPageTIFF())
	{
		if (m_nPageNum == 0)
			m_nPageNum = m_pDib->m_FileInfo.m_nImageCount - 1;
		else
			m_nPageNum--;
		LoadPicture(&m_pDib, m_sFileName);
		return TRUE;
	}

	return FALSE;
}

void CPictureDoc::OnViewPreviousPageFrame() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(this, TRUE))
	{
		if (::GetFileExt(m_sFileName) == _T(".gif"))
		{
			if (m_GifAnimationThread.IsAlive()		&&
				m_GifAnimationThread.IsRunning()	&&
				m_GifAnimationThread.m_dwDibAnimationCount > 1)
			{
				::AfxMessageBox(ML_STRING(1329, "Stop playing the Animated GIF before viewing it frame by frame."));
				return;
			}
		}
		ViewPreviousPageFrame();
	}
}

void CPictureDoc::OnUpdateViewPreviousPageFrame(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(IsMultiPageTIFF() || m_GifAnimationThread.IsAlive());
}

BOOL CPictureDoc::ViewFirstPageFrame() 
{
	if (::GetFileExt(m_sFileName) == _T(".gif"))
	{
		if (m_GifAnimationThread.IsAlive()		&&
			!m_GifAnimationThread.IsRunning()	&&
			m_GifAnimationThread.m_dwDibAnimationCount > 1)
		{
			m_GifAnimationThread.ShowFirst();
			CRect rcClient;
			GetView()->GetClientRect(&rcClient);
			GetView()->InvalidateRect(rcClient, FALSE);
			SetDocumentTitle();
			UpdateImageInfo();
			return TRUE;
		}
	}
	else if (IsMultiPageTIFF())
	{
		m_nPageNum = 0;
		LoadPicture(&m_pDib, m_sFileName);
		return TRUE;
	}

	return FALSE;
}

BOOL CPictureDoc::ViewLastPageFrame() 
{
	if (::GetFileExt(m_sFileName) == _T(".gif"))
	{
		if (m_GifAnimationThread.IsAlive()		&&
			!m_GifAnimationThread.IsRunning()	&&
			m_GifAnimationThread.m_dwDibAnimationCount > 1)
		{
			m_GifAnimationThread.ShowLast();
			CRect rcClient;
			GetView()->GetClientRect(&rcClient);
			GetView()->InvalidateRect(rcClient, FALSE);
			SetDocumentTitle();
			UpdateImageInfo();
			return TRUE;
		}
	}
	else if (IsMultiPageTIFF())
	{
		m_nPageNum = m_pDib->m_FileInfo.m_nImageCount - 1;
		LoadPicture(&m_pDib, m_sFileName);
		return TRUE;
	}
	
	return FALSE;
}

void CPictureDoc::ViewBackgroundColorDlg() 
{
	// Image Has a Defined Background Color?
	COLORREF* pBackgroundColor;
	if (m_bImageBackgroundColor)
		pBackgroundColor = &m_crImageBackgroundColor;
	else
		pBackgroundColor = &m_crBackgroundColor;

	// Force Cursor
	GetView()->ForceCursor();

	// Show Color Dialog
	if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(*pBackgroundColor, GetView()))
	{
		// Update Control
		CColorButtonPicker* pBkgColorButtonPicker =
					&(((CPictureToolBar*)((CToolBarChildFrame*)
					(GetView()->GetParentFrame()))->GetToolBar())->m_BkgColorButtonPicker);
		pBkgColorButtonPicker->SetColor(*pBackgroundColor);

		// Re-Render Animated Gif frames with new background color
		if (::GetFileExt(m_sFileName) == _T(".gif")	&&
			m_GifAnimationThread.IsAlive()			&&
			m_GifAnimationThread.m_dwDibAnimationCount > 1)
		{
			m_GifAnimationThread.AlphaRender(*pBackgroundColor);
		}

		// Re-Render with new background color
		UpdateAlphaRenderedDib();

		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("BackgroundColor"),
										m_crBackgroundColor);

		InvalidateAllViews(FALSE);
	}

	// Do not Force Cursor
	GetView()->ForceCursor(FALSE);
}

void CPictureDoc::OnViewBackgroundColorMenu() 
{
	ViewBackgroundColorDlg();
}

void CPictureDoc::OnUpdateViewBackgroundColorMenu(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!m_pHLSDlg				&&
					!m_bCrop				&&
					!m_pRedEyeDlg			&&
					!m_bDoRedEyeColorPickup);	
}

void CPictureDoc::ViewMap()
{
	if (m_pDib && !GetView()->m_bFullScreenMode)
	{
		CString sQuery, sUrl;

		// Gps coordinates
		if (m_pDib->GetExifInfo()								&&
			m_pDib->GetExifInfo()->bGpsInfoPresent				&&
			m_pDib->GetExifInfo()->GpsLat[GPS_DEGREE]  >= 0.0f	&&
			m_pDib->GetExifInfo()->GpsLat[GPS_MINUTES] >= 0.0f	&&
			m_pDib->GetExifInfo()->GpsLat[GPS_SECONDS] >= 0.0f	&&
			m_pDib->GetExifInfo()->GpsLatRef[0]					&&
			m_pDib->GetExifInfo()->GpsLong[GPS_DEGREE]  >= 0.0f	&&
			m_pDib->GetExifInfo()->GpsLong[GPS_MINUTES] >= 0.0f	&&
			m_pDib->GetExifInfo()->GpsLong[GPS_SECONDS] >= 0.0f &&
			m_pDib->GetExifInfo()->GpsLongRef[0])
		{
			CString sLat, sLong;
			double dLat =	(double)m_pDib->GetExifInfo()->GpsLat[GPS_DEGREE]				+ 
							(double)m_pDib->GetExifInfo()->GpsLat[GPS_MINUTES] / 60.0		+
							(double)m_pDib->GetExifInfo()->GpsLat[GPS_SECONDS] / 3600.0;	
			sLat.Format(_T("%s%0.6f"), CString(m_pDib->GetExifInfo()->GpsLatRef), dLat);
			double dLong =	(double)m_pDib->GetExifInfo()->GpsLong[GPS_DEGREE]				+ 
							(double)m_pDib->GetExifInfo()->GpsLong[GPS_MINUTES] / 60.0		+
							(double)m_pDib->GetExifInfo()->GpsLong[GPS_SECONDS] / 3600.0;	
			sLong.Format(_T("%s%0.6f"), CString(m_pDib->GetExifInfo()->GpsLongRef), dLong);
			sQuery = sLat + _T(", ") + sLong;
			sUrl.Format(_T("https://maps.google.com/maps?q=%s"), ::UrlEncode(sQuery, TRUE));
			::ShellExecute(NULL, _T("open"), sUrl, NULL, NULL, SW_SHOWNORMAL);
			return;
		}

		// Location
		if (m_pDib->GetXmpInfo() &&
			m_pDib->GetXmpInfo()->Location != _T(""))
			sQuery = m_pDib->GetXmpInfo()->Location;

		// City
		if (m_pDib->GetIptcFromXmpInfo() &&
			m_pDib->GetIptcFromXmpInfo()->City != _T(""))
		{
			if (sQuery != _T(""))
				sQuery += _T(", ");
			sQuery += m_pDib->GetIptcFromXmpInfo()->City;
		}
		else if (m_pDib->GetIptcLegacyInfo() &&
				m_pDib->GetIptcLegacyInfo()->City != _T(""))
		{
			if (sQuery != _T(""))
				sQuery += _T(", ");
			sQuery += m_pDib->GetIptcLegacyInfo()->City;
		}

		// Province/State
		if (m_pDib->GetIptcFromXmpInfo() &&
			m_pDib->GetIptcFromXmpInfo()->ProvinceState != _T(""))
		{
			if (sQuery != _T(""))
				sQuery += _T(", ");
			sQuery += m_pDib->GetIptcFromXmpInfo()->ProvinceState;
		}
		else if (m_pDib->GetIptcLegacyInfo() &&
				m_pDib->GetIptcLegacyInfo()->ProvinceState != _T(""))
		{
			if (sQuery != _T(""))
				sQuery += _T(", ");
			sQuery += m_pDib->GetIptcLegacyInfo()->ProvinceState;
		}

		// Country
		if (m_pDib->GetIptcFromXmpInfo() &&
			m_pDib->GetIptcFromXmpInfo()->Country != _T(""))
		{
			if (sQuery != _T(""))
				sQuery += _T(", ");
			sQuery += m_pDib->GetIptcFromXmpInfo()->Country;
		}
		else if (m_pDib->GetIptcLegacyInfo() &&
				m_pDib->GetIptcLegacyInfo()->Country != _T(""))
		{
			if (sQuery != _T(""))
				sQuery += _T(", ");
			sQuery += m_pDib->GetIptcLegacyInfo()->Country;
		}

		// Use comment, filename or user's country
		if (sQuery == _T("") && CDib::IsJPEG(m_sFileName) && m_pDib->GetMetadata())
		{
			sQuery = ::FromUTF8((const unsigned char*)(LPCSTR)(m_pDib->GetMetadata()->m_sJpegComment),
								m_pDib->GetMetadata()->m_sJpegComment.GetLength());
		}
		if (sQuery == _T("") && ::GetFileExt(m_sFileName) == _T(".gif"))
		{
			if (m_GifAnimationThread.IsAlive() &&
				m_GifAnimationThread.m_dwDibAnimationCount > 1 &&
				!m_GifAnimationThread.IsRunning())
			{
				CDib* pDib = m_GifAnimationThread.m_DibAnimationArray.GetAt(m_GifAnimationThread.m_dwDibAnimationPos);
				if (pDib && pDib->GetGif())
					sQuery = pDib->GetGif()->GetComment();
			}
			else
			{
				if (m_pDib->GetGif())
					sQuery = m_pDib->GetGif()->GetComment();
			}
		}
		if (sQuery == _T(""))
			sQuery = ::GetShortFileNameNoExt(m_sFileName);
		if (sQuery == _T(""))
		{
			int nSize = ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGCOUNTRY, NULL, 0);
			if (nSize > 0)
			{
				::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGCOUNTRY, sQuery.GetBuffer(nSize), nSize);
				sQuery.ReleaseBuffer();
			}
		}

		// Start browser
		sUrl.Format(_T("https://maps.google.com/maps?q=%s"), ::UrlEncode(sQuery, TRUE));
		::ShellExecute(NULL, _T("open"), sUrl, NULL, NULL, SW_SHOWNORMAL);
	}
}

void CPictureDoc::OnViewMap() 
{
	ViewMap();
}

void CPictureDoc::OnUpdateViewMap(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pDib && !GetView()->m_bFullScreenMode);
}

void CPictureDoc::ViewNoBorders()
{
	m_bNoBorders = !m_bNoBorders;
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("NoBorders"),
									m_bNoBorders);
	GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
}

void CPictureDoc::OnViewNoBorders() 
{
	ViewNoBorders();
}

void CPictureDoc::OnUpdateViewNoBorders(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_bPrintPreviewMode);
	pCmdUI->SetCheck(m_bNoBorders ? 1 : 0);
}

void CPictureDoc::OnBackgroundColor() 
{
	// Control Pointer
	CColorButtonPicker* pBkgColorButtonPicker =
		&(((CPictureToolBar*)((CToolBarChildFrame*)
		(GetView()->GetParentFrame()))->GetToolBar())->m_BkgColorButtonPicker);
	
	// Image Has a Defined Background Color?
	COLORREF* pBackgroundColor;
	if (m_bImageBackgroundColor)
		pBackgroundColor = &m_crImageBackgroundColor;
	else
		pBackgroundColor = &m_crBackgroundColor;

	// Get The Color
	if (pBkgColorButtonPicker->GetColor() == CLR_DEFAULT)
		*pBackgroundColor = pBkgColorButtonPicker->GetDefaultColor();
	else
		*pBackgroundColor = pBkgColorButtonPicker->GetColor();

	// Re-Render Animated Gif frames with new background color
	if (::GetFileExt(m_sFileName) == _T(".gif")	&&
		m_GifAnimationThread.IsAlive()			&&
		m_GifAnimationThread.m_dwDibAnimationCount > 1)
	{
		m_GifAnimationThread.AlphaRender(*pBackgroundColor);
	}

	// Re-Render with new background color
	UpdateAlphaRenderedDib();

	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("BackgroundColor"),
									m_crBackgroundColor);

	InvalidateAllViews(FALSE);
}

void CPictureDoc::OnUpdateBackgroundColor(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!m_pHLSDlg &&
					!m_bCrop &&
					!m_pRedEyeDlg &&
					!m_bDoRedEyeColorPickup);
}

void CPictureDoc::OnFileClose() 
{
	CloseDocument();
}

void CPictureDoc::OnViewEnableOsd() 
{
	if (GetView()->m_bFullScreenMode)
	{
		m_bEnableOsd = !m_bEnableOsd;
		ShowOsd(m_bEnableOsd);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("EnableOSD"),
										m_bEnableOsd);
	}
}

void CPictureDoc::OnUpdateViewEnableOsd(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bEnableOsd ? 1 : 0);
}

void CPictureDoc::OnOsdClose() 
{
	OnViewEnableOsd();
}

void CPictureDoc::OnOsdDefaults() 
{
	if (m_pOsdDlg)
	{
		m_pOsdDlg->Defaults();
		m_pOsdDlg->UpdateDisplay();
	}
}

void CPictureDoc::OnUpdateOsdDefaults(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bEnableOsd);	
}

void CPictureDoc::ShowOsd(BOOL bShow) 
{
	if (m_pOsdDlg)
	{
		if (!bShow)
		{
			// m_pOsdDlg pointer is set to NULL
			// from the dialog class (selfdeletion)
			m_pOsdDlg->Close();
		}
	}
	else
	{
		if (bShow)
		{
			m_pOsdDlg = new COsdDlg(this);
			m_pOsdDlg->ShowWindow(SW_HIDE);
			m_pOsdDlg->UpdateDisplay();
		}
	}
}

void CPictureDoc::OnOsdAutosize() 
{
	if (m_pOsdDlg)
	{
		m_pOsdDlg->SetAutoSize(!m_pOsdDlg->DoAutoSize());
		m_pOsdDlg->UpdateDisplay();
	}
}

void CPictureDoc::OnUpdateOsdAutosize(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoAutoSize() ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdBkgcolorImage() 
{
	if (m_pOsdDlg)
	{
		m_pOsdDlg->SetUsePictureDocBackgroundColor(TRUE);
		m_pOsdDlg->UpdateDisplay();
	}
}

void CPictureDoc::OnUpdateOsdBkgcolorImage(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoUsePictureDocBackgroundColor() ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdBkgcolorSelect() 
{
	if (m_pOsdDlg)
	{
		COLORREF crBackgroundColor;
		GetView()->ForceCursor();
		m_pOsdDlg->ForceShow();
		if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(crBackgroundColor, GetView()))
		{
			m_pOsdDlg->SetUsePictureDocBackgroundColor(FALSE);
			m_pOsdDlg->SetBackgroundColor(crBackgroundColor);
		}
		m_pOsdDlg->ForceShow(FALSE);
		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::OnUpdateOsdBkgcolorSelect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdFontcolorSelect() 
{
	if (m_pOsdDlg)
	{
		COLORREF crFontColor;
		GetView()->ForceCursor();
		m_pOsdDlg->ForceShow();
		if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(crFontColor, GetView()))
			m_pOsdDlg->SetFontColor(crFontColor);
		m_pOsdDlg->ForceShow(FALSE);
		GetView()->ForceCursor(FALSE);
	}
}

void CPictureDoc::OnUpdateOsdFontcolorSelect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bEnableOsd);		
}

void CPictureDoc::OnOsdFontsizeSmall() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetFontSize(COsdDlg::FONT_SMALL);
}

void CPictureDoc::OnUpdateOsdFontsizeSmall(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetFontSize() == COsdDlg::FONT_SMALL ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdFontsizeMedium() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetFontSize(COsdDlg::FONT_MEDIUM);
}

void CPictureDoc::OnUpdateOsdFontsizeMedium(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetFontSize() == COsdDlg::FONT_MEDIUM ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdFontsizeBig() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetFontSize(COsdDlg::FONT_BIG);
}

void CPictureDoc::OnUpdateOsdFontsizeBig(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetFontSize() == COsdDlg::FONT_BIG ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOfftimeout3() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetTimeout(COsdDlg::TIMEOUT_3SEC);
}

void CPictureDoc::OnUpdateOsdOfftimeout3(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetTimeout() == COsdDlg::TIMEOUT_3SEC ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOfftimeout4() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetTimeout(COsdDlg::TIMEOUT_4SEC);
}

void CPictureDoc::OnUpdateOsdOfftimeout4(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetTimeout() == COsdDlg::TIMEOUT_4SEC ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOfftimeout5() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetTimeout(COsdDlg::TIMEOUT_5SEC);
}

void CPictureDoc::OnUpdateOsdOfftimeout5(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetTimeout() == COsdDlg::TIMEOUT_5SEC ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOfftimeout6() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetTimeout(COsdDlg::TIMEOUT_6SEC);	
}

void CPictureDoc::OnUpdateOsdOfftimeout6(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetTimeout() == COsdDlg::TIMEOUT_6SEC ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOfftimeout7() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetTimeout(COsdDlg::TIMEOUT_7SEC);
}

void CPictureDoc::OnUpdateOsdOfftimeout7(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetTimeout() == COsdDlg::TIMEOUT_7SEC ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOfftimeoutInfinite() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetTimeout(COsdDlg::TIMEOUT_INFINITE);
}

void CPictureDoc::OnUpdateOsdOfftimeoutInfinite(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetTimeout() == COsdDlg::TIMEOUT_INFINITE ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOpacity100() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetMaxOpacity(100); 
}

void CPictureDoc::OnUpdateOsdOpacity100(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetMaxOpacity() == 100 ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOpacity80() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetMaxOpacity(80);
}

void CPictureDoc::OnUpdateOsdOpacity80(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetMaxOpacity() == 80 ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOpacity60() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetMaxOpacity(60);
}

void CPictureDoc::OnUpdateOsdOpacity60(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetMaxOpacity() == 60 ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdOpacity40() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->SetMaxOpacity(40);
}

void CPictureDoc::OnUpdateOsdOpacity40(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->GetMaxOpacity() == 40 ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayFilename() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_FILENAME);
}

void CPictureDoc::OnUpdateOsdDisplayFilename(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_FILENAME) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplaySizescompression() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_SIZESCOMPRESSION);
}

void CPictureDoc::OnUpdateOsdDisplaySizescompression(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_SIZESCOMPRESSION) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayDate() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_DATE);
}

void CPictureDoc::OnUpdateOsdDisplayDate(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_DATE) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayMetadatadate() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_METADATADATE);
}

void CPictureDoc::OnUpdateOsdDisplayMetadatadate(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_METADATADATE) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayLocation() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_LOCATION);
}

void CPictureDoc::OnUpdateOsdDisplayLocation(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_LOCATION) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayHeadlineDescription() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_HEADLINEDESCRIPTION);
}

void CPictureDoc::OnUpdateOsdDisplayHeadlineDescription(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_HEADLINEDESCRIPTION) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayFlash() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_FLASH);
}

void CPictureDoc::OnUpdateOsdDisplayFlash(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_FLASH) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayExposuretime() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_EXPOSURETIME);
}

void CPictureDoc::OnUpdateOsdDisplayExposuretime(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_EXPOSURETIME) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

void CPictureDoc::OnOsdDisplayAperture() 
{
	if (m_pOsdDlg)
		m_pOsdDlg->ToggleDisplayState(COsdDlg::DISPLAY_APERTURE);
}

void CPictureDoc::OnUpdateOsdDisplayAperture(CCmdUI* pCmdUI) 
{
	if (m_pOsdDlg)
		pCmdUI->SetCheck(m_pOsdDlg->DoDisplayState(COsdDlg::DISPLAY_APERTURE) ? 1 : 0);
	pCmdUI->Enable(m_bEnableOsd);
}

BOOL CPictureDoc::ViewLayeredDlg() 
{
	if (m_pUpdateLayeredWindow)
	{
		if (m_pLayeredDlg)
		{
			// Kill thread
			m_LayeredDlgThread.Kill();

			// m_pLayeredDlg pointer is set to NULL
			// from the dialog class (selfdeletion)
			m_pLayeredDlg->Close();
			return TRUE;
		}
		else
		{
			// Wait and schedule command if dib not fully loaded!
			if (!IsDibReadyForCommand(ID_VIEW_LAYERED_DLG))
				return FALSE;

			// Allocate new Dlg
			m_pLayeredDlg = new CLayeredDlg(GetView());
			if (!m_pLayeredDlg)
				return FALSE;

			// Set flag
			m_bFirstLayeredDlgUpdate = TRUE;

			// Draw
			GetView()->Draw();
			
			return TRUE;
		}
	}
	else
		return FALSE;
}

void CPictureDoc::OnViewLayeredDlg()
{
	if (!m_bPrintPreviewMode)
		ViewLayeredDlg();
}

void CPictureDoc::OnUpdateViewLayeredDlg(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bPrintPreviewMode);
	pCmdUI->SetCheck(m_pLayeredDlg ? 1 : 0);
}

BOOL CPictureDoc::UpdateLayeredDlg(CDib* pDib)
{
	// Check
	if (!m_bDoUpdateLayeredDlg || !pDib || !pDib->IsValid() ||
		!m_pUpdateLayeredWindow || !m_pLayeredDlg)
		return FALSE;

	// Are the images identical?
	if (m_pLayeredDlg->m_CurrentLayeredDib.IsValid()													&&
		m_pLayeredDlg->m_CurrentLayeredDib.GetBMISize() == pDib->GetBMISize()							&&
		m_pLayeredDlg->m_CurrentLayeredDib.GetImageSize() == pDib->GetImageSize()						&&
		memcmp(m_pLayeredDlg->m_CurrentLayeredDib.GetBMI(), pDib->GetBMI(), pDib->GetBMISize()) == 0	&&
		memcmp(m_pLayeredDlg->m_CurrentLayeredDib.GetBits(), pDib->GetBits(), pDib->GetImageSize()) == 0)
		return TRUE;

	// Exit thread
	m_LayeredDlgThread.Kill();

	// Make copies
	m_pLayeredDlg->m_CurrentLayeredDib = *pDib;
	m_LayeredDlgThread.m_Dib = *pDib;

	// Vars
	m_LayeredDlgThread.m_nMaxsizePercent = m_nLayeredDlgMaxsizePercent;
	m_LayeredDlgThread.m_nSizePerthousand = m_nLayeredDlgSizePerthousand;

	// Adapt in case a preview dib is passed
	if (m_DocRect.Width() != pDib->GetWidth() || m_DocRect.Height() != pDib->GetHeight())
	{
		if (m_DocRect.Width() > m_DocRect.Height())
		{
			if (pDib->GetWidth() > 0)
				m_LayeredDlgThread.m_nSizePerthousand = Round((double)m_DocRect.Width() / (double)pDib->GetWidth() *
																(double)m_LayeredDlgThread.m_nSizePerthousand);
		}
		else
		{
			if (pDib->GetHeight() > 0)
				m_LayeredDlgThread.m_nSizePerthousand = Round((double)m_DocRect.Height() / (double)pDib->GetHeight() *
																(double)m_LayeredDlgThread.m_nSizePerthousand);
		}
	}

	// Calc. dialog size
	m_nLayeredDlgWidth = pDib->GetWidth();
	m_nLayeredDlgHeight = pDib->GetHeight();
	CSize szMonitor = ::AfxGetMainFrame()->GetMonitorSize();
	if (m_LayeredDlgThread.m_nMaxsizePercent)
	{
		int nMaxSizeX = m_LayeredDlgThread.m_nMaxsizePercent * szMonitor.cx / 100;
		int nMaxSizeY = m_LayeredDlgThread.m_nMaxsizePercent * szMonitor.cy / 100;
		if (nMaxSizeX + LAYERED_DLG_LEFTBORDER + LAYERED_DLG_RIGHTBORDER > szMonitor.cx)
			nMaxSizeX = szMonitor.cx - LAYERED_DLG_LEFTBORDER - LAYERED_DLG_RIGHTBORDER;
		if (nMaxSizeY + LAYERED_DLG_TOPBORDER + LAYERED_DLG_BOTTOMBORDER > szMonitor.cy)
			nMaxSizeY = szMonitor.cy - LAYERED_DLG_TOPBORDER - LAYERED_DLG_BOTTOMBORDER;
		if ((int)pDib->GetWidth() > nMaxSizeX || (int)pDib->GetHeight() > nMaxSizeY)
		{
			double dRatioX = (double)pDib->GetWidth() / (double)nMaxSizeX;
			double dRatioY = (double)pDib->GetHeight() / (double)nMaxSizeY;
			double dRatioMax = max(dRatioX, dRatioY);
			m_nLayeredDlgWidth = Round(pDib->GetWidth() / dRatioMax);
			m_nLayeredDlgHeight = Round(pDib->GetHeight() / dRatioMax);
		}
	}
	else
	{
		if (m_LayeredDlgThread.m_nSizePerthousand <= 0)
			m_LayeredDlgThread.m_nSizePerthousand = 125; // Min size
		int nMaxSizeX = LAYERED_DLG_BOUNDARY_PERCENT * szMonitor.cx / 100;
		int nMaxSizeY = LAYERED_DLG_BOUNDARY_PERCENT * szMonitor.cy / 100;
		if (nMaxSizeX + LAYERED_DLG_LEFTBORDER + LAYERED_DLG_RIGHTBORDER > szMonitor.cx)
			nMaxSizeX = szMonitor.cx - LAYERED_DLG_LEFTBORDER - LAYERED_DLG_RIGHTBORDER;
		if (nMaxSizeY + LAYERED_DLG_TOPBORDER + LAYERED_DLG_BOTTOMBORDER > szMonitor.cy)
			nMaxSizeY = szMonitor.cy - LAYERED_DLG_TOPBORDER - LAYERED_DLG_BOTTOMBORDER;
		int nWidth = pDib->GetWidth();
		int nHeight = pDib->GetHeight();
		nWidth *= m_LayeredDlgThread.m_nSizePerthousand / 1000; 
		nHeight *= m_LayeredDlgThread.m_nSizePerthousand / 1000;
		if (nWidth > nMaxSizeX && nHeight > nMaxSizeY)
		{
			m_nLayeredDlgWidth = MIN((int)pDib->GetWidth(), nMaxSizeX * 1000 / m_LayeredDlgThread.m_nSizePerthousand);
			m_nLayeredDlgHeight = MIN((int)pDib->GetHeight(), nMaxSizeY * 1000 / m_LayeredDlgThread.m_nSizePerthousand);
		}
		else if (nWidth > nMaxSizeX)
		{
			m_nLayeredDlgWidth = MIN((int)pDib->GetWidth(), nMaxSizeX * 1000 / m_LayeredDlgThread.m_nSizePerthousand);
			m_nLayeredDlgHeight = pDib->GetHeight();
		}
		else if (nHeight > nMaxSizeY)
		{
			m_nLayeredDlgWidth = pDib->GetWidth();
			m_nLayeredDlgHeight = MIN((int)pDib->GetHeight(), nMaxSizeY * 1000 / m_LayeredDlgThread.m_nSizePerthousand);
		}
		if (m_LayeredDlgThread.m_nSizePerthousand != 1000)
		{
			m_nLayeredDlgWidth = m_nLayeredDlgWidth * m_LayeredDlgThread.m_nSizePerthousand / 1000;
			m_nLayeredDlgHeight = m_nLayeredDlgHeight * m_LayeredDlgThread.m_nSizePerthousand / 1000;
		}
	}
	m_nLayeredDlgWidth	+= (LAYERED_DLG_LEFTBORDER	+ LAYERED_DLG_RIGHTBORDER);
	m_nLayeredDlgHeight	+= (LAYERED_DLG_TOPBORDER	+ LAYERED_DLG_BOTTOMBORDER);

	// Start it
	if ((m_GifAnimationThread.m_dwDibAnimationCount > 1) &&
		m_GifAnimationThread.IsAlive())
	{
		// Set z-order and right size
		::SetWindowPos(	m_pLayeredDlg->GetSafeHwnd(),
						m_pLayeredDlg->IsWindowEnabled() ? HWND_TOPMOST : HWND_NOTOPMOST,
						0, 0,
						m_nLayeredDlgWidth,
						m_nLayeredDlgHeight,
						SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE);

		// First show (see the above SetWindowPos) then modify style,
		// otherwise under Vista it is not working correctly!
		m_pLayeredDlg->ModifyStyleEx(0, WS_EX_LAYERED);

		// Reset flag
		m_bFirstLayeredDlgUpdate = FALSE;

		// No thread
		return m_LayeredDlgThread.DoIt();
	}
	else
	{
		HDC hScreenDC, hMemDC;
		HGDIOBJ hOldBitmap;
		POINT ptWindowScreenPosition;
		SIZE szWindow;
		if (m_bFirstLayeredDlgUpdate)
		{
			// Fully Transparent Empty Dib
			CDib EmptyDib;
			EmptyDib.AllocateBits(	32, BI_RGB,
									m_nLayeredDlgWidth - LAYERED_DLG_LEFTBORDER - LAYERED_DLG_RIGHTBORDER,
									m_nLayeredDlgHeight - LAYERED_DLG_TOPBORDER - LAYERED_DLG_BOTTOMBORDER,
									RGBA(0,0,0,255));
			EmptyDib.SetAlpha(TRUE);
			EmptyDib.AddBorders(LAYERED_DLG_LEFTBORDER,
								LAYERED_DLG_TOPBORDER,
								LAYERED_DLG_RIGHTBORDER,
								LAYERED_DLG_BOTTOMBORDER,
								0);

			// The DCs
			hScreenDC = ::GetDC(NULL);
			hMemDC = ::CreateCompatibleDC(hScreenDC);
			hOldBitmap = ::SelectObject(hMemDC, EmptyDib.GetSafeHandle());

			// Get the window rectangle
			CRect rcDlg;
			m_pLayeredDlg->GetWindowRect(&rcDlg);

			// Calculate the new window position/size based on the bitmap size
			ptWindowScreenPosition.x = rcDlg.left;
			ptWindowScreenPosition.y = rcDlg.top;
			szWindow.cx = EmptyDib.GetWidth();
			szWindow.cy = EmptyDib.GetHeight();
		}

		// Set z-order and right size
		::SetWindowPos(	m_pLayeredDlg->GetSafeHwnd(),
						m_pLayeredDlg->IsWindowEnabled() ? HWND_TOPMOST : HWND_NOTOPMOST,
						0, 0,
						m_nLayeredDlgWidth,
						m_nLayeredDlgHeight,
						SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE);

		// Perform the alpha blend
		if (m_bFirstLayeredDlgUpdate)
		{
			// First show (see the above SetWindowPos) then modify style,
			// otherwise under Vista it is not working correctly!
			m_pLayeredDlg->ModifyStyleEx(0, WS_EX_LAYERED);

			// Setup the blend function
			BLENDFUNCTION blendPixelFunction= {AC_SRC_OVER, 0, (BYTE)m_nLayeredDlgOpacity, AC_SRC_ALPHA};
			POINT ptSrc;
			ptSrc.x = 0;
			ptSrc.y = 0;
			m_pUpdateLayeredWindow(	m_pLayeredDlg->GetSafeHwnd(),
									hScreenDC,
									&ptWindowScreenPosition,
									&szWindow,
									hMemDC,
									&ptSrc,
									0,
									&blendPixelFunction,
									ULW_ALPHA);

			// Clean-up
			::SelectObject(hMemDC, hOldBitmap);

			// Reset flag
			m_bFirstLayeredDlgUpdate = FALSE;
		}

		// NOTE: SetWindowPos() and ModifyStyleEx() cannot be inside the thread
		// because they call the UI thread and this can dead-lock with a Kill()
		// called from the UI thread!
		return m_LayeredDlgThread.Start() ? TRUE : FALSE;
	}
}

void CPictureDoc::OnLayereddlgMaxsize066() 
{
	if (m_nLayeredDlgMaxsizePercent != 66)
	{
		m_nLayeredDlgSizePerthousand = 0;
		m_nLayeredDlgMaxsizePercent = 66;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgMaxsize066(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgMaxsizePercent == 66 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgMaxsize050() 
{
	if (m_nLayeredDlgMaxsizePercent != 50)
	{
		m_nLayeredDlgSizePerthousand = 0;
		m_nLayeredDlgMaxsizePercent = 50;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgMaxsize050(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgMaxsizePercent == 50 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgMaxsize033() 
{
	if (m_nLayeredDlgMaxsizePercent != 33)
	{
		m_nLayeredDlgSizePerthousand = 0;
		m_nLayeredDlgMaxsizePercent = 33;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgMaxsize033(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgMaxsizePercent == 33 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgMaxsize025() 
{
	if (m_nLayeredDlgMaxsizePercent != 25)
	{
		m_nLayeredDlgSizePerthousand = 0;
		m_nLayeredDlgMaxsizePercent = 25;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgMaxsize025(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgMaxsizePercent == 25 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize0125() 
{
	if (m_nLayeredDlgSizePerthousand != 125)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 125;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}	
}

void CPictureDoc::OnUpdateLayereddlgSize0125(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 125 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize025() 
{
	if (m_nLayeredDlgSizePerthousand != 250)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 250;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgSize025(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 250 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize050() 
{
	if (m_nLayeredDlgSizePerthousand != 500)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 500;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgSize050(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 500 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize100() 
{
	if (m_nLayeredDlgSizePerthousand != 1000)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 1000;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgSize100(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 1000 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize200() 
{
	if (m_nLayeredDlgSizePerthousand != 2000)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 2000;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgSize200(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 2000 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize400() 
{
	if (m_nLayeredDlgSizePerthousand != 4000)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 4000;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgSize400(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 4000 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize800() 
{
	if (m_nLayeredDlgSizePerthousand != 8000)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 8000;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgSize800(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 8000 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSize1600() 
{
	if (m_nLayeredDlgSizePerthousand != 16000)
	{
		m_nLayeredDlgMaxsizePercent = 0;
		m_nLayeredDlgSizePerthousand = 16000;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgMaxsizePercent"),
										m_nLayeredDlgMaxsizePercent);
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgSizePerthousand"),
										m_nLayeredDlgSizePerthousand);
	}
}

void CPictureDoc::OnUpdateLayereddlgSize1600(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgSizePerthousand == 16000 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSizeTopleft() 
{
	if (m_nLayeredDlgOrigin != 0)
	{
		m_nLayeredDlgOrigin = 0;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOrigin"),
										m_nLayeredDlgOrigin);
	}
}

void CPictureDoc::OnUpdateLayereddlgSizeTopleft(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOrigin == 0 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSizeTopright() 
{
	if (m_nLayeredDlgOrigin != 1)
	{
		m_nLayeredDlgOrigin = 1;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOrigin"),
										m_nLayeredDlgOrigin);
	}
}

void CPictureDoc::OnUpdateLayereddlgSizeTopright(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOrigin == 1 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSizeBottomleft() 
{
	if (m_nLayeredDlgOrigin != 2)
	{
		m_nLayeredDlgOrigin = 2;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOrigin"),
										m_nLayeredDlgOrigin);
	}
}

void CPictureDoc::OnUpdateLayereddlgSizeBottomleft(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOrigin == 2 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgSizeBottomright() 
{
	if (m_nLayeredDlgOrigin != 3)
	{
		m_nLayeredDlgOrigin = 3;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOrigin"),
										m_nLayeredDlgOrigin);
	}
}

void CPictureDoc::OnUpdateLayereddlgSizeBottomright(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOrigin == 3 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgOpacity100() 
{
	if (m_nLayeredDlgOpacity != 255)
	{
		m_nLayeredDlgOpacity = 255;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOpacity"),
										m_nLayeredDlgOpacity);
	}
}

void CPictureDoc::OnUpdateLayereddlgOpacity100(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOpacity == 255 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgOpacity80() 
{
	if (m_nLayeredDlgOpacity != 204)
	{
		m_nLayeredDlgOpacity = 204;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOpacity"),
										m_nLayeredDlgOpacity);
	}
}

void CPictureDoc::OnUpdateLayereddlgOpacity80(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOpacity == 204 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgOpacity60() 
{
	if (m_nLayeredDlgOpacity != 153)
	{
		m_nLayeredDlgOpacity = 153;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOpacity"),
										m_nLayeredDlgOpacity);
	}
}

void CPictureDoc::OnUpdateLayereddlgOpacity60(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOpacity == 153 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgOpacity40() 
{
	if (m_nLayeredDlgOpacity != 102)
	{
		m_nLayeredDlgOpacity = 102;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOpacity"),
										m_nLayeredDlgOpacity);
	}
}

void CPictureDoc::OnUpdateLayereddlgOpacity40(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOpacity == 102 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgOpacity20() 
{
	if (m_nLayeredDlgOpacity != 51)
	{
		m_nLayeredDlgOpacity = 51;
		if (m_pLayeredDlg)
			m_pLayeredDlg->m_CurrentLayeredDib.Free();
		GetView()->Draw();
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("LayeredDlgOpacity"),
										m_nLayeredDlgOpacity);
	}
}

void CPictureDoc::OnUpdateLayereddlgOpacity20(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nLayeredDlgOpacity == 51 ? 1 : 0);
}

void CPictureDoc::OnLayereddlgPaste() 
{
	// Hide window so that WindowFromPoint() is not
	// getting the LayeredDlg window
	m_pLayeredDlg->ShowWindow(SW_HIDE);

	// To get the right or bottom edges of the visible dialog
	// we use m_nLayeredDlgDibWidth or m_nLayeredDlgDibHeight
	// and not rcDlg.right or rcDlg.bottom!
	CRect rcDlg;
	m_pLayeredDlg->GetWindowRect(&rcDlg);
	CPoint ptCorner;
	switch (m_nLayeredDlgOrigin)
	{
		// Top-Left
		case 0 :
			ptCorner = CPoint(rcDlg.left, rcDlg.top);
			ptCorner.x += LAYERED_DLG_LEFTBORDER;
			ptCorner.y += LAYERED_DLG_TOPBORDER;
			break;
		// Top-Right
		case 1 :
			ptCorner = CPoint(rcDlg.left + m_nLayeredDlgWidth - 1, rcDlg.top);
			ptCorner.x -= LAYERED_DLG_RIGHTBORDER;
			ptCorner.y += LAYERED_DLG_TOPBORDER;
			break;
		// Bottom-Left
		case 2 :
			ptCorner = CPoint(rcDlg.left, rcDlg.top + m_nLayeredDlgHeight - 1);
			ptCorner.x += LAYERED_DLG_LEFTBORDER;
			ptCorner.y -= LAYERED_DLG_BOTTOMBORDER;
			break;
		// Bottom-Right
		default :
			ptCorner = CPoint(rcDlg.left + m_nLayeredDlgWidth - 1, rcDlg.top + m_nLayeredDlgHeight - 1);
			ptCorner.x -= LAYERED_DLG_RIGHTBORDER;
			ptCorner.y -= LAYERED_DLG_BOTTOMBORDER;
			break;
	}
	CWnd* pWnd = CWnd::WindowFromPoint(ptCorner);
	if (!pWnd || !pWnd->IsKindOf(RUNTIME_CLASS(CPictureView)))
	{
		pWnd = CWnd::WindowFromPoint(m_pLayeredDlg->m_ptLastRightClick);
		if (!pWnd || !pWnd->IsKindOf(RUNTIME_CLASS(CPictureView)))
		{
			m_pLayeredDlg->ShowWindow(SW_SHOWNOACTIVATE);
			::AfxMessageBox(ML_STRING(1331, "Please paste over a picture document."), MB_OK | MB_ICONSTOP);
			return;
		}
	}
	m_pLayeredDlg->ShowWindow(SW_SHOWNOACTIVATE);
	CPictureView* pView = (CPictureView*)pWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	if (pDoc									&&
		pDoc->m_dwIDAfterFullLoadCommand == 0	&&
		pDoc->DoEnableCommand())
	{
		// Check whether fully loaded
		if (!pDoc->m_pDib || !pDoc->m_pDib->IsValid())
			::AfxMessageBox(ML_STRING(1332, "Please try again later, picture is loading."), MB_OK | MB_ICONINFORMATION);
		else
		{
			pView->ScreenToClient(&ptCorner);
			OnEditCopy();
			pDoc->EditPasteInto(m_nLayeredDlgOrigin, ptCorner, m_nLayeredDlgOpacity - 255);
		}
	}
}

void CPictureDoc::OnLayereddlgClose() 
{
	ViewLayeredDlg();
}

/////////////////////////////////////////////////////////////////////////////
// CPictureDoc diagnostics

#ifdef _DEBUG
void CPictureDoc::AssertValid() const
{
	CUImagerDoc::AssertValid();
}

void CPictureDoc::Dump(CDumpContext& dc) const
{
	CUImagerDoc::Dump(dc);
}
#endif //_DEBUG