/* svga.c */
#include <stdio.h>
#include <ctype.h>
#include <vga.h>
#include <vgakeyboard.h>

#include "misc.h"	/* for POINT */
#include "convxpm.h"

#include "graphics.h"


/* bitmaps */
#include "xpm/num0.xpm"
#include "xpm/num1.xpm"
#include "xpm/num2.xpm"
#include "xpm/num3.xpm"
#include "xpm/num4.xpm"
#include "xpm/num5.xpm"
#include "xpm/num6.xpm"
#include "xpm/num7.xpm"
#include "xpm/num8.xpm"
#include "xpm/num9.xpm"
#include "xpm/blank.xpm"
#include "xpm/bomb.xpm"
#include "xpm/level.xpm"
#include "xpm/life.xpm"
#include "xpm/plus.xpm"
#include "xpm/score.xpm"
#include "xpm/shield.xpm"

#define NUM_BITMAPS	17

/* conversions of above (all 16x16) */
unsigned char *bitmaps[NUM_BITMAPS];

/* indicies in above array */
#define BMP_NUM0	0
#define BMP_NUM1	1
#define BMP_NUM2	2
#define BMP_NUM3	3
#define BMP_NUM4	4
#define BMP_NUM5	5
#define BMP_NUM6	6
#define BMP_NUM7	7
#define BMP_NUM8	8
#define BMP_NUM9	9
#define BMP_BLANK	10
#define BMP_BOMB	11
#define BMP_LEVEL	12
#define BMP_LIFE	13
#define BMP_PLUS	14
#define BMP_SCORE	15
#define BMP_SHIELD	16


static int current_x=0,current_y=0;

/* this leaves 16 lines at the top for score etc. */
static int width=640,height=464,mindimhalf=232;

/* this oddity is needed to simulate the mapping the original used a
 * Windows call for. MAX_COORD is a power of 2, so when optimised
 * this isn't too evil.
 */
#define convx(x)	(width/2+(x)*mindimhalf/MAX_COORD)
#define convy(y)	(16+height/2-(y)*mindimhalf/MAX_COORD)



int IsKeyDown(int key)
{
switch(key)
  {
  case KEY_F1:		return(keyboard_keypressed(SCANCODE_F1));
  case KEY_TAB:		return(keyboard_keypressed(SCANCODE_TAB));
  case KEY_S:		return(keyboard_keypressed(SCANCODE_S));
  case KEY_LEFT:	return(keyboard_keypressed(SCANCODE_CURSORBLOCKLEFT));
  case KEY_RIGHT:	return(keyboard_keypressed(SCANCODE_CURSORBLOCKRIGHT));
  case KEY_DOWN:	return(keyboard_keypressed(SCANCODE_CURSORBLOCKDOWN));
  case KEY_UP:		return(keyboard_keypressed(SCANCODE_CURSORBLOCKUP));
  case KEY_SPACE:	return(keyboard_keypressed(SCANCODE_SPACE));
  case KEY_ESC:		return(keyboard_keypressed(SCANCODE_ESCAPE));
  default:
    return(0);
  }
}


/* I can't use gl_* funcs as I want it to be able to work in 640x480x4,
 * which means I need my own clipping and thus my own routine.
 * XXX I may rethink not using gl - 16-colour is pretty flickery,
 * and it may not be worth bothering to support it.
 *
 * This is a slightly hacked version of:
 *
 * digline: draw digital line from (x1,y1) to (x2,y2),
 * calling a user-supplied procedure at each pixel.
 * Does no clipping.  Uses Bresenham's algorithm.
 *
 * Paul Heckbert	3 Sep 85
 *
 * ...which is from "Graphics Gems", Academic Press, 1990.
 */

/* absolute value of a */
#define ABS(a)		(((a)<0) ? -(a) : (a))

/* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)		(((a)<0) ? -1 : 1)

void drawline(int x1,int y1,int x2,int y2)
{
int d, x, y, ax, ay, sx, sy, dx, dy;

dx = x2-x1;  ax = ABS(dx)<<1;  sx = SGN(dx);
dy = y2-y1;  ay = ABS(dy)<<1;  sy = SGN(dy);

x = x1;
y = y1;
if (ax>ay)
  {		/* x dominant */
  d = ay-(ax>>1);
  for (;;)
    {
    if(y>16) vga_drawpixel(x,y);
    if (x==x2) return;
    if (d>=0)
      {
      y += sy;
      d -= ax;
      }
    x += sx;
    d += ay;
    }
  }
else 
  {			/* y dominant */
  d = ax-(ay>>1);
  for (;;) 
    {
    if(y>16) vga_drawpixel(x,y);
    if (y==y2) return;
    if (d>=0)
      {
      x += sx;
      d -= ay;
      }
    y += sy;
    d += ax;
    }
  }
}



void MoveTo(int x,int y)
{
current_x=convx(x);
current_y=convy(y);
}


void LineTo(int x,int y)
{
x=convx(x);
y=convy(y);
drawline(current_x,current_y,x,y);
current_x=x;
current_y=y;
}


void Polyline(POINT *pts,int n)
{
int f;

if(n<2) return;

MoveTo(pts->x,pts->y);
pts++;
for(f=1;f<n;f++,pts++)
  LineTo(pts->x,pts->y);
}


/* doesn't set current_[xy] because hyperoid.c doesn't need it to */
void SetPixel(int x,int y,int c)
{
vga_setcolor(c);
vga_drawpixel(convx(x),convy(y));
}


void set_colour(int c)
{
vga_setcolor(c);
}


/* SetIndicator - set a quantity indicator */

int SetIndicator( char * npBuff, char bitmap, int nQuant )
{
	if (nQuant > 5)
	{
		*npBuff++ = bitmap; *npBuff++ = bitmap;
		*npBuff++ = bitmap; *npBuff++ = bitmap;
		*npBuff++ = BMP_PLUS;
	}
	else
	{
		int nBlank = 5 - nQuant;
		while (nQuant--) *npBuff++ = bitmap;
		while (nBlank--) *npBuff++ = BMP_BLANK;
	}
	return( 5 );
}


/* score_graphics - draw score and rest of status display */

void score_graphics(int level,int score,int lives,int shield,int bomb)
{
static char szScore[40];
char szBuff[sizeof(szScore)];
char *npBuff = szBuff;
int nLen, x, y;

*npBuff++ = BMP_LEVEL;
sprintf( npBuff, "%2.2d", level );
while (isdigit( *npBuff ))
	*npBuff = (char)(*npBuff + BMP_NUM0 - '0'), ++npBuff;
*npBuff++ = BMP_BLANK;
*npBuff++ = BMP_SCORE;
sprintf( npBuff, "%7.7d", score );
while (isdigit( *npBuff ))
	*npBuff = (char)(*npBuff + BMP_NUM0 - '0'), ++npBuff;
*npBuff++ = BMP_BLANK;
npBuff += SetIndicator( npBuff, BMP_LIFE, lives );
npBuff += SetIndicator( npBuff, BMP_SHIELD, shield );
npBuff += SetIndicator( npBuff, BMP_BOMB, bomb );
nLen = npBuff - szBuff;

for(y=0;y<16;y++)
  for(x=0;x<nLen;x++)
    vga_drawscansegment(bitmaps[(int)szBuff[x]]+y*16,x*16,y,16);
}


void conv_xpms(int *palrgb)
{
int f;

bitmaps[BMP_NUM0]=xpm2bytemap(num0_xpm,palrgb);
bitmaps[BMP_NUM1]=xpm2bytemap(num1_xpm,palrgb);
bitmaps[BMP_NUM2]=xpm2bytemap(num2_xpm,palrgb);
bitmaps[BMP_NUM3]=xpm2bytemap(num3_xpm,palrgb);
bitmaps[BMP_NUM4]=xpm2bytemap(num4_xpm,palrgb);
bitmaps[BMP_NUM5]=xpm2bytemap(num5_xpm,palrgb);
bitmaps[BMP_NUM6]=xpm2bytemap(num6_xpm,palrgb);
bitmaps[BMP_NUM7]=xpm2bytemap(num7_xpm,palrgb);
bitmaps[BMP_NUM8]=xpm2bytemap(num8_xpm,palrgb);
bitmaps[BMP_NUM9]=xpm2bytemap(num9_xpm,palrgb);

bitmaps[BMP_BLANK ]=xpm2bytemap(blank_xpm ,palrgb);
bitmaps[BMP_BOMB  ]=xpm2bytemap(bomb_xpm  ,palrgb);
bitmaps[BMP_LEVEL ]=xpm2bytemap(level_xpm ,palrgb);
bitmaps[BMP_LIFE  ]=xpm2bytemap(life_xpm  ,palrgb);
bitmaps[BMP_PLUS  ]=xpm2bytemap(plus_xpm  ,palrgb);
bitmaps[BMP_SCORE ]=xpm2bytemap(score_xpm ,palrgb);
bitmaps[BMP_SHIELD]=xpm2bytemap(shield_xpm,palrgb);

for(f=0;f<NUM_BITMAPS;f++)
  if(bitmaps[f]==NULL)
    fprintf(stderr,"bad XPM data, conversion failed!\n"),exit(1);
}


/* note that this actually changes palrgb[] (ouch :-)) */
void graphics_init(int argc,char *argv[],int *palrgb)
{
int f;

vga_init();

conv_xpms(palrgb);

if(vga_hasmode(G640x480x256))
  vga_setmode(G640x480x256);
else
  vga_setmode(G640x480x16);

for(f=0;f<16*3;f++) palrgb[f]>>=2;
vga_setpalvec(0,16,palrgb);

keyboard_init();
}


void graphics_update(void)
{
keyboard_update();
}


void graphics_exit(void)
{
keyboard_close();
vga_setmode(TEXT);
}
