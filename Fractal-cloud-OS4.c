/*
    AmigaOS 4 version of a fractal cloud generator

    UI based on gadtoolsgadgets.c example at https://wiki.amigaos.net/wiki/GadTools_Gadgets

    Tested in 32-bit color mode

    Compiling with VBCC:

    vc Fractal-cloud-OS4.c -o Fractal-cloud-OS4 -lamiga
    
*/

#include <stdio.h>
#include <stdlib.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/datatypes.h>
#include <proto/asl.h>

#define RECTFMT_RGB 0
#define RECTFMT_RGBA 1
#define RECTFMT_ARGB 2
#define LBMI_WIDTH 0x84001001
#define LBMI_HEIGHT 0x84001002

#define MYLEFTEDGE 0
#define MYTOPEDGE  0
#define MYWIDTH    320
#define MYHEIGHT   400

#define NBUFSZ (1024)
UBYTE fnamebuf[NBUFSZ] = {0};
UBYTE *copybuf;

/* Gadget defines of our choosing, to be used as GadgetID's,
** also used as the index into the gadget array my_gads[].
*/
#define MYGAD_SLIDER      (0)
#define MYGAD_BUTTON      (1)
#define MYGAD_BUTTON_SAVE (2)
 
/* Range for the slider: */
#define SLIDER_MIN  (100)
#define SLIDER_MAX  (500)
 
struct TextAttr Topaz11 = { "topaz.font", 11, 0, 0, };
struct Screen   *mysc;
struct Window   *mywin;
struct RastPort *rastPort;

struct IntuitionIFace *IIntuition;
struct GraphicsIFace  *IGraphics;
struct GadToolsIFace  *IGadTools;
struct AslIFace       *IAsl;
struct DataTypesIFace *IDataTypes;

LONG topMarginal;
LONG leftMarginal;

struct TagItem frtags[] =
{
    ASLFR_TitleText,       (uint32)"The Amiga file requester",
    ASLFR_InitialHeight,     MYHEIGHT,
    ASLFR_InitialWidth,      MYWIDTH,
    ASLFR_InitialLeftEdge,   MYLEFTEDGE,
    ASLFR_InitialTopEdge,    MYTOPEDGE,
    ASLFR_PositiveText,     (uint32)"OK",
    ASLFR_NegativeText, (uint32)"Cancel",
    ASLFR_InitialFile,       (uint32)"",
    ASLFR_InitialDrawer,        (uint32)"sys:",
    ASLFR_DoSaveMode, TRUE,
    TAG_END
};

/* Print any error message.  We could do more fancy handling (like
** an EasyRequest()), but this is only a demo.
*/
void errorMessage(STRPTR error)
{
if (error)
    IDOS->Printf("Error: %s\n", error);
}

void generateFractalCloud(UWORD *h) {
      ULONG b1 = 0;
      ULONG b2 = 0;
      ULONG b3 = 0;
      ULONG b4 = 0;
      ULONG r;
      ULONG size = 512;

      float height = (float)*h;    // "height" of the cloud fractal
      
      IGraphics->RectFillColor(rastPort,leftMarginal,topMarginal,size+leftMarginal,size+topMarginal, 0xFF000000);
      
      while (size > 1) {
        for (LONG y = 0; y < 512-size + 1; y+=size) {
          for (LONG x = 0; x < 512-size + 1; x+=size) {
            
            b1 = IGraphics->ReadPixelColor(rastPort,x+leftMarginal,y+topMarginal);
            b2 = IGraphics->ReadPixelColor(rastPort,size+x+leftMarginal,y+topMarginal);
            b3 = IGraphics->ReadPixelColor(rastPort,x+leftMarginal,size+y+topMarginal);
            b4 = IGraphics->ReadPixelColor(rastPort,size+x+leftMarginal,size+y+topMarginal);

            b1 = (b1 & 0x0000FF00) >> 8;
            b2 = (b2 & 0x0000FF00) >> 8;
            b3 = (b3 & 0x0000FF00) >> 8;
            b4 = (b4 & 0x0000FF00) >> 8;


            ULONG centre = IGraphics->ReadPixelColor(rastPort, size / 2 + x + leftMarginal, size / 2 + y + topMarginal);
            centre = centre & 0x000000FF;
            
            if (centre == 0x00000000) {

                ULONG r = rand() % ((ULONG)(height * 0.5 + 1.0));
                if (rand() % 2 == 0) r = -r;

                ULONG a = (b1 + b2 + b3 + b4) / 4 + r;                

                if (a > 255) a = 255;
                if (a < 110) a = 110;
                
                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;

                IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);
      
                IGraphics->WritePixel(rastPort, size / 2 + x + leftMarginal, size / 2 + y + topMarginal);
            }
            
            
            ULONG north = IGraphics->ReadPixelColor(rastPort, size / 2 + x + leftMarginal, y + topMarginal);

            north = (north & 0x0000FF00) >> 8;
            north = north & 0x000000FF;

            if (north == 0) {
                
                ULONG r = rand() % ((ULONG)(height * 0.5 + 1.0));
                if (rand() % 2 == 0) r = -r;

                ULONG a = (b1 + b2) / 2 + r;                

                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;

                IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);
      
                IGraphics->WritePixel(rastPort, size / 2 + x + leftMarginal, y + topMarginal);
            }

            ULONG south = IGraphics->ReadPixelColor(rastPort, size / 2 + x + leftMarginal, y + size + topMarginal);

            south = (south & 0x0000FF00) >> 8;
            south = south & 0x000000FF;

            if (south == 0) {
                
                ULONG r = rand() % ((ULONG)(height * 0.5 + 1.0));
                if (rand() % 2 == 0) r = -r;

                ULONG a = (b3 + b4) / 2 + r;
                
                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;

                IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);
      
                IGraphics->WritePixel(rastPort, size / 2 + x + leftMarginal, y + size + topMarginal);
            }

            ULONG west = IGraphics->ReadPixelColor(rastPort, x + leftMarginal, y + size / 2 + topMarginal);

            west = (west & 0x0000FF00) >> 8;
            west = west & 0x000000FF;

            if (west == 0) {
                
                ULONG r = rand() % ((ULONG)(height * 0.5 + 1.0));
                if (rand() % 2 == 0) r = -r;

                ULONG a = (b1 + b3) / 2 + r; 

                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;

                IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);
      
                IGraphics->WritePixel(rastPort, x + leftMarginal, y + size / 2 + topMarginal);
            }

            ULONG east = IGraphics->ReadPixelColor(rastPort, x + size + leftMarginal, y + size / 2 + topMarginal);

            east = (east & 0x0000FF00) >> 8;
            east = east & 0x000000FF;

            if (east == 0) {
                
                ULONG r = rand() % ((ULONG)(height * 0.5 + 1.0));
                if (rand() % 2 == 0) r = -r;

                ULONG a = (b2 + b4) / 2 + r;

                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;

                IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);
      
                IGraphics->WritePixel(rastPort, size + x + leftMarginal, y + size / 2 + topMarginal);
            }
            
        
      }
      }
      size = size / 2;
      height = height * 0.7f; // decrease randomness

      }

      /*
        Draw pixel at the corners of the square fractal
      */
      
      // upper left corner
      ULONG p1 = IGraphics->ReadPixelColor(rastPort, leftMarginal + 1, topMarginal);
      ULONG p2 = IGraphics->ReadPixelColor(rastPort, leftMarginal, topMarginal + 1);
      
      p1 = (p1 & 0x0000FF00) >> 8;
      p2 = (p2 & 0x0000FF00) >> 8;
      
      r = rand() % ((ULONG)(height * 0.5 + 1.0));
      if (rand() % 2 == 0) r = -r;

      ULONG a = (p1 + p2) / 2 + r; 

      if (a > 255) a = 255;
      if (a < 110) a = 110;

      ULONG color = 0xFF0000FF;
      ULONG green = a << 8;
      ULONG red = a << 16;
      color = color | red | green;
      
      IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);
      
      IGraphics->WritePixel(rastPort, leftMarginal, topMarginal);

      // upper right corner
      p1 = IGraphics->ReadPixelColor(rastPort, leftMarginal + 512 - 1, topMarginal);
      p2 = IGraphics->ReadPixelColor(rastPort, leftMarginal + 512, topMarginal + 1);
      
      p1 = (p1 & 0x0000FF00) >> 8;
      p2 = (p2 & 0x0000FF00) >> 8;
      
      r = rand() % ((ULONG)(height * 0.5 + 1.0));
      if (rand() % 2 == 0) r = -r;

      a = (p1 + p2) / 2 + r; 

      if (a > 255) a = 255;
      if (a < 110) a = 110;

      color = 0xFF0000FF;
      green = a << 8;
      red = a << 16;
      color = color | red | green;

      IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);

      IGraphics->WritePixel(rastPort, leftMarginal+512, topMarginal);

      // lower left corner
      p1 = IGraphics->ReadPixelColor(rastPort, leftMarginal + 1, topMarginal + 512);
      p2 = IGraphics->ReadPixelColor(rastPort, leftMarginal, topMarginal + 512 - 1);
      
      p1 = (p1 & 0x0000FF00) >> 8;
      p2 = (p2 & 0x0000FF00) >> 8;
      
      r = rand() % ((ULONG)(height * 0.5 + 1.0));
      if (rand() % 2 == 0) r = -r;

      a = (p1 + p2) / 2 + r; 

      if (a > 255) a = 255;
      if (a < 110) a = 110;

      color = 0xFF0000FF;
      green = a << 8;
      red = a << 16;
      color = color | red | green;

      IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);

      IGraphics->WritePixel(rastPort, leftMarginal, topMarginal+512);

      // lower right corner
      p1 = IGraphics->ReadPixelColor(rastPort, leftMarginal + 512, topMarginal + 512 - 1);
      p2 = IGraphics->ReadPixelColor(rastPort, leftMarginal + 512 - 1, topMarginal + 512);
      
      p1 = (p1 & 0x0000FF00) >> 8;
      p2 = (p2 & 0x0000FF00) >> 8;
      
      r = rand() % ((ULONG)(height * 0.5 + 1.0));
      if (rand() % 2 == 0) r = -r;

      a = (p1 + p2) / 2 + r; 

      if (a > 255) a = 255;
      if (a < 110) a = 110;

      color = 0xFF0000FF;
      green = a << 8;
      red = a << 16;
      color = color | red | green;

      IGraphics->SetRPAttrs(rastPort,
                    RPTAG_APenColor, color,  // drawing color
                    RPTAG_DrMd, JAM1,             // drawing mode
                  TAG_END);

      IGraphics->WritePixel(rastPort, leftMarginal + 512, topMarginal+512);
}

/*
** Function to handle a GADGETUP or GADGETDOWN event.  For GadTools gadgets,
** it is possible to use this function to handle MOUSEMOVEs as well, with
** little or no work.
*/
VOID handleGadgetEvent(struct Window *win, struct Gadget *gad, UWORD code,
    WORD *slider_level, struct Gadget *my_gads[])
{
switch (gad->GadgetID)
    {
    case MYGAD_SLIDER:
        /* Sliders report their level in the IntuiMessage Code field: */
        //IDOS->Printf("Slider at level %ld\n", code);
        *slider_level = code;
        break;

    case MYGAD_BUTTON:
        //IDOS->Printf("Fractal cloud routine called\n");
        generateFractalCloud(slider_level);
        
        IGadTools->GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER], win, NULL,
                            GTSL_Level, *slider_level,
                            TAG_END);
        break;

    case MYGAD_BUTTON_SAVE:

        if (IAsl != NULL)
        {
            struct FileRequester *fr = IAsl->AllocAslRequest(ASL_FileRequest, frtags);
            if (fr != NULL)
            {
                if (IAsl->AslRequest(fr, NULL))
                {
                    //IDOS->Printf("PATH=%s  FILE=%s\n", fr->fr_Drawer, fr->fr_File);

                    strcpy(copybuf,fr->fr_Drawer);
                    if (IDOS->AddPart(copybuf,fr->fr_File,NBUFSZ)) {
                        savePicture(copybuf);
                    }
                }
                IAsl->FreeAslRequest(fr);
            }
            //else IDOS->Printf("User Cancelled\n");
        }        
        
        break;

    }
}
 
 
/*
** Here is where all the initialization and creation of GadTools gadgets
** take place.  This function requires a pointer to a NULL-initialized
** gadget list pointer.  It returns a pointer to the last created gadget,
** which can be checked for success/failure.
*/
struct Gadget *createAllGadgets(struct Gadget **glistptr, void *vi,
    UWORD topborder, WORD slider_level, struct Gadget *my_gads[])
{
struct NewGadget ng;
struct Gadget *gad;
 
/* All the gadget creation calls accept a pointer to the previous gadget, and
** link the new gadget to that gadget's NextGadget field.  Also, they exit
** gracefully, returning NULL, if any previous gadget was NULL.  This limits
** the amount of checking for failure that is needed.  You only need to check
** before you tweak any gadget structure or use any of its fields, and finally
** once at the end, before you add the gadgets.
*/
 
/* The following operation is required of any program that uses GadTools.
** It gives the toolkit a place to stuff context data.
*/
gad = IGadTools->CreateContext(glistptr);
 
/* Since the NewGadget structure is unmodified by any of the CreateGadget()
** calls, we need only change those fields which are different.
*/
ng.ng_LeftEdge   = 150;
ng.ng_TopEdge    = 520+topborder;
ng.ng_Width      = 320;
ng.ng_Height     = 12;
ng.ng_GadgetText = "Height:    ";
ng.ng_TextAttr   = &Topaz11;
ng.ng_VisualInfo = vi;
ng.ng_GadgetID   = MYGAD_SLIDER;
ng.ng_Flags      = NG_HIGHLABEL;
 
my_gads[MYGAD_SLIDER] = gad = IGadTools->CreateGadget(SLIDER_KIND, gad, &ng,
                    GTSL_Min,         SLIDER_MIN,
                    GTSL_Max,         SLIDER_MAX,
                    GTSL_Level,       slider_level,
                    GTSL_LevelFormat, "%2ld",
                    GTSL_MaxLevelLen, 3,
                    TAG_END);
 
ng.ng_LeftEdge  += 40;
ng.ng_TopEdge   += 20;
ng.ng_Width      = 100;
ng.ng_Height     = 12;
ng.ng_GadgetText = "Generate";
ng.ng_GadgetID   = MYGAD_BUTTON;
ng.ng_Flags      = 0;
gad = IGadTools->CreateGadget(BUTTON_KIND, gad, &ng,
                    TAG_END);

ng.ng_LeftEdge  += 120;
ng.ng_TopEdge   += 0;
ng.ng_Width      = 60;
ng.ng_Height     = 12;
ng.ng_GadgetText = "Save";
ng.ng_GadgetID   = MYGAD_BUTTON_SAVE;
ng.ng_Flags      = 0;
gad = IGadTools->CreateGadget(BUTTON_KIND, gad, &ng,
                    TAG_END);

return(gad);
}
 
/*
** Standard message handling loop with GadTools message handling functions
** used (GT_GetIMsg() and GT_ReplyIMsg()).
*/
VOID process_window_events(struct Window *mywin,
    WORD *slider_level, struct Gadget *my_gads[])
{
struct IntuiMessage *imsg;
ULONG imsgClass;
UWORD imsgCode;
struct Gadget *gad;
BOOL terminated = FALSE;
 
while (!terminated)
    {
    IExec->Wait (1 << mywin->UserPort->mp_SigBit);
 
    /* GT_GetIMsg() returns an IntuiMessage with more friendly information for
    ** complex gadget classes.  Use it wherever you get IntuiMessages where
    ** using GadTools gadgets.
    */
    while ((!terminated) &&
           (imsg = IGadTools->GT_GetIMsg(mywin->UserPort)))
        {
        /* Presuming a gadget, of course, but no harm...
        ** Only dereference this value (gad) where the Class specifies
        ** that it is a gadget event.
        */
        gad = (struct Gadget *)imsg->IAddress;
 
        imsgClass = imsg->Class;
        imsgCode = imsg->Code;
 
        /* Use the toolkit message-replying function here... */
        IGadTools->GT_ReplyIMsg(imsg);
 
        switch (imsgClass)
            {
            /*  --- WARNING --- WARNING --- WARNING --- WARNING --- WARNING ---
            ** GadTools puts the gadget address into IAddress of IDCMP_MOUSEMOVE
            ** messages.  This is NOT true for standard Intuition messages,
            ** but is an added feature of GadTools.
            */
            case IDCMP_MOUSEMOVE:
            case IDCMP_GADGETDOWN:
            case IDCMP_GADGETUP:
                handleGadgetEvent(mywin, gad, imsgCode, slider_level, my_gads);
                break;
            case IDCMP_CLOSEWINDOW:
                terminated = TRUE;
                break;
            case IDCMP_REFRESHWINDOW:
                /* With GadTools, the application must use GT_BeginRefresh()
                ** where it would normally have used BeginRefresh()
                */
                IGadTools->GT_BeginRefresh(mywin);
                IGadTools->GT_EndRefresh(mywin, TRUE);
                break;
            }
        }
    }
}
 
/*
** Prepare for using GadTools, set up gadgets and open window.
** Clean up and when done or on error.
*/
VOID gadtoolsWindow(VOID)
{
struct TextFont *font;

struct Gadget   *glist, *my_gads[3];
void            *vi;
WORD            slider_level = 5;
UWORD           topborder;
 
/* Open topaz 11 font, so we can be sure it's openable
** when we later set ng_TextAttr to &Topaz11:
*/
if (NULL == (font = IGraphics->OpenFont(&Topaz11)))
    errorMessage( "Failed to open Topaz 11");
else
    {
    if (NULL == (mysc = IIntuition->LockPubScreen(NULL)))
        errorMessage( "Couldn't lock default public screen");
    else
        {
        if (NULL == (vi = IGadTools->GetVisualInfo(mysc, TAG_END)))
            errorMessage( "GetVisualInfo() failed");
        else
            {
            /* Here is how we can figure out ahead of time how tall the  */
            /* window's title bar will be:                               */
            topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);
 
            if (NULL == createAllGadgets(&glist, vi, topborder,
                                         slider_level, my_gads))
                errorMessage( "createAllGadgets() failed");
            else
                {
                if (NULL == (mywin = IIntuition->OpenWindowTags(NULL,
                        WA_Title,     "Fractal cloud generator for AmigaOS 4",
                        WA_Gadgets,   glist,      WA_AutoAdjust,    TRUE,
                        WA_InnerWidth,  513,
                        WA_InnerHeight, 563,
                        WA_DragBar,    TRUE,      WA_DepthGadget,   TRUE,
                        WA_Activate,   TRUE,      WA_CloseGadget,   TRUE,
                        WA_SizeGadget, FALSE,      WA_SmartRefresh, TRUE,
                        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
                            IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
                            BUTTONIDCMP,
                        WA_PubScreen, mysc,
                        TAG_END)))
                    errorMessage( "OpenWindow() failed");
                else
                    {

                    // rastport of window
                    rastPort = mywin->RPort;
      
                    topMarginal = mywin->BorderTop;
                    leftMarginal = mywin->BorderLeft;

                    ULONG size = 512;

                    // paint fractal cloud area to black
                    IGraphics->RectFillColor(rastPort,leftMarginal,topMarginal,size+leftMarginal,size+topMarginal, 0xFF000000);

                    /* After window is open, gadgets must be refreshed with a
                    ** call to the GadTools refresh window function.
                    */
                    IGadTools->GT_RefreshWindow(mywin, NULL);
 

                    process_window_events(mywin, &slider_level, my_gads);
 
                    IIntuition->CloseWindow(mywin);
                    }
                }
            /* FreeGadgets() even if createAllGadgets() fails, as some
            ** of the gadgets may have been created...If glist is NULL
            ** then FreeGadgets() will do nothing.
            */
            IGadTools->FreeGadgets(glist);
            IGadTools->FreeVisualInfo(vi);
            }
        IIntuition->UnlockPubScreen(NULL, mysc);
        }
    IGraphics->CloseFont(font);
    }
}
 
 
/*
** Open all libraries and run.  Clean up when finished or on error..
*/
int main()
{
  copybuf = malloc(1024);
  if (!copybuf) {
    printf("Couldn't allocate 1024 bytes for filename buffer!\n");
    return 0;
  }

  struct Library *IntuitionBase = IExec->OpenLibrary("intuition.library", 50);
  IIntuition = (struct IntuitionIFace*)IExec->GetInterface(IntuitionBase, "main", 1, NULL);
  if (IIntuition == NULL)
    errorMessage( "Requires V50 intuition.library");
  else
  {
    struct Library *GfxBase = IExec->OpenLibrary("graphics.library", 50);
    IGraphics = (struct GraphicsIFace*)IExec->GetInterface(GfxBase, "main", 1, NULL);
    if (IGraphics == NULL)
        errorMessage( "Requires V50 graphics.library");
    else
    {
      struct Library *GadToolsBase = IExec->OpenLibrary("gadtools.library", 50);
      IGadTools = (struct GadToolsIFace*)IExec->GetInterface(GadToolsBase, "main", 1, NULL);
      if (IGadTools == NULL)
        errorMessage( "Requires V50 gadtools.library");
      else
      {
        struct Library *AslBase = IExec->OpenLibrary("asl.library", 50);
        IAsl = (struct AslIFace*)IExec->GetInterface(AslBase, "main", 1, NULL);
        if (IAsl == NULL)
            errorMessage( "Requires V50 asl.library");
        else
        {
           struct Library *DataTypesBase = IExec->OpenLibrary("datatypes.library", 50);
           IDataTypes = (struct DataTypesIFace*)IExec->GetInterface(DataTypesBase, "main", 1, NULL);
           if (IDataTypes == NULL)
             errorMessage( "Requires V50 datatypes.library");
           else
           {
                gadtoolsWindow();
                
                IExec->DropInterface((struct Interface*)IDataTypes);
                IExec->CloseLibrary(DataTypesBase);    
           }
            IExec->DropInterface((struct Interface*)IAsl);
            IExec->CloseLibrary(AslBase);
        }
        IExec->DropInterface((struct Interface*)IGadTools);
        IExec->CloseLibrary(GadToolsBase);
      }
      IExec->DropInterface((struct Interface*)IGraphics);
      IExec->CloseLibrary(GfxBase);
    }
    IExec->DropInterface((struct Interface*)IIntuition);
    IExec->CloseLibrary(IntuitionBase);
  }

  free(copybuf);
  return 0;
}

int savePicture(UBYTE *copybuf) {

    struct BitMapHeader *bmhd;
    struct pdtBlitPixelArray bpa;

    Object *dto = NULL;
    dto  = IDataTypes->NewDTObject(NULL,
                DTA_SourceType,DTST_RAM,
                DTA_BaseName, "ilbm",
                PDTA_DestMode, PMODE_V43,
                TAG_END); 

    if (!dto) {
        printf("Failed to create DataType object.\n");
        return -1;
    }

    IDataTypes->GetDTAttrs(dto, PDTA_BitMapHeader, (ULONG)&bmhd, TAG_END);
    bmhd->bmh_Left = 0;
    bmhd->bmh_Top = 0;
    bmhd->bmh_Width = 512;
    bmhd->bmh_Height = 512;
    bmhd->bmh_Depth = 24;
    bmhd->bmh_Masking = 0;

    IDataTypes->SetDTAttrs(dto, NULL, NULL,
                  DTA_ObjName, "Cloud",
                  DTA_NominalHoriz, 512,
                  DTA_NominalVert, 512,
                  PDTA_SourceMode,  PMODE_V43,
                  TAG_END);
    
    UBYTE *buf;

    
    buf = malloc(4*512*512);
    if (!buf) {
        IDataTypes->DisposeDTObject(dto);
        printf("Allocating memory for buffer failed.\n");
        return -2;
    }
    
    IGraphics->ReadPixelArray(rastPort, leftMarginal, topMarginal,
               buf, 0, 0,
               512*4, PBPAFMT_ARGB,
               512, 512);

    bpa.MethodID = PDTM_WRITEPIXELARRAY;
    bpa.pbpa_PixelData = buf;   // here is the picture now
    bpa.pbpa_PixelFormat = PBPAFMT_RGB;
    bpa.pbpa_PixelArrayMod = 512*(32/8);
    bpa.pbpa_Left = 0;
    bpa.pbpa_Top = 0;
    bpa.pbpa_Height = 512;
    bpa.pbpa_Width = 512;

    IIntuition->IDoMethodA(dto, &bpa);  

    printf("Attempting to open file %s for writing\n", copybuf);

    BPTR fp;
    fp = IDOS->Open(copybuf,MODE_NEWFILE);
    if (fp) {
        if (!IDataTypes->DoDTMethod(dto,NULL,NULL,DTM_WRITE,NULL,fp,DTWM_IFF,NULL)) {
            printf("Couldn't save the file!\n");
            IDOS->Close(fp);
            IDataTypes->DisposeDTObject(dto);
            free(buf);
            return -4;
        }
    } else {
        printf("Couldn't open the file!\n");
        IDataTypes->DisposeDTObject(dto);
        free(buf);
        return -5;
    } 
    
    
    IDOS->Close(fp);

    free(buf);
    IDataTypes->DisposeDTObject(dto);
           
    EasyRequester("IFF file saved succesfully!","Ok");

    return 0;
}

int32 EasyRequester(CONST_STRPTR text, CONST_STRPTR button, ...)
{
  struct TagItem tags[] = { ESA_Position, REQPOS_CENTERSCREEN, TAG_END };
  struct EasyStruct easyreq = { 0 };
 
  static TEXT textbuffer[1024];
 
  va_list parameter;
  va_start(parameter,button);
  vsprintf(textbuffer,text,parameter);
  va_end(parameter);
 
  easyreq.es_StructSize   = sizeof(struct EasyStruct);
  easyreq.es_Flags        = ESF_SCREEN | ESF_TAGGED | ESF_EVENSIZE;
  easyreq.es_Title        = "Info";
  easyreq.es_TextFormat   = textbuffer;
  easyreq.es_GadgetFormat = button;
  easyreq.es_Screen       = NULL;
  easyreq.es_TagList      = tags;
 
  return IIntuition->EasyRequestArgs(NULL, &easyreq, NULL, NULL);
}