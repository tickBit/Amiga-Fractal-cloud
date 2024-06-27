/*
    Fractal cloud generator for AmigaOS 1.3

    Can be compiled at least with DICE:

    dcc -// -ffp -lm Fractal-cloud-OS13.c

    DICE will give exit code 5 (warnings only) and compile the executable

    About using the program:

    - Screen flashes, if there are leading or trailing spaces in the filename,
    and the picture won't be saved.

    - The filename can't have quotemarks

    - The filename can have 256 characters including '\0'
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>

#include <stdio.h>
#include <math.h>

#define RowBytes(w) ((((w) + 15) >> 4) << 1)

#define mskNone 0
#define mskHasMask 1
#define mskHasTransparentColor 2
#define mskLasso 3
#define cmpNone 0
#define cmpByteRun1 1


#define IDCMP_flags CLOSEWINDOW|MOUSEMOVE|GADGETDOWN|GADGETUP|SMART_REFRESH
#define Window_flags WINDOWCLOSE|REPORTMOUSE|ACTIVATE

typedef struct {
UWORD w, h;
WORD x, y;
UBYTE nPlanes;
UBYTE masking;
UBYTE compression;
UBYTE pad1;
UWORD transparentColor;
UBYTE xAspect, yAspect;
WORD pageWidth, pageHeight;
} BitMapHeader;

struct TextAttr fonts = { "Topaz.font",8,0,0 };

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;

struct NewScreen MyScreen = { 0,0,320,256,4,0,1,0,CUSTOMSCREEN,&fonts,"Fractal cloud screen",0,0 };

struct Screen *screen;
struct Window *window;
struct RastPort *rp;
struct IntuiMessage *message;

/* The body text for the requester: */
struct IntuiText txt_file_saved=
{
  15,       /* FrontPen */
  1,       /* BackPen, not used since JAM1. */
  JAM1,    /* DrawMode, do not change the background. */
  16,      /* LedtEdge, 15 pixels out. */
  16,       /* TopEdge, 5 lines down. */
  NULL,    /* ITextFont, default font. */
  "IFF file successfully saved", /* IText, the text . */
  NULL,    /* NextText, no more IntuiText structures link. */
};

/* The body text for the requester: */
struct IntuiText txt_file_open_failed=
{
  15,       /* FrontPen */
  1,       /* BackPen, not used since JAM1. */
  JAM1,    /* DrawMode, do not change the background. */
  16,      /* LedtEdge, 15 pixels out. */
  16,       /* TopEdge, 5 lines down. */
  NULL,    /* ITextFont, default font. */
  "Opening file failed!", /* IText, the text . */
  NULL,    /* NextText, no more IntuiText structures link. */
};

/* The OK text: */
struct IntuiText my_ok_text=
{
  15,       /* FrontPen */
  0,       /* BackPen, not used since JAM1. */
  JAM1,    /* DrawMode, do not change the background. */
  6,       /* LedtEdge, 6 pixels out. */
  3,       /* TopEdge, 3 lines down. */
  NULL,    /* ITextFont, default font. */
  "OK",    /* IText, the text that will be printed. */
  NULL,    /* NextText, no more IntuiText structures link. */
};

/* The coordinates for the box: */
SHORT my_points[]=
{
   -7, -4, /* Start at position (-7, -4) */
  200, -4, /* Draw a line to the right to position (200,-4) */
  200, 11, /* Draw a line down to position (200,11) */
   -7, 11, /* Draw a line to the right to position (-7,11) */
   -7, -4  /* Finish of by drawing a line up to position (-7,-4) */ 
};

/* The Border structure: */
struct Border my_border=
{
  0, 0,        /* LeftEdge, TopEdge. */
  15,           /* FrontPen */
  1,           /* BackPen, for the moment unused. */
  JAM1,        /* DrawMode, draw the lines with colour 1. */
  5,           /* Count, 5 pair of coordinates in the array. */
  my_points,   /* XY, pointer to the array with the coordinates. */
  NULL,        /* NextBorder, no other Border structures are connected. */
};



/* The IntuiText structure: */
struct IntuiText my_text=
{
  15,         /* FrontPen */
  1,         /* BackPen */
  JAM1,      /* DrawMode, draw the characters with colour 1, do not */
             /* change the background. */ 
  -82, 0,    /* LeftEdge, TopEdge. */
  NULL,      /* ITextFont, use default font. */
  "Filename:",   /* IText, the text that will be printed. */
  NULL,      /* NextText, no other IntuiText structures. */
};



UBYTE my_buffer[256]; /* 256 characters including the NULL-sign. */
UBYTE my_undo_buffer[256]; /* Must be at least as big as my_buffer. */



struct StringInfo my_string_info=
{
  my_buffer,       /* Buffer, pointer to a null-terminated string. */
  my_undo_buffer,  /* UndoBuffer, pointer to a null-terminated string. */
                   /* (Remember my_buffer is equal to &my_buffer[0]) */
  0,               /* BufferPos, initial position of the cursor. */
  256,              /* MaxChars, 50 characters + null-sign ('\0'). */
  0,               /* DispPos, first character in the string should be */
                   /* first character in the display. */

  /* Intuition initializes and maintaines these variables: */

  0,               /* UndoPos */
  0,               /* NumChars */
  0,               /* DispCount */
  0, 0,            /* CLeft, CTop */
  NULL,            /* LayerPtr */
  NULL,            /* LongInt */
  NULL,            /* AltKeyMap */
};


struct Gadget my_gadget=
{
  NULL,          /* NextGadget, no more gadgets in the list. */
  88,            /* LeftEdge, 98 pixels out. */
  210,            /* TopEdge, 30 lines down. */
  198,           /* Width, 198 pixels wide. */
  8,             /* Height, 8 pixels lines heigh. */
  GADGHCOMP,     /* Flags, draw the select box in the complement */
                 /* colours. Note: it actually only the cursor which */
                 /* will be drawn in the complement colours (yellow). */
                 /* If you set the flag GADGHNONE the cursor will not be */
                 /* highlighted, and the user will therefore not be able */
                 /* to see it. */
  GADGIMMEDIATE| /* Activation, our program will recieve a message when */
  RELVERIFY,     /* the user has selected this gadget, and when the user */
                 /* has released it. */ 
  STRGADGET,     /* GadgetType, a String gadget. */
  (APTR) &my_border, /* GadgetRender, a pointer to our Border structure. */
  NULL,          /* SelectRender, NULL since we do not supply the gadget */
                 /* with an alternative image. */
  &my_text,      /* GadgetText, a pointer to our IntuiText structure. */
  NULL,          /* MutualExclude, no mutual exclude. */
  (APTR) &my_string_info, /* SpecialInfo, a pointer to a StringInfo str. */
  0,             /* GadgetID, no id. */
  NULL           /* UserData, no user data connected to the gadget. */
};

/* The text string: */
UBYTE save_str[]="Save as IFF";

/* The IntuiText structure: */
struct IntuiText save_text =
{
  1,         /* FrontPen */
  15,         /* BackPen */
  JAM2,      /* DrawMode, draw the characters with colour 1, do not */
             /* change the background. */ 
  4, 2,      /* LeftEdge, TopEdge. */
  NULL,      /* ITextFont, use default font. */
  (APTR)&save_str, /* IText, the text that will be printed. */
             /* (Remember my_text = &my_text[0].) */
  NULL,      /* NextText, no other IntuiText structures are connected. */
};

struct Gadget gadget_save =
{
  (APTR)&my_gadget,          /* NextGadget, no more gadgets in the list. */
  20,            /* LeftEdge */
  180,            /* TopEdge */
  96,            /* Width */
  11,            /* Height, 11 pixels lines heigh. */
  GADGHCOMP,     /* Flags, when this gadget is highlighted, the gadget */
                 /* will be rendered in the complement colours. */
                 /* (Colour 0 (00) will be changed to colour 3 (11) */
                 /* (Colour 1 (01)           - " -           2 (10) */
                 /* (Colour 2 (10)           - " -           1 (01) */
                 /* (Colour 3 (11)           - " -           0 (00) */  
  GADGIMMEDIATE| /* Activation, our program will recieve a message when */
  RELVERIFY,     /* the user has selected this gadget, and when the user */
                 /* has released it. */ 
  BOOLGADGET,    /* GadgetType, a Boolean gadget. */
  NULL, /* GadgetRender, a pointer to our Border structure. */
                 /* (Since Intuition does not know if this will be a */
                 /* pointer to a Border structure or an Image structure, */
                 /* Intuition expects an APTR (normal memory pointer). */
                 /* We will therefore have to calm down the compiler by */
                 /* doing some "casting".) */
  NULL,          /* SelectRender, NULL since we do not supply the gadget */
                 /* with an alternative image. (We complement the */
                 /* colours instead) */
  &save_text,      /* GadgetText, a pointer to our IntuiText structure. */
                 /* (See chapter 3 GRAPHICS for more information) */
  NULL,          /* MutualExclude, no mutual exclude. */
  NULL,          /* SpecialInfo, NULL since this is a Boolean gadget. */
                 /* (It is not a Proportional/String or Integer gdget) */
  0,             /* GadgetID, no id. */
  NULL           /* UserData, no user data connected to the gadget. */
};

/* The text string: */
UBYTE gen_string[]="Generate";

/* The IntuiText structure: */
struct IntuiText gen_text =
{
  1,         /* FrontPen, colour register 1. */
  15,         /* BackPen, colour register 0. */
  JAM2,      /* DrawMode, draw the characters with colour 1, do not */
             /* change the background. */ 
  4, 2,      /* LeftEdge, TopEdge. */
  NULL,      /* ITextFont, use default font. */
  (APTR)&gen_string, /* IText, the text that will be printed. */
             /* (Remember my_text = &my_text[0].) */
  NULL,      /* NextText, no other IntuiText structures are connected. */
};

struct Gadget gadget_gen =
{
  (APTR)&gadget_save,          /* NextGadget, no more gadgets in the list. */
  20,            /* LeftEdge, 40 pixels out. */
  160,            /* TopEdge, 20 lines down. */
  71,            /* Width, 71 pixels wide. */
  11,            /* Height, 11 pixels lines heigh. */
  GADGHCOMP,     /* Flags, when this gadget is highlighted, the gadget */
                 /* will be rendered in the complement colours. */
                 /* (Colour 0 (00) will be changed to colour 3 (11) */
                 /* (Colour 1 (01)           - " -           2 (10) */
                 /* (Colour 2 (10)           - " -           1 (01) */
                 /* (Colour 3 (11)           - " -           0 (00) */  
  GADGIMMEDIATE| /* Activation, our program will recieve a message when */
  RELVERIFY,     /* the user has selected this gadget, and when the user */
                 /* has released it. */ 
  BOOLGADGET,    /* GadgetType, a Boolean gadget. */
  NULL, /* GadgetRender, a pointer to our Border structure. */
                 /* (Since Intuition does not know if this will be a */
                 /* pointer to a Border structure or an Image structure, */
                 /* Intuition expects an APTR (normal memory pointer). */
                 /* We will therefore have to calm down the compiler by */
                 /* doing some "casting".) */
  NULL,          /* SelectRender, NULL since we do not supply the gadget */
                 /* with an alternative image. (We complement the */
                 /* colours instead) */
  (APTR)&gen_text,      /* GadgetText, a pointer to our IntuiText structure. */
                 /* (See chapter 3 GRAPHICS for more information) */
  NULL,          /* MutualExclude, no mutual exclude. */
  NULL,          /* SpecialInfo, NULL since this is a Boolean gadget. */
                 /* (It is not a Proportional/String or Integer gdget) */
  0,             /* GadgetID, no id. */
  NULL           /* UserData, no user data connected to the gadget. */
};

struct NewWindow MyWindow =
{ 0,11,320,256-11,1,3,IDCMP_flags,Window_flags,&gadget_gen,0,"Fractal clouds for AmigaOS 1.3",0,0,
  0,0,0,0,CUSTOMSCREEN };

int main(void) {

    UWORD class;
    short exitFlag = FALSE;

    UWORD colourtable[16] = {0x0000, 0x011f, 0x022f, 0x033f,0x044f, 0x055f, 0x066f, 0x077f, 0x088f,0x099f,0x0aaf,0x0bbf,0x0ccf,0x0ddf,0x0eef,0x0fff};

    IntuitionBase = (struct IntuitionBase*)OpenLibrary("intuition.library",0);
    if (!IntuitionBase) goto cleanup;

    GfxBase = (struct GfxBase*)OpenLibrary("graphics.library",0);
    if (!GfxBase) goto cleanup;

    screen = (struct Screen*)OpenScreen(&MyScreen);
    if (!screen) goto cleanup;

    LoadRGB4(&(screen->ViewPort), &colourtable[0], 16);

    MyWindow.Screen = screen;

    strcpy( my_buffer, "" );

    window = (struct Window*)OpenWindow(&MyWindow);
    if (!window) goto cleanup;

    rp = window->RPort;

    generateFractalCloud(20);

    APTR address;

    while (exitFlag == FALSE) {
        Wait(1 << window->UserPort->mp_SigBit);
        while(message = (struct IntuiMessage*)GetMsg(window->UserPort)) {
   
            class = message->Class;
            address = message->IAddress;

            ReplyMsg((struct Message*)message);
   
            switch(class) {
    
                case CLOSEWINDOW:
                    exitFlag=TRUE;
                    break;

                case GADGETUP:
                    if (address == (APTR)&gadget_gen) generateFractalCloud();
                    if (address == (APTR)&gadget_save) {
                        if (iswhitespace(my_buffer)) {
                            DisplayBeep(screen);
                        } else {
                            if (saveIFF() == RETURN_OK) AutoRequest(window, &txt_file_saved, NULL, &my_ok_text, NULL, NULL, 300, 72);
                            
                        }
                        strcpy( my_buffer, "" );
                        RefreshGList(&my_gadget, window, NULL, 1);
                        break;
                    }
                    if (address == (APTR)&my_gadget) {
                        if (iswhitespace(my_buffer)) {
                            DisplayBeep(screen);
                        } else {
                            if (saveIFF() == RETURN_OK) AutoRequest(window, &txt_file_saved, NULL, &my_ok_text, NULL, NULL, 300, 72);
                        }
                        strcpy( my_buffer, "" );
                        RefreshGList(&my_gadget, window, NULL, 1);
                        break;
                    }
                    break;
            }
        }
    }

cleanup:
  
  if (window) CloseWindow(window);
  if (screen) CloseScreen(screen);
  if (GfxBase) CloseLibrary(GfxBase);
  if (IntuitionBase) CloseLibrary(IntuitionBase);

  return 0;

}

int generateFractalCloud() {
    UWORD b1 = 0;
    UWORD b2 = 0;
    UWORD b3 = 0;
    UWORD b4 = 0;
    WORD size = 128;

    WORD leftMarginal = window->BorderLeft;
    WORD topMarginal = window->BorderTop;

    WORD height = 16;    // "height" of the cloud fractal
      
                                                                                      
    SetAPen(rp,0);
    RectFill(rp, leftMarginal, topMarginal, leftMarginal + size, topMarginal + size);
      
    // upper left corner
    SetAPen(rp, rand() % 15 + 1);
    WritePixel(rp,leftMarginal,topMarginal);

    // upper right corner
    SetAPen(rp, rand() % 15 + 1);
    WritePixel(rp,128+leftMarginal,0+topMarginal);

    // lower left corner
    SetAPen(rp, rand() % 15 + 1);
    WritePixel(rp, leftMarginal, 128+topMarginal);

    // lower right corner
    SetAPen(rp, rand() % 15 + 1);
    WritePixel(rp,128+leftMarginal,128+topMarginal);

    do {
        for (WORD y = 0; y <= 128-size; y+=size) {
          for (WORD x = 0; x <= 128-size; x+=size) {
            
            b1 = ReadPixel(rp,x+leftMarginal,y+topMarginal);
            b2 = ReadPixel(rp,size+x+leftMarginal,y+topMarginal);
            b3 = ReadPixel(rp,x+leftMarginal,size+y+topMarginal);
            b4 = ReadPixel(rp,size+x+leftMarginal,size+y+topMarginal);

            UWORD centre = ReadPixel(rp, size / 2 + x + leftMarginal, size / 2 + y + topMarginal);
            
            if (centre == 0) {
        
                WORD r = rand() % ((WORD)(height + 1.0));
                if (rand() % 2 != 0) r = -r;
                UWORD a = (b1 + b2 + b3 + b4) / 4 + r;                

                if (a > 15) a = 15;
                if (a < 1) a = 1;

                SetAPen(rp, a);
                WritePixel(rp, size / 2 + x + leftMarginal, size / 2 + y + topMarginal);
            }
            
            
            UWORD north = ReadPixel(rp, size / 2 + x + leftMarginal, y + topMarginal);

            if (north == 0) {
                
                WORD r = rand() % ((WORD)(height + 1.0));
                if (rand() % 2 != 0) r = -r;
                UWORD a = (b1 + b2) / 2 + r;                

                if (a > 15) a = 15;
                if (a < 1) a = 1;
      
                SetAPen(rp, a);
                WritePixel(rp, size / 2 + x + leftMarginal, y + topMarginal);
            }

            UWORD south = ReadPixel(rp, size / 2 + x + leftMarginal, y + size + topMarginal);

            if (south == 0) {
                
                WORD r = rand() % ((WORD)(height + 1.0));
                if (rand() % 2 != 0) r = -r;
                UWORD a = (b3 + b4) / 2 + r;
                
                if (a > 15) a = 15;
                if (a < 1) a = 1;
      
                SetAPen(rp,a);
                WritePixel(rp, size / 2 + x + leftMarginal, y + size + topMarginal);
            }

            UWORD west = ReadPixel(rp, x + leftMarginal, y + size / 2 + topMarginal);

            if (west == 0) {
                
                
                WORD r = rand() % ((WORD)(height + 1.0));
                if (rand() % 2 != 0) r = -r;
                UWORD a = (b1 + b3) / 2 + r; 

                if (a > 15) a = 15;
                if (a < 1) a = 1;
      
                SetAPen(rp, a);
                WritePixel(rp, x + leftMarginal, y + size / 2 + topMarginal);
            }

            UWORD east = ReadPixel(rp, x + size + leftMarginal, y + size / 2 + topMarginal);

            if (east == 0) {
                
                WORD r = rand() % ((WORD)(height + 1.0));
                if (rand() % 2 != 0) r = -r;
                UWORD a = (b2 + b4) / 2 + r;

                if (a > 15) a = 15;
                if (a < 1) a = 1;
      
                SetAPen(rp, a);
                WritePixel(rp, size + x + leftMarginal, y + size / 2 + topMarginal);
            }
            
        
      }
      
      }
      size = size / 2;
      height = height * 0.7f; // decrease randomness

      } while (size > 1);

    return 0;
}

/*
    Below should be some fail checking with AllocRaster()
*/
int saveIFF() {

    BPTR fileHandle;
    struct BitMap bitmap;
    struct RastPort rp_bm = {0};
    ULONG cmapsize = 16 * 3;
    int ncolors = 1 << 4; // Number of colors based on the depth

    BitMapHeader bmh;
    bmh.w = 128;
    bmh.h = 128;
    bmh.x = 0;
    bmh.y = 0;
    bmh.nPlanes = 4;
    bmh.masking = 0;
    bmh.compression = 0;
    bmh.pad1 = 0;
    bmh.transparentColor = 0;
    bmh.xAspect = 1;
    bmh.yAspect = 1;
    bmh.pageWidth = 128;
    bmh.pageHeight = 128;
    
    InitBitMap(&bitmap, 4, 128, 128);
    bitmap.Planes[0] = (PLANEPTR)AllocRaster(128,128);
    bitmap.Planes[1] = (PLANEPTR)AllocRaster(128,128);
    bitmap.Planes[2] = (PLANEPTR)AllocRaster(128,128);
    bitmap.Planes[3] = (PLANEPTR)AllocRaster(128,128);

    InitRastPort(&rp_bm);
    rp_bm.BitMap = &bitmap;

    // Copy the contents of the window's RastPort to the bitmap
    ClipBlit(rp, window->BorderLeft, window->BorderTop, &rp_bm, 0, 0, 128, 128, 192);

    // Open the file for writing
    fileHandle = Open(my_buffer, MODE_NEWFILE);
    if (!fileHandle) {
        FreeRaster(bitmap.Planes[3],128,128);
        FreeRaster(bitmap.Planes[2],128,128);
        FreeRaster(bitmap.Planes[1],128,128);
        FreeRaster(bitmap.Planes[0],128,128);
        AutoRequest(window, &txt_file_open_failed, NULL, &my_ok_text, NULL, NULL, 300, 72);
        return RETURN_FAIL;
    }

    // Write FORM header
    Write(fileHandle, "FORM", 4);
    ULONG formSize = 0; // Placeholder for size
    Write(fileHandle, &formSize, 4);
    Write(fileHandle, "ILBM", 4);

    // Write BMHD chunk
    Write(fileHandle, "BMHD", 4);
    ULONG bmhdSize = sizeof(BitMapHeader);
    Write(fileHandle, &bmhdSize, 4);

    Write(fileHandle, &bmh, sizeof(BitMapHeader));

    // Write CMAP chunk
    Write(fileHandle, "CMAP", 4);
    Write(fileHandle, &cmapsize, 4);

    struct ColorMap *colorMap = screen->ViewPort.ColorMap;
    for (int i = 0; i < ncolors; i++) {
        ULONG rgb = GetRGB4(colorMap, i);
        UBYTE r = (rgb >> 8) & 0xF;
        UBYTE g = (rgb >> 4) & 0xF;
        UBYTE b = rgb & 0xF;
        UBYTE color[3] = {r << 4 | r, g << 4 | g, b << 4 | b}; // Expand 4-bit to 8-bit
        Write(fileHandle, color, 3);
    }

    // Write BODY chunk
    Write(fileHandle, "BODY", 4);
    ULONG bodySize = RowBytes(128) * 128 * 4;
    Write(fileHandle, &bodySize, 4);

    ULONG rowBytes = bitmap.BytesPerRow; // For source modulo only
    ULONG FileRowBytes = RowBytes(128); // Width to write in bytes

    UBYTE *planes[4];
    for (int iPlane = 0; iPlane < 4; iPlane++) {
        planes[iPlane] = (UBYTE *)bitmap.Planes[iPlane];
    }

    for (int iRow = 128; iRow > 0; iRow--) {
        for (int iPlane = 0; iPlane < 4; iPlane++) {
            Write(fileHandle, planes[iPlane], FileRowBytes);
            planes[iPlane] += rowBytes;
        }
    }

    // Update FORM size
    Seek(fileHandle, 4, OFFSET_BEGINNING);
    ULONG finalFormSize = 4 + 8 + bmhdSize + 8 + cmapsize + 8 + bodySize;
    Write(fileHandle, &finalFormSize, 4);

    // Close file
    Close(fileHandle);

    // Free bitmap
    FreeRaster(bitmap.Planes[3],128,128);
    FreeRaster(bitmap.Planes[2],128,128);
    FreeRaster(bitmap.Planes[1],128,128);
    FreeRaster(bitmap.Planes[0],128,128);

    return RETURN_OK;
}

BOOL iswhitespace(UBYTE *str)
{

  UBYTE *end;
   
  if(*str == 0)  // All spaces?
    return TRUE;

  // Leading space?
  if (isspace(*str)) return TRUE;

  // trailing space
  end = str + strlen(str) - 1;
  if (isspace((UBYTE)*end)) return TRUE;
  
  return FALSE;
}