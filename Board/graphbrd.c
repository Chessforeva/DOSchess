#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <graph.h>
#include <string.h>
#include <math.h>

/*
 Graphics for chess board 
*/

int AutoGame;

// sets cursor and prints text
void printCR(int c,int r, char *s)
{ 
  //Screen c,r = (0,0)-(79,29)
	SetCursorPosition(c,r);				// In mouse.c
	puts(s);
}

// Display texts
void DispLogoTexts(void)
{
	printCR(0,0,"DOS chess");
  printCR(0,1,"forDosBox");
  printCR(0,2,"2013-2022");

	printCR(70,0," uses");
	printCR(70,1,"GnuChess3");
  printCR(71,4,"controls");
  printCR(71,5,"<-mouse");	
  printCR(73,6,"u-undo");		
  printCR(73,7,"n-new");
  printCR(73,8,"a-demo");  
}

/* Bitmaps of board and pieces */

char *BMPboard;					// board image
char *BMPbrdBk[64];			// background for each square
char *BMPpiece[12];			// each type of piece
char *BMPpcMsk[12];			// mask

char *rd_buf_bo;				// buffers for board reading
char *rd_buf_pc;				// buffers for pieces reading

struct parmsBoard
{
	int x0;		// corner left x
	int y0;		// corner upper y
	int w;		// board width
	int h;		// height
	
	int mU;		// margins upper
	int mD; 	// lower
	int mL;		// at left
	int mR;		// at right
	
	int sw;		// square width
	int sh;		// square height
} Board;


#define BI2_READING 1

void GetImage( int x, int y, int wi, int hi, char *addr, char I, char N ) {
	
	int X,Y;
	FILE *f;
	char *p = addr;
	char *fn = "Board/dat??.bi2";
	int W = wi-x+1, H = hi-y+1;
	int Cnt = 6+(W*H);
	
	
	fn[9] = I;
	fn[10] = N;
	
 	_getimage( x, y, wi, hi, addr );

	if(BI2_READING) {
		
		f = fopen( fn, "rb" );
		fread(p,Cnt,1,f);
	 	fclose(f);
		
	}
	else		// writing to .bi2 image files
	 {
		f = fopen( fn, "wb" );
		fwrite(p,Cnt,1,f);
		fclose(f);
	 }	 

}

void ReadBMPpiece(char *pieceBMPfile, int i,  int x0, int y0, int w,int h )
{
  int c,x,y;
  char *p;
  long sz = _imagesize( x0, y0,x0+w-1, y0+h-1 );

	FILE *f;
	
	if(!BI2_READING) {
		
	f = fopen(pieceBMPfile, "r");
	
	for(y=0;y<h;y++)
	{
	 p = rd_buf_pc;
	 fread(p,w,1,f);		
	 for(x=0;x<w;x++)
	  {
	  	c = *(p++);
	  	_setcolor(c);
	  	_setpixel( x0+x, y0+y );
	  }
	}
	fclose(f);
	
	}
	
	
	// save piece image	  
  BMPpiece[i] = malloc(sz);
  
	GetImage( x0, y0, x0+w-1, y0+h-1, BMPpiece[i], 'A', (char) ('A'+i) );

	_setcolor(0);

	sz = _imagesize( x0, y0, x0+w-1, y0+h-1 );
	BMPpcMsk[i] = malloc(sz);
  
	if(!BI2_READING) {

	// create mask-image for piece
	_putimage( x0, y0, BMPpiece[i], _GPRESET  );	// inverted
	for(y=0;y<h;y++)
	 for(x=0;x<w;x++)
	  {
			c = _getpixel( x0+x, y0+y );
			if(c<255)	_setpixel( x0+x, y0+y );
	  }
	}
	
	GetImage( x0, y0, x0+w-1, y0+h-1, BMPpcMsk[i], 'B', (char)('A'+i) );	
}


void ReadBMPboard(void)
{
  int x,y,c;
  char *p;
  short x1,y1,i;
  long sz = _imagesize( Board.x0, Board.y0, Board.x0+Board.w, Board.y0+Board.h );
  
	FILE *f;
	
	if(!BI2_READING) {
			

	f = fopen("Board/BO.bin", "r");			// Already prepared in binary (bmp to bin)

	for(y=0;y<Board.h;y++)
	 {
	 p = rd_buf_bo;
	 fread(p,Board.w,1,f);
	 for(x=0;x<Board.w;x++)
	  {
	  	c = *(p++);
	  	_setcolor(c);
	  	_setpixel( Board.x0+x, Board.y0+y );
	  }
	 }
	fclose(f);
	
	}
		  
  BMPboard = malloc(sz);
	GetImage( Board.x0, Board.y0, Board.x0+Board.w, Board.y0+Board.h, BMPboard, 'C', '0' );

	if(BI2_READING) {
	_putimage( Board.x0, Board.y0, BMPboard, _GPSET  );
	}
	
	i=0;
	sz = _imagesize( 0, 0, Board.sw, Board.sh+6 );
	for(y1=0;y1<8;y1++)
		for(x1=0;x1<8;x1++,i++)
			{
			x = (Board.x0+Board.mL)+ (x1*Board.sw);
			y = (Board.y0+Board.mU)+ (y1*Board.sh);
			BMPbrdBk[i] = malloc(sz);
			GetImage( x, y-3, x+Board.sw, y+Board.sh+3, BMPbrdBk[i],  (char) ('0'+x1),  (char) ('0'+y1) );	
			}
			
}


/* returns i-number for pieces */
int pieceI( char piece )
{
  int i;
  char s[12] = "PNBRQKpnbrqk";
  for(i=0;i<12;i++) if(s[i]==piece) break;
  return i;
}

/* Reads pictures, draws 1st time and saves into memory */
void ReadBitmaps(void)
{
 // screen size 640 x 480
 
 rd_buf_bo = malloc(1024);
 rd_buf_pc = malloc(200);
 
 ReadBMPboard();
 ReadBMPpiece("Board/WP.bin", pieceI('P'), 10, 60, 58, 59 );
 ReadBMPpiece("Board/WN.bin", pieceI('N'), 10, 120, 58, 59 );
 ReadBMPpiece("Board/WB.bin", pieceI('B'), 10, 180, 58, 59 );
 ReadBMPpiece("Board/WR.bin", pieceI('R'), 10, 240, 58, 59 );
 ReadBMPpiece("Board/WQ.bin", pieceI('Q'), 10, 302, 58, 65 );
 ReadBMPpiece("Board/WK.bin", pieceI('K'), 10, 370, 58, 59 );
 
 ReadBMPpiece("Board/BP.bin", pieceI('p'), 570, 60, 58, 59 );
 ReadBMPpiece("Board/BN.bin", pieceI('n'), 570, 120, 58, 59 );
 ReadBMPpiece("Board/BB.bin", pieceI('b'), 570, 180, 58, 59 );
 ReadBMPpiece("Board/BR.bin", pieceI('r'), 570, 240, 58, 59 );
 ReadBMPpiece("Board/BQ.bin", pieceI('q'), 570, 302, 58, 65 );
 ReadBMPpiece("Board/BK.bin", pieceI('k'), 570, 370, 58, 59 );

 free(rd_buf_bo);
 free(rd_buf_pc);
 
}

struct LastX			// save last calculations
{
	int X;
	int Y;
} Ls;

/* put piece on square */
void PutPiece(char at[2], char piece)
{
  short r, c;
  int x,y,i;
  c = at[0]-'a';
  r = 7 - (at[1]-'1');
  i=pieceI(piece);
  
  x = flag.reverse ?
  			Board.x0 + Board.w - Board.mR - (Board.sw * (c+1)) :
  			Board.x0 + Board.mL + (Board.sw * c);
  y = flag.reverse ?
  			Board.y0 + Board.h - Board.mD - (Board.sh * (r+1)) :
  			Board.y0 + Board.mU + (Board.sh * r);
  			
  if(piece=='Q' || piece == 'q')	y-=4;			// larger piece :)s
	_putimage( x, y, BMPpcMsk[i], _GAND  );
	_putimage( x, y, BMPpiece[i], _GOR  );
	
	Ls.X = x;
  Ls.Y = y;		// save last
}

/* set up first pieces */
void SetUpStartingPieces(void)
{
  char at[2];
  char piece;
  short r, c, l;
	
	for (r = 7; r >= 0; r--)
   for (c = 0; c <= 7; c++)
		{
      l = (flag.reverse ? locn (7 - r, 7 - c) : locn (r, c) );
      at[0] = (char) ('a' + (l&7));  
      at[1] = (char) ('1' + (l>>3));
			piece = ChDos_GetPieceAt(at);
			if(piece) PutPiece(at, piece);
    }
}


/* Rotating pixels */

#define _MRPx 20				// max.count of pixels
#define _MRPr 26				// radius

 
struct rotPixels
{
  char at[2];					// square
  int show;						// flag to activate
  int cnt;						// count of pixels
	int x[_MRPx];
	int y[_MRPx];
	short col[_MRPx];		// colour
	char *BMPpx[_MRPx];
} PX;


void MemAllocRotator( struct rotPixels *R )
		{
		 int i;
		 long sz = _imagesize( 0, 0, 3, 3 );
		 for(i=0;i<_MRPx;i++) R->BMPpx[i] = malloc(sz); 	// allocate the memory
		}

void FreeRotator( struct rotPixels *R )
		{
		 int i;
		 for(i=0;i<_MRPx;i++) free( R->BMPpx[i] );		// free the memory
		}
		
void ClearRotator( struct rotPixels *R )
		{
		 int i;
		 for(i=0;i<R->cnt;i++) _putimage( R->x[i], R->y[i], R->BMPpx[i], _GPSET  );		// restore background pixels
		}
		
void DrawRotator( struct rotPixels *R )
		{
		 int i,x,y;

		 for(i=0;i<R->cnt;i++)
		  {
		  	x=R->x[i];
		  	y=R->y[i];
		  	_setcolor(R->col[i]);
				_ellipse( _GFILLINTERIOR, x, y, x+3, y+3 );		  	
		  }
		}
		
void _Rotator ( struct rotPixels *R )
		{
		 int i, m, Pi2, x, y, cp; 
		 short c, r;
		 int Xo,Yo, X1, Y1;
		 double a;
		 int nX[_MRPx], nY[_MRPx];

		 c = R->at[0]-'a';
		 r = 7 - (R->at[1]-'1');
		 if(flag.reverse) { c = 7-c; r= 7-r; }
     Xo = (Board.x0+Board.mL)+(Board.sw*c)+(Board.sw>>1)-1;
     Yo = (Board.y0+Board.mU)+(Board.sh*r)+(Board.sh>>1)-2;		// the center of square
     
		 cp = 0;  	
		 if(R->show && R->cnt <_MRPx)			// add new rotating pixel if no limit
		 	{
			 	Pi2 = (rand() & 0xFFF);
			 	a = (3.14 * Pi2) / 180;			// convert to 2pi
			 		
				X1 = _MRPr * cos(a);
				Y1 = _MRPr * sin(a);

		 		i = R->cnt;
		 		nX[i] = Xo+X1;
		 		nY[i] = Yo+Y1;
		 		R->col[i] = (i & 0xF);
		 		cp = 1;
		 	}
		 
		 // new positions by rotating all of them
		 for(i=0;i<R->cnt;i++)
		 	{
				a = 0.2;	//(rand() & 7);
				a += atan2( R->y[i]-Yo, R->x[i]-Xo );				// new pixel

				X1 = _MRPr * cos(a);
				Y1 = _MRPr * sin(a);
				nX[i] = Xo+X1;
				nY[i] = Yo+Y1;
			}
			
		 ClearRotator( R );
		 if(!R->show) R->cnt=0;
		 else
		 	{
		 	 //ClearMouse();
		 	 R->cnt += cp;	
			}	 
		 for(i=0;i<R->cnt;i++)
		 	{				
				R->x[i] = x = nX[i];					// new x,y
				R->y[i] = y = nY[i];
				_getimage( x, y, x+3, y+3, R->BMPpx[i] );								// save new background pixel
		 	}
	
		 DrawRotator( R );
		 
		}
		
/* Save mouse position and background, draw in the new position */

struct SavedMouse
{
	int Was;
	int Xwas;
	int Ywas;
	char *Bkgr;
} Mouse;

void ClearMouse( void )
{
  if(Mouse.Was==2) Mouse.Was=1;
  else _putimage( Mouse.Xwas, Mouse.Ywas, Mouse.Bkgr, _GPSET );
    
}

// mouse Y detection in Windows DOS shell and DosBox work different
int MouseModifyY( int Y )
{
	return (Y*5)/2;
}


void ShowMouse( void )
{
 long c;
 int Y;
 
 if(Mouse.Was==0)
  {
  long sz = _imagesize( 0,0,3,3 );
  Mouse.Bkgr = malloc(sz);
  Mouse.Was = 1;
  }
 else ClearMouse();
 
 Y = MouseModifyY(MOUSE.Y);
 
 _getimage( MOUSE.X, Y, MOUSE.X+3, Y+3, Mouse.Bkgr );
  Mouse.Xwas=MOUSE.X; Mouse.Ywas=Y;
 
 c = _getcolor();
 _setcolor(32);
 _rectangle( _GBORDER, MOUSE.X, Y, MOUSE.X+3, Y+3 );
 _setcolor(c);
}

/* this draws background at square */
void DrawBk( char at[2] )
{
  int x,y,i;
  short c,r;
	c = at[0]-'a';
	r = at[1]-'1';
	r = 7-r;
	if(flag.reverse) { c = 7-c; r= 7-r; }
  x = (Board.x0+Board.mL)+(Board.sw*c);
  y = (Board.y0+Board.mU)+(Board.sh*r)-3;
  i = (r*8) + c;
	_putimage( x, y, BMPbrdBk[i], _GPSET );
}

/* this simply redraws complete board */
void RedrawBoard( void )
{
	_putimage( Board.x0, Board.y0, BMPboard, _GPSET );
	SetUpStartingPieces();
}

/* Animation of moving piece */

#define _AnmPx 3				// pixels to move (speed)

struct strctAnim
{
  int show;						// flag to activate  
  char fat[2];				// from square
  char tat[2];				// to square
  char piece;					// moving piece
  int cnt;						// counter
  int cTot;						// count to
	int x;							// current x position
	int y;							// current y position
	int sx;							// step x
	int sy;							// step y
	char *BMPanm;				// buffer for background
} AN;

void AnimMemAlloc( struct strctAnim *A )
		{
		 long sz = _imagesize( 0, 0, Board.sw+12, Board.sh+12 );
		 A->BMPanm = malloc(sz); 	// allocate the memory
		}

void AnimFree( struct strctAnim *A )
		{
		 free(A->BMPanm);			// free the memory
		}
		
void SetAnimator ( struct strctAnim *A, char fat[2], char tat[2], char piece )
		{
		  short c1,r1,c2,r2;
		  c1 = fat[0]-'a';
		  r1 = fat[1]-'1';
		  r1 = 7 - r1;
		  c2 = tat[0]-'a';
		  r2 = tat[1]-'1';
		  r2 = 7 - r2;
		  if(flag.reverse) { c1 = 7-c1; r1= 7-r1; c2 = 7-c2; r2= 7-r2; }
		  
			memcpy( &A->fat, fat, 2 );
			memcpy( &A->tat, tat, 2 );
			A->piece = piece;

			A->sx=(c2-c1)*Board.sw;			// pixels to move on x-axis
			A->sy=(r2-r1)*Board.sh;			// pixels to move on y-axis
			A->cTot = sqrt( pow(A->sx,2) + pow(A->sy,2) ) / _AnmPx;		// counter
			
			A->sx /= A->cTot;
			A->sy /= A->cTot;
			A->cnt=0;
			A->show=1;
			DrawBk( tat );
		}

#define ANIMATION_TOO_SLOW 1				// set 1 to disable animation

/* on animation loop */
void _Animator ( struct strctAnim *A )
		{
		long sz = _imagesize( 0, 0, Board.sw+12, Board.sh+12 );
		int i,x,y;
		char piece;
		char ep[2];
		
		if(A->show)
			{
			 if(A->cnt<2) DrawBk(A->fat);
			 else
			 	{
			 	 _putimage( A->x-6, A->y-6, A->BMPanm, _GPSET );								// restore background
				}			
			
				if(A->cnt==0)
					{
						PutPiece(A->fat, A->piece);
						A->x = Ls.X;
						A->y = Ls.Y;
						A->cnt++;
					}
				else
				 if( A->cnt >= A->cTot )
					{
					 
					 PutPiece(A->tat, ChDos_GetPieceAt(A->tat) );
					 A->show=0;
					 /* in castling case should move rook */
					 if(A->fat[0]=='e' && (A->piece=='K' || A->piece=='k'))
					 	{
							if( A->tat[0]=='g' )
						 		  {
						 		  	if(A->tat[1]=='1') SetAnimator ( &AN, "h1", "f1", 'R' );
						 		  	else SetAnimator ( &AN, "h8", "f8", 'r' );
						 		  }	
							if( A->tat[0]=='c' )
						 		  {
						 		  	if(A->tat[1]=='1') SetAnimator ( &AN, "a1", "d1", 'R' );
						 		  	else SetAnimator ( &AN, "a8", "d8", 'r' );
						 		  }		
					 	}
					 else
					 	{
					 	/* in en passant case should remove pawn */
					  if(A->fat[0]!=A->tat[0] && (A->piece=='P' || A->piece=='p'))
					 	  {
					 	  	ep[0]=A->tat[0]; ep[1]=A->fat[1];
					 	  	piece = ChDos_GetPieceAt(ep);
					 	  	if(!piece) DrawBk(ep);
					 	  }
						  memcpy( &PX.at, A->tat, 2 );
					    PX.show = 1;
					  } 
					}
				 else	
					{
						x= (A->x += A->sx);
						y= (A->y += A->sy);
						i=pieceI(A->piece);
						_getimage( x-6, y-6, x+Board.sw+6, y+Board.sh+6, A->BMPanm );								// save new background	
										
						if(ANIMATION_TOO_SLOW) A->cnt = A->cTot;
						else
						 {
							//_putimage( x, y, BMPpcMsk[i], _GAND  );
							_putimage( x, y, BMPpiece[i], _GOR  );
						 	A->cnt++;
						 }
					}
			}
		}

/* Display status of chess */
void _DispStatus(void)
{
 if(flag.mate)
  {
   printCR(70,28,"checkmate");	
   printCR(70,29,(Side2move? "1-0" : "0-1")); 
  }
 else
  {
  	if(AutoGame) printCR(70,28,"demo game");
    else
    {
  	if(Side2move==computer)
  		{
  		 printCR(70,28,"thinking ");
 	 		 ClearMouse(); Mouse.Was=2;
			 ClearRotator( &PX );					
	 		 PX.cnt = 0;
	 		 PX.show = 0;
	 		}
  	else printCR(70,28,"Your move");
  	}
  	
  	printCR(70,29,(ChDos_IsCheck() ? "check+" : "      ")); 
  }
}
	 	
		
/* Initialization on starting */
void StartBoard(void)
{
  Board.w = Board.h = 480;
  Board.x0 = (640-Board.w)/2;
  Board.y0 = (480-Board.h)/2;
  Board.mU = 6;
  Board.mL = 12;
  Board.mR = 12;
  Board.mD = 8;
  Board.sw = (Board.w-Board.mL-Board.mR)/8;
  Board.sh = (Board.h-Board.mU-Board.mD)/8;
	ReadBitmaps();
	SetUpStartingPieces();
	MemAllocRotator(&PX);
	AnimMemAlloc(&AN);
	DispLogoTexts();
	AutoGame = 0;
}

void FreeAllocated(void)
{
  int i;
  free(rd_buf_bo);
  free(rd_buf_pc);  
  for(i=0;i<12;i++)
   {
   	free(BMPpiece[i]);
   	free(BMPpcMsk[i]);
   }
	free(BMPboard);
  for(i=0;i<64;i++) free( BMPbrdBk[i] );
	FreeRotator(&PX);
	AnimFree(&AN);
}


/* Release and clear on ending */
void EndBoard(void)
{
	FreeAllocated();
}


long FPSsec_;								// contains last socond 

/* Frames per seconds... */
void UpdateGraphicsFps (void)
{
 char piece;
 char *fat, *tat;
 long sec;
 
	_Rotator( &PX );		// show rotating pixels
	_Animator( &AN );		// animation
	
	sec = time(NULL);
	if(sec != FPSsec_)
	 {
	 	FPSsec_ = sec;			// once per second...
	 	
	 	 if( !AN.show ) _DispStatus();	 	
	 	
							// And process computer move...
		if( !AN.show && ChDos_SelectMove() )
			{		
				fat = &mvstr[0][0];
				tat = &mvstr[0][2];
				piece = ChDos_GetPieceAt(tat);
				SetAnimator ( &AN, fat, tat, piece );
				_DispStatus();
			}
		}			
}

		
/* mouse has been pressed */
void MousePressed( int x, int y )
{
 int c, r, Y;
 char at[2];
 char piece;
 
 Y = MouseModifyY(y);
 
 if( !AN.show && ( x > Board.x0+Board.mL && x< Board.x0+Board.w-Board.mR ) &&
 								( Y > Board.y0+Board.mU && Y < Board.y0+Board.h-Board.mD ))
 		{
 			c = (x- (Board.x0+Board.mL))/Board.sw;
 			r = (Y- (Board.y0+Board.mU))/Board.sh;
 			r = 7-r;
 			if(flag.reverse) { c = 7-c; r= 7-r; }
 			at[0] = (char) ('a' + c);  
 			at[1] = (char) ('1' + r);
 			
			if( ChDos_IsFrom( at )  )
 						{
 						if( !PX.show || memcmp(at,PX.at,2)!=0 )
 							{
 							 if(PX.show)
 								{
 								DrawBk(PX.at);
								piece = ChDos_GetPieceAt(PX.at);
								if(piece) PutPiece(PX.at, piece);
								}
								
 							ClearRotator( &PX );
 							PX.cnt = 0;
 							memcpy( &PX.at, at, 2 );
 							PX.show = 1;
 							}
 						}
 			else
 			 if( ChDos_DoMove( PX.at, at ))
 						{
 						ClearMouse(); Mouse.Was=2;
 						ClearRotator( &PX );			
 						PX.cnt = 0;
 						PX.show = 0;
 						
						piece = ChDos_GetPieceAt(at);						
 						SetAnimator ( &AN, PX.at, at, piece );
 						
 						} 			 	
 		}
}

/* undo move */
void UndoPressedMove(void)
{
 if( !AN.show )
  {
	 ClearMouse(); Mouse.Was=2;
	 ClearRotator( &PX );					
	 PX.cnt = 0;
	 PX.show = 0;
	 
	 ChDos_UndoMove();
	 
	 RedrawBoard();
  }
}

/* NewGame */
void NewGamePressed(void)
{
  AN.show = 0;		// stop animation
  	
  chDos_SetNewGame();

  ClearMouse(); Mouse.Was=2;	// hide mouse
	ClearRotator( &PX );			 						
	PX.cnt = 0;			// stop rotating pixels
	PX.show = 0;
	AutoGame = 0;
	RedrawBoard();

}

void PressedDemoGame(void)
{
	AutoGame = 1;
}
