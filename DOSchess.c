#include <stdio.h>
#include <conio.h>

#include "Chess/gnu3ch.c"
#include "Board/mouse.c"
#include "Board/graphbrd.c"

int main(int arg_c, char *arg_v[])
{
  char inkey;
  
  int AutoGame;
  
  ChDos_Init();												// chess interface in gnu3ch

  _setvideomode( _VRES256COLOR );
	
  StartBoard();				// draw board in graphbrd
       
  while( !kbhit() || (inkey=getch())!=27 )
  {
   if(INKEY) {
	if(INKEY<10) INKEY--;
	else {
		inkey = INKEY;
		INKEY = 9;
	}
   }
	
   if(inkey)
   	{
 	  if(inkey=='a')	{ PressedDemoGame(); AutoGame=1; }
 	  if(inkey=='u' && !AutoGame) UndoPressedMove();
 	  if(inkey=='n')	{ NewGamePressed(); AutoGame=0; }
 	  inkey=0;
 	  }
 	  
   if(AutoGame) ChDos_AutoMove();

   MouseStatus();
   ShowMouse();
   if(MOUSE.LBUTTON)
   	{
   	MousePressed( MOUSE.X, MOUSE.Y );		// verify user control in graphbrd
   	}
   UpdateGraphicsFps();	
  }
  
  _setvideomode( _DEFAULTMODE );
  
  ChDos_End();

  return 0;
}


