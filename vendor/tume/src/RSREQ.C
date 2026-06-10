/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is tUME: the Universal Map Editor, released 
 * June 18, 2000. 
 * 
 * The Initial Developer of the Original Code is Echidna. 
 * Portions created by Echidna are Copyright (c) 1989-2000 Echidna. 
 * All Rights Reserved. 
 * 
 * Contributor(s): 
 */

/*
 * RSREQUEST.C
 *
 * PROGRAMMER : R.G. Marquez 
 *    VERSION : 00.000
 *    CREATED : 10/03/89 
 *   MODIFIED : 10/19/94
 *       TABS : 04 09
 *
 *	     \|///-_
 *	     \oO///_
 *	-----w/-w-----
 *	 E C H i D N A
 *	--------------
 *
 * DESCRIPTION
 *
 *
 * HISTORY
 *		10/03/89 Tuesday - Created (after MUCH pain). (RGM)
*/
#include <echidna/platform.h>
#include "switches.h"

#include "tuglbl.h"

/**************************** C O N S T A N T S ***************************/

#define	RSI_FLAGS GADGHCOMP, RELVERIFY | STRINGRIGHT | LONGINT, STRGADGET | REQGADGET
#define	RSB_FLAGS GADGHCOMP, RELVERIFY, BOOLGADGET | REQGADGET
#define	TOPAZ_FONT_MACRO (&topaz_8)

#define REQ_X 64
#define REQ_Y 60

#define DEFAULT_ROOM_WIDTH 20
#define DEFAULT_ROOM_HEIGHT 12
#define STR_DEFAULT_ROOM_WIDTH "20"
#define STR_DEFAULT_ROOM_HEIGHT "12"

#define	S_RS_TITLE	"Please Enter Room Size"
#define	S_RS_LABEL1	"Room Width"
#define	S_RS_LABEL2	"Room Height"

#define	S_RU_TITLE	"Please Enter Room Info"
#define	S_RU_LABEL1	"User Type"
#define	S_RU_LABEL2	"User Num"

#define	S_TU_TITLE	"Please Enter Tile Info"
#define	S_TU_LABEL1	"User Type"
#define	S_TU_LABEL2	"User Num"

#define	S_TS_TITLE	"Please Enter Tile Size"
#define	S_TS_LABEL1	"Tile Width"
#define	S_TS_LABEL2	"Tile Height"

#define S_GR_TITLE	"Please Enter Grid Size"
#define S_GR_LABEL1	"Grid Width"
#define S_GR_LABEL2	"Grid Height"

#define S_GU_TITLE	"Please Enter Guide Size"
#define S_GU_LABEL1	"Guide Width"
#define S_GU_LABEL2	"Guide Height"

/******************************** T Y P E S *******************************/

typedef int (*intfuncptr)(void);

/****************************** E X T E R N S *****************************/

#if __AMIGAOS__
extern struct Gadget RW_Integer;
extern struct Gadget RH_Integer;
#endif/*__AMIGAOS__*/
extern struct Requester RS_Req;

/****************************** G L O B A L S *****************************/


/******************************* L O C A L S ******************************/

#if __AMIGAOS__
static int RW_FN (void);
static int RH_FN (void);
static int RAB_FN (void);
static int RCB_FN (void);
static int GetRoomSize (
	int *width,
	int *height
);
static int DoubleRequest (
	int		*one_ptr,
	int		*two_ptr,
	char	*title,
	char	*onelabel,
	char	*twolabel
);
#endif/*__AMIGAOS__*/

static	long int min_one	= 0;
static	long int min_two	= 0;

#if __AMIGAOS__
static	struct	Window	*req_window = NULL;

static	struct TextAttr topaz_8 = {
	(UBYTE *) "topaz.font", 8
};

/***************************/
/* buffers for the results */

static	UBYTE	RW[20];
static	UBYTE	RH[20];
static	UBYTE	UndoBuffer[20];

static	struct StringInfo RW_StringInfo = {
	RW, UndoBuffer, 0, 5, 0, 0, 0, 0, 0, 0, NULL, DEFAULT_ROOM_WIDTH
};

static	struct StringInfo RH_StringInfo = {
	RH, UndoBuffer, 0, 5, 0, 0, 0, 0, 0, 0, NULL, DEFAULT_ROOM_HEIGHT
};

/****************************************/
/* stuff common to both integer gadgets */

static	SHORT	R_Lines[] = {
	0, 0, 41, 0, 41, 9, 0, 9, 0, 0
};

static	struct Border R_Border= {
	-1, -1, 2, 0, JAM1, 5, R_Lines, NULL
};

/*************************************/
/* stuff for the "Room Width" gadget */

static	struct IntuiText RW_Text = {
	2, 0, JAM1, -117, 1, TOPAZ_FONT_MACRO, (UBYTE *) "Room Width", NULL
};

static	struct Gadget RW_Integer = {
	NULL, 132, 28, 40, 8, RSI_FLAGS, (APTR) &R_Border,
	NULL, &RW_Text, NULL, (APTR) &RW_StringInfo, 1, (APTR) &RW_FN
};


/*************************************/
/* stuff for the "Room Height" gadget */

static	struct IntuiText RH_Text = {
	2, 0, JAM1, -117, 1, TOPAZ_FONT_MACRO, (UBYTE *) "Room Height", NULL
};

static	struct Gadget RH_Integer = {
	&RW_Integer, 132, 43, 40, 8, RSI_FLAGS, (APTR) &R_Border,
	NULL, &RH_Text, NULL, (APTR) &RH_StringInfo, 2, (APTR) &RH_FN
};


/***************************************/
/* stuff common to both button gadgets */

static	SHORT	RB2_Lines[] = {
	84, 1, 84, 12, 0, 12
};

static	struct Border RB_Border2 = {
	-1, -1, 2, 0, JAM1, 3, RB2_Lines, NULL
};

static	SHORT	RB1_Lines[] = {
	0, 11, 0, 0, 84, 0
};

static	struct Border RB_Border1 = {
	-1, -1, 1, 0, JAM1, 3, RB1_Lines, &RB_Border2
};


/****************************************/
/* stuff for the "Accept" button gadget */

static	struct IntuiText RBA_Text = {
	2, 0, JAM1, 18, 2, TOPAZ_FONT_MACRO, (UBYTE *) "ACCEPT", NULL
};

static	struct Gadget RS_Accept_Button = {
	&RH_Integer, 7, 64, 83, 11, RSB_FLAGS, (APTR) &RB_Border1,
	NULL, &RBA_Text, NULL, NULL, 3, (APTR) &RAB_FN
};


/****************************************/
/* stuff for the "Cancel" button gadget */

static	struct IntuiText RBC_Text = {
	2, 0, JAM1, 19, 2, TOPAZ_FONT_MACRO, (UBYTE *) "CANCEL", NULL
};

static	struct Gadget RS_Cancel_Button = {
	&RS_Accept_Button, 103, 64, 83, 11, RSB_FLAGS, (APTR) &RB_Border1,
	NULL, &RBC_Text, NULL, NULL, 4, (APTR) &RCB_FN
};


/**********************************/
/* stuff for the Requester itself */

static	SHORT	RS_Req_Lines4[] = {
	190, 2, 190, 79, 191, 79, 191, 2
};

static	struct Border RS_Req_Border4 = {
	-1, -1, 2, 0, JAM1, 4, RS_Req_Lines4, NULL
};

static	SHORT	RS_Req_Lines3[] = {
	0, 79, 191, 79, 191, 80, 0, 80
};

static	struct Border RS_Req_Border3 = {
	-1, -1, 2, 0, JAM1, 4, RS_Req_Lines3, &RS_Req_Border4
};

static	SHORT	RS_Req_Lines2[] = {
	0, 61, 191, 61
};

static	struct Border RS_Req_Border2 = {
	-1, -1, 2, 0, JAM1, 2, RS_Req_Lines2, &RS_Req_Border3
};

static	SHORT	RS_Req_Lines1[] = {
	0, 17, 191, 17
};

static	struct Border RS_Req_Border1 = {
	-1, -1, 2, 0, JAM1, 2, RS_Req_Lines1, &RS_Req_Border2
};

static	struct IntuiText RS_Request_Text = {
	2, 0, JAM1, 8, 5, TOPAZ_FONT_MACRO,
	(UBYTE *) "Please Enter Room Size", NULL
};

static	struct Requester RS_Req = {
	NULL, REQ_X, REQ_Y, 191, 80, 0, 0, &RS_Cancel_Button, &RS_Req_Border1,
	&RS_Request_Text, NULL, 1
};
#endif/*__AMIGAOS__*/

#if 0
static	int S_RS_Width	= DEFAULT_ROOM_WIDTH;
static	int S_RS_Height	= DEFAULT_ROOM_HEIGHT;

static	int S_TS_Width	= 16;
static	int S_TS_Height	= 16;
#endif

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/


#if __AMIGAOS__
/**************************************************************************
 *
 * RW_FN
 *
 * SYNOPSIS
 *		int RW_FN (void)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static	int RW_FN (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RW_FN";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ActivateGadget (&RH_Integer, req_window, &RS_Req);
	return (1);

} /* RW_FN */



/**************************************************************************
 *
 * RH_FN
 *
 * SYNOPSIS
 *		int RH_FN (void)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static	int RH_FN (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RH_FN";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	ActivateGadget (&RW_Integer, req_window, &RS_Req);
	return (1);

} /* RH_FN */


/**************************************************************************
 *
 * RAB_FN
 *
 * SYNOPSIS
 *		int RAB_FN (void)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static	int RAB_FN (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RAB_FN";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	if ((RH_StringInfo.LongInt >= min_two)
				&& (RW_StringInfo.LongInt >= min_one)) {
		return (-1);
	}
	DisplayBeep (MainScreen);
	return (1);

} /* RAB_FN */


/**************************************************************************
 *
 * RCB_FN
 *
 * SYNOPSIS
 *		int RCB_FN (void)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static	int RCB_FN (void)
{

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RCB_FN";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	return (0);

} /* RCB_FN */


/**************************************************************************
 *
 * GetRoomSize
 *
 * SYNOPSIS
 *		int GetRoomSize (
 *			int		*width,
 *			int		*height
 *		)
 *
 * PURPOSE
 *		the function to put up the requester
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static	int GetRoomSize (
	int		*width,
	int		*height
)
{
	ULONG			 class;
	int				 doit = 1;
	struct IntuiMessage	*msg;
	intfuncptr		 func;
	ULONG			 IDCMPFlags;
	struct	RastPort	*rp1;
	struct	Gadget	*gad;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GetRoomSize";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	req_window	= GlobalRoomWindow->Window;
	IDCMPFlags = req_window->IDCMPFlags;
	ModifyIDCMP (req_window, GADGETUP);
	if (Request (&RS_Req, req_window)) {
		rp1     = RS_Req.ReqLayer->rp;

		SetDrMd (rp1, JAM1);

		SetAPen (rp1, 0);
		RectFill (rp1, 1, 61, 188, 77);

		SetAPen (rp1, 3);
		RectFill (rp1, 1, 1, 188, 15);
		RectFill (rp1, 7, 64, 89, 74);
		RectFill (rp1, 103, 64, 185, 74);

		DrawBorder (rp1, &RS_Req_Border1, 0, 0);

		PrintIText (rp1, &RS_Request_Text, 0, 0);

		RefreshGadgets (&RS_Cancel_Button, req_window, &RS_Req);

		ActivateGadget (&RW_Integer, req_window, &RS_Req);
		
		while (doit > 0) {
			while (doit && (msg = (struct IntuiMessage *) GetMsg (req_window->UserPort))) {
				class = msg->Class;
				gad = (struct Gadget *) msg->IAddress;
				ReplyMsg ((struct Message *) msg);
				switch (class) {
				case GADGETUP:
					func = (intfuncptr)gad->UserData;
					doit = (*func)();
				}
			}
			if (doit > 0) {
				WaitPort (req_window->UserPort);
			}
		}
		EndRequest (&RS_Req, req_window);
	}
	ModifyIDCMP (req_window, IDCMPFlags);

	*width = RW_StringInfo.LongInt;
	*height = RH_StringInfo.LongInt;
	return doit;

} /* GetRoomSize */


/**************************************************************************
 *
 * DoubleRequest
 *
 * SYNOPSIS
 *		int DoubleRequest (
 *			int		*one_ptr,
 *			int		*two_ptr,
 *			char	*title,
 *			char	*onelabel,
 *			char	*twolabel
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static	int DoubleRequest (
	int		*one_ptr,
	int		*two_ptr,
	char	*title,
	char	*onelabel,
	char	*twolabel
)
{
	char		hold[32];
	char		t[25];

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "DoubleRequest";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	RW_StringInfo.LongInt = *one_ptr;
	RH_StringInfo.LongInt = *two_ptr;

	sprintf (hold, "%d", *one_ptr);
	strncpy (RW, hold, 5);	
	RW[5] = '\0';

	sprintf (hold, "%d", *two_ptr);
	strncpy (RH, hold, 5);	
	RH[5] = '\0';

	strncpy (t, title, 24);
	t[24] = '\0';

	RW_Text.IText			= (UBYTE *) onelabel;
	RH_Text.IText			= (UBYTE *) twolabel;
	RS_Request_Text.IText	= (UBYTE *) t;

	return (GetRoomSize (one_ptr, two_ptr));

} /* DoubleRequest */
#elif __MSDOS__
#include "numbers.h"
#define DoubleRequest(pone,ptwo,title,plabel1,plabel2) \
		GetTwoNumbers(MainWindow,title,plabel1,plabel2,(short*)pone,(short*)ptwo)
#endif/*__MSDOS__*/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


#if 0
/**************************************************************************
 *
 * RS_Request
 *
 * SYNOPSIS
 *		int RS_Request (
 *			int	*width,
 *			int	*height
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int RS_Request (
	int	*width,
	int	*height
)
{
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RS_Request";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	min_one = 1;
	min_two	= 1;

	/* what you had last */

	result = DoubleRequest (&S_RS_Width, &S_RS_Height, S_RS_TITLE,
					S_RS_LABEL1, S_RS_LABEL2);

	*width	= S_RS_Width;
	*height	= S_RS_Height;

	return (result);

} /* RS_Request */

/**************************************************************************
 *
 * RU_Request
 *
 * SYNOPSIS
 *		int RU_Request (
 *			int	*user_type,
 *			int	*user_num
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int RU_Request (
	int	*user_type,
	int	*user_num
)
{
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "RU_Request";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	min_one = 0;
	min_two	= 0;

	/* what you have now */

	result = DoubleRequest (user_type, user_num, S_RU_TITLE,
					S_RU_LABEL1, S_RU_LABEL2);

	return (result);

} /* RU_Request */


/**************************************************************************
 *
 * TU_Request
 *
 * SYNOPSIS
 *		int TU_Request (
 *			int	*user_type,
 *			int	*user_num
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int TU_Request (
	int	*user_type,
	int	*user_num
)
{
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TU_Request";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	min_one = 0;
	min_two	= 0;

	/* what you have now */

	result = DoubleRequest (user_type, user_num, S_TU_TITLE,
					S_TU_LABEL1, S_TU_LABEL2);

	return (result);

} /* TU_Request */


/**************************************************************************
 *
 * TU2_Request
 *
 * SYNOPSIS
 *		int TU2_Request (
 *			int	*user_type,
 *			int	*user_num,
 *			char *name
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int TU2_Request (
	int	*user_type,
	int	*user_num,
	char *name
)
{
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TU2_Request";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	min_one = 0;
	min_two	= 0;

	/* what you have now */

	result = DoubleRequest (user_type, user_num, name,
					S_TU_LABEL1, S_TU_LABEL2);

	return (result);

} /* TU2_Request */


/**************************************************************************
 *
 * TS_Request
 *
 * SYNOPSIS
 *		int TS_Request (
 *			int	*width,
 *			int	*height
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int TS_Request (
	int	*width,
	int	*height
)
{
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "TS_Request";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	min_one = 1;
	min_two	= 1;

	/* what you had last */

	result = DoubleRequest (&S_TS_Width, &S_TS_Height, S_TS_TITLE,
					S_TS_LABEL1, S_TS_LABEL2);

	*width	= S_TS_Width;
	*height	= S_TS_Height;

	return (result);

} /* TS_Request */
#endif


/**************************************************************************
 *
 * GR_Request
 *
 * SYNOPSIS
 *		int GR_Request (
 *			WORD	*width,
 *			WORD	*height
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int GR_Request (
	WORD	*width,
	WORD	*height
)
{
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GR_Request";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	min_one = 1;
	min_two	= 1;

	/* what you had last */

	result = DoubleRequest (width, height, S_GR_TITLE,
					S_GR_LABEL1, S_GR_LABEL2);

	return (result);

} /* GR_Request */


/**************************************************************************
 *
 * GU_Request
 *
 * SYNOPSIS
 *		int GU_Request (
 *			WORD	*width,
 *			WORD	*height
 *		)
 *
 * PURPOSE
 *
 *
 * USAGE
 *
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int GU_Request (
	WORD	*width,
	WORD	*height
)
{
	int	result;

/*-------------------------------------------------------------------------*/
#if FUNC_NAMES
 CurrentFuncName = "GU_Request";
#endif
#if REQUIRE

#endif
/*-------------------------------------------------------------------------*/

	min_one = 1;
	min_two	= 1;

	/* what you had last */

	result = DoubleRequest (width, height, S_GU_TITLE,
					S_GU_LABEL1, S_GU_LABEL2);

	return (result);

} /* GU_Request */

