/*
 * EVENTS.h
 *
 * Events and State Variables
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
*/

#ifndef EVENTS_H
#define EVENTS_H

#include <echidna/platform.h>
#include "switches.h"

#include <echidna/menus2.h>


/*
 * State variables.
 */
extern short	EditOnlyFloorSTATE;
extern short	HidePointerSTATE;
extern short	KeepDownloadPaletteSTATE;
extern short	ScrollLockSTATE;
extern short	SetStampPaintSTATE;
extern short	SetStampReplaceSTATE;
extern short	SpaceSTATE;
extern short	CycleColorsSTATE;
extern short	DownloadOneScreenSTATE;
extern short	JamPaletteSTATE;
extern short	InvisiLayerSTATE;
extern short	LockLayerSTATE;
extern short	LockRoomSTATE;
extern short	fPixelSelect;
extern short	ShowBrushSTATE;
extern short	ShowGuideSTATE;
extern short	ShowGuide2STATE;
extern short	fShowTileUsage;
extern short	SmartFlipSTATE;
extern short	StratifyPasteSTATE;
extern short	SwankyModeSTATE;
extern short	ShowTitleSTATE;
extern short	UseGridSTATE;
extern short	ShowZoomSTATE;
extern short	UseEditPaletteSTATE;
extern short	WLCSTATE;
extern short	WLESTATE;
extern short	WLSSTATE;
extern short	XFlipBrushSTATE;
extern short	YFlipBrushSTATE;

/*
 * Events.
 */
extern EventState	AboutState;
extern EventState	AppendLayerState;
extern EventState	AppendMapState;
extern EventState	BottomFloorState;
extern EventState	CenterRoomOnCursorState;
extern EventState	ChoosePrinterState;
extern EventState	ClearAllLayerShiftsState;
extern EventState	ClearLayerShiftState;
extern EventState	ClearMapState;
extern EventState	ClearRoomState;
extern EventState	ClearRoom2State;
extern EventState	ClearRoom3State;
extern EventState	CopyColors1State;
extern EventState	CopyColors2State;
extern EventState	CopyColors3State;
extern EventState	CopyrightStatusState;
extern EventState	CountCharsState;
extern EventState	CreateRoomState;
extern EventState	CursorStatusState;
extern EventState	DebugState;
extern EventState	DecBackColorState;
extern EventState	DeleteLayerState;
extern EventState	DeleteRoomState;
extern EventState	DeleteTilesetState;
extern EventState	Download16State;
extern EventState	Download256State;
extern EventState	DrawRandomState;
extern EventState	EditOnlyFloorState;
extern EventState	EditRoomState;
extern EventState	ExportBrushState;
extern EventState	ExportRoomState;
extern EventState	ExportScreenState;
extern EventState	FlipPanesState;
extern EventState	FloorDownState;
extern EventState	FloorUpState;
extern EventState	GetGridFromBrushState;
extern EventState	GetGuideFromBrushState;
extern EventState	GoEndState;
extern EventState	GoHomeState;
extern EventState	GroupSaveTilesState;
extern EventState	HideCursorState;
extern EventState	HighlightTileState;
extern EventState	IncBackColorState;
extern EventState	InsertLayerState;
extern EventState	KeepDownloadPaletteState;
extern EventState	LastMenuEventState;
extern EventState	LayerShiftStatusState;
extern EventState	LoadLayerState;
extern EventState	LoadMapState;
extern EventState	LoadPaletteState;
extern EventState	LoadPaletteRangeState;
extern EventState	LoadRoomState;
extern EventState	LoadTilesAllTiledState;
extern EventState	LoadTilesBoxedState;
extern EventState	LoadTilesCookieCutterState;
extern EventState	LoadTilesFullTiledState;
extern EventState	LoadTilesFullTiledNoBlankState;
extern EventState	MakeCompositeTilesState;
extern EventState	NextRoomState;
extern EventState	OpenLayerState;
extern EventState	PreviousRoomState;
extern EventState	PrintMapState;
extern EventState	QuitAndExitState;
extern EventState	ReplaceState;
extern EventState	RoomStatusState;
extern EventState	SaveLayerState;
extern EventState	SaveMapState;
extern EventState	SavePaletteState;
extern EventState	SavePaletteRangeState;
extern EventState	SaveRoomState;
extern EventState	SaveRoomAllState;
extern EventState	SaveTilesAsBrushesState;
extern EventState	ScrollLockState;
extern EventState	ScrollLockKEYState;
extern EventState	SearchNextState;
extern EventState	SelectBlockState;
extern EventState	SelectSquareState;
extern EventState	SetGridOriginState;
extern EventState	SetGridSizeState;
extern EventState	SetGuideOriginState;
extern EventState	SetGuideSizeState;
extern EventState	SetMaxTileUsageState;
extern EventState	SetRoomInfoState;
extern EventState	SetSearchBufferState;
extern EventState	SetStampPaintState;
extern EventState	SetStampReplaceState;
extern EventState	SetTheColorsState;
extern EventState	SetTilesetInfoState;
extern EventState	ShowBrushCountState;
extern EventState	SpaceToggleState;
extern EventState	StripDownBlockCopyState;
extern EventState	TileStatusState;
extern EventState	TMGCSaveTilesState;
extern EventState	ToggleCycleColorsState;
extern EventState	ToggleDownloadOneScreenState;
extern EventState	ToggleJamPaletteState;
extern EventState	ToggleLAllState;
extern EventState	ToggleLInvisiState;
extern EventState	ToggleLLockState;
extern EventState	ToggleLockRoomState;
extern EventState	TogglePixelSelectState;
extern EventState	ToggleShowBrushState;
extern EventState	ToggleShowGuideState;
extern EventState	ToggleShowGuide2State;
extern EventState	ToggleShowTileUsageState;
extern EventState	ToggleSmartFlipState;
extern EventState	ToggleStratifyPasteState;
extern EventState	ToggleSwankyModeState;
extern EventState	ToggleTitleBarState;
extern EventState	ToggleUseGridState;
extern EventState	ToggleZoomState;
extern EventState	TopFloorState;
extern EventState	tumeInfoState;
extern EventState	UndoState;
extern EventState	UseEditPaletteState;
extern EventState	UserStatusState;
extern EventState	VersionStatusState;
extern EventState	WCloseState;
extern EventState	WCreateState;
extern EventState	WLockClearState;
extern EventState	WLockToEditState;
extern EventState	WLockToSameState;
extern EventState	WLockToSourceState;
extern EventState	XFlipBrushState;
extern EventState	YFlipBrushState;
extern EventState	ZaveMapState;
extern EventState	ZeroBackColorState;
extern EventState	ZoomInState;
extern EventState	ZoomOutState;

#endif /* EVENTS_H */

