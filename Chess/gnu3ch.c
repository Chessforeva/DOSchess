/*
  GNU Chess version 3.1  1990

  Copyright (C) 1986, 1987, 1988, 1989, 1990 Free Software Foundation, Inc.
  Copyright (c) 1988, 1989, 1990  John Stanback

  CHESS is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY.  No author or distributor accepts responsibility to anyone for
  the consequences of using it or for whether it serves any particular
  purpose or works at all, unless he says so in writing.  Refer to the CHESS
  General Public License for full details.

  Everyone is granted permission to copy, modify and redistribute CHESS, but
  only under the conditions described in the CHESS General Public License.
  A copy of this license is supposed to have been given to you along with
  CHESS so you can know your rights and responsibilities.  It should be in a
  file named COPYING.  Among other things, the copyright notice and this
  notice must be preserved on all copies.
  
  
  
  Modified version for Watcom C compiler for MSDOS
   by Chessforeva 2013
   
  Reason: good source in a single file playing decent chess.
  Set input:
    100 moves in 1 minute
    Operator time = 10000
         and it plays fastly!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include <signal.h>
#include <dos.h>
#include <conio.h>


/* <stdio.h */
extern int fclose (FILE *);
extern int fscanf (FILE *, const char *, ...);
extern int fprintf (FILE *, const char *, ...);
extern int fflush (FILE *);

/*
  ttblsz must be a power of 2.
  Setting ttblsz 0 removes the transposition tables.
*/
//#define ttblsz (1 << 9)
#define ttblsz (1 << 16)
#define huge


#define maxdepth 30
#define white 0
#define black 1
#define neutral 2
#define no_piece 0
#define pawn 1
#define knight 2
#define bishop 3
#define rook 4
#define queen 5
#define king 6
#define bpawn 7
#define pmask 0x0007
#define promote 0x0008
#define cstlmask 0x0010
#define epmask 0x0020
#define exact 0x0040
#define pwnthrt 0x0080
#define check 0x0100
#define capture 0x0200
#define draw 0x0400
#define maxdepth 30
#define false 0
#define true 1

struct leaf
{
  short f, t, score, reply;
  unsigned short flags;
};
struct GameRec
{
  unsigned short gmove;
  short score, depth, time, piece, color;
  long nodes;
};
struct TimeControlRec
{
  short moves[2];
  long clock[2];
};
struct BookEntry
{
  struct BookEntry *next;
  unsigned short *mv;
};

struct flags
{
  short mate;           /* the game is over */
  short post;           /* show principle variation */
  short quit;           /* quit/exit gnuchess */
  short reverse;        /* reverse board display */
  short bothsides;      /* computer plays both sides */
  short hash;           /* enable/disable transposition table */
  short force;          /* enter moves */
  short easy;           /* disable thinking on opponents time */
  short beep;           /* enable/disable beep */
  short timeout;        /* time to make a move */
  short rcptr;          /* enable/disable recapture heuristics */
};

extern struct leaf Tree[2000], *root;
extern short TrPnt[maxdepth];
extern short board[64], color[64];
extern short PieceList[2][16], PawnCnt[2][8];
extern short castld[2], Mvboard[64];
extern short svalue[64];
extern struct flags flag;
extern short opponent, computer, Awindow, Bwindow, INCscore;
extern short dither, player;
extern short xwndw, epsquare, contempt;
extern long ResponseTime, ExtraTime, Level, et, et0, time0, ft;
extern long NodeCnt, ETnodes, EvalNodes, HashCnt, HashCol;
extern struct GameRec GameList[512];
extern short GameCnt, Game50;
extern short Sdepth, MaxSearchDepth;
extern struct BookEntry *Book;
extern struct TimeControlRec TimeControl;
extern short TCflag, TCmoves, TCminutes, OperatorTime;
extern const short otherside[3];
extern const short Stboard[64];
extern const short Stcolor[64];
extern unsigned short hint, PrVar[maxdepth];

#define row(a) ((a) >> 3)
#define column(a) ((a) & 7)
#define locn(a,b) (((a) << 3) | b)

#define distance(a,b) distdata[a][b]
extern short distdata[64][64];

/* gnuchess.c external functions */
extern void NewGame (void);
/* book.c */
extern int parse (FILE * fd, unsigned short int *mv, short int side);
extern void GetOpenings (void);
extern void OpeningBook (unsigned short int *hint);
/* search.c */
extern void repetition (short int *cnt);
extern void SelectMove (short int side, short int iop);
extern int search (short int side,
                   short int ply,
                   short int depth,
                   short int alpha,
                   short int beta,
                   unsigned short int *bstline,
                   short int *rpt);
/* tran.c */
#if ttblsz
extern int ProbeTTable (short int side,
                        short int depth,
                        short int *alpha,
                        short int *beta,
                        short int *score);
extern void PutInTTable (short int side,
                         short int score,
                         short int depth,
                         short int alpha,
                         short int beta,
                         unsigned short int mv);
extern void ZeroTTable (void);
extern void ZeroRPT (void);
#ifdef HASHFILE

extern int ProbeFTable (short int side,
                        short int depth,
                        short int *alpha,
                        short int *beta,
                        short int *score);
extern void PutInFTable (short int side,
                         short int score,
                         short int depth,
                         short int alpha,
                         short int beta,
                         short unsigned int f,
                         short unsigned int t);
#endif /* HASHFILE */
#endif /* ttblsz */
/* move.c */
extern void Initialize_moves (void);
extern void MoveList (short int side, short int ply);
extern void CaptureList (short int side, short int ply);
extern int castle (short int side, short int kf, short int kt, short int iop);
extern void MakeMove (short int side,
                      struct leaf * node,
                      short int *tempb,
                      short int *tempc,
                      short int *tempsf,
                      short int *tempst,
                      short int *INCscore);
extern void UnmakeMove (short int side,
                        struct leaf * node,
                        short int *tempb,
                        short int *tempc,
                        short int *tempsf,
                        short int *tempst);
extern void InitializeStats (void);
/* eval.c */
extern int SqAtakd (short int sq, short int side);
extern int evaluate (short int side,
                     short int ply,
                     short int alpha,
                     short int beta,
                     short int INCscore,
                     short int *slk,
                     short int *InChk);
extern void ScoreLoneKing (short int side, short int *score);
extern void ScorePosition (short int side, short int *score);
extern void ExaminePosition (void);
extern void UpdateWeights (void);

/* *dsp.c external functions */
extern void Initialize (void);
extern void InputCommand (void);
extern void ExitChess (void);
extern void ClrScreen (void);
extern void SetTimeControl (void);
extern void SelectLevel (void);
extern void UpdateDisplay (short int f,
                           short int t,
                           short int flag,
                           short int iscastle);
extern void ElapsedTime (short int iop);
extern void ShowSidetomove (void);
extern void SearchStartStuff (short int side);
extern void ShowDepth (char ch);
extern void ShowResults (short int score,
                         unsigned short int *bstline,
                         char ch);
extern void algbr (short int f, short int t, short int flag);
extern void OutputMove (void);
extern void ShowCurrentMove (short int pnt, short int f, short int t);
extern void ListGame (void);
extern void ShowMessage (char *s);
extern void ClrScreen (void);
extern void gotoXY (short int x, short int y);
extern void ClrEoln (void);
extern void DrawPiece (short int sq);
extern void UpdateClocks (void);

#define RWA_ACC "r+b"
#define WA_ACC "w+b"

/* <stdlib.h> */
extern int abs (int);
extern int atoi (const char *);
/* <time.h> */

extern time_t time (time_t *);

/* <string.h> */
extern void *memset (void *, int, size_t);

#define valueP 100
#define valueN 350
#define valueB 355
#define valueR 550
#define valueQ 1100
#define valueK 1200
#define ctlP 0x4000
#define ctlN 0x2800
#define ctlB 0x1800
#define ctlR 0x0400
#define ctlQ 0x0200
#define ctlK 0x0100
#define ctlBQ 0x1200
#define ctlBN 0x0800
#define ctlRQ 0x0600
#define ctlNN 0x2000
#define Patak(c, u) (atak[c][u] > ctlP)
#define Anyatak(c, u) (atak[c][u] > 0)

#if ttblsz
#define truescore 0x0001
#define lowerbound 0x0002
#define upperbound 0x0004
#define kingcastle 0x0008
#define queencastle 0x0010

struct hashval
{
  unsigned long key,bd;
};
struct hashentry
{
  unsigned long hashbd;
  unsigned short mv;
  unsigned char flags, depth;   /* char saves some space */
  short score;
  
#ifdef HASHTEST
  unsigned char bd[32];
#endif /* HASHTEST */

};

#ifdef HASHFILE
/*
  persistent transposition table.
  The size must be a power of 2. If you change the size,
  be sure to run gnuchess -t before anything else.
*/
#define frehash 6

//#define filesz (1 << 11)
#define filesz (1 << 17)

struct fileentry
{
  unsigned char bd[32];
  unsigned char f, t, flags, depth, sh, sl;
};
/*
  In a networked enviroment gnuchess might be compiled on different
  hosts with different random number generators, that is not acceptable
  if they are going to share the same transposition table.
*/
unsigned long int next = 1;

unsigned int urand (void)
{
  next *= 1103515245;
  next += 12345;
  return ((unsigned int) (next >> 16) & 0xFFFF);
}

void srand (unsigned int seed)
{
  next = seed;
}
#else
#define urand rand
#endif /* HASHFILE */

static unsigned long hashkey, hashbd;
static struct hashval hashcode[2][7][64];
static struct hashentry huge ttable[2][ttblsz];
#endif /* ttblsz */
static short rpthash[2][256];

FILE *hashfile;
struct leaf Tree[2000], *root;
short TrPnt[maxdepth];
short PieceList[2][16], PawnCnt[2][8];
#define wking PieceList[white][0]
#define bking PieceList[black][0]
#define EnemyKing PieceList[c2][0]
short castld[2], Mvboard[64];
short svalue[64];
struct flags flag;
short opponent, computer, Awindow, Bwindow, dither, INCscore;
long ResponseTime, ExtraTime, Level, et, et0, time0, ft;
long NodeCnt, ETnodes, EvalNodes, HashCnt, FHashCnt, HashCol;
short player, xwndw, rehash;
struct GameRec GameList[512];
short Sdepth, GameCnt, Game50, MaxSearchDepth;
short epsquare, contempt;
struct BookEntry *Book;
struct TimeControlRec TimeControl;
short TCflag, TCmoves, TCminutes, OperatorTime;
const short otherside[3] =
{1, 0, 2};
unsigned short hint, PrVar[maxdepth];

static short Pindex[64];
static short PieceCnt[2];
static short c1, c2, *atk1, *atk2, *PC1, *PC2, atak[2][64];
static short mtl[2], pmtl[2], emtl[2], hung[2];
static short FROMsquare, TOsquare, Zscore, zwndw;
static short HasKnight[2], HasBishop[2], HasRook[2], HasQueen[2];
static short ChkFlag[maxdepth], CptrFlag[maxdepth], PawnThreat[maxdepth];
static short Pscore[maxdepth], Tscore[maxdepth];
static const short qrook[3] =
{0, 56, 0};
static const short krook[3] =
{7, 63, 0};
static const short kingP[3] =
{4, 60, 0};
static const short rank7[3] =
{6, 1, 0};
static const short sweep[8] =
{false, false, false, true, true, true, false, false};
static unsigned short killr0[maxdepth], killr1[maxdepth];
static unsigned short killr2[maxdepth], killr3[maxdepth];
static unsigned short PV, Swag0, Swag1, Swag2, Swag3, Swag4;
static unsigned char history[8192];

static short Mwpawn[64], Mbpawn[64], Mknight[2][64], Mbishop[2][64];
static short Mking[2][64], Kfield[2][64];
static const short value[7] =
{0, valueP, valueN, valueB, valueR, valueQ, valueK};
static const short control[7] =
{0, ctlP, ctlN, ctlB, ctlR, ctlQ, ctlK};
static const short PassedPawn0[8] =
{0, 60, 80, 120, 200, 360, 600, 800};
static const short PassedPawn1[8] =
{0, 30, 40, 60, 100, 180, 300, 800};
static const short PassedPawn2[8] =
{0, 15, 25, 35, 50, 90, 140, 800};
static const short PassedPawn3[8] =
{0, 5, 10, 15, 20, 30, 140, 800};
static const short ISOLANI[8] =
{-12, -16, -20, -24, -24, -20, -16, -12};
static const short BACKWARD[16] =
{-6, -10, -15, -21, -28, -28, -28, -28,
 -28, -28, -28, -28, -28, -28, -28, -28};
static const short BMBLTY[14] =
{-2, 0, 2, 4, 6, 8, 10, 12, 13, 14, 15, 16, 16, 16};
static const short RMBLTY[15] =
{0, 2, 4, 6, 8, 10, 11, 12, 13, 14, 14, 14, 14, 14, 14};
static const short KTHRT[36] =
{0, -8, -20, -36, -52, -68, -80, -80, -80, -80, -80, -80,
 -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80,
 -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80};
static short KNIGHTPOST, KNIGHTSTRONG, BISHOPSTRONG, KATAK;
static short PEDRNK2B, PWEAKH, PADVNCM, PADVNCI, PAWNSHIELD, PDOUBLED, PBLOK;
static short RHOPN, RHOPNX, KHOPN, KHOPNX, KSFTY;
static short ATAKD, HUNGP, HUNGX, KCASTLD, KMOVD, XRAY, PINVAL;
static short stage, stage2, Developed[2];
static short PawnBonus, BishopBonus, RookBonus;
static const short KingOpening[64] =
{0, 0, -4, -10, -10, -4, 0, 0,
 -4, -4, -8, -12, -12, -8, -4, -4,
 -12, -16, -20, -20, -20, -20, -16, -12,
 -16, -20, -24, -24, -24, -24, -20, -16,
 -16, -20, -24, -24, -24, -24, -20, -16,
 -12, -16, -20, -20, -20, -20, -16, -12,
 -4, -4, -8, -12, -12, -8, -4, -4,
 0, 0, -4, -10, -10, -4, 0, 0};
static const short KingEnding[64] =
{0, 6, 12, 18, 18, 12, 6, 0,
 6, 12, 18, 24, 24, 18, 12, 6,
 12, 18, 24, 30, 30, 24, 18, 12,
 18, 24, 30, 36, 36, 30, 24, 18,
 18, 24, 30, 36, 36, 30, 24, 18,
 12, 18, 24, 30, 30, 24, 18, 12,
 6, 12, 18, 24, 24, 18, 12, 6,
 0, 6, 12, 18, 18, 12, 6, 0};
static const short DyingKing[64] =
{0, 8, 16, 24, 24, 16, 8, 0,
 8, 32, 40, 48, 48, 40, 32, 8,
 16, 40, 56, 64, 64, 56, 40, 16,
 24, 48, 64, 72, 72, 64, 48, 24,
 24, 48, 64, 72, 72, 64, 48, 24,
 16, 40, 56, 64, 64, 56, 40, 16,
 8, 32, 40, 48, 48, 40, 32, 8,
 0, 8, 16, 24, 24, 16, 8, 0};
static const short KBNK[64] =
{99, 90, 80, 70, 60, 50, 40, 40,
 90, 80, 60, 50, 40, 30, 20, 40,
 80, 60, 40, 30, 20, 10, 30, 50,
 70, 50, 30, 10, 0, 20, 40, 60,
 60, 40, 20, 0, 10, 30, 50, 70,
 50, 30, 10, 20, 30, 40, 60, 80,
 40, 20, 30, 40, 50, 60, 80, 90,
 40, 40, 50, 60, 70, 80, 90, 99};
static const short pknight[64] =
{0, 4, 8, 10, 10, 8, 4, 0,
 4, 8, 16, 20, 20, 16, 8, 4,
 8, 16, 24, 28, 28, 24, 16, 8,
 10, 20, 28, 32, 32, 28, 20, 10,
 10, 20, 28, 32, 32, 28, 20, 10,
 8, 16, 24, 28, 28, 24, 16, 8,
 4, 8, 16, 20, 20, 16, 8, 4,
 0, 4, 8, 10, 10, 8, 4, 0};
static const short pbishop[64] =
{14, 14, 14, 14, 14, 14, 14, 14,
 14, 22, 18, 18, 18, 18, 22, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 22, 18, 18, 18, 18, 22, 14,
 14, 14, 14, 14, 14, 14, 14, 14};
static const short PawnAdvance[64] =
{0, 0, 0, 0, 0, 0, 0, 0,
 4, 4, 4, 0, 0, 4, 4, 4,
 6, 8, 2, 10, 10, 2, 8, 6,
 6, 8, 12, 16, 16, 12, 8, 6,
 8, 12, 16, 24, 24, 16, 12, 8,
 12, 16, 24, 32, 32, 24, 16, 12,
 12, 16, 24, 32, 32, 24, 16, 12,
 0, 0, 0, 0, 0, 0, 0, 0};


/* .... MOVE GENERATION VARIABLES AND INITIALIZATIONS .... */


#define taxicab(a,b) taxidata[a][b]
short distdata[64][64], taxidata[64][64];

static inline void
Initialize_dist (void)
{
  register short a, b, d, di;

  for (a = 0; a < 64; a++)
    for (b = 0; b < 64; b++)
      {
        d = abs (column (a) - column (b));
        di = abs (row (a) - row (b));
        taxidata[a][b] = d + di;
        distdata[a][b] = (d > di ? d : di);
      }
}

const short Stboard[64] =
{rook, knight, bishop, queen, king, bishop, knight, rook,
 pawn, pawn, pawn, pawn, pawn, pawn, pawn, pawn,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 pawn, pawn, pawn, pawn, pawn, pawn, pawn, pawn,
 rook, knight, bishop, queen, king, bishop, knight, rook};
const short Stcolor[64] =
{white, white, white, white, white, white, white, white,
 white, white, white, white, white, white, white, white,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 black, black, black, black, black, black, black, black,
 black, black, black, black, black, black, black, black};
short board[64], color[64];
static unsigned char nextpos[8][64][64];
static unsigned char nextdir[8][64][64];
/*
  ptype is used to separate white and black pawns, like this;
  ptyp = ptype[side][piece]
  piece can be used directly in nextpos/nextdir when generating moves
  for pieces that are not black pawns.
*/
static const short ptype[2][8] =
{
  no_piece, pawn, knight, bishop, rook, queen, king, no_piece,
  no_piece, bpawn, knight, bishop, rook, queen, king, no_piece};
static const short direc[8][8] =
{
  0, 0, 0, 0, 0, 0, 0, 0,
  10, 9, 11, 0, 0, 0, 0, 0,
  8, -8, 12, -12, 19, -19, 21, -21,
  9, 11, -9, -11, 0, 0, 0, 0,
  1, 10, -1, -10, 0, 0, 0, 0,
  1, 10, -1, -10, 9, 11, -9, -11,
  1, 10, -1, -10, 9, 11, -9, -11,
  -10, -9, -11, 0, 0, 0, 0, 0};
static const short max_steps[8] =
{0, 2, 1, 7, 7, 7, 1, 2};
static const short nunmap[120] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 0, 1, 2, 3, 4, 5, 6, 7, -1,
  -1, 8, 9, 10, 11, 12, 13, 14, 15, -1,
  -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
  -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
  -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
  -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
  -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


void Initialize_moves (void)

/*
  This procedure pre-calculates all moves for every piece from every square.
  This data is stored in nextpos/nextdir and used later in the move generation
  routines.
*/

{
  short ptyp, po, p0, d, di, s, delta;
  unsigned char *ppos, *pdir;
  short dest[8][8];
  short steps[8];
  short sorted[8];

  for (ptyp = 0; ptyp < 8; ptyp++)
    for (po = 0; po < 64; po++)
      for (p0 = 0; p0 < 64; p0++)
        {
          nextpos[ptyp][po][p0] = (unsigned char) po;
          nextdir[ptyp][po][p0] = (unsigned char) po;
        }
  for (ptyp = 1; ptyp < 8; ptyp++)
    for (po = 21; po < 99; po++)
      if (nunmap[po] >= 0)
        {
          ppos = nextpos[ptyp][nunmap[po]];
          pdir = nextdir[ptyp][nunmap[po]];
          /* dest is a function of direction and steps */
          for (d = 0; d < 8; d++)
            {
              dest[d][0] = nunmap[po];
              delta = direc[ptyp][d];
              if (delta != 0)
                {
                  p0 = po;
                  for (s = 0; s < max_steps[ptyp]; s++)
                    {
                      p0 = p0 + delta;
                      /*
                        break if (off board) or
                        (pawns only move two steps from home square)
                      */
                      if (nunmap[p0] < 0 || (ptyp == pawn || ptyp == bpawn)
                          && s > 0 && (d > 0 || Stboard[nunmap[po]] != pawn))
                        break;
                      else
                        dest[d][s] = nunmap[p0];
                    }
                }
              else
                s = 0;

              /*
                sort dest in number of steps order
                currently no sort is done due to compability with
                the move generation order in old gnu chess
              */
              steps[d] = s;
              for (di = d; s > 0 && di > 0; di--)
                if (steps[sorted[di - 1]] == 0) /* should be: < s */
                  sorted[di] = sorted[di - 1];
                else
                  break;
              sorted[di] = d;
            }

          /*
            update nextpos/nextdir,
            pawns have two threads (capture and no capture)
          */
          p0 = nunmap[po];
          if (ptyp == pawn || ptyp == bpawn)
            {
              for (s = 0; s < steps[0]; s++)
                {
                  ppos[p0] = (unsigned char) dest[0][s];
                  p0 = dest[0][s];
                }
              p0 = nunmap[po];
              for (d = 1; d < 3; d++)
                {
                  pdir[p0] = (unsigned char) dest[d][0];
                  p0 = dest[d][0];
                }
            }
          else
            {
              pdir[p0] = (unsigned char) dest[sorted[0]][0];
              for (d = 0; d < 8; d++)
                for (s = 0; s < steps[sorted[d]]; s++)
                  {
                    ppos[p0] = (unsigned char) dest[sorted[d]][s];
                    p0 = dest[sorted[d]][s];
                    if (d < 7)
                      pdir[p0] = (unsigned char) dest[sorted[d + 1]][0];
                    /* else is already initialized */
                  }
            }
        }
}

/* hmm.... shouldn`t main be moved to the interface routines */
//int main (int argc, char **argv)
int main_of_Gnu_Chess_v3 (int argc, char **argv)
{
  short int ahead = true, hash = true;
  char *xwin = 0;

  while (argc > 1 && ((argv[1][0] == '-') || (argv[1][0] == '+')))
    {
      switch (argv[1][1])
        {
        case 'a':
          ahead = (argv[1][0] == '-') ? false : true;
          break;
        case 'h':
          hash = (argv[1][0] == '-') ? false : true;
          break;
#if ttblsz
#ifdef HASHFILE
        case 't': /* create or test persistent transposition table */
          {
            if ((hashfile = fopen (HASHFILE, RWA_ACC)) == NULL)
              hashfile = fopen (HASHFILE , WA_ACC);
            if (hashfile != NULL)
              {
                long i, j;
                int nr[maxdepth];
                struct fileentry n;

                printf("Counting transposition file entries, wait!\n");
                for(i = 0; i < maxdepth; i++) nr[i] = 0;
                fseek(hashfile, 0L, SEEK_END);
                i = ftell(hashfile) / sizeof(struct fileentry);
                fseek(hashfile, 0L, SEEK_SET);
                for (j = 0; j < i; j++)
                  {
                    fread(&n, sizeof(struct fileentry), 1, hashfile);
                    if (n.depth)
                      {
                        nr[n.depth]++;
                        nr[0]++;
                      }
                  }
                printf("The file contains %d entries out of max %d\n",
                       nr[0], i);
                for(j = 1; j < maxdepth; j++) printf("%d ",nr[j]);
                printf("\n");
                if (i < filesz)
                  {
                    printf("Filling transposition file, wait!\n");
                    for(j = 0; j < 32; j++)
                      n.bd[j] = 0;
                    n.f = n.t = 0;
                    n.flags = 0;
                    n.depth = 0;
                    n.sh = n.sl = 0;
                    for (j = i; j < filesz; j++)
                      fwrite (&n, sizeof(struct fileentry), 1, hashfile);
                  }
                fclose(hashfile);
              }
            return(0);
            break;
          }
#endif /* HASHFILE */
#endif /* ttblsz */
        case 'x':
          xwin = &argv[1][2];
          break;
        default:
          fprintf (stderr, "Usage: gnuchess [-a] [-t] [-x xwndw]\n");
        }
      argv++;
      argc--;
    }
  Level = 0;
  TCflag = false;
  OperatorTime = 0;
  if (argc == 2)
    Level = atoi (argv[1]);
  if (argc == 3)
    {
      TCmoves = atoi (argv[1]);
      TCminutes = atoi (argv[2]);
      TCflag = true;
    }
  Initialize ();
  Initialize_dist ();
  Initialize_moves ();
  NewGame ();
  GetOpenings ();

  flag.easy = ahead;
  flag.hash = hash;
  if (xwin)
    xwndw = atoi (xwin);

  hashfile = NULL;
#if ttblsz
#ifdef HASHFILE
  hashfile = fopen (HASHFILE, RWA_ACC);
#endif /* HASHFILE */
#endif  /* ttblsz */
  while (!(flag.quit))
    {
      if (flag.bothsides && !flag.mate)
        SelectMove (opponent, 1);
      else
        InputCommand ();
      if (!(flag.quit || flag.mate || flag.force))
        SelectMove (computer, 1);
    }
#if ttblsz
#ifdef HASHFILE
  if (hashfile) fclose(hashfile);
#endif /* HASHFILE */
#endif /* ttblsz */

  ExitChess ();
  return (0);
}

void NewGame (void)

/*
  Reset the board and other variables to start a new game.
*/

{
  short l, c, p;

  stage = stage2 = -1;          /* the game is not yet started */
  flag.mate = flag.post = flag.quit = flag.reverse = flag.bothsides = false;
  flag.force = false;
  flag.hash = flag.easy = flag.beep = flag.rcptr = true;
  NodeCnt = et0 = epsquare = 0;
  dither = 0;
  Awindow = 90;
  Bwindow = 90;
  xwndw = 90;
  MaxSearchDepth = 29;
  contempt = 0;
  GameCnt = 0;
  Game50 = 1;
  hint = 0x0C14;
  ZeroRPT ();
  Developed[white] = Developed[black] = false;
  castld[white] = castld[black] = false;
  PawnThreat[0] = CptrFlag[0] = false;
  Pscore[0] = 12000;
  Tscore[0] = 12000;
  opponent = white;
  computer = black;
  for (l = 0; l < 2000; l++)
    Tree[l].f = Tree[l].t = 0;
#if ttblsz
  rehash = 6;
  ZeroTTable ();
  srand ((unsigned int) 1);
  for (c = white; c <= black; c++)
    for (p = pawn; p <= king; p++)
      for (l = 0; l < 64; l++)
        {
          hashcode[c][p][l].key = (((unsigned long) urand ()));
          hashcode[c][p][l].key += (((unsigned long) urand ()) << 16);
          hashcode[c][p][l].bd = (((unsigned long) urand ()));
          hashcode[c][p][l].bd += (((unsigned long) urand ()) << 16);
          /*
          if (sizeof(long) > 4)
            {
              hashcode[c][p][l].key += (((unsigned long) urand ()) << 32);
              hashcode[c][p][l].key += (((unsigned long) urand ()) << 48);
              hashcode[c][p][l].bd += (((unsigned long) urand ()) << 32);
              hashcode[c][p][l].bd += (((unsigned long) urand ()) << 48);
            }
          */  
        }
#endif /* ttblsz */
  for (l = 0; l < 64; l++)
    {
      board[l] = Stboard[l];
      color[l] = Stcolor[l];
      Mvboard[l] = 0;
    }
  ClrScreen ();
  if (TCflag)
    SetTimeControl ();
  else if (Level == 0)
    SelectLevel ();
  InitializeStats ();
  time0 = time (NULL);
  ElapsedTime (1);
  //let's disable output of chess engine
  //UpdateDisplay (0, 0, 1, 0);
}


/* ............    MOVE GENERATION & SEARCH ROUTINES    .............. */

static inline void pick (short int p1, short int p2)

/*
  Find the best move in the tree between indexes p1 and p2. Swap the best
  move into the p1 element.
*/

{
  register short p, s;
  short p0, s0;
  struct leaf temp;

  s0 = Tree[p1].score;
  p0 = p1;
  for (p = p1 + 1; p <= p2; p++)
    if ((s = Tree[p].score) > s0)
      {
        s0 = s;
        p0 = p;
      }
  if (p0 != p1)
    {
      temp = Tree[p1];
      Tree[p1] = Tree[p0];
      Tree[p0] = temp;
    }
}

void SelectMove (short int side, short int iop)

/*
  Select a move by calling function search() at progressively deeper ply
  until time is up or a mate or draw is reached. An alpha-beta window of -90
  to +90 points is set around the score returned from the previous
  iteration. If Sdepth != 0 then the program has correctly predicted the
  opponents move and the search will start at a depth of Sdepth+1 rather
  than a depth of 1.
*/

{
  static short i, tempb, tempc, tempsf, tempst, xside, rpt;
  static short alpha, beta, score;

  flag.timeout = false;
  xside = otherside[side];
  if (iop != 2)
    player = side;
  if (TCflag)
    {
      if ((TimeControl.moves[side] + 3) != 0)
        ResponseTime = (TimeControl.clock[side]) /
          (TimeControl.moves[side] + 3) -
          OperatorTime;
      else
        ResponseTime = 0;
      ResponseTime += (ResponseTime * TimeControl.moves[side]) / (2 * TCmoves + 1);
    }
  else
    ResponseTime = Level;
  if (iop == 2)
    ResponseTime = 99999;
  if (Sdepth > 0 && root->score > Zscore - zwndw)
    ResponseTime -= ft;
  else if (ResponseTime < 1)
    ResponseTime = 1;
  ExtraTime = 0;
  ExaminePosition ();
  ScorePosition (side, &score);
  /* Pscore[0] = -score; */
  ShowSidetomove ();

  if (Sdepth == 0)
    {
#if ttblsz
      /* ZeroTTable (); */
#endif /* ttblsz */
      SearchStartStuff (side);
#ifdef NOMEMSET
      for (i = 0; i < 8192; i++)
        history[i] = 0;
#else
      memset ((char *) history, 0, sizeof (history));
#endif /* NOMEMSET */
      FROMsquare = TOsquare = -1;
      PV = 0;
      if (iop != 2)
        hint = 0;
      for (i = 0; i < maxdepth; i++)
        PrVar[i] = killr0[i] = killr1[i] = killr2[i] = killr3[i] = 0;
      alpha = score - 90;
      beta = score + 90;
      rpt = 0;
      TrPnt[1] = 0;
      root = &Tree[0];
      MoveList (side, 1);
      for (i = TrPnt[1]; i < TrPnt[2]; i++)
        pick (i, TrPnt[2] - 1);
      if (Book != NULL)
        OpeningBook (&hint);
      if (Book != NULL)
        flag.timeout = true;
      NodeCnt = ETnodes = EvalNodes = HashCnt = FHashCnt = HashCol = 0;
      Zscore = 0;
      zwndw = 20;
    }
  while (!flag.timeout && Sdepth < MaxSearchDepth)
    {
      Sdepth++;
      ShowDepth (' ');
      score = search (side, 1, Sdepth, alpha, beta, PrVar, &rpt);
      for (i = 1; i <= Sdepth; i++)
        killr0[i] = PrVar[i];
      if (score < alpha)
        {
          ShowDepth ('-');
          ExtraTime = 10 * ResponseTime;
          /* ZeroTTable (); */
          score = search (side, 1, Sdepth, -9000, score, PrVar, &rpt);
        }
      if (score > beta && !(root->flags & exact))
        {
          ShowDepth ('+');
          ExtraTime = 0;
          /* ZeroTTable (); */
          score = search (side, 1, Sdepth, score, 9000, PrVar, &rpt);
        }
      score = root->score;
      if (!flag.timeout)
        for (i = TrPnt[1] + 1; i < TrPnt[2]; i++)
          pick (i, TrPnt[2] - 1);
      ShowResults (score, PrVar, '.');
      for (i = 1; i <= Sdepth; i++)
        killr0[i] = PrVar[i];
      if (score > Zscore - zwndw && score > Tree[1].score + 250)
        ExtraTime = 0;
      else if (score > Zscore - 3 * zwndw)
        ExtraTime = ResponseTime;
      else
        ExtraTime = 3 * ResponseTime;
      if (root->flags & exact)
        flag.timeout = true;
      if (Tree[1].score < -9000)
        flag.timeout = true;
      if (4 * et > 2 * ResponseTime + ExtraTime)
        flag.timeout = true;
      if (!flag.timeout)
        {
          Tscore[0] = score;
          if (Zscore == 0)
            Zscore = score;
          else
            Zscore = (Zscore + score) / 2;
        }
      zwndw = 20 + abs (Zscore / 12);
      beta = score + Bwindow;
      if (Zscore < score)
        alpha = Zscore - Awindow - zwndw;
      else
        alpha = score - Awindow - zwndw;
    }

  score = root->score;
  if (rpt >= 2 || score < -12000)
    root->flags |= draw;
  if (iop == 2)
    return;
  if (Book == NULL)
    hint = PrVar[2];
  ElapsedTime (1);

  if (score > -9999 && rpt <= 2)
    {
      MakeMove (side, root, &tempb, &tempc, &tempsf, &tempst, &INCscore);
      algbr (root->f, root->t, (short) root->flags);
    }
  else
    algbr (0, 0, 0);
  //let's disable output of chess engine  
  //OutputMove ();
  
  if (score == -9999 || score == 9998)
    flag.mate = true;
  if (flag.mate)
    hint = 0;
  if ((board[root->t] == pawn)
      || (root->flags & capture)
      || (root->flags & cstlmask))
    {
      Game50 = GameCnt;
      ZeroRPT ();
    }
  GameList[GameCnt].score = score;
  GameList[GameCnt].nodes = NodeCnt;
  GameList[GameCnt].time = (short) et;
  GameList[GameCnt].depth = Sdepth;
  if (TCflag)
    {
      TimeControl.clock[side] -= (et + OperatorTime);
      if (--TimeControl.moves[side] == 0)
        SetTimeControl ();
    }
  if ((root->flags & draw) && flag.bothsides)
    flag.mate = true;
  if (GameCnt > 470)
    flag.mate = true; /* out of move store, you loose */
  player = xside;
  Sdepth = 0;
  fflush (stdin);
}

int parse (FILE *fd, unsigned short int *mv, short int side)
{
  int c, i, r1, r2, c1, c2;
  char s[100];
  while ((c = getc (fd)) == ' ') ;
  i = 0;
  s[0] = (char) c;
  while (c != ' ' && c != '\n' && c != EOF)
    s[++i] = (char) (c = getc (fd));
  s[++i] = '\0';
  if (c == EOF)
    return (-1);
  if (s[0] == '!' || s[0] == ';' || i < 3)
    {
      while (c != '\n' && c != EOF)
        c = getc (fd);
      return (0);
    }
  if (s[4] == 'o')
    if (side == black)
      *mv = 0x3C3A;
    else
      *mv = 0x0402;
  else if (s[0] == 'o')
    if (side == black)
      *mv = 0x3C3E;
    else
      *mv = 0x0406;
  else
    {
      c1 = s[0] - 'a';
      r1 = s[1] - '1';
      c2 = s[2] - 'a';
      r2 = s[3] - '1';
      *mv = (locn (r1, c1) << 8) | locn (r2, c2);
    }
  return (1);
}

void GetOpenings (void)

/*
   Read in the Opening Book file and parse the algebraic notation for a move
   into an unsigned integer format indicating the from and to square. Create
   a linked list of opening lines of play, with entry->next pointing to the
   next line and entry->move pointing to a chunk of memory containing the
   moves. More Opening lines of up to 256 half moves may be added to
   gnuchess.book.
*/

{
  FILE *fd;
  int c, i, j, side;
  struct BookEntry *entry;
  unsigned short mv, *mp, tmp[100];

  fd = fopen ("Chess/gnu3ch.boo", "r");
  if (fd != NULL)
    {
      Book = NULL;
      i = 0;
      side = white;
      while ((c = parse (fd, &mv, side)) >= 0)
        if (c == 1)
          {
            tmp[++i] = mv;
            side = otherside[side];
          }
        else if (c == 0 && i > 0)
          {
            entry = (struct BookEntry *) malloc (sizeof (struct BookEntry));
            mp = (unsigned short *) malloc ((i + 1) * sizeof (unsigned short));
            if (!entry || !mp)
              {
                Book = NULL;
                ShowMessage ("warning: can\'t load book, out of memory.");
                return;
              }
            entry->mv = mp;
            entry->next = Book;
            Book = entry;
            for (j = 1; j <= i; j++)
              *(mp++) = tmp[j];
            *mp = 0;
            i = 0;
            side = white;
          }
      fclose (fd);
    }
  else
    ShowMessage ("warning: can\'t find book.");
}


void OpeningBook (unsigned short *hint)

/*
  Go thru each of the opening lines of play and check for a match with the
  current game listing. If a match occurs, generate a random number. If this
  number is the largest generated so far then the next move in this line
  becomes the current "candidate". After all lines are checked, the
  candidate move is put at the top of the Tree[] array and will be played by
  the program. Note that the program does not handle book transpositions.
*/

{
  short j, pnt;
  unsigned short m, *mp;
  unsigned r, r0;
  struct BookEntry *p;

  srand ((unsigned int) time (NULL));
  r0 = m = 0;
  p = Book;
  while (p != NULL)
    {
      mp = p->mv;
      for (j = 1; j <= GameCnt; j++)
        if (GameList[j].gmove != *(mp++))
          break;
      if (j > GameCnt)
        if ((r = urand ()) > r0)
          {
            r0 = r;
            m = *mp;
            *hint = *(++mp);
          }
      p = p->next;
    }
  for (pnt = TrPnt[1]; pnt < TrPnt[2]; pnt++)
    if (((Tree[pnt].f << 8) | Tree[pnt].t) == m)
      Tree[pnt].score = 0;
  pick (TrPnt[1], TrPnt[2] - 1);
  if (Tree[TrPnt[1]].score < 0)
    Book = NULL;
}


inline void repetition (short int *cnt)

/*
  Check for draw by threefold repetition.
*/

{
  register short i, c, f, t;
  short b[64];
  unsigned short m;

  *cnt = c = 0;
  if (GameCnt > Game50 + 3)
    {
#ifdef NOMEMSET
      for (i = 0; i < 64; b[i++] = 0) ;
#else
      memset ((char *) b, 0, sizeof (b));
#endif /* NOMEMSET */
      for (i = GameCnt; i > Game50; i--)
        {
          m = GameList[i].gmove;
          f = m >> 8;
          t = m & 0xFF;
          if (++b[f] == 0)
            c--;
          else
            c++;
          if (--b[t] == 0)
            c--;
          else
            c++;
          if (c == 0)
            (*cnt)++;
        }
    }
}

int search (short int side,
        short int ply,
        short int depth,
        short int alpha,
        short int beta,
        unsigned short int *bstline,
        short int *rpt)

/*
  Perform an alpha-beta search to determine the score for the current board
  position. If depth <= 0 only capturing moves, pawn promotions and
  responses to check are generated and searched, otherwise all moves are
  processed. The search depth is modified for check evasions, certain
  re-captures and threats. Extensions may continue for up to 11 ply beyond
  the nominal search depth.
*/

#define UpdateSearchStatus \
{\
   if (flag.post) ShowCurrentMove(pnt,node->f,node->t);\
     if (pnt > TrPnt[1])\
       {\
          d = best-Zscore; e = best-node->score;\
            if (best < alpha) ExtraTime = 10*ResponseTime;\
            else if (d > -zwndw && e > 4*zwndw) ExtraTime = -ResponseTime/3;\
            else if (d > -zwndw) ExtraTime = 0;\
            else if (d > -3*zwndw) ExtraTime = ResponseTime;\
            else if (d > -9*zwndw) ExtraTime = 3*ResponseTime;\
            else ExtraTime = 5*ResponseTime;\
            }\
            }
#define prune (cf && score+node->score < alpha)
#define ReCapture (flag.rcptr && score > alpha && score < beta &&\
                   ply > 2 && CptrFlag[ply-1] && CptrFlag[ply-2])
/* && depth == Sdepth-ply+1 */
#define Parry (hung[side] > 1 && ply == Sdepth+1)
#define MateThreat (ply < Sdepth+4 && ply > 4 &&\
                    ChkFlag[ply-2] && ChkFlag[ply-4] &&\
                    ChkFlag[ply-2] != ChkFlag[ply-4])

{
  register short j, pnt;
  short best, tempb, tempc, tempsf, tempst;
  short xside, pbst, d, e, cf, score, rcnt, slk, InChk;
  unsigned short mv, nxtline[maxdepth];
  struct leaf *node, tmp;

  NodeCnt++;
  xside = otherside[side];

  if ((ply <= Sdepth + 3) && rpthash[side][hashkey & 0xFF] > 0)
    repetition (rpt);
  else
    *rpt = 0;
  /* Detect repetitions a bit earlier. SMC. 12/89 */
  if (*rpt == 1 && ply > 1)
    return (0);
  /* if (*rpt >= 2) return(0); */

  score = evaluate (side, ply, alpha, beta, INCscore, &slk, &InChk);
  if (score > 9000)
    {
      bstline[ply] = 0;
      return (score);
    }
  if (depth > 0)
    {
      /* Allow opponent a chance to check again */
      if (InChk)
        depth = (depth < 2) ? 2 : depth;
      else if (PawnThreat[ply - 1] || ReCapture)
        ++depth;
    }
  else
    {
      if (score >= alpha &&
          (InChk || PawnThreat[ply - 1] || Parry))
        depth = 1;
      else if (score <= beta && MateThreat)
        depth = 1;
    }

#if ttblsz
  if (depth > 0 && flag.hash && ply > 1)
    {
      if (ProbeTTable (side, depth, &alpha, &beta, &score) == false)
#ifdef HASHFILE
        if (hashfile && (depth > 5) && (GameCnt < 12))
          ProbeFTable (side, depth, &alpha, &beta, &score);
#else
      /* do nothing */;
#endif /* HASHFILE */
      bstline[ply] = PV;
      bstline[ply + 1] = 0;
      if (beta == -20000)
        return (score);
      if (alpha > beta)
        return (alpha);
    }
#endif /* ttblsz */
  if (Sdepth == 1)
    d = 7;
  else
    d = 11;
  if (ply > Sdepth + d || (depth < 1 && score > beta))
    /* score >= beta ?? */
    return (score);

  if (ply > 1)
    if (depth > 0)
      MoveList (side, ply);
    else
      CaptureList (side, ply);

  if (TrPnt[ply] == TrPnt[ply + 1])
    return (score);

  cf = (depth < 1 && ply > Sdepth + 1 && !ChkFlag[ply - 2] && !slk);

  if (depth > 0)
    best = -12000;
  else
    best = score;
  if (best > alpha)
    alpha = best;

  for (pnt = pbst = TrPnt[ply];
       pnt < TrPnt[ply + 1] && best <= beta;    /* best < beta ?? */
       pnt++)
    {
      if (ply > 1)
        pick (pnt, TrPnt[ply + 1] - 1);
      node = &Tree[pnt];
      mv = (node->f << 8) | node->t;
      nxtline[ply + 1] = 0;

      if (prune)
        break;
      if (ply == 1)
        UpdateSearchStatus;

      if (!(node->flags & exact))
        {
          MakeMove (side, node, &tempb, &tempc, &tempsf, &tempst, &INCscore);
          CptrFlag[ply] = (node->flags & capture);
          PawnThreat[ply] = (node->flags & pwnthrt);
          Tscore[ply] = node->score;
          PV = node->reply;
          node->score = -search (xside, ply + 1,
                                 (depth > 0) ? depth - 1 : 0,
                                 -beta, -alpha,
                                 nxtline, &rcnt);
          if (abs (node->score) > 9000)
            node->flags |= exact;
          else if (rcnt == 1)
            node->score /= 2;
          if (rcnt >= 2 || GameCnt - Game50 > 99 ||
              (node->score == 9999 - ply && !ChkFlag[ply]))
            {
              node->flags |= draw;
              node->flags |= exact;
              if (side == computer)
                node->score = contempt;
              else
                node->score = -contempt;
            }
          node->reply = nxtline[ply + 1];
          UnmakeMove (side, node, &tempb, &tempc, &tempsf, &tempst);
        }
      if (node->score > best && !flag.timeout)
        {
          if (depth > 0)
            if (node->score > alpha && !(node->flags & exact))
              node->score += depth;
          best = node->score;
          pbst = pnt;
          if (best > alpha)
            alpha = best;
          for (j = ply + 1; nxtline[j] > 0; j++)
            bstline[j] = nxtline[j];
          bstline[j] = 0;
          bstline[ply] = mv;
          if (ply == 1)
            {
              if (best > root->score)
                {
                  tmp = Tree[pnt];
                  for (j = pnt - 1; j >= 0; j--)
                    Tree[j + 1] = Tree[j];
                  Tree[0] = tmp;
                  pbst = 0;
                }
              if (Sdepth > 2)
                if (best > beta)
                  ShowResults (best, bstline, '+');
                else if (best < alpha)
                  ShowResults (best, bstline, '-');
                else
                  ShowResults (best, bstline, '&');
            }
        }
      if (NodeCnt > ETnodes)
        ElapsedTime (0);
      if (flag.timeout)
        return (-Tscore[ply - 1]);
    }

  node = &Tree[pbst];
  mv = (node->f << 8) | node->t;
#if ttblsz
  if (flag.hash && ply <= Sdepth && *rpt == 0 && best == alpha)
    {
      PutInTTable (side, best, depth, alpha, beta, mv);
#ifdef HASHFILE
      if (hashfile && (depth > 5) && (GameCnt < 12))
        PutInFTable (side, best, depth, alpha, beta, node->f, node->t);
#endif /* HASHFILE */
    }
#endif /* ttblsz */
  if (depth > 0)
    {
      j = (node->f << 6) | node->t;
      if (side == black)
        j |= 0x1000;
      if (history[j] < 150)
        history[j] += (unsigned char) 2 * depth;
      if (node->t != (GameList[GameCnt].gmove & 0xFF))
        if (best <= beta)
          killr3[ply] = mv;
        else if (mv != killr1[ply])
          {
            killr2[ply] = killr1[ply];
            killr1[ply] = mv;
          }
      if (best > 9000)
        killr0[ply] = mv;
      else
        killr0[ply] = 0;
    }
  return (best);
}

#if ttblsz
/*
  hashbd contains a 32 bit "signature" of the board position. hashkey
  contains a 16 bit code used to address the hash table. When a move is
  made, XOR'ing the hashcode of moved piece on the from and to squares with
  the hashbd and hashkey values keeps things current.
*/
#define UpdateHashbd(side, piece, f, t) \
{\
  if ((f) >= 0)\
    {\
      hashbd ^= hashcode[side][piece][f].bd;\
      hashkey ^= hashcode[side][piece][f].key;\
    }\
  if ((t) >= 0)\
    {\
      hashbd ^= hashcode[side][piece][t].bd;\
      hashkey ^= hashcode[side][piece][t].key;\
    }\
}

#define CB(i) (unsigned char) ((color[2 * (i)] ? 0x80 : 0)\
               | (board[2 * (i)] << 4)\
               | (color[2 * (i) + 1] ? 0x8 : 0)\
               | (board[2 * (i) + 1]))

int ProbeTTable (short int side,
             short int depth,
             short int *alpha,
             short int *beta,
             short int *score)

/*
  Look for the current board position in the transposition table.
*/

{
  register struct hashentry *ptbl;
  register unsigned short i;

  ptbl = &ttable[side][hashkey & (ttblsz - 1)];

  /* rehash max rehash times */
  for (i = 1; ptbl->hashbd != hashbd && i <= rehash; i++)
    ptbl = &ttable[side][(hashkey + i) & (ttblsz - 1)];
  if ((short) ptbl->depth >= depth && ptbl->hashbd == hashbd)
    {
      HashCnt++;
#ifdef HASHTEST
      for (i = 0; i < 32; i++)
        {
          if (ptbl->bd[i] != CB(i))
            {
              HashCol++;
              ShowMessage("ttable collision detected");
              break;
            }
        }
#endif /* HASHTEST */

      PV = ptbl->mv;
      if (ptbl->flags & truescore)
        {
          *score = ptbl->score;
          *beta = -20000;
        }
#if 0 /* commented out, why? */
      else if (ptbl->flags & upperbound)
        {
          if (ptbl->score < *beta) *beta = ptbl->score+1;
        }
#endif
      else if (ptbl->flags & lowerbound)
        {
          if (ptbl->score > *alpha)
            *alpha = ptbl->score - 1;
        }
      return(true);
    }
  return(false);
}

void PutInTTable (short int side,
             short int score,
             short int depth,
             short int alpha,
             short int beta,
             unsigned short int mv)

/*
  Store the current board position in the transposition table.
*/

{
  register struct hashentry *ptbl;
  register unsigned short i;

  ptbl = &ttable[side][hashkey & (ttblsz - 1)];

  /* rehash max rehash times */
  for (i = 1; depth < ptbl->depth && ptbl->hashbd != hashbd && i <= rehash; i++)
    ptbl = &ttable[side][(hashkey + i) & (ttblsz - 1)];
  if (depth >= ptbl->depth || ptbl->hashbd != hashbd)
    {
      ptbl->hashbd = hashbd;
      ptbl->depth = (unsigned char) depth;
      ptbl->score = score;
      ptbl->mv = mv;
      ptbl->flags = 0;
      if (score < alpha)
        ptbl->flags |= upperbound;
      else if (score > beta)
        ptbl->flags |= lowerbound;
      else
        ptbl->flags |= truescore;
#ifdef HASHTEST
      for (i = 0; i < 32; i++)
        {
          ptbl->bd[i] = CB(i);
        }
#endif /* HASHTEST */
    }
}

void ZeroTTable (void)
{
  register int side, i;

  if (flag.hash)
    for (side = white; side <= black; side++)
      for (i = 0; i < ttblsz; i++)
        ttable[side][i].depth = 0;
}

#ifdef HASHFILE
int ProbeFTable(short int side,
            short int depth,
            short int *alpha,
            short int *beta,
            short int *score)

/*
  Look for the current board position in the persistent transposition table.
*/

{
  register unsigned short i, j;
  register unsigned long hashix;
  short s;
  struct fileentry new, t;

  if (side == white)
    hashix = hashkey & 0xFFFFFFFE & (filesz - 1);
  else
    hashix = hashkey | 1 & (filesz - 1);

  for (i = 0; i < 32; i++)
    new.bd[i] = CB(i);
  new.flags = 0;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[qrook[side]] == 0))
    new.flags |= queencastle;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[krook[side]] == 0))
    new.flags |= kingcastle;

  for (i = 0; i < frehash; i++)
    {
      fseek(hashfile,
            sizeof(struct fileentry) * ((hashix + 2 * i) & (filesz - 1)),
            SEEK_SET);
      fread(&t, sizeof(struct fileentry), 1, hashfile);
      for (j = 0; j < 32; j++)
        if (t.bd[j] != new.bd[j])
          break;
      if (((short) t.depth >= depth) && (j >= 32)
          && (new.flags == (t.flags & (kingcastle | queencastle))))
        {
          FHashCnt++;
          PV = (t.f << 8) | t.t;
          s = (t.sh << 8) | t.sl;
          if (t.flags & truescore)
            {
              *score = s;
              *beta = -20000;
            }
          else if (t.flags & lowerbound)
            {
              if (s > *alpha)
                *alpha = s - 1;
            }
          return(true);
        }
    }
  return(false);
}

void PutInFTable (short int side,
             short int score,
             short int depth,
             short int alpha,
             short int beta,
             short unsigned int f,
             short unsigned int t)

/*
  Store the current board position in the persistent transposition table.
*/

{
  register unsigned short i;
  register unsigned long hashix;
  struct fileentry new, tmp;

  if (side == white)
    hashix = hashkey & 0xFFFFFFFE & (filesz - 1);
  else
    hashix = hashkey | 1 & (filesz - 1);

  for (i = 0; i < 32; i++)
    new.bd[i] = CB(i);
  new.f = (unsigned char) f;
  new.t = (unsigned char) t;
  new.flags = 0;
  if (score < alpha)
    new.flags |= upperbound;
  else if (score > beta)
    new.flags |= lowerbound;
  else
    new.flags |= truescore;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[qrook[side]] == 0))
    new.flags |= queencastle;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[krook[side]] == 0))
    new.flags |= kingcastle;
  new.depth = (unsigned char) depth;
  new.sh = (unsigned char) (score >> 8);
  new.sl = (unsigned char) (score & 0xFF);

  for (i = 0; i < frehash; i++)
    {
      fseek(hashfile,
            sizeof(struct fileentry) * ((hashix + 2 * i) & (filesz - 1)),
            SEEK_SET);
      fread(&tmp, sizeof(struct fileentry), 1, hashfile);
      if ((short) tmp.depth <= depth)
        {
          fseek(hashfile,
                sizeof(struct fileentry) * ((hashix + 2 * i) & (filesz - 1)),
                SEEK_SET);
          fwrite (&new, sizeof(struct fileentry), 1, hashfile);
          break;
        }
    }
}
#endif /* HASHFILE */
#endif /* ttblsz */

void ZeroRPT (void)
{
  register int side, i;

  for (side = white; side <= black; side++)
    for (i = 0; i < 256; i++)
      rpthash[side][i] = 0;
}

#define Link(from,to,flag,s) \
{\
   node->f = from; node->t = to;\
     node->reply = 0;\
       node->flags = flag;\
         node->score = s;\
           ++node;\
             ++TrPnt[ply+1];\
             }

static inline void LinkMove (short int ply,
          short int f,
          short int t,
          short int flag,
          short int xside)

/*
  Add a move to the tree.  Assign a bonus to order the moves
  as follows:
  1. Principle variation
  2. Capture of last moved piece
  3. Other captures (major pieces first)
  4. Killer moves
  5. "history" killers
*/

{
  register short s, z;
  register unsigned short mv;
  register struct leaf *node;

  node = &Tree[TrPnt[ply + 1]];
  mv = (f << 8) | t;
  s = 0;
  if (mv == Swag0)
    s = 2000;
  else if (mv == Swag1)
    s = 60;
  else if (mv == Swag2)
    s = 50;
  else if (mv == Swag3)
    s = 40;
  else if (mv == Swag4)
    s = 30;
  z = (f << 6) | t;
  if (xside == white)
    z |= 0x1000;
  s += history[z];
  if (color[t] != neutral)
    {
      if (t == TOsquare)
        s += 500;
      s += value[board[t]] - board[f];
    }
  if (board[f] == pawn)
    if (row (t) == 0 || row (t) == 7)
      {
        flag |= promote;
        s += 800;
        Link (f, t, flag | queen, s - 20000);
        s -= 200;
        Link (f, t, flag | knight, s - 20000);
        s -= 50;
        Link (f, t, flag | rook, s - 20000);
        flag |= bishop;
        s -= 50;
      }
    else if (row (t) == 1 || row (t) == 6)
      {
        flag |= pwnthrt;
        s += 600;
      }
  Link (f, t, flag, s - 20000);
}


static inline void GenMoves (short int ply, short int sq, short int side, short int xside)

/*
  Generate moves for a piece. The moves are taken from the precalulated
  array nextpos/nextdir. If the board is free, next move is choosen from
  nextpos else from nextdir.
*/

{
  register short u, piece;
  register unsigned char *ppos, *pdir;

  piece = board[sq];
  ppos = nextpos[ptype[side][piece]][sq];
  pdir = nextdir[ptype[side][piece]][sq];
  if (piece == pawn)
    {
      u = ppos[sq];     /* follow no captures thread */
      if (color[u] == neutral)
        {
          LinkMove (ply, sq, u, 0, xside);
          u = ppos[u];
          if (color[u] == neutral)
            LinkMove (ply, sq, u, 0, xside);
        }
      u = pdir[sq];     /* follow captures thread */
      if (color[u] == xside)
        LinkMove (ply, sq, u, capture, xside);
      else
        if (u == epsquare)
          LinkMove (ply, sq, u, capture | epmask, xside);
      u = pdir[u];
      if (color[u] == xside)
        LinkMove (ply, sq, u, capture, xside);
      else
        if (u == epsquare)
          LinkMove (ply, sq, u, capture | epmask, xside);
    }
  else
    {
      u = ppos[sq];
      do
        {
          if (color[u] == neutral)
            {
              LinkMove (ply, sq, u, 0, xside);
              u = ppos[u];
            }
          else
            {
              if (color[u] == xside)
                LinkMove (ply, sq, u, capture, xside);
              u = pdir[u];
            }
      } while (u != sq);
    }
}

void MoveList (short int side, short int ply)

/*
  Fill the array Tree[] with all available moves for side to play. Array
  TrPnt[ply] contains the index into Tree[] of the first move at a ply.
*/

{
  register short i, xside, f;

  xside = otherside[side];
  TrPnt[ply + 1] = TrPnt[ply];
  if (PV == 0)
    Swag0 = killr0[ply];
  else
    Swag0 = PV;
  Swag1 = killr1[ply];
  Swag2 = killr2[ply];
  Swag3 = killr3[ply];
  if (ply > 2)
    Swag4 = killr1[ply - 2];
  else
    Swag4 = 0;
  for (i = PieceCnt[side]; i >= 0; i--)
    GenMoves (ply, PieceList[side][i], side, xside);
  if (!castld[side])
    {
      f = PieceList[side][0];
      if (castle (side, f, f + 2, 0))
        {
          LinkMove (ply, f, f + 2, cstlmask, xside);
        }
      if (castle (side, f, f - 2, 0))
        {
          LinkMove (ply, f, f - 2, cstlmask, xside);
        }
    }
}

void CaptureList (short int side, short int ply)

/*
  Fill the array Tree[] with all available cature and promote moves for
  side to play. Array TrPnt[ply] contains the index into Tree[]
  of the first move at a ply.
*/

{
  register short u, sq, xside;
  register struct leaf *node;
  register unsigned char *ppos, *pdir;
  short i, piece, *PL, r7;

  xside = otherside[side];
  TrPnt[ply + 1] = TrPnt[ply];
  node = &Tree[TrPnt[ply]];
  r7 = rank7[side];
  PL = PieceList[side];
  for (i = 0; i <= PieceCnt[side]; i++)
    {
      sq = PL[i];
      piece = board[sq];
      if (sweep[piece])
        {
          ppos = nextpos[piece][sq];
          pdir = nextdir[piece][sq];
          u = ppos[sq];
          do
            {
              if (color[u] == neutral)
                u = ppos[u];
              else
                {
                  if (color[u] == xside)
                    Link (sq, u, capture,
                          value[board[u]] + svalue[board[u]] - piece);
                  u = pdir[u];
                }
          } while (u != sq);
        }
      else
        {
          pdir = nextdir[ptype[side][piece]][sq];
          if (piece == pawn && row (sq) == r7)
            {
              u = pdir[sq];
              if (color[u] == xside)
                Link (sq, u, capture | promote | queen, valueQ);
              u = pdir[u];
              if (color[u] == xside)
                Link (sq, u, capture | promote | queen, valueQ);
              ppos = nextpos[ptype[side][piece]][sq];
              u = ppos[sq]; /* also generate non capture promote */
              if (color[u] == neutral)
                Link (sq, u, promote | queen, valueQ);
            }
          else
            {
              u = pdir[sq];
              do
                {
                  if (color[u] == xside)
                    Link (sq, u, capture,
                          value[board[u]] + svalue[board[u]] - piece);
                  u = pdir[u];
              } while (u != sq);
            }
        }
    }
}


int castle (short int side, short int kf, short int kt, short int iop)

/* Make or Unmake a castling move. */

{
  register short rf, rt, t0, xside;

  xside = otherside[side];
  if (kt > kf)
    {
      rf = kf + 3;
      rt = kt - 1;
    }
  else
    {
      rf = kf - 4;
      rt = kt + 1;
    }
  if (iop == 0)
    {
      if (kf != kingP[side] ||
          board[kf] != king ||
          board[rf] != rook ||
          Mvboard[kf] != 0 ||
          Mvboard[rf] != 0 ||
          color[kt] != neutral ||
          color[rt] != neutral ||
          color[kt - 1] != neutral ||
          SqAtakd (kf, xside) ||
          SqAtakd (kt, xside) ||
          SqAtakd (rt, xside))
        return (false);
    }
  else
    {
      if (iop == 1)
        {
          castld[side] = true;
          Mvboard[kf]++;
          Mvboard[rf]++;
        }
      else
        {
          castld[side] = false;
          Mvboard[kf]--;
          Mvboard[rf]--;
          t0 = kt;
          kt = kf;
          kf = t0;
          t0 = rt;
          rt = rf;
          rf = t0;
        }
      board[kt] = king;
      color[kt] = side;
      Pindex[kt] = 0;
      board[kf] = no_piece;
      color[kf] = neutral;
      board[rt] = rook;
      color[rt] = side;
      Pindex[rt] = Pindex[rf];
      board[rf] = no_piece;
      color[rf] = neutral;
      PieceList[side][Pindex[kt]] = kt;
      PieceList[side][Pindex[rt]] = rt;
#if ttblsz
      UpdateHashbd (side, king, kf, kt);
      UpdateHashbd (side, rook, rf, rt);
#endif /* ttblsz */
    }
  return (true);
}


static inline void EnPassant (short int xside, short int f, short int t, short int iop)

/*
  Make or unmake an en passant move.
*/

{
  register short l;

  if (t > f)
    l = t - 8;
  else
    l = t + 8;
  if (iop == 1)
    {
      board[l] = no_piece;
      color[l] = neutral;
    }
  else
    {
      board[l] = pawn;
      color[l] = xside;
    }
  InitializeStats ();
}


static inline void UpdatePieceList (short int side, short int sq, short int iop)

/*
  Update the PieceList and Pindex arrays when a piece is captured or when a
  capture is unmade.
*/

{
  register short i;
  if (iop == 1)
    {
      PieceCnt[side]--;
      for (i = Pindex[sq]; i <= PieceCnt[side]; i++)
        {
          PieceList[side][i] = PieceList[side][i + 1];
          Pindex[PieceList[side][i]] = i;
        }
    }
  else
    {
      PieceCnt[side]++;
      PieceList[side][PieceCnt[side]] = sq;
      Pindex[sq] = PieceCnt[side];
    }
}

void MakeMove (short int side,
          struct leaf * node,
          short int *tempb,
          short int *tempc,
          short int *tempsf,
          short int *tempst,
          short int *INCscore)

/*
  Update Arrays board[], color[], and Pindex[] to reflect the new board
  position obtained after making the move pointed to by node. Also update
  miscellaneous stuff that changes when a move is made.
*/

{
  register short f, t, xside, ct, cf;

  xside = otherside[side];
  GameCnt++;
  f = node->f;
  t = node->t;
  epsquare = -1;
  FROMsquare = f;
  TOsquare = t;
  *INCscore = 0;
  GameList[GameCnt].gmove = (f << 8) | t;
  if (node->flags & cstlmask)
    {
      GameList[GameCnt].piece = no_piece;
      GameList[GameCnt].color = side;
      (void) castle (side, f, t, 1);
    }
  else
    {
      if (!(node->flags & capture) && (board[f] != pawn))
        rpthash[side][hashkey & 0xFF]++;
      *tempc = color[t];
      *tempb = board[t];
      *tempsf = svalue[f];
      *tempst = svalue[t];
      GameList[GameCnt].piece = *tempb;
      GameList[GameCnt].color = *tempc;
      if (*tempc != neutral)
        {
          UpdatePieceList (*tempc, t, 1);
          if (*tempb == pawn)
            --PawnCnt[*tempc][column (t)];
          if (board[f] == pawn)
            {
              --PawnCnt[side][column (f)];
              ++PawnCnt[side][column (t)];
              cf = column (f);
              ct = column (t);
              if (PawnCnt[side][ct] > 1 + PawnCnt[side][cf])
                *INCscore -= 15;
              else if (PawnCnt[side][ct] < 1 + PawnCnt[side][cf])
                *INCscore += 15;
              else if (ct == 0 || ct == 7 || PawnCnt[side][ct + ct - cf] == 0)
                *INCscore -= 15;
            }
          mtl[xside] -= value[*tempb];
          if (*tempb == pawn)
            pmtl[xside] -= valueP;
#if ttblsz
          UpdateHashbd (xside, *tempb, -1, t);
#endif /* ttblsz */
          *INCscore += *tempst;
          Mvboard[t]++;
        }
      color[t] = color[f];
      board[t] = board[f];
      svalue[t] = svalue[f];
      Pindex[t] = Pindex[f];
      PieceList[side][Pindex[t]] = t;
      color[f] = neutral;
      board[f] = no_piece;
      if (board[t] == pawn)
        if (t - f == 16)
          epsquare = f + 8;
        else if (f - t == 16)
          epsquare = f - 8;
      if (node->flags & promote)
        {
          board[t] = node->flags & pmask;
          if (board[t] == queen)
            HasQueen[side]++;
          else if (board[t] == rook)
            HasRook[side]++;
          else if (board[t] == bishop)
            HasBishop[side]++;
          else if (board[t] == knight)
            HasKnight[side]++;
          --PawnCnt[side][column (t)];
          mtl[side] += value[board[t]] - valueP;
          pmtl[side] -= valueP;
#if ttblsz
          UpdateHashbd (side, pawn, f, -1);
          UpdateHashbd (side, board[t], f, -1);
#endif /* ttblsz */
          *INCscore -= *tempsf;
        }
      if (node->flags & epmask)
        EnPassant (xside, f, t, 1);
      else
#if ttblsz
        UpdateHashbd (side, board[t], f, t);
#else
        /* NOOP */;
#endif /* ttblsz */
      Mvboard[f]++;
    }
}

void UnmakeMove (short int side,
            struct leaf * node,
            short int *tempb,
            short int *tempc,
            short int *tempsf,
            short int *tempst)

/*
  Take back a move.
*/

{
  register short f, t, xside;

  xside = otherside[side];
  f = node->f;
  t = node->t;
  epsquare = -1;
  GameCnt--;
  if (node->flags & cstlmask)
    (void) castle (side, f, t, 2);
  else
    {
      color[f] = color[t];
      board[f] = board[t];
      svalue[f] = *tempsf;
      Pindex[f] = Pindex[t];
      PieceList[side][Pindex[f]] = f;
      color[t] = *tempc;
      board[t] = *tempb;
      svalue[t] = *tempst;
      if (node->flags & promote)
        {
          board[f] = pawn;
          ++PawnCnt[side][column (t)];
          mtl[side] += valueP - value[node->flags & pmask];
          pmtl[side] += valueP;
#if ttblsz
          UpdateHashbd (side, (short) node->flags & pmask, -1, t);
          UpdateHashbd (side, pawn, -1, t);
#endif /* ttblsz */
        }
      if (*tempc != neutral)
        {
          UpdatePieceList (*tempc, t, 2);
          if (*tempb == pawn)
            ++PawnCnt[*tempc][column (t)];
          if (board[f] == pawn)
            {
              --PawnCnt[side][column (t)];
              ++PawnCnt[side][column (f)];
            }
          mtl[xside] += value[*tempb];
          if (*tempb == pawn)
            pmtl[xside] += valueP;
#if ttblsz
          UpdateHashbd (xside, *tempb, -1, t);
#endif /* ttblsz */
          Mvboard[t]--;
        }
      if (node->flags & epmask)
        EnPassant (xside, f, t, 2);
      else
#if ttblsz
        UpdateHashbd (side, board[f], f, t);
#else
      /* NOOP */;
#endif /* ttblsz */
      Mvboard[f]--;
      if (!(node->flags & capture) && (board[f] != pawn))
        rpthash[side][hashkey & 0xFF]--;
    }
}


void InitializeStats (void)

/*
  Scan thru the board seeing what's on each square. If a piece is found,
  update the variables PieceCnt, PawnCnt, Pindex and PieceList. Also
  determine the material for each side and set the hashkey and hashbd
  variables to represent the current board position. Array
  PieceList[side][indx] contains the location of all the pieces of either
  side. Array Pindex[sq] contains the indx into PieceList for a given
  square.
*/

{
  register short i, sq;
  epsquare = -1;
  for (i = 0; i < 8; i++)
    PawnCnt[white][i] = PawnCnt[black][i] = 0;
  mtl[white] = mtl[black] = pmtl[white] = pmtl[black] = 0;
  PieceCnt[white] = PieceCnt[black] = 0;
#if ttblsz
  hashbd = hashkey = 0;
#endif /* ttblsz */
  for (sq = 0; sq < 64; sq++)
    if (color[sq] != neutral)
      {
        mtl[color[sq]] += value[board[sq]];
        if (board[sq] == pawn)
          {
            pmtl[color[sq]] += valueP;
            ++PawnCnt[color[sq]][column (sq)];
          }
        if (board[sq] == king)
          Pindex[sq] = 0;
        else
          Pindex[sq] = ++PieceCnt[color[sq]];
        PieceList[color[sq]][Pindex[sq]] = sq;
#if ttblsz
        hashbd ^= hashcode[color[sq]][board[sq]][sq].bd;
        hashkey ^= hashcode[color[sq]][board[sq]][sq].key;
#endif /* ttblsz */
      }
}


int SqAtakd (short int sq, short int side)

/*
  See if any piece with color 'side' ataks sq.  First check pawns then Queen,
  Bishop, Rook and King and last Knight.
*/

{
  register short u;
  register unsigned char *ppos, *pdir;
  short xside;

  xside = otherside[side];
  pdir = nextdir[ptype[xside][pawn]][sq];
  u = pdir[sq];         /* follow captures thread */
  if (u != sq)
    {
      if (board[u] == pawn && color[u] == side)
        return (true);
      u = pdir[u];
      if (u != sq && board[u] == pawn && color[u] == side)
        return (true);
    }
  /* king capture */
  if (distance (sq, PieceList[side][0]) == 1)
    return (true);
  /* try a queen bishop capture */
  ppos = nextpos[bishop][sq];
  pdir = nextdir[bishop][sq];
  u = ppos[sq];
  do
    {
      if (color[u] == neutral)
        u = ppos[u];
      else
        {
          if (color[u] == side &&
              (board[u] == queen || board[u] == bishop))
            return (true);
          u = pdir[u];
        }
  } while (u != sq);
  /* try a queen rook capture */
  ppos = nextpos[rook][sq];
  pdir = nextdir[rook][sq];
  u = ppos[sq];
  do
    {
      if (color[u] == neutral)
        u = ppos[u];
      else
        {
          if (color[u] == side &&
              (board[u] == queen || board[u] == rook))
            return (true);
          u = pdir[u];
        }
  } while (u != sq);
  /* try a knight capture */
  pdir = nextdir[knight][sq];
  u = pdir[sq];
  do
    {
      if (color[u] == side && board[u] == knight)
        return (true);
      u = pdir[u];
  } while (u != sq);
  return (false);
}

static inline void ataks (short int side, short int *a)

/*
  Fill array atak[][] with info about ataks to a square.  Bits 8-15 are set
  if the piece (king..pawn) ataks the square.  Bits 0-7 contain a count of
  total ataks to the square.
*/

{
  register short u, c, sq;
  register unsigned char *ppos, *pdir;
  short i, piece, *PL;

#ifdef NOMEMSET
  for (u = 64; u; a[--u] = 0) ;
#else
  memset ((char *) a, 0, 64 * sizeof (a[0]));
#endif /* NOMEMSET */
  PL = PieceList[side];
  for (i = PieceCnt[side]; i >= 0; i--)
    {
      sq = PL[i];
      piece = board[sq];
      c = control[piece];
      if (sweep[piece])
        {
          ppos = nextpos[piece][sq];
          pdir = nextdir[piece][sq];
          u = ppos[sq];
          do
            {
              a[u] = ++a[u] | c;
              u = (color[u] == neutral) ? ppos[u] : pdir[u];
          } while (u != sq);
        }
      else
        {
          pdir = nextdir[ptype[side][piece]][sq];
          u = pdir[sq]; /* follow captures thread for pawns */
          do
            {
              a[u] = ++a[u] | c;
              u = pdir[u];
          } while (u != sq);
        }
    }
}


/* ............    POSITIONAL EVALUATION ROUTINES    ............ */

int evaluate (short int side,
          short int ply,
          short int alpha,
          short int beta,
          short int INCscore,
          short int *slk,
          short int *InChk)

/*
  Compute an estimate of the score by adding the positional score from the
  previous ply to the material difference. If this score falls inside a
  window which is 180 points wider than the alpha-beta window (or within a
  50 point window during quiescence search) call ScorePosition() to
  determine a score, otherwise return the estimated score. If one side has
  only a king and the other either has no pawns or no pieces then the
  function ScoreLoneKing() is called.
*/

{
  register short evflag, xside;
  short s;

  xside = otherside[side];
  s = -Pscore[ply - 1] + mtl[side] - mtl[xside] - INCscore;
  hung[white] = hung[black] = 0;
  *slk = ((mtl[white] == valueK && (pmtl[black] == 0 || emtl[black] == 0)) ||
         (mtl[black] == valueK && (pmtl[white] == 0 || emtl[white] == 0)));

  if (*slk)
    evflag = false;
  else
    evflag =
      (ply == 1 || ply < Sdepth ||
       ((ply == Sdepth + 1 || ply == Sdepth + 2) &&
        (s > alpha - xwndw && s < beta + xwndw)) ||
       (ply > Sdepth + 2 && s >= alpha - 25 && s <= beta + 25));

  if (evflag)
    {
      EvalNodes++;
      ataks (side, atak[side]);
      if (Anyatak (side, PieceList[xside][0]))
        return (10001 - ply);
      ataks (xside, atak[xside]);
      *InChk = Anyatak (xside, PieceList[side][0]);
      ScorePosition (side, &s);
    }
  else
    {
      if (SqAtakd (PieceList[xside][0], side))
        return (10001 - ply);
      *InChk = SqAtakd (PieceList[side][0], xside);
      if (*slk)
        ScoreLoneKing (side, &s);
    }

  Pscore[ply] = s - mtl[side] + mtl[xside];
  if (*InChk)
    ChkFlag[ply - 1] = Pindex[TOsquare];
  else
    ChkFlag[ply - 1] = 0;
  return (s);
}


static inline int ScoreKPK (short int side,
          short int winner,
          short int loser,
          short int king1,
          short int king2,
          short int sq)

/*
  Score King and Pawns versus King endings.
*/

{
  register short s, r;

  if (PieceCnt[winner] == 1)
    s = 50;
  else
    s = 120;
  if (winner == white)
    {
      if (side == loser)
        r = row (sq) - 1;
      else
        r = row (sq);
      if (row (king2) >= r && distance (sq, king2) < 8 - r)
        s += 10 * row (sq);
      else
        s = 500 + 50 * row (sq);
      if (row (sq) < 6)
        sq += 16;
      else
        if (row(sq) == 6)
          sq += 8;
    }
  else
    {
      if (side == loser)
        r = row (sq) + 1;
      else
        r = row (sq);
      if (row (king2) <= r && distance (sq, king2) < r + 1)
        s += 10 * (7 - row (sq));
      else
        s = 500 + 50 * (7 - row (sq));
      if (row (sq) > 1)
        sq -= 16;
      else
        if (row(sq) == 1)
          sq -= 8;
    }
  s += 8 * (taxicab (king2, sq) - taxicab (king1, sq));
  return (s);
}


static inline int ScoreKBNK (short int winner, short int king1, short int king2)


/*
  Score King+Bishop+Knight versus King endings.
  This doesn't work all that well but it's better than nothing.
*/

{
  register short s, sq, KBNKsq = 0;

  for (sq = 0; sq < 64; sq++)
    if (board[sq] == bishop)
      if (row (sq) % 2 == column (sq) % 2)
        KBNKsq = 0;
      else
        KBNKsq = 7;

  s = emtl[winner] - 300;
  if (KBNKsq == 0)
    s += KBNK[king2];
  else
    s += KBNK[locn (row (king2), 7 - column (king2))];
  s -= taxicab (king1, king2);
  s -= distance (PieceList[winner][1], king2);
  s -= distance (PieceList[winner][2], king2);
  return (s);
}


void ScoreLoneKing (short int side, short int *score)

/*
  Static evaluation when loser has only a king and winner has no pawns or no
  pieces.
*/

{
  register short winner, loser, king1, king2, s, i;

  UpdateWeights ();
  if (mtl[white] > mtl[black])
    winner = white;
  else
    winner = black;
  loser = otherside[winner];
  king1 = PieceList[winner][0];
  king2 = PieceList[loser][0];

  s = 0;

  if (pmtl[winner] > 0)
    for (i = 1; i <= PieceCnt[winner]; i++)
      s += ScoreKPK (side, winner, loser, king1, king2, PieceList[winner][i]);

  else if (emtl[winner] == valueB + valueN)
    s = ScoreKBNK (winner, king1, king2);

  else if (emtl[winner] > valueB)
    s = 500 + emtl[winner] - DyingKing[king2] - 2 * distance (king1, king2);

  if (side == winner)
    *score = s;
  else
    *score = -s;
}


static inline void BRscan (short int sq, short int *s, short int *mob)

/*
  Find Bishop and Rook mobility, XRAY attacks, and pins. Increment the
  hung[] array if a pin is found.
*/
{
  register short u, piece, pin;
  register unsigned char *ppos, *pdir;
  short *Kf;

  Kf = Kfield[c1];
  *mob = 0;
  piece = board[sq];
  ppos = nextpos[piece][sq];
  pdir = nextdir[piece][sq];
  u = ppos[sq];
  pin = -1;                     /* start new direction */
  do
    {
      *s += Kf[u];
      if (color[u] == neutral)
        {
          (*mob)++;
          if (ppos[u] == pdir[u])
            pin = -1;           /* oops new direction */
          u = ppos[u];
        }
      else if (pin < 0)
        {
          if (board[u] == pawn || board[u] == king)
            u = pdir[u];
          else
            {
              if (ppos[u] != pdir[u])
                pin = u;        /* not on the edge and on to find a pin */
              u = ppos[u];
            }
        }
      else
        {
          if (color[u] == c2 && (board[u] > piece || atk2[u] == 0))
            {
              if (color[pin] == c2)
                {
                  *s += PINVAL;
                  if (atk2[pin] == 0 ||
                      atk1[pin] > control[board[pin]] + 1)
                    ++hung[c2];
                }
              else
                *s += XRAY;
            }
          pin = -1;             /* new direction */
          u = pdir[u];
        }
  } while (u != sq);
}


static inline void KingScan (short int sq, short int *s)

/*
  Assign penalties if king can be threatened by checks, if squares
  near the king are controlled by the enemy (especially the queen),
  or if there are no pawns near the king.
  The following must be true:
  board[sq] == king
  c1 == color[sq]
  c2 == otherside[c1]
*/

#define ScoreThreat \
if (color[u] != c2)\
  if (atk1[u] == 0 || (atk2[u] & 0xFF) > 1) ++cnt;\
  else *s -= 3

{
  register short u;
  register unsigned char *ppos, *pdir;
  register short cnt, ok;

  cnt = 0;
  if (HasBishop[c2] || HasQueen[c2])
    {
      ppos = nextpos[bishop][sq];
      pdir = nextdir[bishop][sq];
      u = ppos[sq];
      do
        {
          if (atk2[u] & ctlBQ)
            ScoreThreat;
          u = (color[u] == neutral) ? ppos[u] : pdir[u];
      } while (u != sq);
    }
  if (HasRook[c2] || HasQueen[c2])
    {
      ppos = nextpos[rook][sq];
      pdir = nextdir[rook][sq];
      u = ppos[sq];
      do
        {
          if (atk2[u] & ctlRQ)
            ScoreThreat;
          u = (color[u] == neutral) ? ppos[u] : pdir[u];
      } while (u != sq);
    }
  if (HasKnight[c2])
    {
      pdir = nextdir[knight][sq];
      u = pdir[sq];
      do
        {
          if (atk2[u] & ctlNN)
            ScoreThreat;
          u = pdir[u];
      } while (u != sq);
    }
  *s += (KSFTY * KTHRT[cnt]) / 16;

  cnt = 0;
  ok = false;
  pdir = nextpos[king][sq];
  u = pdir[sq];
  do
    {
      if (board[u] == pawn)
        ok = true;
      if (atk2[u] > atk1[u])
        {
          ++cnt;
          if (atk2[u] & ctlQ)
            if (atk2[u] > ctlQ + 1 && atk1[u] < ctlQ)
              *s -= 4 * KSFTY;
        }
      u = pdir[u];
  } while (u != sq);
  if (!ok)
    *s -= KSFTY;
  if (cnt > 1)
    *s -= KSFTY;
}


static inline int trapped (short int sq)

/*
  See if the attacked piece has unattacked squares to move to.
  The following must be true:
  c1 == color[sq]
  c2 == otherside[c1]
*/

{
  register short u, piece;
  register unsigned char *ppos, *pdir;

  piece = board[sq];
  ppos = nextpos[ptype[c1][piece]][sq];
  pdir = nextdir[ptype[c1][piece]][sq];
  if (piece == pawn)
    {
      u = ppos[sq];     /* follow no captures thread */
      if (color[u] == neutral)
        {
          if (atk1[u] >= atk2[u])
            return (false);
          if (atk2[u] < ctlP)
            {
              u = ppos[u];
              if (color[u] == neutral && atk1[u] >= atk2[u])
                return (false);
            }
        }
      u = pdir[sq];     /* follow captures thread */
      if (color[u] == c2)
        return (false);
      u = pdir[u];
      if (color[u] == c2)
        return (false);
    }
  else
    {
      u = ppos[sq];
      do
        {
          if (color[u] != c1)
            if (atk2[u] == 0 || board[u] >= piece)
              return (false);
          u = (color[u] == neutral) ? ppos[u] : pdir[u];
      } while (u != sq);
    }
  return (true);
}


static inline int PawnValue (short int sq, short int side)

/*
  Calculate the positional value for a pawn on 'sq'.
*/

{
  register short j, fyle, rank;
  register short s, a1, a2, in_square, r, e;

  a1 = (atk1[sq] & 0x4FFF);
  a2 = (atk2[sq] & 0x4FFF);
  rank = row (sq);
  fyle = column (sq);
  s = 0;
  if (c1 == white)
    {
      s = Mwpawn[sq];
      if ((sq == 11 && color[19] != neutral)
          || (sq == 12 && color[20] != neutral))
        s += PEDRNK2B;
      if ((fyle == 0 || PC1[fyle - 1] == 0)
          && (fyle == 7 || PC1[fyle + 1] == 0))
        s += ISOLANI[fyle];
      else if (PC1[fyle] > 1)
        s += PDOUBLED;
      if (a1 < ctlP && atk1[sq + 8] < ctlP)
        {
          s += BACKWARD[a2 & 0xFF];
          if (PC2[fyle] == 0)
            s += PWEAKH;
          if (color[sq + 8] != neutral)
            s += PBLOK;
        }
      if (PC2[fyle] == 0)
        {
          if (side == black)
            r = rank - 1;
          else
            r = rank;
          in_square = (row (bking) >= r && distance (sq, bking) < 8 - r);
          if (a2 == 0 || side == white)
            e = 0;
          else
            e = 1;
          for (j = sq + 8; j < 64; j += 8)
            if (atk2[j] >= ctlP)
              {
                e = 2;
                break;
              }
            else if (atk2[j] > 0 || color[j] != neutral)
              e = 1;
          if (e == 2)
            s += (stage * PassedPawn3[rank]) / 10;
          else if (in_square || e == 1)
            s += (stage * PassedPawn2[rank]) / 10;
          else if (emtl[black] > 0)
            s += (stage * PassedPawn1[rank]) / 10;
          else
            s += PassedPawn0[rank];
        }
    }
  else if (c1 == black)
    {
      s = Mbpawn[sq];
      if ((sq == 51 && color[43] != neutral)
          || (sq == 52 && color[44] != neutral))
        s += PEDRNK2B;
      if ((fyle == 0 || PC1[fyle - 1] == 0) &&
          (fyle == 7 || PC1[fyle + 1] == 0))
        s += ISOLANI[fyle];
      else if (PC1[fyle] > 1)
        s += PDOUBLED;
      if (a1 < ctlP && atk1[sq - 8] < ctlP)
        {
          s += BACKWARD[a2 & 0xFF];
          if (PC2[fyle] == 0)
            s += PWEAKH;
          if (color[sq - 8] != neutral)
            s += PBLOK;
        }
      if (PC2[fyle] == 0)
        {
          if (side == white)
            r = rank + 1;
          else
            r = rank;
          in_square = (row (wking) <= r && distance (sq, wking) < r + 1);
          if (a2 == 0 || side == black)
            e = 0;
          else
            e = 1;
          for (j = sq - 8; j >= 0; j -= 8)
            if (atk2[j] >= ctlP)
              {
                e = 2;
                break;
              }
            else if (atk2[j] > 0 || color[j] != neutral)
              e = 1;
          if (e == 2)
            s += (stage * PassedPawn3[7 - rank]) / 10;
          else if (in_square || e == 1)
            s += (stage * PassedPawn2[7 - rank]) / 10;
          else if (emtl[white] > 0)
            s += (stage * PassedPawn1[7 - rank]) / 10;
          else
            s += PassedPawn0[7 - rank];
        }
    }
  if (a2 > 0)
    {
      if (a1 == 0 || a2 > ctlP + 1)
        {
          s += HUNGP;
          ++hung[c1];
          if (trapped (sq))
            ++hung[c1];
        }
      else
        if (a2 > a1)
          s += ATAKD;
    }
  return (s);
}


static inline int KnightValue (short int sq, short int side)

/*
  Calculate the positional value for a knight on 'sq'.
*/

{
  register short s, a2, a1;

  s = Mknight[c1][sq];
  a2 = (atk2[sq] & 0x4FFF);
  if (a2 > 0)
    {
      a1 = (atk1[sq] & 0x4FFF);
      if (a1 == 0 || a2 > ctlBN + 1)
        {
          s += HUNGP;
          ++hung[c1];
          if (trapped (sq))
            ++hung[c1];
        }
      else
        if (a2 >= ctlBN || a1 < ctlP)
          s += ATAKD;
    }
  return (s);
}


static inline int BishopValue (short int sq, short int side)

/*
  Calculate the positional value for a bishop on 'sq'.
*/

{
  register short a2, a1;
  short s, mob;

  s = Mbishop[c1][sq];
  BRscan (sq, &s, &mob);
  s += BMBLTY[mob];
  a2 = (atk2[sq] & 0x4FFF);
  if (a2 > 0)
    {
      a1 = (atk1[sq] & 0x4FFF);
      if (a1 == 0 || a2 > ctlBN + 1)
        {
          s += HUNGP;
          ++hung[c1];
          if (trapped (sq))
            ++hung[c1];
        }
      else
        if (a2 >= ctlBN || a1 < ctlP)
          s += ATAKD;
    }
  return (s);
}


static inline int RookValue (short int sq, short int side)

/*
  Calculate the positional value for a rook on 'sq'.
*/

{
  register short fyle, a2, a1;
  short s, mob;

  s = RookBonus;
  BRscan (sq, &s, &mob);
  s += RMBLTY[mob];
  fyle = column (sq);
  if (PC1[fyle] == 0)
    s += RHOPN;
  if (PC2[fyle] == 0)
    s += RHOPNX;
  if (pmtl[c2] > 100 && row (sq) == rank7[c1])
    s += 10;
  if (stage > 2)
    s += 14 - taxicab (sq, EnemyKing);
  a2 = (atk2[sq] & 0x4FFF);
  if (a2 > 0)
    {
      a1 = (atk1[sq] & 0x4FFF);
      if (a1 == 0 || a2 > ctlR + 1)
        {
          s += HUNGP;
          ++hung[c1];

          if (trapped (sq))
            ++hung[c1];
        }
      else
        if (a2 >= ctlR || a1 < ctlP)
          s += ATAKD;
    }
  return (s);
}


static inline int QueenValue (short int sq, short int side)

/*
  Calculate the positional value for a queen on 'sq'.
*/

{
  register short s, a2, a1;

  s = (distance (sq, EnemyKing) < 3) ? 12 : 0;
  if (stage > 2)
    s += 14 - taxicab (sq, EnemyKing);
  a2 = (atk2[sq] & 0x4FFF);
  if (a2 > 0)
    {
      a1 = (atk1[sq] & 0x4FFF);
      if (a1 == 0 || a2 > ctlQ + 1)
        {
          s += HUNGP;
          ++hung[c1];
          if (trapped (sq))
            ++hung[c1];
        }
      else
        if (a2 >= ctlQ || a1 < ctlP)
          s += ATAKD;
    }
  return (s);
}


static inline int KingValue (short int sq, short int side)

/*
  Calculate the positional value for a king on 'sq'.
*/

{
  register short fyle, a2, a1;
  short s;

  s = Mking[c1][sq];
  if (KSFTY > 0)
    if (Developed[c2] || stage > 0)
      KingScan (sq, &s);
  if (castld[c1])
    s += KCASTLD;
  else if (Mvboard[kingP[c1]])
    s += KMOVD;

  fyle = column (sq);
  if (PC1[fyle] == 0)
    s += KHOPN;
  if (PC2[fyle] == 0)
    s += KHOPNX;
  switch (fyle)
    {
    case 5:
      if (PC1[7] == 0)
        s += KHOPN;
      if (PC2[7] == 0)
        s += KHOPNX;
      /* Fall through */
    case 4:
    case 6:
    case 0:
      if (PC1[fyle + 1] == 0)
        s += KHOPN;
      if (PC2[fyle + 1] == 0)
        s += KHOPNX;
      break;
    case 2:
      if (PC1[0] == 0)
        s += KHOPN;
      if (PC2[0] == 0)
        s += KHOPNX;
      /* Fall through */
    case 3:
    case 1:
    case 7:
      if (PC1[fyle - 1] == 0)
        s += KHOPN;
      if (PC2[fyle - 1] == 0)
        s += KHOPNX;
      break;
    default:
      /* Impossible! */
      break;
    }

  a2 = (atk2[sq] & 0x4FFF);
  if (a2 > 0)
    {
      a1 = (atk1[sq] & 0x4FFF);
      if (a1 == 0 || a2 > ctlK + 1)
        {
          s += HUNGP;
          ++hung[c1];
        }
      else
        s += ATAKD;
    }
  return (s);
}


void ScorePosition (short int side, short int *score)

/*
  Perform normal static evaluation of board position. A score is generated
  for each piece and these are summed to get a score for each side.
*/

{
  register short sq, s, i, xside;
  short pscore[2];

  UpdateWeights ();
  xside = otherside[side];
  pscore[white] = pscore[black] = 0;

  for (c1 = white; c1 <= black; c1++)
    {
      c2 = otherside[c1];
      atk1 = atak[c1];
      atk2 = atak[c2];
      PC1 = PawnCnt[c1];
      PC2 = PawnCnt[c2];
      for (i = PieceCnt[c1]; i >= 0; i--)
        {
          sq = PieceList[c1][i];
          switch (board[sq])
            {
            case pawn:
              s = PawnValue(sq, side);
              break;
            case knight:
              s = KnightValue(sq, side);
              break;
            case bishop:
              s = BishopValue(sq, side);
              break;
            case rook:
              s = RookValue(sq, side);
              break;
            case queen:
              s = QueenValue(sq, side);
              break;
            case king:
              s = KingValue(sq, side);
              break;
            default:
              s = 0;
              break;
            }
          pscore[c1] += s;
          svalue[sq] = s;
        }
    }
  if (hung[side] > 1)
    pscore[side] += HUNGX;
  if (hung[xside] > 1)
    pscore[xside] += HUNGX;

  *score = mtl[side] - mtl[xside] + pscore[side] - pscore[xside] + 10;
  if (dither)
    *score += urand () % dither;

  if (*score > 0 && pmtl[side] == 0)
    if (emtl[side] < valueR)
      *score = 0;
    else if (*score < valueR)
      *score /= 2;
  if (*score < 0 && pmtl[xside] == 0)
    if (emtl[xside] < valueR)
      *score = 0;
    else if (-*score < valueR)
      *score /= 2;

  if (mtl[xside] == valueK && emtl[side] > valueB)
    *score += 200;
  if (mtl[side] == valueK && emtl[xside] > valueB)
    *score -= 200;
}


static inline void BlendBoard (const short int a[64], const short int b[64], short int c[64])
{
  register int sq;

  for (sq = 0; sq < 64; sq++)
    c[sq] = (a[sq] * (10 - stage) + b[sq] * stage) / 10;
}


static inline void CopyBoard (const short int a[64], short int b[64])
{
  register int sq;

  for (sq = 0; sq < 64; sq++)
    b[sq] = a[sq];
}

void ExaminePosition (void)

/*
  This is done one time before the search is started. Set up arrays
  Mwpawn, Mbpawn, Mknight, Mbishop, Mking which are used in the
  SqValue() function to determine the positional value of each piece.
*/

{
  register short i, sq;
  short wpadv, bpadv, wstrong, bstrong, z, side, pp, j, k, val, Pd, fyle, rank;
  static short PawnStorm = false;

  ataks (white, atak[white]);
  ataks (black, atak[black]);
  UpdateWeights ();
  HasKnight[white] = HasKnight[black] = 0;
  HasBishop[white] = HasBishop[black] = 0;
  HasRook[white] = HasRook[black] = 0;
  HasQueen[white] = HasQueen[black] = 0;
  for (side = white; side <= black; side++)
    for (i = PieceCnt[side]; i >= 0; i--)
      switch (board[PieceList[side][i]])
        {
        case knight:
          ++HasKnight[side];
          break;
        case bishop:
          ++HasBishop[side];
          break;
        case rook:
          ++HasRook[side];
          break;
        case queen:
          ++HasQueen[side];
          break;
        }
  if (!Developed[white])
    Developed[white] = (board[1] != knight && board[2] != bishop &&
                        board[5] != bishop && board[6] != knight);
  if (!Developed[black])
    Developed[black] = (board[57] != knight && board[58] != bishop &&
                        board[61] != bishop && board[62] != knight);
  if (!PawnStorm && stage < 5)
    PawnStorm = ((column (wking) < 3 && column (bking) > 4) ||
                 (column (wking) > 4 && column (bking) < 3));

  CopyBoard (pknight, Mknight[white]);
  CopyBoard (pknight, Mknight[black]);
  CopyBoard (pbishop, Mbishop[white]);
  CopyBoard (pbishop, Mbishop[black]);
  BlendBoard (KingOpening, KingEnding, Mking[white]);
  BlendBoard (KingOpening, KingEnding, Mking[black]);

  for (sq = 0; sq < 64; sq++)
    {
      fyle = column (sq);
      rank = row (sq);
      wstrong = bstrong = true;
      for (i = sq; i < 64; i += 8)
        if (Patak (black, i))
          {
            wstrong = false;
            break;
          }
      for (i = sq; i >= 0; i -= 8)
        if (Patak (white, i))
          {
            bstrong = false;
            break;
          }
      wpadv = bpadv = PADVNCM;
      if ((fyle == 0 || PawnCnt[white][fyle - 1] == 0) &&
          (fyle == 7 || PawnCnt[white][fyle + 1] == 0))
        wpadv = PADVNCI;
      if ((fyle == 0 || PawnCnt[black][fyle - 1] == 0) &&
          (fyle == 7 || PawnCnt[black][fyle + 1] == 0))
        bpadv = PADVNCI;
      Mwpawn[sq] = (wpadv * PawnAdvance[sq]) / 10;
      Mbpawn[sq] = (bpadv * PawnAdvance[63 - sq]) / 10;
      Mwpawn[sq] += PawnBonus;
      Mbpawn[sq] += PawnBonus;
      if (Mvboard[kingP[white]])
        {
          if ((fyle < 3 || fyle > 4) && distance (sq, wking) < 3)
            Mwpawn[sq] += PAWNSHIELD;
        }
      else if (rank < 3 && (fyle < 2 || fyle > 5))
        Mwpawn[sq] += PAWNSHIELD / 2;
      if (Mvboard[kingP[black]])
        {
          if ((fyle < 3 || fyle > 4) && distance (sq, bking) < 3)
            Mbpawn[sq] += PAWNSHIELD;
        }
      else if (rank > 4 && (fyle < 2 || fyle > 5))
        Mbpawn[sq] += PAWNSHIELD / 2;
      if (PawnStorm)
        {
          if ((column (wking) < 4 && fyle > 4) ||
              (column (wking) > 3 && fyle < 3))
            Mwpawn[sq] += 3 * rank - 21;
          if ((column (bking) < 4 && fyle > 4) ||
              (column (bking) > 3 && fyle < 3))
            Mbpawn[sq] -= 3 * rank;
        }
      Mknight[white][sq] += 5 - distance (sq, bking);
      Mknight[white][sq] += 5 - distance (sq, wking);
      Mknight[black][sq] += 5 - distance (sq, wking);
      Mknight[black][sq] += 5 - distance (sq, bking);
      Mbishop[white][sq] += BishopBonus;
      Mbishop[black][sq] += BishopBonus;
      for (i = PieceCnt[black]; i >= 0; i--)
        if (distance (sq, PieceList[black][i]) < 3)
          Mknight[white][sq] += KNIGHTPOST;
      for (i = PieceCnt[white]; i >= 0; i--)
        if (distance (sq, PieceList[white][i]) < 3)
          Mknight[black][sq] += KNIGHTPOST;
      if (wstrong)
        Mknight[white][sq] += KNIGHTSTRONG;
      if (bstrong)
        Mknight[black][sq] += KNIGHTSTRONG;
      if (wstrong)
        Mbishop[white][sq] += BISHOPSTRONG;
      if (bstrong)
        Mbishop[black][sq] += BISHOPSTRONG;

      if (HasBishop[white] == 2)
        Mbishop[white][sq] += 8;
      if (HasBishop[black] == 2)
        Mbishop[black][sq] += 8;
      if (HasKnight[white] == 2)
        Mknight[white][sq] += 5;
      if (HasKnight[black] == 2)
        Mknight[black][sq] += 5;

      Kfield[white][sq] = Kfield[black][sq] = 0;
      if (distance (sq, wking) == 1)
        Kfield[black][sq] = KATAK;
      if (distance (sq, bking) == 1)
        Kfield[white][sq] = KATAK;

      Pd = 0;
      for (k = 0; k <= PieceCnt[white]; k++)
        {
          i = PieceList[white][k];
          if (board[i] == pawn)
            {
              pp = true;
              if (row (i) == 6)
                z = i + 8;
              else
                z = i + 16;
              for (j = i + 8; j < 64; j += 8)
                if (Patak (black, j) || board[j] == pawn)
                  {
                    pp = false;
                    break;
                  }
              if (pp)
                Pd += 5 * taxicab (sq, z);
              else
                Pd += taxicab (sq, z);
            }
        }
      for (k = 0; k <= PieceCnt[black]; k++)
        {
          i = PieceList[black][k];
          if (board[i] == pawn)
            {
              pp = true;
              if (row (i) == 1)
                z = i - 8;
              else
                z = i - 16;
              for (j = i - 8; j >= 0; j -= 8)
                if (Patak (white, j) || board[j] == pawn)
                  {
                    pp = false;
                    break;
                  }
              if (pp)
                Pd += 5 * taxicab (sq, z);
              else
                Pd += taxicab (sq, z);
            }
        }
      if (Pd != 0)
        {
          val = (Pd * stage2) / 10;
          Mking[white][sq] -= val;
          Mking[black][sq] -= val;
        }
    }
}

void UpdateWeights (void)

/*
  If material balance has changed, determine the values for the positional
  evaluation terms.
*/

{
  register short tmtl, s1;

  emtl[white] = mtl[white] - pmtl[white] - valueK;
  emtl[black] = mtl[black] - pmtl[black] - valueK;
  tmtl = emtl[white] + emtl[black];
  s1 = (tmtl > 6600) ? 0 : ((tmtl < 1400) ? 10 : (6600 - tmtl) / 520);
  if (s1 != stage)
    {
      stage = s1;
      stage2 = (tmtl > 3600) ? 0 : ((tmtl < 1400) ? 10 : (3600 - tmtl) / 220);
      PEDRNK2B = -15;   /* centre pawn on 2nd rank & blocked */
      PBLOK = -4;               /* blocked backward pawn */
      PDOUBLED = -14;   /* doubled pawn */
      PWEAKH = -4;              /* weak pawn on half open file */
      PAWNSHIELD = 10 - stage;  /* pawn near friendly king */
      PADVNCM = 10;             /* advanced pawn multiplier */
      PADVNCI = 7;              /* muliplier for isolated pawn */
      PawnBonus = stage;

      KNIGHTPOST = (stage + 2) / 3;     /* knight near enemy pieces */
      KNIGHTSTRONG = (stage + 6) / 2;   /* occupies pawn hole */

      BISHOPSTRONG = (stage + 6) / 2;   /* occupies pawn hole */
      BishopBonus = 2 * stage;

      RHOPN = 10;               /* rook on half open file */
      RHOPNX = 4;
      RookBonus = 6 * stage;

      XRAY = 8;         /* Xray attack on piece */
      PINVAL = 10;              /* Pin */

      KHOPN = (3 * stage - 30) / 2;     /* king on half open file */
      KHOPNX = KHOPN / 2;
      KCASTLD = 10 - stage;
      KMOVD = -40 / (stage + 1);        /* king moved before castling */
      KATAK = (10 - stage) / 2; /* B,R attacks near enemy king */
      if (stage < 8)
        KSFTY = 16 - 2 * stage;
      else
        KSFTY = 0;

      ATAKD = -6;               /* defender > attacker */
      HUNGP = -8;               /* each hung piece */
      HUNGX = -12;              /* extra for >1 hung piece */
    }
}


/*
  nondsp.c - NON-DISPLAY interface for Chess
*/


#define pxx " PNBRQK"
#define qxx " pnbrqk"
#define rxx "12345678"
#define cxx "abcdefgh"
#define printz printf
#define scanz scanf

static char mvstr[4][6];
static char* ColorStr[2] = {"White", "Black"};
static long evrate;
static int mycnt1, mycnt2;
static int ahead;

void Initialize (void)
{
  mycnt1 = mycnt2 = 0;
}

void ExitChess (void)
{
  ListGame ();
  exit (0);
}

void TerminateSearch (int sig)
{
  sig++;                                /* shut up the compiler */
  flag.timeout = true;
  flag.bothsides = false;
}

void algbr (short int f, short int t, short int flag)


/*
   Generate move strings in different formats.
*/

{
  int m3p;

  if (f != t)
    {
      /* algebraic notation */
      mvstr[0][0] = cxx[column (f)];
      mvstr[0][1] = rxx[row (f)];
      mvstr[0][2] = cxx[column (t)];
      mvstr[0][3] = rxx[row (t)];
      mvstr[0][4] = mvstr[3][0] = '\0';
      if ((mvstr[1][0] = pxx[board[f]]) == 'P')
        {
          if (mvstr[0][0] == mvstr[0][2])       /* pawn did not eat */
            {
              mvstr[2][0] = mvstr[1][0] = mvstr[0][2];  /* to column */
              mvstr[2][1] = mvstr[1][1] = mvstr[0][3];  /* to row */
              m3p = 2;
            }
          else
            /* pawn ate */
            {
              mvstr[2][0] = mvstr[1][0] = mvstr[0][0];  /* from column */
              mvstr[2][1] = mvstr[1][1] = mvstr[0][2];  /* to column */
              mvstr[2][2] = mvstr[0][3];
              m3p = 3;          /* to row */
            }
          mvstr[2][m3p] = mvstr[1][2] = '\0';
          if (flag & promote)
            {
              mvstr[0][4] = mvstr[1][2] = mvstr[2][m3p] = qxx[flag & pmask];
              mvstr[1][3] = mvstr[2][m3p + 1] = mvstr[0][5] = '\0';
            }
        }
      else
        /* not a pawn */
        {
          mvstr[2][0] = mvstr[1][0];
          mvstr[2][1] = mvstr[0][1];
          mvstr[2][2] = mvstr[1][1] = mvstr[0][2];      /* to column */
          mvstr[2][3] = mvstr[1][2] = mvstr[0][3];      /* to row */
          mvstr[2][4] = mvstr[1][3] = '\0';
          strcpy (mvstr[3], mvstr[2]);
          mvstr[3][1] = mvstr[0][0];
          if (flag & cstlmask)
            {
              if (t > f)
                {
                  strcpy (mvstr[1], "o-o");
                  strcpy (mvstr[2], "O-O");
                }
              else
                {
                  strcpy (mvstr[1], "o-o-o");
                  strcpy (mvstr[2], "O-O-O");
                }
            }
        }
    }
  else
    mvstr[0][0] = mvstr[1][0] = mvstr[2][0] = mvstr[3][0] = '\0';
}


int VerifyMove (char *s, short int iop, unsigned short int *mv)

/*
   Compare the string 's' to the list of legal moves available for the
   opponent. If a match is found, make the move on the board.
*/

{
  static short pnt, tempb, tempc, tempsf, tempst, cnt;
  static struct leaf xnode;
  struct leaf *node;

  *mv = 0;
  if (iop == 2)
    {
      UnmakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst);
      return (false);
    }
  cnt = 0;
  MoveList (opponent, 2);
  pnt = TrPnt[2];
  while (pnt < TrPnt[3])
    {
      node = &Tree[pnt++];
      algbr (node->f, node->t, (short) node->flags);
      if (strcmp (s, mvstr[0]) == 0 || strcmp (s, mvstr[1]) == 0 ||
          strcmp (s, mvstr[2]) == 0 || strcmp (s, mvstr[3]) == 0)
        {
          cnt++;
          xnode = *node;
        }
    }
  if (cnt == 1)
    {
      MakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst, &INCscore);
      if (SqAtakd (PieceList[opponent][0], computer))
        {
          UnmakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst);
          //let's disable output of chess engine
          //printz ("Illegal move\n");
          return (false);
        }
      else
        {
          if (iop == 1)
            return (true);
          UpdateDisplay (xnode.f, xnode.t, 0, (short) xnode.flags);
          if ((board[xnode.t] == pawn)
              || (xnode.flags & capture)
              || (xnode.flags & cstlmask))
            {
              Game50 = GameCnt;
              ZeroRPT ();
            }
          GameList[GameCnt].depth = GameList[GameCnt].score = 0;
          GameList[GameCnt].nodes = 0;
          ElapsedTime (1);
          GameList[GameCnt].time = (short) et;
          TimeControl.clock[opponent] -= et;
          --TimeControl.moves[opponent];
          *mv = (xnode.f << 8) | xnode.t;
          algbr (xnode.f, xnode.t, false);
          return (true);
        }
    }

  if (cnt > 1)
    ShowMessage ("Ambiguous Move!");

  return (false);
}

void help (void)
{
  ClrScreen ();
  printz ("CHESS command summary\n");
  printz ("----------------------------------------------------------------\n");
  printz ("g1f3      move from g1 to f3      quit      Exit Chess\n");
  printz ("Nf3       move knight to f3       beep      on/off\n");
  printz ("o-o       castle king side        easy      on/off\n");
  printz ("o-o-o     castle queen side       hash      on/off\n");
  printz ("bd        redraw board            reverse   board display\n");
  printz ("list      game to chess.lst       book      on/off\n");
  printz ("undo      undo last ply           remove    take back a move\n");
  printz ("edit      edit board              force     enter game moves\n");
  printz ("switch    sides with computer     both      computer match\n");
  printz ("white     computer plays white    black     computer plays black\n");
  printz ("depth     set search depth        clock     set time control\n");
  printz ("post      principle variation     hint      suggest a move\n");
  printz ("save      game to file            get       game from file\n");
  printz ("random    randomize play          new       start new game\n");
  printz ("----------------------------------------------------------------\n");
  printz ("Computer: %-12s Opponent:            %s\n",
          ColorStr[computer], ColorStr[opponent]);
  printz ("Depth:    %-12d Response time:       %d sec\n",
          MaxSearchDepth, Level);
  printz ("Random:   %-12s Easy mode:           %s\n",
          (dither) ? "ON" : "OFF", (flag.easy) ? "ON" : "OFF");
  printz ("Beep:     %-12s Transposition table: %s\n",
          (flag.beep) ? "ON" : "OFF", (flag.hash) ? "ON" : "OFF");
  signal (SIGINT, TerminateSearch);
}

void EditBoard (void)
/*
   Set up a board position. Pieces are entered by typing the piece followed
   by the location. For example, Nf3 will place a knight on square f3.
*/

{
  short a, r, c, sq, i;
  char s[80];

  ClrScreen ();
  UpdateDisplay (0, 0, 1, 0);
  printz (".   exit to main\n");
  printz ("#   clear board\n");
  printz ("c   change sides\n");
  printz ("enter piece & location: \n");

  a = white;
  do
    {
      scanz ("%s", s);
      if (s[0] == '#')
        for(sq = 0; sq < 64; sq++)
          {
            board[sq] = no_piece;
            color[sq] = neutral;
          }
      if (s[0] == 'c' || s[0] == 'C')
        a = otherside[a];
      c = s[1] - 'a';
      r = s[2] - '1';
      if ((c >= 0) && (c < 8) && (r >= 0) && (r < 8))
        {
          sq = locn (r, c);
          color[sq] = a;
          board[sq] = no_piece;
          for(i = no_piece; i <= king; i++)
            if ((s[0] == pxx[i]) || (s[0] == qxx[i]))
              {
                board[sq] = i;
                break;
              }
        }
  } while (s[0] != '.');
  for (sq = 0; sq < 64; sq++)
    Mvboard[sq] = (board[sq] != Stboard[sq]) ? 10 : 0;
  GameCnt = 0;
  Game50 = 1;
  ZeroRPT ();
  InitializeStats ();
  ClrScreen ();
  UpdateDisplay (0, 0, 1, 0);
}

void SetupBoard (void)

/*
   Set up a board position. Eight lines of eight characters are used
   to setup the board. a8-h8 is the first line.
   Black pieces are  represented  by  uppercase characters.
*/

{
  short r, c, sq, i;
  char ch;
  char s[80];

  NewGame ();

  gets (s);                     /* skip "setup" command */
  for (r = 7; r >= 0; r--)
    {
      gets (s);
      for (c = 0; c <= 7; c++)
        {
          ch = s[c];
          sq = locn (r, c);
          color[sq] = neutral;
          board[sq] = no_piece;
          for(i = no_piece; i <= king; i++)
            if (ch == pxx[i])
              {
                color[sq] = black;
                board[sq] = i;
                break;
              }
            else
              if (ch == qxx[i])
                {
                  color[sq] = white;
                  board[sq] = i;
                  break;
                }
        }
    }
  for (sq = 0; sq < 64; sq++)
    Mvboard[sq] = (board[sq] != Stboard[sq]) ? 10 : 0;
  InitializeStats ();
  ClrScreen ();
  UpdateDisplay (0, 0, 1, 0);
  printz ("Setup successful\n");
}

void ShowDepth (char ch)
{
  ch++;                                 /* shut up the compiler */
}

void ShowResults (short int score, unsigned short int *bstline, char ch)
{
  register int i;
  if (flag.post)
    {
      fprintf (stderr, "%2d%c %6d %4ld %8ld  ", Sdepth, ch, score, et, NodeCnt);
      for (i = 1; bstline[i] > 0; i++)
        {
          if ((i > 1) && (i % 8 == 1))
            fprintf (stderr, "\n                          ");
          algbr ((short) (bstline[i] >> 8), (short) (bstline[i] & 0xFF), false);
          fprintf (stderr, "%5s ", mvstr[0]);
        }
      fprintf (stderr, "\n");
    }
}

void SearchStartStuff (short int side)
{
  signal (SIGINT, TerminateSearch);
  if (flag.post)
    {
      fprintf (stderr, "\nMove# %d    Target= %ld    Clock: %ld\n",
               TCmoves - TimeControl.moves[side] + 1,
               ResponseTime, TimeControl.clock[side]);
    }
}

void OutputMove (void)
{
  printz ("%d. ... %s\n", ++mycnt1, mvstr[0]);
  if (root->flags & draw)
    {
      printz ("Draw\n");
      ExitChess ();
    }
  if (root->score == -9999)
    {
      printz ("%s\n", ColorStr[opponent]);
      ExitChess ();
    }
  if (root->score == 9998)
    {
      printz ("%s\n", ColorStr[computer]);
      ExitChess ();
    }
  if (flag.post)
    {
      fprintf (stderr, "Nodes= %ld  Eval= %ld  Rate= %ld  Seconds= %ld  ",
               NodeCnt, EvalNodes, evrate, et + ft);
      fprintf (stderr, "Hash/Coll= %ld/%ld\n",
               HashCnt, HashCol);
    }

  UpdateDisplay (root->f, root->t, 0, root->flags);
  fprintf (stderr, "My move is: %s\n", mvstr[0]);
  if (flag.beep)
    printz ("%c", 7);

  if (root->flags & draw)
    fprintf (stderr, "Drawn game!\n");
  else if (root->score == -9999)
    fprintf (stderr, "opponent mates!\n");
  else if (root->score == 9998)
    fprintf (stderr, "computer mates!\n");
  else if (root->score < -9000)
    fprintf (stderr, "opponent will soon mate!\n");
  else if (root->score > 9000)
    fprintf (stderr, "computer will soon mate!\n");
}

void ElapsedTime (short int iop)


/*
   Determine the time that has passed since the search was started. If the
   elapsed time exceeds the target (ResponseTime+ExtraTime) then set timeout
   to true which will terminate the search.
*/

{
  if (ahead)
    {
      if (kbhit())
        {
          flag.timeout = true;
          flag.bothsides = false;
        }
    }
  et = time (NULL) - time0;
  if (et < 0)
    et = 0;
  ETnodes += 50;
  if (et > et0 || iop == 1)
    {
      if (et > ResponseTime + ExtraTime && Sdepth > 1)
        flag.timeout = true;
      et0 = et;
      if (iop == 1)
        {
          time0 = time (NULL);
          et0 = 0;
        }
      if (et > 0)
        /* evrate used to be Nodes / cputime I dont` know why */
        evrate = NodeCnt / (et + ft);
      else
        evrate = 0;
      ETnodes = NodeCnt + 50;
    }
}

void SetTimeControl (void)
{
  if (TCflag)
    {
      TimeControl.moves[white] = TimeControl.moves[black] = TCmoves;
      TimeControl.clock[white] = TimeControl.clock[black] = 60L * TCminutes;
    }
  else
    {
      TimeControl.moves[white] = TimeControl.moves[black] = 0;
      TimeControl.clock[white] = TimeControl.clock[black] = 0;
      Level = 60L * TCminutes;
    }
  et = 0;
  ElapsedTime (1);
}

void ClrScreen (void)
{
  printz ("\n");
}

void UpdateDisplay (short int f, short int t, short int redraw, short int isspec)
{
  short r, c, l;
  if (redraw)
    {
      printz ("\n");
      for (r = 7; r >= 0; r--)
        {
          for (c = 0; c <= 7; c++)
            {
              if (flag.reverse)
                l = locn (7 - r, 7 - c);
              else
                l = locn (r, c);
              if (color[l] == neutral)
                printz (" -");
              else if (color[l] == white)
                printz (" %c", qxx[board[l]]);
              else
                printz (" %c", pxx[board[l]]);
            }
          printz ("\n");
        }
      printz ("\n");
    }
  f++; t++; isspec++;                   /* shut up the compiler */
}

void GetGame (void)
{
  FILE *fd;
  char fname[256];
  int c;
  short sq;
  unsigned short m;

  printz ("Enter file name: ");
  scanz ("%s", fname);
  if (fname[0] == '\0')
    strcpy (fname, "chess.000");
  if ((fd = fopen (fname, "r")) != NULL)
    {
      fscanf (fd, "%hd%hd%hd", &computer, &opponent, &Game50);
      fscanf (fd, "%hd%hd", &castld[white], &castld[black]);
      fscanf (fd, "%hd%hd", &TCflag, &OperatorTime);
      fscanf (fd, "%ld%ld%hd%hd",
              &TimeControl.clock[white], &TimeControl.clock[black],
              &TimeControl.moves[white], &TimeControl.moves[black]);
      for (sq = 0; sq < 64; sq++)
        {
          fscanf (fd, "%hd%hd", &m, &Mvboard[sq]);
          board[sq] = (m >> 8);
          color[sq] = (m & 0xFF);
          if (color[sq] == 0)
            color[sq] = neutral;
          else
            --color[sq];
        }
      GameCnt = 0;
      c = '?';
      while (c != EOF)
        {
          ++GameCnt;
          c = fscanf (fd, "%hd%hd%hd%ld%hd%hd%hd", &GameList[GameCnt].gmove,
                      &GameList[GameCnt].score, &GameList[GameCnt].depth,
                      &GameList[GameCnt].nodes, &GameList[GameCnt].time,
                      &GameList[GameCnt].piece, &GameList[GameCnt].color);
          if (GameList[GameCnt].color == 0)
            GameList[GameCnt].color = neutral;
          else
            --GameList[GameCnt].color;
        }
      GameCnt--;
      if (TimeControl.clock[white] > 0)
        TCflag = true;
      computer--;
      opponent--;
    }
  fclose (fd);
  InitializeStats ();
  UpdateDisplay (0, 0, 1, 0);
  Sdepth = 0;
}

void SaveGame (void)
{
  FILE *fd;
  char fname[256];
  short sq, i, c;

  printz ("Enter file name: ");
  scanz ("%s", fname);

  if (fname[0] == '\0')
    strcpy (fname, "chess.000");
  if ((fd = fopen (fname, "w")) != NULL)
    {
      fprintf (fd, "%d %d %d\n", computer + 1, opponent + 1, Game50);
      fprintf (fd, "%d %d\n", castld[white], castld[black]);
      fprintf (fd, "%d %d\n", TCflag, OperatorTime);
      fprintf (fd, "%ld %ld %d %d\n",
               TimeControl.clock[white], TimeControl.clock[black],
               TimeControl.moves[white], TimeControl.moves[black]);
      for (sq = 0; sq < 64; sq++)
        {
          if (color[sq] == neutral)
            c = 0;
          else
            c = color[sq] + 1;
          fprintf (fd, "%d %d\n", 256 * board[sq] + c, Mvboard[sq]);
        }
      for (i = 1; i <= GameCnt; i++)
        {
          if (GameList[i].color == neutral)
            c = 0;
          else
            c = GameList[i].color + 1;
          fprintf (fd, "%d %d %d %ld %d %d %d\n",
                   GameList[i].gmove, GameList[i].score, GameList[i].depth,
                   GameList[i].nodes, GameList[i].time,
                   GameList[i].piece, c);
        }
      fclose (fd);
      printz ("Game saved on file: %s", fname);
    }
  else
    printz ("Could not open file: %s", fname);
}

void ListGame (void)
{
  FILE *fd;
  short i, f, t;
  fd = fopen ("chess.lst", "w");
  fprintf (fd, "\n");
  fprintf (fd, "       score  depth   nodes  time         ");
  fprintf (fd, "       score  depth   nodes  time\n");
  for (i = 1; i <= GameCnt; i++)
    {
      f = GameList[i].gmove >> 8;
      t = (GameList[i].gmove & 0xFF);
      algbr (f, t, false);
      if ((i % 2) == 0)
        fprintf (fd, "         ");
      else
        fprintf (fd, "\n");
      fprintf (fd, "%5s  %5d     %2d %7ld %5d", mvstr[0],
               GameList[i].score, GameList[i].depth,
               GameList[i].nodes, GameList[i].time);
    }
  fprintf (fd, "\n\n");
  fclose (fd);
}

void Undo (void)
/*
   Undo the most recent half-move.
*/

{
  short f, t;
  f = GameList[GameCnt].gmove >> 8;
  t = GameList[GameCnt].gmove & 0xFF;
  if (board[t] == king && distance (t, f) > 1)
    (void) castle (GameList[GameCnt].color, f, t, 2);
  else
    {
      /* Check for promotion: */
      if ((row (f) == 6 && row (t) == 7) || (row (f) == 1 && row (t) == 0))
        {
          int g, from = f, c= color[t];
          for (g = GameCnt - 1; g >= 0; g--)
            if (GameList[g].gmove & 0xFF == from)
              from = GameList[g].gmove >> 8;
          if ((row(from) == 1 && row(f) == 6)
             || (row(from) == 6 && row(f) == 1))
            board[t] = pawn;
        }

      board[f] = board[t];
      color[f] = color[t];
      board[t] = GameList[GameCnt].piece;
      color[t] = GameList[GameCnt].color;
      if (color[t] != neutral)
        Mvboard[t]--;
      Mvboard[f]--;
    }
  if (TCflag)
    ++TimeControl.moves[color[f]];
  GameCnt--;
  computer = otherside[computer];
  opponent = otherside[opponent];
  flag.mate = false;
  Sdepth = 0;
  //UpdateDisplay (0, 0, 1, 0);
  InitializeStats ();
}

void ShowMessage (char *s)
{
  fprintf (stderr, "%s\n", s);
}

void ShowSidetomove (void)
{
}

void PromptForMove (void)
{
  printz ("\nYour move is? ");
}


void ShowCurrentMove (short int pnt, short int f, short int t)
{
  f++; t++; pnt++;                      /* shut up the compiler */
}

void ChangeAlphaWindow (void)
{
  printz ("window: ");
  scanz ("%hd", &Awindow);
}

void ChangeBetaWindow (void)
{
  printz ("window: ");
  scanz ("%hd", &Bwindow);
}

void GiveHint (void)
{
  algbr ((short) (hint >> 8), (short) (hint & 0xFF), false);
  fprintf (stderr, "Hint: %s\n", mvstr[0]);
}

void SelectLevel (void)
{
  printz ("Enter #moves #minutes: ");
  scanz ("%hd %hd", &TCmoves, &TCminutes);
  printz ("Operator time= ");
  scanz ("%hd", &OperatorTime);
  TCflag = (TCmoves > 1);
  SetTimeControl ();
}

void ChangeSearchDepth (void)
{
  printz ("depth= ");
  scanz ("%hd", &MaxSearchDepth);
}

void SetContempt (void)
{
  printz ("contempt= ");
  scanz ("%hd", &contempt);
}

void ChangeXwindow (void)
{
  printz ("xwndw= ");
  scanz ("%hd", &xwndw);
}

void TestSpeed(void (*f) (short int side, short int ply))
{
  short i;
  long cnt, rate, t1, t2;

  t1 = time (NULL);
  for (i = 0; i < 10000; i++)
    {
      f (opponent, 2);
    }
  t2 = time (NULL);
  cnt = 10000L * (TrPnt[3] - TrPnt[2]);
  rate = cnt / (t2 - t1);
  printz ("Nodes= %ld Nodes/sec= %ld\n", cnt, rate);
}


void InputCommand (void)
/*
   Process the users command. If easy mode is OFF (the computer is thinking
   on opponents time) and the program is out of book, then make the 'hint'
   move on the board and call SelectMove() to find a response. The user
   terminates the search by entering ^C (quit siqnal) before entering a
   command. If the opponent does not make the hint move, then set Sdepth to
   zero.
*/

{
  int i;
  short ok, tmp;
  unsigned short mv;
  char s[80];

  ok = flag.quit = false;
  player = opponent;
  ft = 0;
  if (hint > 0 && !flag.easy && Book == NULL)
    {
      fflush (stdout);
      time0 = time (NULL);
      algbr ((short) hint >> 8, (short) hint & 0xFF, false);
      strcpy (s, mvstr[0]);
      tmp = epsquare;
      if (flag.post)
        GiveHint ();
      if (VerifyMove (s, 1, &mv))
        {
          ahead = 1;
          SelectMove (computer, 2);
          VerifyMove (mvstr[0], 2, &mv);
          if (Sdepth > 0)
            Sdepth--;
        }
      ft = time (NULL) - time0;
      epsquare = tmp;
    }
  ahead = 0;
  while (!(ok || flag.quit))
    {
      UpdateDisplay (0, 0, 1, 0);
      PromptForMove ();
      i = scanz ("%s", s);
      if (i == EOF || s[0] == 0)
        ExitChess ();
      player = opponent;
      if (strcmp (s, "bd") == 0)
        {
          ClrScreen ();
          UpdateDisplay (0, 0, 1, 0);
        }
      else if (strcmp (s, "alg") == 0) /* noop */ ;
      else if ((strcmp (s, "quit") == 0) || (strcmp(s, "exit") == 0))
        flag.quit = true;
      else if (strcmp (s, "post") == 0)
        flag.post = !flag.post;
      else if ((strcmp (s, "set") == 0) || (strcmp(s, "edit") == 0))
        EditBoard ();
      else if (strcmp (s, "setup") == 0)
        SetupBoard ();
      else if (strcmp (s, "go") == 0)
        ok = true;
      else if (strcmp (s, "help") == 0)
        help ();
      else if (strcmp (s, "force") == 0)
        flag.force = !flag.force;
      else if (strcmp (s, "book") == 0)
        Book = NULL;
      else if (strcmp (s, "new") == 0)
        NewGame ();
      else if (strcmp (s, "list") == 0)
        ListGame ();
      else if (strcmp (s, "level") == 0 || strcmp (s, "clock") == 0)
        SelectLevel ();
      else if (strcmp (s, "hash") == 0)
        flag.hash = !flag.hash;
      else if (strcmp (s, "beep") == 0)
        flag.beep = !flag.beep;
      else if (strcmp (s, "Awindow") == 0)
        ChangeAlphaWindow ();
      else if (strcmp (s, "Bwindow") == 0)
        ChangeBetaWindow ();
      else if (strcmp (s, "rcptr") == 0)
        flag.rcptr = !flag.rcptr;
      else if (strcmp (s, "hint") == 0)
        GiveHint ();
      else if (strcmp (s, "both") == 0)
        {
          flag.bothsides = !flag.bothsides;
          Sdepth = 0;
          ElapsedTime (1);
          SelectMove (opponent, 1);
          ok = true;
        }
      else if (strcmp (s, "reverse") == 0)
        {
          flag.reverse = !flag.reverse;
          ClrScreen ();
          UpdateDisplay (0, 0, 1, 0);
        }
      else if (strcmp (s, "switch") == 0)
        {
          computer = otherside[computer];
          opponent = otherside[opponent];
          flag.force = false;
          Sdepth = 0;
          ok = true;
        }
      else if (strcmp (s, "white") == 0 || strcmp (s, "first") == 0)
        {
          computer = white;
          opponent = black;
          ok = true;
          flag.force = false;
          Sdepth = 0;
        }
      else if (strcmp (s, "black") == 0)
        {
          computer = black;
          opponent = white;
          ok = true;
          flag.force = false;
          Sdepth = 0;
        }
      else if (strcmp (s, "undo") == 0 && GameCnt > 0)
        Undo ();
      else if (strcmp (s, "remove") == 0 && GameCnt > 1)
        {
          Undo ();
          Undo ();
        }
      else if (strcmp (s, "get") == 0)
        GetGame ();
      else if (strcmp (s, "save") == 0)
        SaveGame ();
      else if (strcmp (s, "depth") == 0)
        ChangeSearchDepth ();
      else if (strcmp (s, "random") == 0)
        dither = 6;
      else if (strcmp (s, "easy") == 0)
        flag.easy = !flag.easy;
      else if (strcmp (s, "contempt") == 0)
        SetContempt ();
      else if (strcmp (s, "xwndw") == 0)
        ChangeXwindow ();
      else if (strcmp (s, "test") == 0)
        {
          ShowMessage("Testing MoveList Speed");
          TestSpeed (MoveList);
          ShowMessage("Testing CaptureList Speed");
          TestSpeed (CaptureList);
        }
      else
        {
          ok = VerifyMove (s, 0, &mv);
          if (ok && mv != hint)
            {
              Sdepth = 0;
              ft = 0;
            }
        }
    }

  ElapsedTime (1);
  if (flag.force)
    {
      computer = opponent;
      opponent = otherside[computer];
    }
  signal (SIGINT, TerminateSearch);
}



/*
-------------------------------------

 This part is the interface to engine
 
-------------------------------------
*/

short Side2move;


// on init for the first time
void ChDos_Init(void)
{
  Level = 0;
  TCflag = false;
  OperatorTime = 0;
  TCmoves = 100;
  TCminutes = 5;
  TCflag = true;
  Initialize ();
  Initialize_dist ();
  Initialize_moves ();
  NewGame ();
  GetOpenings ();
  hashfile = NULL;
  
	flag.reverse = (time(NULL) & 1);		// we are black or white
	
	if(flag.reverse)
		{
    computer = otherside[computer];
    opponent = otherside[opponent];
		}     
  Side2move=white;
  
#if ttblsz
#ifdef HASHFILE
  hashfile = fopen (HASHFILE, RWA_ACC);
#endif /* HASHFILE */
#endif  /* ttblsz */
}

// on exit at the end
void ChDos_End(void)
{ 
#if ttblsz
#ifdef HASHFILE
  if (hashfile) fclose(hashfile);
#endif /* HASHFILE */
#endif /* ttblsz */
  ExitChess ();
}

// Returns piece ( PN..qk )  from given square (a1..h8) 
char ChDos_GetPieceAt( char at[2] )
{
  short r, c, l;
  c = at[0]-'a';
  r = at[1]-'1';
  l = locn (r, c);
  if(board[l]==0) return 0;
  if(color[l]==neutral) return 0;
  return ( color[l]!=white ? qxx[board[l]] : pxx[board[l]] );            
}

// Returns 1 if there is move from square 
int ChDos_IsFrom( char at[2] )
{
  static short pnt;
  struct leaf *node;
  
  if(Side2move==opponent && !flag.mate)
  {
  MoveList (opponent, 2);
  pnt = TrPnt[2];
  while (pnt < TrPnt[3])
    {
      node = &Tree[pnt++];
      algbr (node->f, node->t, (short) node->flags);
      if ( at[0] == mvstr[0][0] && at[1] == mvstr[0][1]) return (true);
    }
  }  
  return (false);
}


// Does movement by user, returns 1 on success
int ChDos_DoMove( char fat[2], char tat[2] )
{
	unsigned short int mv;
  static short pnt;
  struct leaf *node;
  char s[80];
  
	if(Side2move==opponent && !flag.mate)
  {
  MoveList (opponent, 2);
  pnt = TrPnt[2];
  while (pnt < TrPnt[3])
    {
      node = &Tree[pnt++];
      algbr (node->f, node->t, (short) node->flags);
      strcpy (s, mvstr[0]);
      
      if( ( fat[0] == mvstr[0][0] && fat[1] == mvstr[0][1] &&
      		 tat[0] == mvstr[0][2] && tat[1] == mvstr[0][3]	) &&
						VerifyMove (s, 0, &mv) )
							{
							Side2move = otherside[Side2move];
							return(true);		
							}
    }
  }  
 
	return(false);	
}

// Returns 1 if king is in check currently
int ChDos_IsCheck(void)
{
 return SqAtakd (PieceList[Side2move][0], otherside[Side2move]);
}

// Selects move by searching 
int ChDos_SelectMove(void)
{
  if(Side2move==computer && !flag.mate)
  {
	SelectMove (computer, 1);
	
	if(&mvstr[0][0]!=0)
		{		
		Side2move = otherside[Side2move];
		return true;
		}
	}
	return (false);
}

void ChDos_UndoMove(void)
{
  if(GameCnt>1)			// undo one complete move
   {  	
    Undo();
    Undo();
	 }
}

// play GnuChess vs GnuChess
void ChDos_AutoMove(void)
{
  if(Side2move==opponent && !flag.mate)
  {
    computer = otherside[computer];
    opponent = otherside[opponent];
  }
}

// new game by pressed button
void chDos_SetNewGame(void)
{
  int flag_rev;
  
  flag_rev = flag.reverse;
  
  NewGame ();
  GetOpenings ();
  hashfile = NULL;
  
	flag.reverse = otherside[flag_rev];					// we are opposit now
	
	if(flag.reverse)
		{
    computer = otherside[computer];
    opponent = otherside[opponent];
		}     
  Side2move=white;
}
