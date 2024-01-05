/* --------------- print.c ----------- */

#include "dflat.h"

extern DBOX PrintSetup;

#define CHARSLINE 80
#define LINESPAGE 66

static int LineCtr;
static int CharCtr;

static char *ports[] = {
	"Lpt1",	"Lpt2",	"Lpt3",
	"Com1",	"Com2",	"Com3",	"Com4",
 	 NULL
};

/* ------- print a character -------- */
static void PrintChar(FILE *prn, int c)
{
	int i;
    if (c == '\n' || CharCtr == cfg.RightMargin)	{
		fputs("\r\n", prn);
		LineCtr++;
		if (LineCtr == cfg.BottomMargin)	{
    		fputc('\f', prn);
			for (i = 0; i < cfg.TopMargin; i++)
	    		fputc('\n', prn);
			LineCtr = cfg.TopMargin;
		}
		CharCtr = 0;
		if (c == '\n')
			return;
	}
	if (CharCtr == 0)	{
		for (i = 0; i < cfg.LeftMargin; i++)	{
			fputc(' ', prn);
			CharCtr++;
		}
	}
	CharCtr++;
    fputc(c, prn);
}

/* --- print the current notepad --- */
void PrintPad(WINDOW wnd)
{
	if (*cfg.PrinterPort)	{
		FILE *prn;
		if ((prn = fopen(cfg.PrinterPort, "wt")) != NULL)	{
			long percent;
			BOOL KeepPrinting = TRUE;
		    ATTRCHR *text = GetText(wnd);
			unsigned oldpct = 100, cct = 0, len = tstrlen(text);
			WINDOW swnd = SliderBox(20, GetTitle(wnd), "Printing");
    		/* ------- print the notepad text --------- */
			LineCtr = CharCtr = 0;
			while (KeepPrinting && text->ch)	{
				PrintChar(prn, text++->ch);
				percent = ((long) ++cct * 100) / len;
				if ((int) percent != oldpct)	{
					oldpct = (int) percent;
					KeepPrinting = SendMessage(swnd, PAINT, 0, oldpct);
				}
    		}
			if (KeepPrinting)
				/* ---- user did not cancel ---- */
				if (oldpct < 100)
					SendMessage(swnd, PAINT, 0, 100);
   			/* ------- follow with a form feed? --------- */
   			if (YesNoBox("Form Feed?"))
       			fputc('\f', prn);
			fclose(prn);
		}
		else
			ErrorMessage("Cannot open printer file");
	}
	else
		ErrorMessage("No printer selected");
}


int PrintSetupProc(WINDOW wnd, MESSAGE msg, PARAM p1, PARAM p2)
{
	int rtn, i = 0, mar;
	char marg[10];
	WINDOW cwnd;
    switch (msg)    {
		case CREATE_WINDOW:
		    rtn = DefaultWndProc(wnd, msg, p1, p2);
			PutItemText(wnd, ID_PRINTERPORT, cfg.PrinterPort);
			while (ports[i] != NULL)
				PutComboListText(wnd, ID_PRINTERPORT, ports[i++]);
			for (mar = CHARSLINE; mar >= 0; --mar)	{
				sprintf(marg, "%3d", mar);
				PutItemText(wnd, ID_LEFTMARGIN, marg);
				PutItemText(wnd, ID_RIGHTMARGIN, marg);
			}
			for (mar = LINESPAGE; mar >= 0; --mar)	{
				sprintf(marg, "%3d", mar);
				PutItemText(wnd, ID_TOPMARGIN, marg);
				PutItemText(wnd, ID_BOTTOMMARGIN, marg);
			}
			cwnd = ControlWindow(&PrintSetup, ID_LEFTMARGIN);
			SendMessage(cwnd, LB_SETSELECTION,
				CHARSLINE-cfg.LeftMargin, 0);
			cwnd = ControlWindow(&PrintSetup, ID_RIGHTMARGIN);
			SendMessage(cwnd, LB_SETSELECTION,
				CHARSLINE-cfg.RightMargin, 0);
			cwnd = ControlWindow(&PrintSetup, ID_TOPMARGIN);
			SendMessage(cwnd, LB_SETSELECTION,
				LINESPAGE-cfg.TopMargin, 0);
			cwnd = ControlWindow(&PrintSetup, ID_BOTTOMMARGIN);
			SendMessage(cwnd, LB_SETSELECTION,
				LINESPAGE-cfg.BottomMargin, 0);
			return rtn;
		case COMMAND:
			if ((int) p1 == ID_OK && (int) p2 == 0)	{
				GetItemText(wnd, ID_PRINTERPORT, cfg.PrinterPort, 4);
				cwnd = ControlWindow(&PrintSetup, ID_LEFTMARGIN);
				cfg.LeftMargin = CHARSLINE -
					SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
				cwnd = ControlWindow(&PrintSetup, ID_RIGHTMARGIN);
				cfg.RightMargin = CHARSLINE -
					SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
				cwnd = ControlWindow(&PrintSetup, ID_TOPMARGIN);
				cfg.TopMargin = LINESPAGE -
					SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
				cwnd = ControlWindow(&PrintSetup, ID_BOTTOMMARGIN);
				cfg.BottomMargin = LINESPAGE -
					SendMessage(cwnd, LB_CURRENTSELECTION, 0, 0);
			}
			break;
        default:
            break;
	}
    return DefaultWndProc(wnd, msg, p1, p2);
}

