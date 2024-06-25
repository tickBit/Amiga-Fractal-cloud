/*
    AmigaOS 3 version of a fractal cloud generator

    This is version is meant to be run on 24 bit Workbench screen.

    UI based on gadtoolsgadgets.c example at https://wiki.amigaos.net/wiki/GadTools_Gadgets

    Compiling with VBCC:

    vc Fractal-cloud-OS3.c -o Fractal-cloud-OS3 -lamiga -fpu=688881
    
*/

#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <dos/dos.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <datatypes/pictureclass.h>
#include <libraries/iffparse.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <libraries/picasso96.h>
#include <proto/picasso96.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>




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
UBYTE *copybuf; // filename to IFF to be saved

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

struct FileRequester *fr;

struct Library  *IntuitionBase;
struct Library        *GfxBase;
struct Library   *GadToolsBase;
struct Library        *AslBase;
struct Library  *IFFParseBase;
struct Library        *DosBase;
struct Library		*P96Base;

LONG topMarginal;
LONG leftMarginal;

/* Print any error message.  We could do more fancy handling (like
** an EasyRequest()), but this is only a demo.
*/
void errorMessage(STRPTR error)
{
if (error)
    printf("Error: %s\n", error);
}

void generateFractalCloud(UWORD *h) {
      ULONG b1 = 0;
      ULONG b2 = 0;
      ULONG b3 = 0;
      ULONG b4 = 0;
      ULONG r;
      ULONG size = 512;

      float height = (float)*h;    // "height" of the cloud fractal
      
                                                                                      
      p96RectFill(rastPort,leftMarginal,topMarginal,size+leftMarginal,size+topMarginal, 0xFF000000);
      
    
      while (size > 1) {
        for (LONG y = 0; y < 512-size + 1; y+=size) {
          for (LONG x = 0; x < 512-size + 1; x+=size) {

            
            b1 = p96ReadPixel(rastPort,x+leftMarginal,y+topMarginal);
            b2 = p96ReadPixel(rastPort,size+x+leftMarginal,y+topMarginal);
            b3 = p96ReadPixel(rastPort,x+leftMarginal,size+y+topMarginal);
            b4 = p96ReadPixel(rastPort,size+x+leftMarginal,size+y+topMarginal);

            b1 = (b1 & 0x0000FF00) >> 8;
            b2 = (b2 & 0x0000FF00) >> 8;
            b3 = (b3 & 0x0000FF00) >> 8;
            b4 = (b4 & 0x0000FF00) >> 8;


            ULONG centre = p96ReadPixel(rastPort, size / 2 + x + leftMarginal, size / 2 + y + topMarginal);
            centre = centre & 0x000000FF;
            
            if (centre == 0x00000000) {

                ULONG r1 = rand() % ((ULONG)(height * 0.5 + 1.0));
                ULONG r3 = rand() % 2;
                if (r3 == 0) r = -r1 ; else r = r1 ;

                ULONG a = (b1 + b2 + b3 + b4) / 4 + r;                

                if (a > 255) a = 255;
                if (a < 110) a = 110;
                
                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;
      
                p96WritePixel(rastPort, size / 2 + x + leftMarginal, size / 2 + y + topMarginal, color);
            }
            
            
            ULONG north = p96ReadPixel(rastPort, size / 2 + x + leftMarginal, y + topMarginal);

            north = (north & 0x0000FF00) >> 8;
            north = north & 0x000000FF;

            if (north == 0) {
                
                ULONG r1 = rand() % ((ULONG)(height * 0.5 + 1.0));
                ULONG r3 = rand() % 2;
                if (r3 == 0) r = -r1 ; else r = r1 ;

                ULONG a = (b1 + b2) / 2 + r;                

                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;
      
                p96WritePixel(rastPort, size / 2 + x + leftMarginal, y + topMarginal, color);
            }

            ULONG south = p96ReadPixel(rastPort, size / 2 + x + leftMarginal, y + size + topMarginal);

            south = (south & 0x0000FF00) >> 8;
            south = south & 0x000000FF;

            if (south == 0) {
                
                ULONG r1 = rand() % ((ULONG)(height * 0.5 + 1.0));
                ULONG r3 = rand() % 2;
                if (r3 == 0) r = -r1 ; else r = r1 ;

                ULONG a = (b3 + b4) / 2 + r;
                
                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;
      
                p96WritePixel(rastPort, size / 2 + x + leftMarginal, y + size + topMarginal, color);
            }

            ULONG west = p96ReadPixel(rastPort, x + leftMarginal, y + size / 2 + topMarginal);

            west = (west & 0x0000FF00) >> 8;
            west = west & 0x000000FF;

            if (west == 0) {
                
                
                ULONG r1 = rand() % ((ULONG)(height * 0.5 + 1.0));
                ULONG r3 = rand() % 2;
                if (r3 == 0) r = -r1 ; else r = r1 ;

                ULONG a = (b1 + b3) / 2 + r; 

                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;
      
                p96WritePixel(rastPort, x + leftMarginal, y + size / 2 + topMarginal, color);
            }

            ULONG east = p96ReadPixel(rastPort, x + size + leftMarginal, y + size / 2 + topMarginal);

            east = (east & 0x0000FF00) >> 8;
            east = east & 0x000000FF;

            if (east == 0) {
                
                ULONG r1 = rand() % ((ULONG)(height * 0.5 + 1.0));
                ULONG r3 = rand() % 2;
                if (r3 == 0) r = -r1 ; else r = r1;

                ULONG a = (b2 + b4) / 2 + r;

                if (a > 255) a = 255;
                if (a < 110) a = 110;

                ULONG color = 0xFF0000FF;
                ULONG green = a << 8;
                ULONG red = a << 16;
                color = color | red | green;
      
                p96WritePixel(rastPort, size + x + leftMarginal, y + size / 2 + topMarginal, color);
            }
            
        
      }
      
      }
      size = size / 2;
      height = height * 0.7f; // decrease randomness

      }
      
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

        *slider_level = code;
        break;

    case MYGAD_BUTTON:

        generateFractalCloud(slider_level);
        
        GT_SetGadgetAttrs(my_gads[MYGAD_SLIDER], win, NULL,
                            GTSL_Level, *slider_level,
                            TAG_DONE);
        break;

    case MYGAD_BUTTON_SAVE:

            fr = AllocAslRequest(ASL_FileRequest, NULL);

            if (fr != NULL) {
              AslRequestTags((APTR)fr,
                ASL_Hail,       "Filename to save IFF",
                ASL_Height,     MYHEIGHT,
                ASL_Width,      MYWIDTH,
                ASL_LeftEdge,   MYLEFTEDGE,
                ASL_TopEdge,    MYTOPEDGE,
                ASL_OKText,     "Ok",
                ASL_CancelText, "Cancel",
                ASL_File,       "",
                ASL_Dir,        "sys:",
                ASL_FuncFlags, FILF_SAVE,
                TAG_DONE);
            
            
    
                    strcpy(copybuf,fr->fr_Drawer);
                    if (AddPart(copybuf,fr->fr_File,NBUFSZ)) {
                        savePicture(copybuf);
                    }
             
                FreeAslRequest(fr);
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
gad = CreateContext(glistptr);
 
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
 
my_gads[MYGAD_SLIDER] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
                    GTSL_Min,         SLIDER_MIN,
                    GTSL_Max,         SLIDER_MAX,
                    GTSL_Level,       slider_level,
                    GTSL_LevelFormat, "%2ld",
                    GTSL_MaxLevelLen, 3,
                    TAG_DONE);
 
ng.ng_LeftEdge  += 40;
ng.ng_TopEdge   += 20;
ng.ng_Width      = 100;
ng.ng_Height     = 12;
ng.ng_GadgetText = "Generate";
ng.ng_GadgetID   = MYGAD_BUTTON;
ng.ng_Flags      = 0;
gad = CreateGadget(BUTTON_KIND, gad, &ng,
                    TAG_DONE);

ng.ng_LeftEdge  += 120;
ng.ng_TopEdge   += 0;
ng.ng_Width      = 60;
ng.ng_Height     = 12;
ng.ng_GadgetText = "Save";
ng.ng_GadgetID   = MYGAD_BUTTON_SAVE;
ng.ng_Flags      = 0;
gad = CreateGadget(BUTTON_KIND, gad, &ng,
                    TAG_DONE);

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
    Wait (1 << mywin->UserPort->mp_SigBit);
 
    /* GT_GetIMsg() returns an IntuiMessage with more friendly information for
    ** complex gadget classes.  Use it wherever you get IntuiMessages where
    ** using GadTools gadgets.
    */
    while ((!terminated) &&
           (imsg = GT_GetIMsg(mywin->UserPort)))
        {
        /* Presuming a gadget, of course, but no harm...
        ** Only dereference this value (gad) where the Class specifies
        ** that it is a gadget event.
        */
        gad = (struct Gadget *)imsg->IAddress;
 
        imsgClass = imsg->Class;
        imsgCode = imsg->Code;
 
        /* Use the toolkit message-replying function here... */
        GT_ReplyIMsg(imsg);
 
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
                GT_BeginRefresh(mywin);
                GT_EndRefresh(mywin, TRUE);
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
if (NULL == (font = OpenFont(&Topaz11)))
    errorMessage( "Failed to open Topaz 11");
else
    {
    if (NULL == (mysc = LockPubScreen(NULL)))
        errorMessage( "Couldn't lock default public screen");
    else
        {
        if (NULL == (vi = GetVisualInfo(mysc, TAG_DONE)))
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

                if (NULL == (mywin = OpenWindowTags(NULL,
                        WA_Title,     "Fractal cloud generator for AmigaOS 3",
                        WA_Gadgets,   glist,      WA_AutoAdjust,    TRUE,
                        WA_InnerWidth,  512,
                        WA_InnerHeight, 562,
                        WA_DragBar,    TRUE,      WA_DepthGadget,   TRUE,
                        WA_Activate,   TRUE,      WA_CloseGadget,   TRUE,
                        WA_SizeGadget, FALSE,      WA_SmartRefresh, TRUE,
                        WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
                            IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
                            BUTTONIDCMP,
                        WA_PubScreen, NULL,
                        WA_AutoAdjust, TRUE,
                        TAG_DONE)))
                    errorMessage( "OpenWindow() failed");
                else
                    {

                    // rastport of window
                    rastPort = mywin->RPort;
      
                    topMarginal = mywin->BorderTop;
                    leftMarginal = mywin->BorderLeft;
                    
                    int size = 512;

                    // paint fractal cloud area to black
                    p96RectFill(rastPort,leftMarginal,topMarginal,size+leftMarginal,size+topMarginal, 0xFF000000);

                    /* After window is open, gadgets must be refreshed with a
                    ** call to the GadTools refresh window function.
                    */
                    GT_RefreshWindow(mywin, NULL);
 

                    process_window_events(mywin, &slider_level, my_gads);
 
                    CloseWindow(mywin);
                    }
                }
            /* FreeGadgets() even if createAllGadgets() fails, as some
            ** of the gadgets may have been created...If glist is NULL
            ** then FreeGadgets() will do nothing.
            */
            FreeGadgets(glist);
            FreeVisualInfo(vi);
            }
        UnlockPubScreen(NULL, mysc);
        }
    CloseFont(font);
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

  if ((IntuitionBase=OpenLibrary("intuition.library",40))==NULL)
    errorMessage( "Requires V40 intuition.library");
  else
  {
    if ((GfxBase=OpenLibrary("graphics.library",40))==NULL)
        errorMessage( "Requires V40 graphics.library");
    else
    {
      if ((GadToolsBase=OpenLibrary("gadtools.library",40))==NULL)
        errorMessage( "Requires V40 gadtools.library");
      else
      {
        if ((AslBase=OpenLibrary("asl.library",40))==NULL)
            errorMessage( "Requires V40 asl.library");
        else
        {
           if ((IFFParseBase=OpenLibrary("iffparse.library",40))==NULL)
                errorMessage( "Requires V40 iffparse.library");
            else
           {
            if ((DosBase=OpenLibrary("dos.library",40))==NULL)
                errorMessage( "Requires V40 dos.library");
            else
            {
             if ((P96Base=OpenLibrary("Picasso96API.library",2))==NULL)
                errorMessage( "Requires V2 Picasso96API.library");
            else
            {
                    gadtoolsWindow();
                    CloseLibrary(P96Base);
            }
                CloseLibrary(DosBase);
            }
              CloseLibrary(IFFParseBase);    
           }
            CloseLibrary(AslBase);
        }
        CloseLibrary(GadToolsBase);
      }
      CloseLibrary(GfxBase);
    }
    CloseLibrary(IntuitionBase);
  }

  free(copybuf);
  return 0;
}

int bit_at(UBYTE *buffer, int x, int y, int col, int b) {
    UBYTE c = buffer[(y * 512 + x) * 4 + col];
    c = (c >> b) & 1;
    return c;
}

#define WIDTH 512
#define HEIGHT 512
#define DEPTH 24

int savePicture(UBYTE *copybuf) {

    BPTR fileHandle;
    struct IFFHandle *iff;
    struct BitMapHeader bmhd;

    // Open the file for writing
    fileHandle = Open(copybuf, MODE_NEWFILE);
    if (!fileHandle) {
        printf("Failed to open file for writing.\n");
        return RETURN_FAIL;
    }

    // Initialize IFF handle
    iff = AllocIFF();
    if (!iff) {
        printf("Failed to allocate IFF handle.\n");
        Close(fileHandle);
        return RETURN_FAIL;
    }

    // Set file and open IFF handle
    iff->iff_Stream = (ULONG)fileHandle;
    InitIFFasDOS(iff);

    // Start a new IFF file
    if (OpenIFF(iff, IFFF_WRITE) != 0) {
        printf("Failed to open IFF file.\n");
        FreeIFF(iff);
        Close(fileHandle);
        return RETURN_FAIL;
    }

    // Add FORM chunk
    if (PushChunk(iff, ID_ILBM, ID_FORM, IFFSIZE_UNKNOWN) != 0) {
        printf("Failed to push FORM chunk.\n");
        CloseIFF(iff);
        FreeIFF(iff);
        Close(fileHandle);
        return RETURN_FAIL;
    }

    // Initialize BitMapHeader structure
    bmhd.bmh_Width = WIDTH; // Width in pixels
    bmhd.bmh_Height = HEIGHT; // Height in pixels
    bmhd.bmh_Depth = 24; // Bit depth
    bmhd.bmh_Masking = mskNone;
    bmhd.bmh_Compression = cmpNone;
    bmhd.bmh_Transparent = 0;
    bmhd.bmh_XAspect = 1;
    bmhd.bmh_YAspect = 1;
    bmhd.bmh_PageWidth = WIDTH;
    bmhd.bmh_PageHeight = HEIGHT;

    // Add BMHD chunk
    if (PushChunk(iff, ID_ILBM, ID_BMHD, sizeof(struct BitMapHeader)) != 0) {
        printf("Failed to push BMHD chunk.\n");
        CloseIFF(iff);
        FreeIFF(iff);
        Close(fileHandle);
        return RETURN_FAIL;
    }

    // Write BitMapHeader data
    if (WriteChunkBytes(iff, (APTR)&bmhd, sizeof(struct BitMapHeader)) != sizeof(struct BitMapHeader)) {
        printf("Failed to write BMHD data.\n");
        CloseIFF(iff);
        FreeIFF(iff);
        Close(fileHandle);
        return RETURN_FAIL;
    }

    // Close BMHD chunk
    PopChunk(iff);

    // Add BODY chunk
    if (PushChunk(iff, ID_ILBM, ID_BODY, IFFSIZE_UNKNOWN) != 0) {
        printf("Failed to push BODY chunk.\n");
        CloseIFF(iff);
        FreeIFF(iff);
        Close(fileHandle);
        return RETURN_FAIL;
    }

	/* Write out the BODY contents */

    UBYTE *buffer = (BYTE *)AllocMem(WIDTH * HEIGHT * 4, MEMF_ANY);
    if (!buffer) {
        printf("Failed to allocate memory for buffer.\n");
        CloseIFF(iff);
        Close(fileHandle);
        FreeIFF(iff);
        return RETURN_FAIL;
    }
    
    struct RenderInfo ri;
    ri.Memory = buffer;
    ri.BytesPerRow = WIDTH * 4;
    ri.RGBFormat = p96GetBitMapAttr(rastPort->BitMap, P96BMA_RGBFORMAT); // Get RGB format

    /* Read rastPort's bitmap into buffer */
    p96ReadPixelArray(&ri, 0, 0, rastPort, leftMarginal, topMarginal, 512, 512);

    int p_index = 0;

    UBYTE *body_data = (UBYTE *)malloc(512 * 512 * 3);

    // Iterate through picture and create BODY data
    for (int y = 0; y < 512; ++y) {
        for (int col = 2; col >-1; col--) { // R = 0, G = 1, B = 2
            for (int b = 0; b < 8; ++b) {
                UBYTE line[512];
                for (int x = 0; x < 512; ++x) {
                    line[x] = bit_at(buffer, x, y, col, b);
                }
                for (int n = 0; n < 512; n += 8) {
                    UBYTE byte = 0;
                    for (int bit = 0; bit < 8; ++bit) {
                        byte |= (line[n + bit] << (7 - bit));
                    }
                    body_data[p_index++] = byte;
                }
            }
        }
    }

    if (WriteChunkBytes(iff, body_data, p_index) != p_index) {
        CloseIFF(iff);
        Close(fileHandle);
        FreeIFF(iff);
        free(body_data);
        return RETURN_FAIL;
    }


    free(body_data);

    // Close BODY chunk
    PopChunk(iff);

    // Close FORM chunk
    PopChunk(iff);

    // Close IFF file
    CloseIFF(iff);

    // Close file
    Close(fileHandle);

    // Free IFF handle
    FreeIFF(iff);

    printf("IFF file saved successfully.\n");

    return RETURN_OK;
}