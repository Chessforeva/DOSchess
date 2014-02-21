
#include <dos.h>
#include <i86.h>

/* detects if mouse driver is present
 DosBox and Windows - always yes
 DOS may be not
*/
int MousePresent()
{
    union REGS  inregs, outregs;
    inregs.w.ax = 0;
    int386( 0x33, &inregs, &outregs );
    return (outregs.w.ax == 0xffff);
}

/* This routine should be called in main FPS loop */  

struct mouseinfo
{
	int X;
	int Y;
	int LBUTTON;
	int RBUTTON;
} MOUSE;

/* Reads mouse status */
void MouseStatus( void )
{
    union REGS  inregs, outregs;
		inregs.w.ax = 0x3;
		int386( 0x33, &inregs, &outregs );
		MOUSE.X = outregs.w.cx;									// x position
		MOUSE.Y = outregs.w.dx;									// y position
		MOUSE.LBUTTON = ( outregs.w.bx & 1 );		// left button
		MOUSE.RBUTTON = ( outregs.w.bx & 2 );		// right button
}

/* This sets text cursor position
 Screen c,r = (0,0)-(79,29)
 use before puts(..) also in video graphics mode */
void SetCursorPosition(int c, int r)
{
	  union REGS  inregs, outregs;
    inregs.h.ah = 2;
    inregs.h.dh = r;
    inregs.h.dl = c;
    inregs.w.bx = 0;
    int386( 0x10, &inregs, &outregs );
}
