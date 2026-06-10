/*
 * EVENTS.c
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
#include <echidna/platform.h>
#include "switches.h"

#include <echidna/menus2.h>
#include <echidna/listfunc.h>
#include "EVENTS.H"

#define LOCAL	static
#define FORWARD	extern
				/* To distingish between actual      */
				/* references and forward references */


/*
 * Extern all functions.
 */
extern short		About ();
extern short		AppendLayer ();
extern short		AMap ();
extern short		BottomFloor ();
extern short		CenterRoomOnCursor ();
extern short		ChoosePrinterReq ();
extern short		ClearAllLayerShifts ();
extern short		ClearLayerShift ();
extern short		NMap ();
extern short		ClearRoom ();
extern short		ClearRoom2 ();
extern short		ClearRoom3 ();
extern short		CopyColors1 ();
extern short		CopyColors2 ();
extern short		CopyColors3 ();
extern short		CopyrightStatus ();
extern short		CountChars ();
extern short		CRoom ();
extern short		CursorStatus ();
extern short		TE_Debug ();
extern short		DecGColor ();
extern short		DeleteLayer ();
extern short		DRoom ();
extern short		DeleteTileSet ();
extern short		DownloadMap16 ();
extern short		DownloadMap256 ();
extern short		DrawRandom ();
extern short		EditOnlyFloor ();
extern short		EditRoom ();
extern short		ExportBrush ();
extern short		ExportRoom ();
extern short		ExportScreen ();
extern short		WFRoom ();
extern short		FloorDown ();
extern short		FloorUp ();
extern short		GetGridFromBrush ();
extern short		GetGuideFromBrush ();
extern short		GoEnd ();
extern short		GoHome ();
extern short		SMap3 ();
extern short		TE_TogglePointer ();
extern short		HighlightTile ();
extern short		IncGColor ();
extern short		InsertLayer ();
extern short		KeepDownloadPalette ();
extern short		LastMenuEvent ();
extern short		LayerShiftStatus ();
extern short		LoadLayer ();
extern short		LMap ();
extern short		LoadPalette ();
extern short		LoadPaletteRange ();
extern short		LRoom ();
extern short		TE_LoadTilesAll ();
extern short		TE_LoadTilesBoxed ();
extern short		TE_LoadTilesCookieCutter ();
extern short		TE_LoadTilesFullTiled ();
extern short		TE_LoadTilesFullTiledNoBlank ();
extern short		MakeCompositeTiles ();
extern short		WNRoom ();
extern short		OpenLayer ();
extern short		WPRoom ();
extern short		PrintMap ();
extern short		QuitAndExit ();
extern short		Replace ();
extern short		RoomStatus ();
extern short		SaveLayer ();
extern short		SMap1 ();
extern short		SavePalette ();
extern short		SavePaletteRange ();
extern short		SRoom ();
extern short		SRoomAll ();
extern short		TE_SaveTilesAsBrushes ();
extern short		ScrollLock ();
extern short		ScrollLockKEY ();
extern short		SearchNext ();
extern short		US ();
extern short		US2 ();
extern short		SetGridOrigin ();
extern short		SetGridSize ();
extern short		SetGuideOrigin ();
extern short		SetGuideSize ();
extern short		SetMaxTileUsage ();
extern short		SetRoomInfo ();
extern short		SetSearchBuffer ();
extern short		SetStampPaint ();
extern short		SetStampReplace ();
extern short		SetTheColors ();
extern short		SetTilesetInfo ();
extern short		ShowBrushCount ();
extern short		SpaceToggle ();
extern short		StripDownBlockCopy ();
extern short		TUserStatus ();
extern short		SMap4 ();
extern short		ToggleCycleColors ();
extern short		ToggleDownloadOneScreen ();
extern short		ToggleJamPalette ();
extern short		ToggleLAll ();
extern short		ToggleLInvisi ();
extern short		ToggleLLock ();
extern short		ToggleLockRoom ();
extern short		TogglePixelSelect ();
extern short		ToggleShowBrush ();
extern short		ToggleShowGuide ();
extern short		ToggleShowGuide2 ();
extern short		ToggleShowTileUsage ();
extern short		ToggleSwankyMode ();
extern short		ToggleSwankyMode ();
extern short		ToggleSwankyMode ();
extern short		ToggleTitleBar ();
extern short		ToggleUseGrid ();
extern short		ToggleZoom ();
extern short		TopFloor ();
extern short		tumeInfo ();
extern short		Undo ();
extern short		UseEditPalette ();
extern short		UserStatus ();
extern short		VersionStatus ();
extern short		WClose ();
extern short		WCreate ();
extern short		WLCl ();
extern short		WLE ();
extern short		WLC ();
extern short		WLS ();
extern short		XFlipBrush ();
extern short		YFlipBrush ();
extern short		SMap2 ();
extern short		ZGColor ();
extern short		ZoomIn ();
extern short		ZoomOut ();

/*
 * Forward all States.
 */
FORWARD EventState	AboutState;
FORWARD EventState	AppendLayerState;
FORWARD EventState	AppendMapState;
FORWARD EventState	BottomFloorState;
FORWARD EventState	CenterRoomOnCursorState;
FORWARD EventState	ChoosePrinterState;
FORWARD EventState	ClearAllLayerShiftsState;
FORWARD EventState	ClearLayerShiftState;
FORWARD EventState	ClearMapState;
FORWARD EventState	ClearRoomState;
FORWARD EventState	ClearRoom2State;
FORWARD EventState	ClearRoom3State;
FORWARD EventState	CopyColors1State;
FORWARD EventState	CopyColors2State;
FORWARD EventState	CopyColors3State;
FORWARD EventState	CopyrightStatusState;
FORWARD EventState	CountCharsState;
FORWARD EventState	CreateRoomState;
FORWARD EventState	CursorStatusState;
FORWARD EventState	DebugState;
FORWARD EventState	DecBackColorState;
FORWARD EventState	DeleteLayerState;
FORWARD EventState	DeleteRoomState;
FORWARD EventState	DeleteTilesetState;
FORWARD EventState	Download16State;
FORWARD EventState	Download256State;
FORWARD EventState	DrawRandomState;
FORWARD EventState	EditOnlyFloorState;
FORWARD EventState	EditRoomState;
FORWARD EventState	ExportBrushState;
FORWARD EventState	ExportRoomState;
FORWARD EventState	ExportScreenState;
FORWARD EventState	FlipPanesState;
FORWARD EventState	FloorDownState;
FORWARD EventState	FloorUpState;
FORWARD EventState	GetGridFromBrushState;
FORWARD EventState	GetGuideFromBrushState;
FORWARD EventState	GoEndState;
FORWARD EventState	GoHomeState;
FORWARD EventState	GroupSaveTilesState;
FORWARD EventState	HideCursorState;
FORWARD EventState	HighlightTileState;
FORWARD EventState	IncBackColorState;
FORWARD EventState	InsertLayerState;
FORWARD EventState	KeepDownloadPaletteState;
FORWARD EventState	LastMenuEventState;
FORWARD EventState	LayerShiftStatusState;
FORWARD EventState	LoadLayerState;
FORWARD EventState	LoadMapState;
FORWARD EventState	LoadPaletteState;
FORWARD EventState	LoadPaletteRangeState;
FORWARD EventState	LoadRoomState;
FORWARD EventState	LoadTilesAllTiledState;
FORWARD EventState	LoadTilesBoxedState;
FORWARD EventState	LoadTilesCookieCutterState;
FORWARD EventState	LoadTilesFullTiledState;
FORWARD EventState	LoadTilesFullTiledNoBlankState;
FORWARD EventState	MakeCompositeTilesState;
FORWARD EventState	NextRoomState;
FORWARD EventState	OpenLayerState;
FORWARD EventState	PreviousRoomState;
FORWARD EventState	PrintMapState;
FORWARD EventState	QuitAndExitState;
FORWARD EventState	ReplaceState;
FORWARD EventState	RoomStatusState;
FORWARD EventState	SaveLayerState;
FORWARD EventState	SaveMapState;
FORWARD EventState	SavePaletteState;
FORWARD EventState	SavePaletteRangeState;
FORWARD EventState	SaveRoomState;
FORWARD EventState	SaveRoomAllState;
FORWARD EventState	SaveTilesAsBrushesState;
FORWARD EventState	ScrollLockState;
FORWARD EventState	ScrollLockKEYState;
FORWARD EventState	SearchNextState;
FORWARD EventState	SelectBlockState;
FORWARD EventState	SelectSquareState;
FORWARD EventState	SetGridOriginState;
FORWARD EventState	SetGridSizeState;
FORWARD EventState	SetGuideOriginState;
FORWARD EventState	SetGuideSizeState;
FORWARD EventState	SetMaxTileUsageState;
FORWARD EventState	SetRoomInfoState;
FORWARD EventState	SetSearchBufferState;
FORWARD EventState	SetStampPaintState;
FORWARD EventState	SetStampReplaceState;
FORWARD EventState	SetTheColorsState;
FORWARD EventState	SetTilesetInfoState;
FORWARD EventState	ShowBrushCountState;
FORWARD EventState	SpaceToggleState;
FORWARD EventState	StripDownBlockCopyState;
FORWARD EventState	TileStatusState;
FORWARD EventState	TMGCSaveTilesState;
FORWARD EventState	ToggleCycleColorsState;
FORWARD EventState	ToggleDownloadOneScreenState;
FORWARD EventState	ToggleJamPaletteState;
FORWARD EventState	ToggleLAllState;
FORWARD EventState	ToggleLInvisiState;
FORWARD EventState	ToggleLLockState;
FORWARD EventState	ToggleLockRoomState;
FORWARD EventState	TogglePixelSelectState;
FORWARD EventState	ToggleShowBrushState;
FORWARD EventState	ToggleShowGuideState;
FORWARD EventState	ToggleShowGuide2State;
FORWARD EventState	ToggleShowTileUsageState;
FORWARD EventState	ToggleSmartFlipState;
FORWARD EventState	ToggleStratifyPasteState;
FORWARD EventState	ToggleSwankyModeState;
FORWARD EventState	ToggleTitleBarState;
FORWARD EventState	ToggleUseGridState;
FORWARD EventState	ToggleZoomState;
FORWARD EventState	TopFloorState;
FORWARD EventState	tumeInfoState;
FORWARD EventState	UndoState;
FORWARD EventState	UseEditPaletteState;
FORWARD EventState	UserStatusState;
FORWARD EventState	VersionStatusState;
FORWARD EventState	WCloseState;
FORWARD EventState	WCreateState;
FORWARD EventState	WLockClearState;
FORWARD EventState	WLockToEditState;
FORWARD EventState	WLockToSameState;
FORWARD EventState	WLockToSourceState;
FORWARD EventState	XFlipBrushState;
FORWARD EventState	YFlipBrushState;
FORWARD EventState	ZaveMapState;
FORWARD EventState	ZeroBackColorState;
FORWARD EventState	ZoomInState;
FORWARD EventState	ZoomOutState;


/*
 * Exclude Lists.
 */

/*
 * Exclude list for 'SetStampPaint'
 */
FORWARD ListType	__SetStampPaintExcList;

StateTracker __SetStampPaintExcSetStampReplace = {
	{
		(NodeType *)&__SetStampPaintExcList.Tail,
		(NodeType *)&__SetStampPaintExcList,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&SetStampReplaceState,
};

ListType __SetStampPaintExcList = {
	(NodeType *)&__SetStampPaintExcSetStampReplace,
	NULL,
	(NodeType *)&__SetStampPaintExcSetStampReplace,
	sizeof (ListType),
	0,
	NULL,
};

/*
 * Exclude list for 'SetStampReplace'
 */
FORWARD ListType	__SetStampReplaceExcList;

StateTracker __SetStampReplaceExcSetStampPaint = {
	{
		(NodeType *)&__SetStampReplaceExcList.Tail,
		(NodeType *)&__SetStampReplaceExcList,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&SetStampPaintState,
};

ListType __SetStampReplaceExcList = {
	(NodeType *)&__SetStampReplaceExcSetStampPaint,
	NULL,
	(NodeType *)&__SetStampReplaceExcSetStampPaint,
	sizeof (ListType),
	0,
	NULL,
};

/*
 * Exclude list for 'WLockClear'
 */
FORWARD ListType	__WLockClearExcList;
FORWARD StateTracker	__WLockClearExcWLockToEdit;

StateTracker __WLockClearExcWLockToSource = {
	{
		(NodeType *)&__WLockClearExcWLockToEdit,
		(NodeType *)&__WLockClearExcList,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&WLockToSourceState,
};

StateTracker __WLockClearExcWLockToEdit = {
	{
		(NodeType *)&__WLockClearExcList.Tail,
		(NodeType *)&__WLockClearExcWLockToSource,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&WLockToEditState,
};

ListType __WLockClearExcList = {
	(NodeType *)&__WLockClearExcWLockToSource,
	NULL,
	(NodeType *)&__WLockClearExcWLockToEdit,
	sizeof (ListType),
	0,
	NULL,
};

/*
 * Exclude list for 'WLockToEdit'
 */
FORWARD ListType	__WLockToEditExcList;
FORWARD StateTracker	__WLockToEditExcWLockClear;

StateTracker __WLockToEditExcWLockToSource = {
	{
		(NodeType *)&__WLockToEditExcWLockClear,
		(NodeType *)&__WLockToEditExcList,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&WLockToSourceState,
};

StateTracker __WLockToEditExcWLockClear = {
	{
		(NodeType *)&__WLockToEditExcList.Tail,
		(NodeType *)&__WLockToEditExcWLockToSource,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&WLockClearState,
};

ListType __WLockToEditExcList = {
	(NodeType *)&__WLockToEditExcWLockToSource,
	NULL,
	(NodeType *)&__WLockToEditExcWLockClear,
	sizeof (ListType),
	0,
	NULL,
};

/*
 * Exclude list for 'WLockToSource'
 */
FORWARD ListType	__WLockToSourceExcList;
FORWARD StateTracker	__WLockToSourceExcWLockClear;

StateTracker __WLockToSourceExcWLockToEdit = {
	{
		(NodeType *)&__WLockToSourceExcWLockClear,
		(NodeType *)&__WLockToSourceExcList,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&WLockToEditState,
};

StateTracker __WLockToSourceExcWLockClear = {
	{
		(NodeType *)&__WLockToSourceExcList.Tail,
		(NodeType *)&__WLockToSourceExcWLockToEdit,
		sizeof (StateTracker),
		0,
		NULL,
	},
	&WLockClearState,
};

ListType __WLockToSourceExcList = {
	(NodeType *)&__WLockToSourceExcWLockToEdit,
	NULL,
	(NodeType *)&__WLockToSourceExcWLockClear,
	sizeof (ListType),
	0,
	NULL,
};

/*
 * The States.
 */
FORWARD ListType	EventListx;

/*
 * 'About' State
 */
FORWARD EventState	AppendLayerState;
EventState AboutState = {
	{
		(NodeType *)&AppendLayerState,
		(NodeType *)&EventListx.Head,
		sizeof (EventState),
		0,
		"About",
	},
	About,
	NULL,
	NULL,
	NULL,
};

/*
 * 'AppendLayer' State
 */
FORWARD EventState	AppendMapState;
EventState AppendLayerState = {
	{
		(NodeType *)&AppendMapState,
		(NodeType *)&AboutState,
		sizeof (EventState),
		0,
		"AppendLayer",
	},
	AppendLayer,
	NULL,
	NULL,
	NULL,
};

/*
 * 'AppendMap' State
 */
FORWARD EventState	BottomFloorState;
EventState AppendMapState = {
	{
		(NodeType *)&BottomFloorState,
		(NodeType *)&AppendLayerState,
		sizeof (EventState),
		0,
		"AppendMap",
	},
	AMap,
	NULL,
	NULL,
	NULL,
};

/*
 * 'BottomFloor' State
 */
FORWARD EventState	CenterRoomOnCursorState;
EventState BottomFloorState = {
	{
		(NodeType *)&CenterRoomOnCursorState,
		(NodeType *)&AppendMapState,
		sizeof (EventState),
		0,
		"BottomFloor",
	},
	BottomFloor,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CenterRoomOnCursor' State
 */
FORWARD EventState	ChoosePrinterState;
EventState CenterRoomOnCursorState = {
	{
		(NodeType *)&ChoosePrinterState,
		(NodeType *)&BottomFloorState,
		sizeof (EventState),
		0,
		"CenterRoomOnCursor",
	},
	CenterRoomOnCursor,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ChoosePrinter' State
 */
FORWARD EventState	ClearAllLayerShiftsState;
EventState ChoosePrinterState = {
	{
		(NodeType *)&ClearAllLayerShiftsState,
		(NodeType *)&CenterRoomOnCursorState,
		sizeof (EventState),
		0,
		"ChoosePrinter",
	},
	ChoosePrinterReq,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ClearAllLayerShifts' State
 */
FORWARD EventState	ClearLayerShiftState;
EventState ClearAllLayerShiftsState = {
	{
		(NodeType *)&ClearLayerShiftState,
		(NodeType *)&ChoosePrinterState,
		sizeof (EventState),
		0,
		"ClearAllLayerShifts",
	},
	ClearAllLayerShifts,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ClearLayerShift' State
 */
FORWARD EventState	ClearMapState;
EventState ClearLayerShiftState = {
	{
		(NodeType *)&ClearMapState,
		(NodeType *)&ClearAllLayerShiftsState,
		sizeof (EventState),
		0,
		"ClearLayerShift",
	},
	ClearLayerShift,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ClearMap' State
 */
FORWARD EventState	ClearRoomState;
EventState ClearMapState = {
	{
		(NodeType *)&ClearRoomState,
		(NodeType *)&ClearLayerShiftState,
		sizeof (EventState),
		0,
		"ClearMap",
	},
	NMap,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ClearRoom' State
 */
FORWARD EventState	ClearRoom2State;
EventState ClearRoomState = {
	{
		(NodeType *)&ClearRoom2State,
		(NodeType *)&ClearMapState,
		sizeof (EventState),
		0,
		"ClearRoom",
	},
	ClearRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ClearRoom2' State
 */
FORWARD EventState	ClearRoom3State;
EventState ClearRoom2State = {
	{
		(NodeType *)&ClearRoom3State,
		(NodeType *)&ClearRoomState,
		sizeof (EventState),
		0,
		"ClearRoom2",
	},
	ClearRoom2,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ClearRoom3' State
 */
FORWARD EventState	CopyColors1State;
EventState ClearRoom3State = {
	{
		(NodeType *)&CopyColors1State,
		(NodeType *)&ClearRoom2State,
		sizeof (EventState),
		0,
		"ClearRoom3",
	},
	ClearRoom3,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CopyColors1' State
 */
FORWARD EventState	CopyColors2State;
EventState CopyColors1State = {
	{
		(NodeType *)&CopyColors2State,
		(NodeType *)&ClearRoom3State,
		sizeof (EventState),
		0,
		"CopyColors1",
	},
	CopyColors1,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CopyColors2' State
 */
FORWARD EventState	CopyColors3State;
EventState CopyColors2State = {
	{
		(NodeType *)&CopyColors3State,
		(NodeType *)&CopyColors1State,
		sizeof (EventState),
		0,
		"CopyColors2",
	},
	CopyColors2,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CopyColors3' State
 */
FORWARD EventState	CopyrightStatusState;
EventState CopyColors3State = {
	{
		(NodeType *)&CopyrightStatusState,
		(NodeType *)&CopyColors2State,
		sizeof (EventState),
		0,
		"CopyColors3",
	},
	CopyColors3,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CopyrightStatus' State
 */
FORWARD EventState	CountCharsState;
EventState CopyrightStatusState = {
	{
		(NodeType *)&CountCharsState,
		(NodeType *)&CopyColors3State,
		sizeof (EventState),
		0,
		"CopyrightStatus",
	},
	CopyrightStatus,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CountChars' State
 */
FORWARD EventState	CreateRoomState;
EventState CountCharsState = {
	{
		(NodeType *)&CreateRoomState,
		(NodeType *)&CopyrightStatusState,
		sizeof (EventState),
		0,
		"CountChars",
	},
	CountChars,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CreateRoom' State
 */
FORWARD EventState	CursorStatusState;
EventState CreateRoomState = {
	{
		(NodeType *)&CursorStatusState,
		(NodeType *)&CountCharsState,
		sizeof (EventState),
		0,
		"CreateRoom",
	},
	CRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'CursorStatus' State
 */
FORWARD EventState	DebugState;
EventState CursorStatusState = {
	{
		(NodeType *)&DebugState,
		(NodeType *)&CreateRoomState,
		sizeof (EventState),
		0,
		"CursorStatus",
	},
	CursorStatus,
	NULL,
	NULL,
	NULL,
};

/*
 * 'Debug' State
 */
FORWARD EventState	DecBackColorState;
EventState DebugState = {
	{
		(NodeType *)&DecBackColorState,
		(NodeType *)&CursorStatusState,
		sizeof (EventState),
		0,
		"Debug",
	},
	TE_Debug,
	NULL,
	NULL,
	NULL,
};

/*
 * 'DecBackColor' State
 */
FORWARD EventState	DeleteLayerState;
EventState DecBackColorState = {
	{
		(NodeType *)&DeleteLayerState,
		(NodeType *)&DebugState,
		sizeof (EventState),
		0,
		"DecBackColor",
	},
	DecGColor,
	NULL,
	NULL,
	NULL,
};

/*
 * 'DeleteLayer' State
 */
FORWARD EventState	DeleteRoomState;
EventState DeleteLayerState = {
	{
		(NodeType *)&DeleteRoomState,
		(NodeType *)&DecBackColorState,
		sizeof (EventState),
		0,
		"DeleteLayer",
	},
	DeleteLayer,
	NULL,
	NULL,
	NULL,
};

/*
 * 'DeleteRoom' State
 */
FORWARD EventState	DeleteTilesetState;
EventState DeleteRoomState = {
	{
		(NodeType *)&DeleteTilesetState,
		(NodeType *)&DeleteLayerState,
		sizeof (EventState),
		0,
		"DeleteRoom",
	},
	DRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'DeleteTileset' State
 */
FORWARD EventState	Download16State;
EventState DeleteTilesetState = {
	{
		(NodeType *)&Download16State,
		(NodeType *)&DeleteRoomState,
		sizeof (EventState),
		0,
		"DeleteTileset",
	},
	DeleteTileSet,
	NULL,
	NULL,
	NULL,
};

/*
 * 'Download16' State
 */
FORWARD EventState	Download256State;
EventState Download16State = {
	{
		(NodeType *)&Download256State,
		(NodeType *)&DeleteTilesetState,
		sizeof (EventState),
		0,
		"Download16",
	},
	DownloadMap16,
	NULL,
	NULL,
	NULL,
};

/*
 * 'Download256' State
 */
FORWARD EventState	DrawRandomState;
EventState Download256State = {
	{
		(NodeType *)&DrawRandomState,
		(NodeType *)&Download16State,
		sizeof (EventState),
		0,
		"Download256",
	},
	DownloadMap256,
	NULL,
	NULL,
	NULL,
};

/*
 * 'DrawRandom' State
 */
FORWARD EventState	EditOnlyFloorState;
EventState DrawRandomState = {
	{
		(NodeType *)&EditOnlyFloorState,
		(NodeType *)&Download256State,
		sizeof (EventState),
		0,
		"DrawRandom",
	},
	DrawRandom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'EditOnlyFloor' State
 */
FORWARD EventState	EditRoomState;
EventState EditOnlyFloorState = {
	{
		(NodeType *)&EditRoomState,
		(NodeType *)&DrawRandomState,
		sizeof (EventState),
		0,
		"EditOnlyFloor",
	},
	EditOnlyFloor,
	&EditOnlyFloorSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'EditRoom' State
 */
FORWARD EventState	ExportBrushState;
EventState EditRoomState = {
	{
		(NodeType *)&ExportBrushState,
		(NodeType *)&EditOnlyFloorState,
		sizeof (EventState),
		0,
		"EditRoom",
	},
	EditRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ExportBrush' State
 */
FORWARD EventState	ExportRoomState;
EventState ExportBrushState = {
	{
		(NodeType *)&ExportRoomState,
		(NodeType *)&EditRoomState,
		sizeof (EventState),
		0,
		"ExportBrush",
	},
	ExportBrush,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ExportRoom' State
 */
FORWARD EventState	ExportScreenState;
EventState ExportRoomState = {
	{
		(NodeType *)&ExportScreenState,
		(NodeType *)&ExportBrushState,
		sizeof (EventState),
		0,
		"ExportRoom",
	},
	ExportRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ExportScreen' State
 */
FORWARD EventState	FlipPanesState;
EventState ExportScreenState = {
	{
		(NodeType *)&FlipPanesState,
		(NodeType *)&ExportRoomState,
		sizeof (EventState),
		0,
		"ExportScreen",
	},
	ExportScreen,
	NULL,
	NULL,
	NULL,
};

/*
 * 'FlipPanes' State
 */
FORWARD EventState	FloorDownState;
EventState FlipPanesState = {
	{
		(NodeType *)&FloorDownState,
		(NodeType *)&ExportScreenState,
		sizeof (EventState),
		0,
		"FlipPanes",
	},
	WFRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'FloorDown' State
 */
FORWARD EventState	FloorUpState;
EventState FloorDownState = {
	{
		(NodeType *)&FloorUpState,
		(NodeType *)&FlipPanesState,
		sizeof (EventState),
		0,
		"FloorDown",
	},
	FloorDown,
	NULL,
	NULL,
	NULL,
};

/*
 * 'FloorUp' State
 */
FORWARD EventState	GetGridFromBrushState;
EventState FloorUpState = {
	{
		(NodeType *)&GetGridFromBrushState,
		(NodeType *)&FloorDownState,
		sizeof (EventState),
		0,
		"FloorUp",
	},
	FloorUp,
	NULL,
	NULL,
	NULL,
};

/*
 * 'GetGridFromBrush' State
 */
FORWARD EventState	GetGuideFromBrushState;
EventState GetGridFromBrushState = {
	{
		(NodeType *)&GetGuideFromBrushState,
		(NodeType *)&FloorUpState,
		sizeof (EventState),
		0,
		"GetGridFromBrush",
	},
	GetGridFromBrush,
	NULL,
	NULL,
	NULL,
};

/*
 * 'GetGuideFromBrush' State
 */
FORWARD EventState	GoEndState;
EventState GetGuideFromBrushState = {
	{
		(NodeType *)&GoEndState,
		(NodeType *)&GetGridFromBrushState,
		sizeof (EventState),
		0,
		"GetGuideFromBrush",
	},
	GetGuideFromBrush,
	NULL,
	NULL,
	NULL,
};

/*
 * 'GoEnd' State
 */
FORWARD EventState	GoHomeState;
EventState GoEndState = {
	{
		(NodeType *)&GoHomeState,
		(NodeType *)&GetGuideFromBrushState,
		sizeof (EventState),
		0,
		"GoEnd",
	},
	GoEnd,
	NULL,
	NULL,
	NULL,
};

/*
 * 'GoHome' State
 */
FORWARD EventState	GroupSaveTilesState;
EventState GoHomeState = {
	{
		(NodeType *)&GroupSaveTilesState,
		(NodeType *)&GoEndState,
		sizeof (EventState),
		0,
		"GoHome",
	},
	GoHome,
	NULL,
	NULL,
	NULL,
};

/*
 * 'GroupSaveTiles' State
 */
FORWARD EventState	HideCursorState;
EventState GroupSaveTilesState = {
	{
		(NodeType *)&HideCursorState,
		(NodeType *)&GoHomeState,
		sizeof (EventState),
		0,
		"GroupSaveTiles",
	},
	SMap3,
	NULL,
	NULL,
	NULL,
};

/*
 * 'HideCursor' State
 */
FORWARD EventState	HighlightTileState;
EventState HideCursorState = {
	{
		(NodeType *)&HighlightTileState,
		(NodeType *)&GroupSaveTilesState,
		sizeof (EventState),
		0,
		"HideCursor",
	},
	TE_TogglePointer,
	&HidePointerSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'HighlightTile' State
 */
FORWARD EventState	IncBackColorState;
EventState HighlightTileState = {
	{
		(NodeType *)&IncBackColorState,
		(NodeType *)&HideCursorState,
		sizeof (EventState),
		0,
		"HighlightTile",
	},
	HighlightTile,
	NULL,
	NULL,
	NULL,
};

/*
 * 'IncBackColor' State
 */
FORWARD EventState	InsertLayerState;
EventState IncBackColorState = {
	{
		(NodeType *)&InsertLayerState,
		(NodeType *)&HighlightTileState,
		sizeof (EventState),
		0,
		"IncBackColor",
	},
	IncGColor,
	NULL,
	NULL,
	NULL,
};

/*
 * 'InsertLayer' State
 */
FORWARD EventState	KeepDownloadPaletteState;
EventState InsertLayerState = {
	{
		(NodeType *)&KeepDownloadPaletteState,
		(NodeType *)&IncBackColorState,
		sizeof (EventState),
		0,
		"InsertLayer",
	},
	InsertLayer,
	NULL,
	NULL,
	NULL,
};

/*
 * 'KeepDownloadPalette' State
 */
FORWARD EventState	LastMenuEventState;
EventState KeepDownloadPaletteState = {
	{
		(NodeType *)&LastMenuEventState,
		(NodeType *)&InsertLayerState,
		sizeof (EventState),
		0,
		"KeepDownloadPalette",
	},
	KeepDownloadPalette,
	&KeepDownloadPaletteSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'LastMenuEvent' State
 */
FORWARD EventState	LayerShiftStatusState;
EventState LastMenuEventState = {
	{
		(NodeType *)&LayerShiftStatusState,
		(NodeType *)&KeepDownloadPaletteState,
		sizeof (EventState),
		0,
		"LastMenuEvent",
	},
	LastMenuEvent,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LayerShiftStatus' State
 */
FORWARD EventState	LoadLayerState;
EventState LayerShiftStatusState = {
	{
		(NodeType *)&LoadLayerState,
		(NodeType *)&LastMenuEventState,
		sizeof (EventState),
		0,
		"LayerShiftStatus",
	},
	LayerShiftStatus,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadLayer' State
 */
FORWARD EventState	LoadMapState;
EventState LoadLayerState = {
	{
		(NodeType *)&LoadMapState,
		(NodeType *)&LayerShiftStatusState,
		sizeof (EventState),
		0,
		"LoadLayer",
	},
	LoadLayer,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadMap' State
 */
FORWARD EventState	LoadPaletteState;
EventState LoadMapState = {
	{
		(NodeType *)&LoadPaletteState,
		(NodeType *)&LoadLayerState,
		sizeof (EventState),
		0,
		"LoadMap",
	},
	LMap,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadPalette' State
 */
FORWARD EventState	LoadPaletteRangeState;
EventState LoadPaletteState = {
	{
		(NodeType *)&LoadPaletteRangeState,
		(NodeType *)&LoadMapState,
		sizeof (EventState),
		0,
		"LoadPalette",
	},
	LoadPalette,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadPaletteRange' State
 */
FORWARD EventState	LoadRoomState;
EventState LoadPaletteRangeState = {
	{
		(NodeType *)&LoadRoomState,
		(NodeType *)&LoadPaletteState,
		sizeof (EventState),
		0,
		"LoadPaletteRange",
	},
	LoadPaletteRange,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadRoom' State
 */
FORWARD EventState	LoadTilesAllTiledState;
EventState LoadRoomState = {
	{
		(NodeType *)&LoadTilesAllTiledState,
		(NodeType *)&LoadPaletteRangeState,
		sizeof (EventState),
		0,
		"LoadRoom",
	},
	LRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadTilesAllTiled' State
 */
FORWARD EventState	LoadTilesBoxedState;
EventState LoadTilesAllTiledState = {
	{
		(NodeType *)&LoadTilesBoxedState,
		(NodeType *)&LoadRoomState,
		sizeof (EventState),
		0,
		"LoadTilesAllTiled",
	},
	TE_LoadTilesAll,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadTilesBoxed' State
 */
FORWARD EventState	LoadTilesCookieCutterState;
EventState LoadTilesBoxedState = {
	{
		(NodeType *)&LoadTilesCookieCutterState,
		(NodeType *)&LoadTilesAllTiledState,
		sizeof (EventState),
		0,
		"LoadTilesBoxed",
	},
	TE_LoadTilesBoxed,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadTilesCookieCutter' State
 */
FORWARD EventState	LoadTilesFullTiledState;
EventState LoadTilesCookieCutterState = {
	{
		(NodeType *)&LoadTilesFullTiledState,
		(NodeType *)&LoadTilesBoxedState,
		sizeof (EventState),
		0,
		"LoadTilesCookieCutter",
	},
	TE_LoadTilesCookieCutter,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadTilesFullTiled' State
 */
FORWARD EventState	LoadTilesFullTiledNoBlankState;
EventState LoadTilesFullTiledState = {
	{
		(NodeType *)&LoadTilesFullTiledNoBlankState,
		(NodeType *)&LoadTilesCookieCutterState,
		sizeof (EventState),
		0,
		"LoadTilesFullTiled",
	},
	TE_LoadTilesFullTiled,
	NULL,
	NULL,
	NULL,
};

/*
 * 'LoadTilesFullTiledNoBlank' State
 */
FORWARD EventState	MakeCompositeTilesState;
EventState LoadTilesFullTiledNoBlankState = {
	{
		(NodeType *)&MakeCompositeTilesState,
		(NodeType *)&LoadTilesFullTiledState,
		sizeof (EventState),
		0,
		"LoadTilesFullTiledNoBlank",
	},
	TE_LoadTilesFullTiledNoBlank,
	NULL,
	NULL,
	NULL,
};

/*
 * 'MakeCompositeTiles' State
 */
FORWARD EventState	NextRoomState;
EventState MakeCompositeTilesState = {
	{
		(NodeType *)&NextRoomState,
		(NodeType *)&LoadTilesFullTiledNoBlankState,
		sizeof (EventState),
		0,
		"MakeCompositeTiles",
	},
	MakeCompositeTiles,
	NULL,
	NULL,
	NULL,
};

/*
 * 'NextRoom' State
 */
FORWARD EventState	OpenLayerState;
EventState NextRoomState = {
	{
		(NodeType *)&OpenLayerState,
		(NodeType *)&MakeCompositeTilesState,
		sizeof (EventState),
		0,
		"NextRoom",
	},
	WNRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'OpenLayer' State
 */
FORWARD EventState	PreviousRoomState;
EventState OpenLayerState = {
	{
		(NodeType *)&PreviousRoomState,
		(NodeType *)&NextRoomState,
		sizeof (EventState),
		0,
		"OpenLayer",
	},
	OpenLayer,
	NULL,
	NULL,
	NULL,
};

/*
 * 'PreviousRoom' State
 */
FORWARD EventState	PrintMapState;
EventState PreviousRoomState = {
	{
		(NodeType *)&PrintMapState,
		(NodeType *)&OpenLayerState,
		sizeof (EventState),
		0,
		"PreviousRoom",
	},
	WPRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'PrintMap' State
 */
FORWARD EventState	QuitAndExitState;
EventState PrintMapState = {
	{
		(NodeType *)&QuitAndExitState,
		(NodeType *)&PreviousRoomState,
		sizeof (EventState),
		0,
		"PrintMap",
	},
	PrintMap,
	NULL,
	NULL,
	NULL,
};

/*
 * 'QuitAndExit' State
 */
FORWARD EventState	ReplaceState;
EventState QuitAndExitState = {
	{
		(NodeType *)&ReplaceState,
		(NodeType *)&PrintMapState,
		sizeof (EventState),
		0,
		"QuitAndExit",
	},
	QuitAndExit,
	NULL,
	NULL,
	NULL,
};

/*
 * 'Replace' State
 */
FORWARD EventState	RoomStatusState;
EventState ReplaceState = {
	{
		(NodeType *)&RoomStatusState,
		(NodeType *)&QuitAndExitState,
		sizeof (EventState),
		0,
		"Replace",
	},
	Replace,
	NULL,
	NULL,
	NULL,
};

/*
 * 'RoomStatus' State
 */
FORWARD EventState	SaveLayerState;
EventState RoomStatusState = {
	{
		(NodeType *)&SaveLayerState,
		(NodeType *)&ReplaceState,
		sizeof (EventState),
		0,
		"RoomStatus",
	},
	RoomStatus,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SaveLayer' State
 */
FORWARD EventState	SaveMapState;
EventState SaveLayerState = {
	{
		(NodeType *)&SaveMapState,
		(NodeType *)&RoomStatusState,
		sizeof (EventState),
		0,
		"SaveLayer",
	},
	SaveLayer,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SaveMap' State
 */
FORWARD EventState	SavePaletteState;
EventState SaveMapState = {
	{
		(NodeType *)&SavePaletteState,
		(NodeType *)&SaveLayerState,
		sizeof (EventState),
		0,
		"SaveMap",
	},
	SMap1,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SavePalette' State
 */
FORWARD EventState	SavePaletteRangeState;
EventState SavePaletteState = {
	{
		(NodeType *)&SavePaletteRangeState,
		(NodeType *)&SaveMapState,
		sizeof (EventState),
		0,
		"SavePalette",
	},
	SavePalette,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SavePaletteRange' State
 */
FORWARD EventState	SaveRoomState;
EventState SavePaletteRangeState = {
	{
		(NodeType *)&SaveRoomState,
		(NodeType *)&SavePaletteState,
		sizeof (EventState),
		0,
		"SavePaletteRange",
	},
	SavePaletteRange,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SaveRoom' State
 */
FORWARD EventState	SaveRoomAllState;
EventState SaveRoomState = {
	{
		(NodeType *)&SaveRoomAllState,
		(NodeType *)&SavePaletteRangeState,
		sizeof (EventState),
		0,
		"SaveRoom",
	},
	SRoom,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SaveRoomAll' State
 */
FORWARD EventState	SaveTilesAsBrushesState;
EventState SaveRoomAllState = {
	{
		(NodeType *)&SaveTilesAsBrushesState,
		(NodeType *)&SaveRoomState,
		sizeof (EventState),
		0,
		"SaveRoomAll",
	},
	SRoomAll,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SaveTilesAsBrushes' State
 */
FORWARD EventState	ScrollLockState;
EventState SaveTilesAsBrushesState = {
	{
		(NodeType *)&ScrollLockState,
		(NodeType *)&SaveRoomAllState,
		sizeof (EventState),
		0,
		"SaveTilesAsBrushes",
	},
	TE_SaveTilesAsBrushes,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ScrollLock' State
 */
FORWARD EventState	ScrollLockKEYState;
EventState ScrollLockState = {
	{
		(NodeType *)&ScrollLockKEYState,
		(NodeType *)&SaveTilesAsBrushesState,
		sizeof (EventState),
		0,
		"ScrollLock",
	},
	ScrollLock,
	&ScrollLockSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ScrollLockKEY' State
 */
FORWARD EventState	SearchNextState;
EventState ScrollLockKEYState = {
	{
		(NodeType *)&SearchNextState,
		(NodeType *)&ScrollLockState,
		sizeof (EventState),
		0,
		"ScrollLockKEY",
	},
	ScrollLockKEY,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SearchNext' State
 */
FORWARD EventState	SelectBlockState;
EventState SearchNextState = {
	{
		(NodeType *)&SelectBlockState,
		(NodeType *)&ScrollLockKEYState,
		sizeof (EventState),
		0,
		"SearchNext",
	},
	SearchNext,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SelectBlock' State
 */
FORWARD EventState	SelectSquareState;
EventState SelectBlockState = {
	{
		(NodeType *)&SelectSquareState,
		(NodeType *)&SearchNextState,
		sizeof (EventState),
		0,
		"SelectBlock",
	},
	US,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SelectSquare' State
 */
FORWARD EventState	SetGridOriginState;
EventState SelectSquareState = {
	{
		(NodeType *)&SetGridOriginState,
		(NodeType *)&SelectBlockState,
		sizeof (EventState),
		0,
		"SelectSquare",
	},
	US2,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetGridOrigin' State
 */
FORWARD EventState	SetGridSizeState;
EventState SetGridOriginState = {
	{
		(NodeType *)&SetGridSizeState,
		(NodeType *)&SelectSquareState,
		sizeof (EventState),
		0,
		"SetGridOrigin",
	},
	SetGridOrigin,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetGridSize' State
 */
FORWARD EventState	SetGuideOriginState;
EventState SetGridSizeState = {
	{
		(NodeType *)&SetGuideOriginState,
		(NodeType *)&SetGridOriginState,
		sizeof (EventState),
		0,
		"SetGridSize",
	},
	SetGridSize,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetGuideOrigin' State
 */
FORWARD EventState	SetGuideSizeState;
EventState SetGuideOriginState = {
	{
		(NodeType *)&SetGuideSizeState,
		(NodeType *)&SetGridSizeState,
		sizeof (EventState),
		0,
		"SetGuideOrigin",
	},
	SetGuideOrigin,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetGuideSize' State
 */
FORWARD EventState	SetMaxTileUsageState;
EventState SetGuideSizeState = {
	{
		(NodeType *)&SetMaxTileUsageState,
		(NodeType *)&SetGuideOriginState,
		sizeof (EventState),
		0,
		"SetGuideSize",
	},
	SetGuideSize,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetMaxTileUsage' State
 */
FORWARD EventState	SetRoomInfoState;
EventState SetMaxTileUsageState = {
	{
		(NodeType *)&SetRoomInfoState,
		(NodeType *)&SetGuideSizeState,
		sizeof (EventState),
		0,
		"SetMaxTileUsage",
	},
	SetMaxTileUsage,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetRoomInfo' State
 */
FORWARD EventState	SetSearchBufferState;
EventState SetRoomInfoState = {
	{
		(NodeType *)&SetSearchBufferState,
		(NodeType *)&SetMaxTileUsageState,
		sizeof (EventState),
		0,
		"SetRoomInfo",
	},
	SetRoomInfo,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetSearchBuffer' State
 */
FORWARD EventState	SetStampPaintState;
EventState SetSearchBufferState = {
	{
		(NodeType *)&SetStampPaintState,
		(NodeType *)&SetRoomInfoState,
		sizeof (EventState),
		0,
		"SetSearchBuffer",
	},
	SetSearchBuffer,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetStampPaint' State
 */
FORWARD EventState	SetStampReplaceState;
EventState SetStampPaintState = {
	{
		(NodeType *)&SetStampReplaceState,
		(NodeType *)&SetSearchBufferState,
		sizeof (EventState),
		0,
		"SetStampPaint",
	},
	SetStampPaint,
	&SetStampPaintSTATE,
	MNU_HAS_RADIO,
	(ListType *)&__SetStampPaintExcList,
};

/*
 * 'SetStampReplace' State
 */
FORWARD EventState	SetTheColorsState;
EventState SetStampReplaceState = {
	{
		(NodeType *)&SetTheColorsState,
		(NodeType *)&SetStampPaintState,
		sizeof (EventState),
		0,
		"SetStampReplace",
	},
	SetStampReplace,
	&SetStampReplaceSTATE,
	MNU_HAS_RADIO,
	(ListType *)&__SetStampReplaceExcList,
};

/*
 * 'SetTheColors' State
 */
FORWARD EventState	SetTilesetInfoState;
EventState SetTheColorsState = {
	{
		(NodeType *)&SetTilesetInfoState,
		(NodeType *)&SetStampReplaceState,
		sizeof (EventState),
		0,
		"SetTheColors",
	},
	SetTheColors,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SetTilesetInfo' State
 */
FORWARD EventState	ShowBrushCountState;
EventState SetTilesetInfoState = {
	{
		(NodeType *)&ShowBrushCountState,
		(NodeType *)&SetTheColorsState,
		sizeof (EventState),
		0,
		"SetTilesetInfo",
	},
	SetTilesetInfo,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ShowBrushCount' State
 */
FORWARD EventState	SpaceToggleState;
EventState ShowBrushCountState = {
	{
		(NodeType *)&SpaceToggleState,
		(NodeType *)&SetTilesetInfoState,
		sizeof (EventState),
		0,
		"ShowBrushCount",
	},
	ShowBrushCount,
	NULL,
	NULL,
	NULL,
};

/*
 * 'SpaceToggle' State
 */
FORWARD EventState	StripDownBlockCopyState;
EventState SpaceToggleState = {
	{
		(NodeType *)&StripDownBlockCopyState,
		(NodeType *)&ShowBrushCountState,
		sizeof (EventState),
		0,
		"SpaceToggle",
	},
	SpaceToggle,
	&SpaceSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'StripDownBlockCopy' State
 */
FORWARD EventState	TileStatusState;
EventState StripDownBlockCopyState = {
	{
		(NodeType *)&TileStatusState,
		(NodeType *)&SpaceToggleState,
		sizeof (EventState),
		0,
		"StripDownBlockCopy",
	},
	StripDownBlockCopy,
	NULL,
	NULL,
	NULL,
};

/*
 * 'TileStatus' State
 */
FORWARD EventState	TMGCSaveTilesState;
EventState TileStatusState = {
	{
		(NodeType *)&TMGCSaveTilesState,
		(NodeType *)&StripDownBlockCopyState,
		sizeof (EventState),
		0,
		"TileStatus",
	},
	TUserStatus,
	NULL,
	NULL,
	NULL,
};

/*
 * 'TMGCSaveTiles' State
 */
FORWARD EventState	ToggleCycleColorsState;
EventState TMGCSaveTilesState = {
	{
		(NodeType *)&ToggleCycleColorsState,
		(NodeType *)&TileStatusState,
		sizeof (EventState),
		0,
		"TMGCSaveTiles",
	},
	SMap4,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ToggleCycleColors' State
 */
FORWARD EventState	ToggleDownloadOneScreenState;
EventState ToggleCycleColorsState = {
	{
		(NodeType *)&ToggleDownloadOneScreenState,
		(NodeType *)&TMGCSaveTilesState,
		sizeof (EventState),
		0,
		"ToggleCycleColors",
	},
	ToggleCycleColors,
	&CycleColorsSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleDownloadOneScreen' State
 */
FORWARD EventState	ToggleJamPaletteState;
EventState ToggleDownloadOneScreenState = {
	{
		(NodeType *)&ToggleJamPaletteState,
		(NodeType *)&ToggleCycleColorsState,
		sizeof (EventState),
		0,
		"ToggleDownloadOneScreen",
	},
	ToggleDownloadOneScreen,
	&DownloadOneScreenSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleJamPalette' State
 */
FORWARD EventState	ToggleLAllState;
EventState ToggleJamPaletteState = {
	{
		(NodeType *)&ToggleLAllState,
		(NodeType *)&ToggleDownloadOneScreenState,
		sizeof (EventState),
		0,
		"ToggleJamPalette",
	},
	ToggleJamPalette,
	&JamPaletteSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleLAll' State
 */
FORWARD EventState	ToggleLInvisiState;
EventState ToggleLAllState = {
	{
		(NodeType *)&ToggleLInvisiState,
		(NodeType *)&ToggleJamPaletteState,
		sizeof (EventState),
		0,
		"ToggleLAll",
	},
	ToggleLAll,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ToggleLInvisi' State
 */
FORWARD EventState	ToggleLLockState;
EventState ToggleLInvisiState = {
	{
		(NodeType *)&ToggleLLockState,
		(NodeType *)&ToggleLAllState,
		sizeof (EventState),
		0,
		"ToggleLInvisi",
	},
	ToggleLInvisi,
	&InvisiLayerSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleLLock' State
 */
FORWARD EventState	ToggleLockRoomState;
EventState ToggleLLockState = {
	{
		(NodeType *)&ToggleLockRoomState,
		(NodeType *)&ToggleLInvisiState,
		sizeof (EventState),
		0,
		"ToggleLLock",
	},
	ToggleLLock,
	&LockLayerSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleLockRoom' State
 */
FORWARD EventState	TogglePixelSelectState;
EventState ToggleLockRoomState = {
	{
		(NodeType *)&TogglePixelSelectState,
		(NodeType *)&ToggleLLockState,
		sizeof (EventState),
		0,
		"ToggleLockRoom",
	},
	ToggleLockRoom,
	&LockRoomSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'TogglePixelSelect' State
 */
FORWARD EventState	ToggleShowBrushState;
EventState TogglePixelSelectState = {
	{
		(NodeType *)&ToggleShowBrushState,
		(NodeType *)&ToggleLockRoomState,
		sizeof (EventState),
		0,
		"TogglePixelSelect",
	},
	TogglePixelSelect,
	&fPixelSelect,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleShowBrush' State
 */
FORWARD EventState	ToggleShowGuideState;
EventState ToggleShowBrushState = {
	{
		(NodeType *)&ToggleShowGuideState,
		(NodeType *)&TogglePixelSelectState,
		sizeof (EventState),
		0,
		"ToggleShowBrush",
	},
	ToggleShowBrush,
	&ShowBrushSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleShowGuide' State
 */
FORWARD EventState	ToggleShowGuide2State;
EventState ToggleShowGuideState = {
	{
		(NodeType *)&ToggleShowGuide2State,
		(NodeType *)&ToggleShowBrushState,
		sizeof (EventState),
		0,
		"ToggleShowGuide",
	},
	ToggleShowGuide,
	&ShowGuideSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleShowGuide2' State
 */
FORWARD EventState	ToggleShowTileUsageState;
EventState ToggleShowGuide2State = {
	{
		(NodeType *)&ToggleShowTileUsageState,
		(NodeType *)&ToggleShowGuideState,
		sizeof (EventState),
		0,
		"ToggleShowGuide2",
	},
	ToggleShowGuide2,
	&ShowGuide2STATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleShowTileUsage' State
 */
FORWARD EventState	ToggleSmartFlipState;
EventState ToggleShowTileUsageState = {
	{
		(NodeType *)&ToggleSmartFlipState,
		(NodeType *)&ToggleShowGuide2State,
		sizeof (EventState),
		0,
		"ToggleShowTileUsage",
	},
	ToggleShowTileUsage,
	&fShowTileUsage,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleSmartFlip' State
 */
FORWARD EventState	ToggleStratifyPasteState;
EventState ToggleSmartFlipState = {
	{
		(NodeType *)&ToggleStratifyPasteState,
		(NodeType *)&ToggleShowTileUsageState,
		sizeof (EventState),
		0,
		"ToggleSmartFlip",
	},
	ToggleSwankyMode,
	&SmartFlipSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleStratifyPaste' State
 */
FORWARD EventState	ToggleSwankyModeState;
EventState ToggleStratifyPasteState = {
	{
		(NodeType *)&ToggleSwankyModeState,
		(NodeType *)&ToggleSmartFlipState,
		sizeof (EventState),
		0,
		"ToggleStratifyPaste",
	},
	ToggleSwankyMode,
	&StratifyPasteSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleSwankyMode' State
 */
FORWARD EventState	ToggleTitleBarState;
EventState ToggleSwankyModeState = {
	{
		(NodeType *)&ToggleTitleBarState,
		(NodeType *)&ToggleStratifyPasteState,
		sizeof (EventState),
		0,
		"ToggleSwankyMode",
	},
	ToggleSwankyMode,
	&SwankyModeSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleTitleBar' State
 */
FORWARD EventState	ToggleUseGridState;
EventState ToggleTitleBarState = {
	{
		(NodeType *)&ToggleUseGridState,
		(NodeType *)&ToggleSwankyModeState,
		sizeof (EventState),
		0,
		"ToggleTitleBar",
	},
	ToggleTitleBar,
	&ShowTitleSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleUseGrid' State
 */
FORWARD EventState	ToggleZoomState;
EventState ToggleUseGridState = {
	{
		(NodeType *)&ToggleZoomState,
		(NodeType *)&ToggleTitleBarState,
		sizeof (EventState),
		0,
		"ToggleUseGrid",
	},
	ToggleUseGrid,
	&UseGridSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ToggleZoom' State
 */
FORWARD EventState	TopFloorState;
EventState ToggleZoomState = {
	{
		(NodeType *)&TopFloorState,
		(NodeType *)&ToggleUseGridState,
		sizeof (EventState),
		0,
		"ToggleZoom",
	},
	ToggleZoom,
	&ShowZoomSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'TopFloor' State
 */
FORWARD EventState	tumeInfoState;
EventState TopFloorState = {
	{
		(NodeType *)&tumeInfoState,
		(NodeType *)&ToggleZoomState,
		sizeof (EventState),
		0,
		"TopFloor",
	},
	TopFloor,
	NULL,
	NULL,
	NULL,
};

/*
 * 'tumeInfo' State
 */
FORWARD EventState	UndoState;
EventState tumeInfoState = {
	{
		(NodeType *)&UndoState,
		(NodeType *)&TopFloorState,
		sizeof (EventState),
		0,
		"tumeInfo",
	},
	tumeInfo,
	NULL,
	NULL,
	NULL,
};

/*
 * 'Undo' State
 */
FORWARD EventState	UseEditPaletteState;
EventState UndoState = {
	{
		(NodeType *)&UseEditPaletteState,
		(NodeType *)&tumeInfoState,
		sizeof (EventState),
		0,
		"Undo",
	},
	Undo,
	NULL,
	NULL,
	NULL,
};

/*
 * 'UseEditPalette' State
 */
FORWARD EventState	UserStatusState;
EventState UseEditPaletteState = {
	{
		(NodeType *)&UserStatusState,
		(NodeType *)&UndoState,
		sizeof (EventState),
		0,
		"UseEditPalette",
	},
	UseEditPalette,
	&UseEditPaletteSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'UserStatus' State
 */
FORWARD EventState	VersionStatusState;
EventState UserStatusState = {
	{
		(NodeType *)&VersionStatusState,
		(NodeType *)&UseEditPaletteState,
		sizeof (EventState),
		0,
		"UserStatus",
	},
	UserStatus,
	NULL,
	NULL,
	NULL,
};

/*
 * 'VersionStatus' State
 */
FORWARD EventState	WCloseState;
EventState VersionStatusState = {
	{
		(NodeType *)&WCloseState,
		(NodeType *)&UserStatusState,
		sizeof (EventState),
		0,
		"VersionStatus",
	},
	VersionStatus,
	NULL,
	NULL,
	NULL,
};

/*
 * 'WClose' State
 */
FORWARD EventState	WCreateState;
EventState WCloseState = {
	{
		(NodeType *)&WCreateState,
		(NodeType *)&VersionStatusState,
		sizeof (EventState),
		0,
		"WClose",
	},
	WClose,
	NULL,
	NULL,
	NULL,
};

/*
 * 'WCreate' State
 */
FORWARD EventState	WLockClearState;
EventState WCreateState = {
	{
		(NodeType *)&WLockClearState,
		(NodeType *)&WCloseState,
		sizeof (EventState),
		0,
		"WCreate",
	},
	WCreate,
	NULL,
	NULL,
	NULL,
};

/*
 * 'WLockClear' State
 */
FORWARD EventState	WLockToEditState;
EventState WLockClearState = {
	{
		(NodeType *)&WLockToEditState,
		(NodeType *)&WCreateState,
		sizeof (EventState),
		0,
		"WLockClear",
	},
	WLCl,
	&WLCSTATE,
	MNU_HAS_RADIO,
	(ListType *)&__WLockClearExcList,
};

/*
 * 'WLockToEdit' State
 */
FORWARD EventState	WLockToSameState;
EventState WLockToEditState = {
	{
		(NodeType *)&WLockToSameState,
		(NodeType *)&WLockClearState,
		sizeof (EventState),
		0,
		"WLockToEdit",
	},
	WLE,
	&WLESTATE,
	MNU_HAS_RADIO,
	(ListType *)&__WLockToEditExcList,
};

/*
 * 'WLockToSame' State
 */
FORWARD EventState	WLockToSourceState;
EventState WLockToSameState = {
	{
		(NodeType *)&WLockToSourceState,
		(NodeType *)&WLockToEditState,
		sizeof (EventState),
		0,
		"WLockToSame",
	},
	WLC,
	NULL,
	NULL,
	NULL,
};

/*
 * 'WLockToSource' State
 */
FORWARD EventState	XFlipBrushState;
EventState WLockToSourceState = {
	{
		(NodeType *)&XFlipBrushState,
		(NodeType *)&WLockToSameState,
		sizeof (EventState),
		0,
		"WLockToSource",
	},
	WLS,
	&WLSSTATE,
	MNU_HAS_RADIO,
	(ListType *)&__WLockToSourceExcList,
};

/*
 * 'XFlipBrush' State
 */
FORWARD EventState	YFlipBrushState;
EventState XFlipBrushState = {
	{
		(NodeType *)&YFlipBrushState,
		(NodeType *)&WLockToSourceState,
		sizeof (EventState),
		0,
		"XFlipBrush",
	},
	XFlipBrush,
	&XFlipBrushSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'YFlipBrush' State
 */
FORWARD EventState	ZaveMapState;
EventState YFlipBrushState = {
	{
		(NodeType *)&ZaveMapState,
		(NodeType *)&XFlipBrushState,
		sizeof (EventState),
		0,
		"YFlipBrush",
	},
	YFlipBrush,
	&YFlipBrushSTATE,
	MNU_HAS_CHECK,
	NULL,
};

/*
 * 'ZaveMap' State
 */
FORWARD EventState	ZeroBackColorState;
EventState ZaveMapState = {
	{
		(NodeType *)&ZeroBackColorState,
		(NodeType *)&YFlipBrushState,
		sizeof (EventState),
		0,
		"ZaveMap",
	},
	SMap2,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ZeroBackColor' State
 */
FORWARD EventState	ZoomInState;
EventState ZeroBackColorState = {
	{
		(NodeType *)&ZoomInState,
		(NodeType *)&ZaveMapState,
		sizeof (EventState),
		0,
		"ZeroBackColor",
	},
	ZGColor,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ZoomIn' State
 */
FORWARD EventState	ZoomOutState;
EventState ZoomInState = {
	{
		(NodeType *)&ZoomOutState,
		(NodeType *)&ZeroBackColorState,
		sizeof (EventState),
		0,
		"ZoomIn",
	},
	ZoomIn,
	NULL,
	NULL,
	NULL,
};

/*
 * 'ZoomOut' State
 */
EventState ZoomOutState = {
	{
		(NodeType *)&EventListx.Tail,
		(NodeType *)&ZoomInState,
		sizeof (EventState),
		0,
		"ZoomOut",
	},
	ZoomOut,
	NULL,
	NULL,
	NULL,
};

ListType EventListx = {
	(NodeType *)&AboutState,
	NULL,
	(NodeType *)&ZoomOutState,
	sizeof (ListType),
	0,
	NULL,
};

ListType *EventList = &EventListx;

