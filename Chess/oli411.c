/* OliThink 4.1.1 - Bitboard Version (c) Oliver Brausch 19.Sep.2003, ob112@web.de
 
 Watcom compiled OliThink is slow.
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <signal.h>
#include <math.h>
#include <conio.h>
#include <time.h>

typedef unsigned long long u64;
typedef unsigned long u32;

#define I64U "%llu"

const u32 hashSize = 0x040000; // This is 8 MB		(default was 0x100000 - it is 16 MB)
const u32 hashMask = 0x02FFFF; // This must be hashSize - 2 (default was 0x0FFFFE )

const u32 pawnSize = 0x020000; // This is 2 MB
const u32 pawnMask = 0x01FFFF; // This must be pawnSize - 1

int inputmove(int);
void rookmoves(int, int, int, int);

#define COL_N -1
#define COL_W 0
#define COL_B 1

#define ENP_W 0
#define ENP_B 1
#define PAW_W 2
#define PAW_B 3
#define KNI_W 4
#define KNI_B 5
#define BIS_W 6
#define BIS_B 7
#define ROO_W 8
#define ROO_B 9
#define QUE_W 10
#define QUE_B 11
#define KIN_W 12
#define KIN_B 13
#define EMPTY 14

const char piece_char[16] = "**PpNnBbRrQqKk  ";
const int piece_val[16] = {100,-100,100,-100,300,-300,310,-310,490,-490,+900,-900,+25000,-25000,0,0};
const int pos_val[16] = {10,-10,10,-10,6,-6,8,-8,4,-4,1,-1,5,-5,0,0};

#define FROM(x) ((x) & 0x3F)
#define TO(x) (((x) >> 6) & 0x3F)
#define ONMOVE(x) (((x) >> 12) & 0x01)
#define PROMOTE(x) (((x) >> 12) & 0x0F)
#define P_TO(x) (((x) >> 16) & 0x0F)
#define IS_PROM(x) (((x) >> 20) & 0x01)
#define PIECE(x) (IS_PROM(x) ? 0x02 | ONMOVE(x)  : PROMOTE(x))
#define P_WH(x) (IS_PROM(x) ? 0x02 : (PROMOTE(x) & 0xE))
#define VAL(x) (((x) >> 21) & 0x7FF)
#define MOVEMASK(x) ((x) & 0x1FFFFF)
#define IDENTMV(x, y) (MOVEMASK(x) == MOVEMASK(y))
#define PSWAP(x) (piece_val[PROMOTE(x)] - piece_val[PIECE(x)] - piece_val[P_TO(x)])

#define R000(x) ((int)(P000 >> _r000shift[x]) & 0xFF)
#define R090(x) ((int)(P090 >> _r090shift[x]) & 0xFF)
#define R045(x) ((int)(P045 >> _r045shift[x]) & _r045lenmask[x])
#define R135(x) ((int)(P135 >> _r135shift[x]) & _r135lenmask[x])
#define A000(x) a000Attack[x][R000(x)]
#define A090(x) a090Attack[x][R090(x)]
#define A045(x) a045Attack[x][R045(x)]
#define A135(x) a135Attack[x][R135(x)]

u64 DirA[10][64], Bef[2][8][64];
u64 NeiB[64], NeiR[64], LineT[64], LineD[64], LineS[64];

const int PAWN[2]={PAW_W,PAW_B};
const int BISHOP[2]={BIS_W,BIS_B};
const int KNIGHT[2]={KNI_W,KNI_B};
const int ROOK[2]={ROO_W,ROO_B};
const int QUEEN[2]={QUE_W,QUE_B};
const int KING[2]={KIN_W,KIN_B};
const int ENPAS[2]={ENP_W,ENP_B};
int kingpos[2];
int board[64];

char boardStr[]=
"-----------------"
"|r|n|b|q|k|b|n|r|"
"-----------------"
"|p|p|p|p|p|p|p|p|"
"-----------------"
"| | | | | | | | |"
"-----------------"
"| | | | | | | | |"
"-----------------"
"| | | | | | | | |"
"-----------------"
"| | | | | | | | |"
"-----------------"
"|P|P|P|P|P|P|P|P|"
"-----------------"
"|R|N|B|Q|K|B|N|R|"
"-----------------";

u64 R000BitB[14];
u64 R045BitB[14];
u64 R090BitB[14];
u64 R135BitB[14];
u64 QuRo, QuBi, Empty;
u64 P000, P045, P090, P135;

u64 a000Attack[64][256];
u64 a090Attack[64][256];
u64 a045Attack[64][256];
u64 a135Attack[64][256];
u64 aSrtAttack[64][16][4];
int knightmobil[64];
int kingmobil[64];
int hitval[16][16];

const int r045map[64] = { 
28,36,43,49,54,58,61,63,
21,29,37,44,50,55,59,62,
15,22,30,38,45,51,56,60,
10,16,23,31,39,46,52,57,
 6,11,17,24,32,40,47,53,
 3, 7,12,18,25,33,41,48,
 1, 4, 8,13,19,26,34,42,
 0, 2, 5, 9,14,20,27,35
};

const int _r045shift[64] = {
28,36,43,49,54,58,61,63,
21,28,36,43,49,54,58,61,
15,21,28,36,43,49,54,58,
10,15,21,28,36,43,49,54,
 6,10,15,21,28,36,43,49,
 3, 6,10,15,21,28,36,43,
 1, 3, 6,10,15,21,28,36,
 0, 1, 3, 6,10,15,21,28
};

const int r045length[64] = { 
8,7,6,5,4,3,2,1,
7,8,7,6,5,4,3,2,
6,7,8,7,6,5,4,3,
5,6,7,8,7,6,5,4,
4,5,6,7,8,7,6,5,
3,4,5,6,7,8,7,6,
2,3,4,5,6,7,8,7,
1,2,3,4,5,6,7,8 
};

const int pawnpos[64] = {
9,9,9,9,9,9,9,9,
8,8,8,8,8,8,8,8,
6,6,6,6,6,6,6,6,
4,4,5,5,5,5,4,4,
2,2,4,4,4,4,2,2,
2,2,2,2,2,2,2,2,
1,1,1,1,1,1,1,1,
0,0,0,0,0,0,0,0
};

int kingmoves[9] = {8,-9,-1,7,8,9,1,-7,-8};
int knightmoves[9] = {8,-17,-10,6,15,17,10,-6,-15};
int pawnwcaps[3] = {2,7,9};
int pawnbcaps[3] = {2,-9,-7};
int pawnwmoves[2] = {1,8};
int pawnbmoves[2] = {1,-8};
int pawnwfmove[3] = {2,8,16};
int pawnbfmove[3] = {2,-8,-16};
int runto[64][64];
int direction[64][64];
int pawn_val[64][2];

char _LSB[0x10000];
char _HSB[0x100000];
u64 hashMAP[64][16];
u64 pawnMAP[64][16];
u64 hashBoard;
u64 hashPawn;
u64 hashstack[960];

typedef int Move;
typedef int Attack;
Attack att[16];

typedef struct {
	u64 rec;
	Move move;
	short int w;
	char depth;
	char flag;
} HashREC;
HashREC* hashREC;
HashREC* pawnREC;

Move hashmv[960];
Move movestack[960];
Move pv[64][64];
Move killer[64];
Move killer2[64];
Move movelist[256][64];
Move pmove;
int pnumber;
int pvlength[64];
int enpstack[960];
int cststack[960];
int history[0x10000];
int enpflag;
int cstflag;
int nstack;
int neval;
int nodes;
int material;
int mytime;
int sd;
int nps;
int machine;
int xboard;
int comp;
int sabort;
int icmd;
int doponder = 1;
int dopost = 1;

#define CASTLE_SHORT_W 1
#define CASTLE_SHORT_B 2
#define CASTLE_LONG_W 4
#define CASTLE_LONG_B 8

int pow2(int pow) { return (int)1<<pow; }

u64 _r000m[64];
u64 __r000m[64][64];
u64 r000m(int f) { return (u64)1 << f; }

int _r000shift[64];
int r000shift(int f) { return f & 0x38; }
int t000to090(int f) { return ((~f & 0x07) << 3) + ((f >> 3) & 0x07); }

u64 _r090m[64];
u64 __r090m[64][64];
u64 r090m(int f) { return (u64)1 << t000to090(f); }

int _r090shift[64];
int r090shift(int f) { return r000shift(t000to090(f)); }

u64 _r045m[64];
u64 __r045m[64][64];
u64 r045m(int f) { return (u64)1 << r045map[f]; }

int _r045lenmask[64];
int r045lenmask(int f) { return pow2(r045length[f]) - 1; }
int t045to135(int f) { return (f & 0x38) + ((~f) & 0x07); }

u64 _r135m[64];
u64 __r135m[64][64];
u64 r135m(int f) { return (u64)1 << r045map[t045to135(f)]; }

int _r135shift[64];
int r135shift(int f) { return _r045shift[t045to135(f)]; }
int r135length(int f) { return r045length[t045to135(f)]; }

int _r135lenmask[64];
int r135lenmask(int f) { return pow2(r135length(f)) - 1; }

static u32 r_x = 30903, r_y = 30903, r_z = 30903, r_w = 30903, r_carry = 0;
u32 rand32()
{
   u32 t;
   r_x = r_x * 69069 + 1;
   r_y ^= r_y << 13;
   r_y ^= r_y >> 17;
   r_y ^= r_y << 5;
   t = (r_w << 1) + r_z + r_carry;
   r_carry = ((r_z >> 2) + (r_w >> 3) + (r_carry >> 2)) >> 30;
   r_z = r_w;
   r_w = t;
   return r_x + r_y + r_w;
}

u64 rand64() { u64 c = rand32(); return rand32() | (c << 32); }

void init_slowpieces(int i, int p, int h, int* m) {
	int j, n;
	for (j = 1; j <= m[0]; j++) {
		n = i + m[j];
		if (n < 64 && n >= 0 && abs((n & 7) - (i & 7)) <= 2) {
			if ((p == PAW_W && h == 0 && n >= 56) || (p == PAW_B && h == 0 && n < 8)) 
				aSrtAttack[i][p][2] |= r000m(n);  // non capture promotion
			else 
				aSrtAttack[i][p][h] |= r000m(n);
		}
	}
}

char slowLSB(int i) {
	char k = -1;
	while (i) { k++; if (i & 1) break; i >>= 1; }
	return k;
}

char slowHSB(int i) {
	char k = 32;
	while (i) { k--; if (i & 0x80000000) break; i <<= 1; }
	return (char)((k < 32) ? 2*k+1+(((i<<1) & 0x80000000)!=0) : 0);
}

int bitcount(u64 bit) {
	int count=0;
	while (bit) { bit &= (bit-1); count++; }
    return count;
}

void init_arrays() /* precomputing stuff */
{
	int i,j,k,n,m;
	u64 hbit;

	hashREC = (HashREC*) malloc(hashSize * sizeof(HashREC));
	pawnREC = (HashREC*) malloc(pawnSize * sizeof(HashREC));

	if (!hashREC || !pawnREC) { printf("NO MEMORY\n"); exit(1); }

	memset(hashREC, 0, sizeof(hashREC));
	memset(aSrtAttack, 0, sizeof(aSrtAttack));
	memset(hashstack, 0, sizeof(hashstack));
	memset(hitval, 0, sizeof(hitval));
	memset(runto, -1, sizeof(runto));
	memset(direction, 0, sizeof(direction));

	for (i = 0; i < 0x100000; i++) _HSB[i] = slowHSB(i);
	for (i = 0; i < 0x10000; i++) _LSB[i] = slowLSB(i);
	for (i = 0; i < 64; i++) {
		_r000m[i] = r000m(i);
		_r045m[i] = r045m(i);
		_r090m[i] = r090m(i);
		_r135m[i] = r135m(i);
		_r000shift[i] = r000shift(i);
		_r090shift[i] = r090shift(i);
		_r135shift[i] = r135shift(i);
		_r045lenmask[i] = r045lenmask(i);
		_r135lenmask[i] = r135lenmask(i);
	}
	for (i = 0;i < 64;i++) {
		for (j = 0; j < 16; j++) {
			hashMAP[i][j] = rand64();
			pawnMAP[i][j] = ((j | 1) == PAW_B) ? hashMAP[i][j] : 0;
		}
		for (j = 0;j < 64;j++) {
			__r000m[i][j] = _r000m[i] | _r000m[j];
			__r045m[i][j] = _r045m[i] | _r045m[j];
			__r090m[i][j] = _r090m[i] | _r090m[j];
			__r135m[i][j] = _r135m[i] | _r135m[j];
		}
		NeiB[i] = NeiR[i] = _r000m[i];
		for (k = 1; k <= kingmoves[0]; k++) {
			DirA[k][i] = 0;
			for (n = -1, j = i;;) {
			int nf = j + kingmoves[k];
			if (nf < 0 || nf > 63 || (j % 8 == 0 && nf % 8 == 7) || (j % 8 == 7 && nf % 8 == 0)) 
				break;
			direction[i][nf] = kingmoves[k];
			DirA[k][i] |= _r000m[nf];
			runto[i][nf] = n;
			if (n == -1) { 
				if (abs(nf-j) == 1) NeiB[i] |= _r000m[nf];
				if (nf - j == 1) NeiR[i] |= _r000m[nf];
			}
			n = j = nf;
			}
		}
		init_slowpieces(i, KNI_W, 0, knightmoves);
		init_slowpieces(i, KNI_B, 0, knightmoves);
		init_slowpieces(i, KNI_W, 1, knightmoves);
		init_slowpieces(i, KNI_B, 1, knightmoves);
		init_slowpieces(i, KIN_W, 0, kingmoves);
		init_slowpieces(i, KIN_B, 0, kingmoves);
		init_slowpieces(i, KIN_W, 1, kingmoves);
		init_slowpieces(i, KIN_B, 1, kingmoves);
		init_slowpieces(i, PAW_W, 0, pawnwmoves);
		init_slowpieces(i, PAW_B, 0, pawnbmoves);
		init_slowpieces(i, PAW_W, 1, pawnwcaps);
		init_slowpieces(i, PAW_B, 1, pawnbcaps);
		knightmobil[i] = bitcount(aSrtAttack[i][KNI_W][1]);
		kingmobil[i] = bitcount(aSrtAttack[i][KIN_W][1]) + bitcount(aSrtAttack[i][KNI_W][1]);
		pawn_val[i][COL_W] = pawnpos[63-i];
		pawn_val[i][COL_B] = -pawnpos[i];

		if (i/8 == 1) init_slowpieces(i, PAW_W, 3, pawnwfmove);
		if (i/8 == 6) init_slowpieces(i, PAW_B, 3, pawnbfmove);
		for (j = 0;j < 256;j++) {
			hbit = 0;
			for (k = i + 1; k & 7; k++) {
				hbit |= r000m(k);
				if (j & pow2(k & 7)) break;
			}
			for (k = i - 1; (k & 7) != 7; k--) {
				hbit |= r000m(k);
				if (j & pow2(k & 7)) break;
			}
			a000Attack[i][j] = hbit;
			
			hbit = 0;
			for (k = i + 8; k < 64; k += 8) {
				hbit |= r000m(k);
				if (j & pow2(k / 8)) break;
			}
			for (k = i - 8; k >= 0; k -= 8) {
				hbit |= r000m(k);
				if (j & pow2(k / 8)) break;
			}
			a090Attack[i][j] = hbit;
			
			hbit = 0;
			for (k = i - 9, n = 0; k >= 0 && (k & 7) != 7; k -= 9, n++);
			for (k = i + 9, m = n + 1; k < 64 && (k & 7) != 0; k += 9, m++) {
				hbit |= r000m(k);
				if (j & pow2(m)) break;
			}
			for (k = i-9, m = n -1; k >= 0 && (k & 7) != 7; k -= 9, m--) {
				hbit |= r000m(k);
				if (j & pow2(m)) break;
			}
			a045Attack[i][j] = hbit;
			
			hbit = 0;
			for (k = i - 7, n = 0; k >= 0 && (k & 7) != 0; k -= 7, n++);
			for (k = i + 7, m = n + 1; k < 64 && (k & 7) != 7; k += 7, m++) {
				hbit |= r000m(k);
				if (j & pow2(m)) break;
			}
			for (k = i - 7, m = n - 1; k >= 0 && (k & 7) != 0; k -= 7, m--) {
				hbit |= r000m(k);
				if (j & pow2(m)) break;
			}
			a135Attack[i][j] = hbit;
		}
	}
	P000 = hbit = 0;
	for (i = 0; i < 64; i++) {
		LineT[i] = LineS[i] = A090(i) | _r000m[i];
		if ((i & 0x07) > 0) LineT[i] |= A090(i-1) | _r000m[i-1];
		if ((i & 0x07) < 7) LineT[i] |= A090(i+1) | _r000m[i+1];
		LineD[i] = LineT[i] & (~LineS[i]);
		hbit |= A000(i) | _r000m[i];
		Bef[0][0][i] = ~hbit | A000(i) | _r000m[i];
		Bef[1][0][i] = hbit | A000(i) | _r000m[i];
	}
	for (i = 0; i < 64; i++)
		for (j = 1; j < 8; j++) {
			Bef[0][j][i] = i + j * 8 < 64 ? Bef[0][0][i + j * 8] : 0;
			Bef[1][j][i] = i - j * 8 > 0 ? Bef[1][0][i - j * 8] : 0;
		}
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			if (piece_val[j] != 0) {
				hitval[i][j] = 100 * abs(piece_val[i]) / abs(piece_val[j]);
			}
		}
	}
}

char lookforP(char c) {
	char* c1 = strchr(piece_char, c);
	return (char)(c1 ? c1 - piece_char : 0);
}

void parseBoardStr(char *p) {
	int c, i = 56;
	while (*p) {
		if (*p++ == '|') { 
			c = lookforP(*p);
			if (c) board[i++] = c;
			else i -= 16;
		}
	}
}

void upBitHelpers() {
	QuRo = R000BitB[QUE_W] | R000BitB[QUE_B] | R000BitB[ROO_W] | R000BitB[ROO_B];
	QuBi = R000BitB[QUE_W] | R000BitB[QUE_B] | R000BitB[BIS_W] | R000BitB[BIS_B];
	P000 = R000BitB[COL_W] | R000BitB[COL_B];
	P045 = R045BitB[COL_W] | R045BitB[COL_B];
	P090 = R090BitB[COL_W] | R090BitB[COL_B];
	P135 = R135BitB[COL_W] | R135BitB[COL_B];
	Empty = ~P000;
}

#define UPBITB(x, y) { R000BitB[x] ^= _r000m[y]; R045BitB[x] ^= _r045m[y]; R090BitB[x] ^= _r090m[y]; R135BitB[x] ^= _r135m[y]; }

int init_board(char *s) {
	int i,p,c;
	if (s) parseBoardStr(s);
	hashBoard = hashPawn = hashMAP[1][0];
	memset(R000BitB, 0, sizeof(R000BitB));
	memset(R045BitB, 0, sizeof(R045BitB));
	memset(R090BitB, 0, sizeof(R090BitB));
	memset(R135BitB, 0, sizeof(R135BitB));
	material = comp = 0;
	for (i = 0; i < 64; i++) {
		p=board[i];
		if (p == EMPTY) continue;
		material += piece_val[p];
		if (p == KIN_W) kingpos[COL_W] = i;
		if (p == KIN_B) kingpos[COL_B] = i;
		c = p & 1;
		UPBITB(p, i);
		UPBITB(c, i);
		hashBoard ^= hashMAP[i][p];
		hashPawn ^= pawnMAP[i][p];
	}
	upBitHelpers();
	nstack = 0;
	enpflag = 0;
	enpstack[0] = 0;
	cstflag = 0;
	if (board[4] == KIN_W && board[7] == ROO_W) cstflag |= CASTLE_SHORT_W;
	if (board[4] == KIN_W && board[0] == ROO_W) cstflag |= CASTLE_LONG_W;
	if (board[60] == KIN_B && board[63] == ROO_B) cstflag |= CASTLE_SHORT_B;
	if (board[60] == KIN_B && board[56] == ROO_B) cstflag |= CASTLE_LONG_B;
	hashBoard ^= hashMAP[cstflag][14];
	cststack[0] = cstflag;
	hashstack[0] = hashBoard;
	memset(pvlength, 0, sizeof(pvlength));
	memset(killer, 0, sizeof(killer));
	memset(killer2, 0, sizeof(killer2));
	machine = COL_B;
	pmove = pnumber = 0;
	return COL_W;
}

long get_time(void)
{
  time_t seconds;
  seconds = time (NULL);
  return (seconds * 1000);
}

void display64(u64 bit) {
	int i,j;
	for (i = 56;i >= 0;i -= 8) {
		printf("\n----------------\n");
		for (j = i; j < i + 8; j++) printf("%c|", '0'+(char)((bit >> (u64)j) & 1));
	}
	printf("\n----------------"I64U"\n",bit);
}

void displaym(Move move) {
	printf("%c%c%c%c%c", 'a' + FROM(move) % 8, '1' + FROM(move) / 8,
		P_TO(move) == EMPTY ? '-' : 'x', 'a' + TO(move) % 8, '1' + TO(move) / 8);
	if (PIECE(move) != PROMOTE(move)) printf("%c", piece_char[PROMOTE(move) | 1]);
}

void displayb() {
	int i,j;
	for (i = 56; i >= 0; i -= 8) {
		printf("\n-----------------\n|");
		for (j = i; j < i + 8; j++) printf("%c|",piece_char[board[j]]);
	}
	printf("\n-----------------\n");
	fflush(stdout);
}

void displaypv() {
	int i;
	for (i = 0; i < pvlength[0]; i++) {
		displaym(pv[0][i]); printf(" ");
	}
}

int LSB (u64 bit) {
	u32 n = (u32) bit;
	if (n) {
		if (n & 0xffff) return _LSB[n & 0xffff];
		else return 16 + _LSB[(n >> 16) & 0xffff];
	} else {
		n = (u32)(bit >> 32);
		if (n & 0xffff) return 32 + _LSB[n & 0xffff];
		else return 48 + _LSB[(n >> 16) & 0xffff];
	}
}

Move newmove(int from, int to, int p, int p_to, int promote, int ply) {

	Move move = from | (to << 6) | (promote << 12) |  (p_to << 16) | ( (p!=promote) << 20);
	int val = (_HSB[history[move & 0xFFFF] & 0xFFFFF]);

	if (IDENTMV(move, hashmv[ply])) val += 512;
	if (IDENTMV(move, killer[ply])) val += 50;
	if (IDENTMV(move, killer2[ply])) val += 50;
	if (p_to != EMPTY) val += hitval[p_to][p] << 3;
	if (p != promote) val+= hitval[promote][QUE_W];

	return move | (val << 21);
}

u64 attackbit(int i) {
	return 
	(aSrtAttack[i][PAW_W][1] & R000BitB[PAW_B]) |
	(aSrtAttack[i][PAW_B][1] & R000BitB[PAW_W]) |
	(aSrtAttack[i][KIN_W][1] & (R000BitB[KIN_W] | R000BitB[KIN_B])) |
	(aSrtAttack[i][KNI_W][1] & (R000BitB[KNI_W] | R000BitB[KNI_B])) |
	((A000(i) | A090(i)) & QuRo) |
	((A045(i) | A135(i)) & QuBi);
}

#define _PTATT(a, p) {int n = LSB(a & R000BitB[p]); a ^= _r000m[n]; att[na++] = n | ((p) << 12);}

int addattack(int i, int c, int apawn) {
	int na = 0;
	u64 attack = attackbit(i) & R000BitB[c];
	if (!apawn) attack &= (~R000BitB[PAWN[c]]);

	while (attack) {
		if (attack & R000BitB[PAW_W | c]) _PTATT(attack, PAW_W | c) 
		else if (attack & R000BitB[KNI_W | c]) _PTATT(attack, KNI_W | c) 
		else if (attack & R000BitB[BIS_W | c]) _PTATT(attack, BIS_W | c) 
		else if (attack & R000BitB[ROO_W | c]) _PTATT(attack, ROO_W | c) 
		else if (attack & R000BitB[QUE_W | c]) _PTATT(attack, QUE_W | c) 
		else _PTATT(attack, KIN_W | c) 
	}
	return na;
}

int attacked(int i, int on_move) {
	if (aSrtAttack[i][PAWN[on_move]][1] & R000BitB[PAWN[on_move^1]]) return 1;
	if (aSrtAttack[i][KIN_W][1] & R000BitB[KING[on_move^1]]) return 1;
	if (aSrtAttack[i][KNI_W][1] & R000BitB[KNIGHT[on_move^1]]) return 1;
	if (((A000(i) | A090(i)) & QuRo) & R000BitB[on_move^1]) return 1;
  	if (((A045(i) | A135(i)) & QuBi) & R000BitB[on_move^1]) return 1;
	return 0;
}

u64 behindFigure(u64 a, int f, int dir) {
	switch (dir) {
		case -9: return a | (A045(f) & QuBi & DirA[1][f]);
		case -1: return a | (A000(f) & QuRo & DirA[2][f]);
		case 7: return a | (A135(f) & QuBi & DirA[3][f]);
		case 8: return a | (A090(f) & QuRo & DirA[4][f]);
		case 9: return a | (A045(f) & QuBi & DirA[5][f]);
		case 1: return a | (A000(f) & QuRo & DirA[6][f]);
		case -7: return a | (A135(f) & QuBi & DirA[7][f]);
		case -8: return a | (A090(f) & QuRo & DirA[8][f]);
		default: return a;
	}
}

int swap(Move move) {
	int to = TO(move);
	int from = FROM(move);
	int p = PIECE(move) | 1;
	u64 attack = attackbit(to) ^ _r000m[from];
	int sList[64];
	int c = ONMOVE(move) ^1;
	int lastV = -piece_val[p];
	int n = 1;
	int lsb, dir;
	sList[0] = c ? PSWAP(move) : -PSWAP(move);
	dir = direction[to][from];
	if (dir && p != KIN_B) attack = behindFigure(attack, from, dir);

	while (attack) {
		if (attack & R000BitB[PAW_W | c]) {lsb = LSB(attack & R000BitB[PAW_W | c]); p = PAW_B;}
		else if (attack & R000BitB[KNI_W | c]) {lsb = LSB(attack & R000BitB[KNI_W | c]); p = KNI_B;}
		else if (attack & R000BitB[BIS_W | c]) {lsb = LSB(attack & R000BitB[BIS_W | c]); p = BIS_B;}
		else if (attack & R000BitB[ROO_W | c]) {lsb = LSB(attack & R000BitB[ROO_W | c]); p = ROO_B;}
		else if (attack & R000BitB[QUE_W | c]) {lsb = LSB(attack & R000BitB[QUE_W | c]); p = QUE_B;}
		else if (attack & R000BitB[KIN_W | c]) {lsb = LSB(attack & R000BitB[KIN_W | c]); p = KIN_B;}
		else break;

		sList[n] = -sList[n-1] + lastV;
		lastV = -piece_val[p];
		attack ^= _r000m[lsb];
		dir = direction[to][lsb];
		if (dir && p != KIN_B) attack = behindFigure(attack, lsb, dir);
		n++;
		c ^= 1;
	}

	while (--n) {
		if (sList[n] > -sList[n-1]) sList[n-1] = -sList[n];
	}
	return sList[0];
}

#define NO3ATT(x, y, z, c) (!attacked(x, c) && !attacked(y, c) && !attacked(z, c))

void addcsts(int *number, int on_move, int ply) {
	if (on_move) {
		if (cstflag & CASTLE_SHORT_B) {
			if (!(NeiR[61] & P000) && NO3ATT(60, 61, 62, on_move))
				movelist[(*number)++][ply] = newmove(60,62,KIN_B,EMPTY,KIN_B,ply);
		}
		if (cstflag & CASTLE_LONG_B) {
			if (!(NeiB[58] & P000) && NO3ATT(60, 59, 58, on_move))
				movelist[(*number)++][ply] = newmove(60,58,KIN_B,EMPTY,KIN_B,ply);
		}
	} else {
		if (cstflag & CASTLE_SHORT_W) {
			if (!(NeiR[5] & P000) && NO3ATT(4, 5, 6, on_move))
				movelist[(*number)++][ply] = newmove(4,6,KIN_W,EMPTY,KIN_W,ply);
		}
		if (cstflag & CASTLE_LONG_W) {
			if (!(NeiB[2] & P000) && NO3ATT(4, 3, 2, on_move))
				movelist[(*number)++][ply] = newmove(4,2,KIN_W,EMPTY,KIN_W,ply);
		}
	}
}

#define CLRSETP(i) {P000 ^= _r000m[i]; P045 ^= _r045m[i]; P090 ^= _r090m[i]; P135 ^= _r135m[i];}

int is_bound(int i, int k, int on_move) {
	u64 bit; 
	int dir = direction[k][i];
	if (!dir) return 0;

	CLRSETP(i);
	bit = behindFigure(0, k, dir);
	CLRSETP(i);
	if (bit & R000BitB[on_move^1] & behindFigure(0, i, dir)) return 1;
	return 0;
}

#define IS_RANK1(x) (((x) & 0x38) == 0x00)
#define IS_RANK2(x) (((x) & 0x38) == 0x08)
#define IS_RANK4(x) (((x) & 0x38) == 0x18)
#define IS_RANK5(x) (((x) & 0x38) == 0x20)
#define IS_RANK7(x) (((x) & 0x38) == 0x30)
#define IS_RANK8(x) (((x) & 0x38) == 0x38)
#define IS_RANKL(x, y) (y ? IS_RANK1(x) : IS_RANK8(x))
#define IS_RANKM(x, y) (y ? IS_RANK4(x) : IS_RANK5(x))
#define ONEBACK(x, y) (y ? x + 0x08 : x - 0x08)
#define ONEFORW(x, y) (y ? x - 0x08 : x + 0x08)

int generate_check_esc(int on_move, int ply, int number) {
	int k = kingpos[on_move];
	int c = addattack(k, on_move^1, 1);
	int from = FROM(att[0]);
	int piece = PROMOTE(att[0]) | 1;
	u64 movebit= aSrtAttack[k][KIN_W][1] & (~R000BitB[on_move]);

	CLRSETP(k);
	while (movebit) {
		int n = LSB(movebit);
		movebit ^= _r000m[n];
		if (!attacked(n, on_move))
		movelist[number++][ply] = newmove(k, n, KIN_W | on_move, board[n], KIN_W | on_move, ply);
	}
	CLRSETP(k);

	if (c != 1) return number;
	c = addattack(from, on_move, 1);

	if (piece == PAW_B && IS_RANKM(from, on_move) && enpflag == ONEFORW(from, on_move)) {
		if ((NeiR[from] & R000BitB[PAW_W|on_move]) && !is_bound(from+1, k, on_move))
			movelist[number++][ply] = newmove(from+1,enpflag,PAWN[on_move],ENPAS[on_move^1],PAWN[on_move],ply);
		if ((NeiR[from-1] & R000BitB[PAW_W|on_move]) && !is_bound(from-1, k, on_move))
			movelist[number++][ply] = newmove(from-1,enpflag,PAWN[on_move],ENPAS[on_move^1],PAWN[on_move],ply);
	}

	for (;;) {
		while (c--) {
			int p = PROMOTE(att[c]);
			int i = FROM(att[c]);
			if ((p | 1) == KIN_B) continue;
			if (is_bound(i, k, on_move)) continue;

			if ((p | 1) == PAW_B && IS_RANKL(from, on_move)) {
				movelist[number++][ply] = newmove(i, from, p, board[from], QUEEN[on_move], ply);
				movelist[number++][ply] = newmove(i, from, p, board[from], KNIGHT[on_move], ply);
				movelist[number++][ply] = newmove(i, from, p, board[from], ROOK[on_move], ply);
				movelist[number++][ply] = newmove(i, from, p, board[from], BISHOP[on_move], ply);
			} else
				movelist[number++][ply] = newmove(i,from,p,board[from],p,ply);
		}

		if (piece == PAW_B || piece == KNI_B || piece == KIN_B) break;
		from = runto[k][from];
		if (from == -1) break;
		c = addattack(from, on_move, 0);
	
		{   int ip = ONEBACK(from, on_move);
			if (ip > 7 && ip < 56 && board[ip] == PAWN[on_move]) {
				att[c++] = ip | PAWN[on_move] << 12;
			}
			if (IS_RANKM(from, on_move^1)) {
				ip = ONEBACK(ip, on_move);
				if (ip > 7 && ip < 56 && board[ip] == PAWN[on_move]) {
					if (board[ONEBACK(from, on_move)] == EMPTY) {
						att[c++] = ip | PAWN[on_move] << 12;
					}
				}
			}
		}
	}	
	return number;
}

int generate_moves(int on_move, int ply, int caps) {
	int i, number=0;
	int p = PAWN[on_move];
    u64 attackedbit;
	u64 piecebit;

	u64 movebit, hbit = R000BitB[p];
	piecebit = R000BitB[on_move^1];
	if (!caps) piecebit |= Empty;

	while (hbit) { // First pawns
		i = LSB(hbit);
		hbit ^= _r000m[i];

		attackedbit = aSrtAttack[i][p][1] & (P000 | (enpflag ? _r000m[enpflag]:0));
		movebit = (attackedbit & (~R000BitB[on_move])) | (aSrtAttack[i][p][2] & Empty);
        if (!caps) {
			attackedbit = aSrtAttack[i][p][0] & Empty;
			if (attackedbit) attackedbit |= aSrtAttack[i][p][3] & Empty;
			movebit |= attackedbit;
		}
		
		while (movebit) {
			int msb = LSB(movebit);
			movebit ^= _r000m[msb];

			if (IS_RANKL(msb, on_move)) {	// Promotions
				movelist[number++][ply] = newmove(i,msb,p,board[msb],QUEEN[on_move],ply);
				movelist[number++][ply] = newmove(i,msb,p,board[msb],KNIGHT[on_move],ply);
				movelist[number++][ply] = newmove(i,msb,p,board[msb],ROOK[on_move],ply);
				movelist[number++][ply] = newmove(i,msb,p,board[msb],BISHOP[on_move],ply);
			} else if (((i ^ msb) & 0x07) && board[msb] == EMPTY) {  // Enpassant
				movelist[number++][ply] = newmove(i,msb,p,ENPAS[on_move^1],p,ply);
			} else { // Standard
				movelist[number++][ply] = newmove(i,msb,p,board[msb],p,ply);
			}
		}
	}	
	
	hbit=R000BitB[on_move] & (~R000BitB[p]);
	while (hbit) { // The rest
		i = LSB(hbit);
		hbit ^= _r000m[i];
		p = board[i];

        if (p == QUEEN[on_move]) movebit = (A000(i) | A090(i) | A045(i) | A135(i)) & piecebit;
        else if (p == ROOK[on_move]) movebit = (A000(i) | A090(i)) & piecebit;
        else if (p == BISHOP[on_move]) movebit = (A045(i) | A135(i)) & piecebit;
        else movebit = aSrtAttack[i][p][1] & piecebit;

		while (movebit) {
			int a = LSB(movebit);
			movebit ^= _r000m[a];
			movelist[number++][ply] = newmove(i, a, p, board[a], p, ply);
		}
	} 
	if (!caps && cstflag) addcsts(&number, on_move, ply);
	return number;
}

void upBitboards(Move move) {
	int from = FROM(move);
	int to = TO(move);
	int piece = PIECE(move);
	int p_to = P_TO(move);
	int promote = PROMOTE(move);
	int color = ONMOVE(move);

	hashBoard ^= hashMAP[to][piece];
	hashBoard ^= hashMAP[from][piece];
	hashPawn ^= pawnMAP[to][piece];
	hashPawn ^= pawnMAP[from][piece];

	R000BitB[piece] ^= __r000m[from][to];
	R000BitB[color] ^= __r000m[from][to];	
	R045BitB[piece] ^= __r045m[from][to];
	R045BitB[color] ^= __r045m[from][to];
	R090BitB[piece] ^= __r090m[from][to];
	R090BitB[color] ^= __r090m[from][to];
	R135BitB[piece] ^= __r135m[from][to];
	R135BitB[color] ^= __r135m[from][to];
	
	if ((p_to | 1) == ENP_B) {
		p_to = PAWN[color^1];
		to = ONEBACK(to, color);
	}
	if (p_to != EMPTY) {
		hashBoard ^= hashMAP[to][p_to];
		hashPawn ^= pawnMAP[to][p_to];
		UPBITB(p_to, to);
		UPBITB(color^1, to);
	}
	if (piece != promote) {
		hashBoard ^= hashMAP[to][piece];
		hashBoard ^= hashMAP[to][promote];
		hashPawn ^= pawnMAP[to][piece];
		UPBITB(piece, to);
		UPBITB(promote, to);
	}
	upBitHelpers();
}

void domove(Move move) {
	if (P_WH(move) == KIN_W) {
		rookmoves(FROM(move), TO(move), PIECE(move), 1);
		if (ONMOVE(move) == COL_W) kingpos[COL_W] = TO(move);
		else kingpos[COL_B] = TO(move);
	}
	
	board[FROM(move)] = EMPTY;
	board[TO(move)] = PROMOTE(move);
	if ((P_TO(move)|1) == ENP_B) {
		board[ONEBACK(TO(move), ONMOVE(move))] = EMPTY;
	}

	material += PSWAP(move);

	hashBoard ^= hashMAP[cstflag][14];
	hashBoard ^= hashMAP[enpflag][15];

	enpflag = 0;
	if (PIECE(move) == PAW_W && IS_RANK2(FROM(move)) && IS_RANK4(TO(move))) {	
		if (NeiB[TO(move)] & R000BitB[PAW_B]) enpflag = TO(move) - 0x08;
	}
	else if (PIECE(move) == PAW_B && IS_RANK7(FROM(move)) && IS_RANK5(TO(move))) {
		if (NeiB[TO(move)] & R000BitB[PAW_W]) enpflag = TO(move) + 0x08;
	}

	if (cstflag) {
		if (FROM(move) == 4) cstflag &= (~(CASTLE_SHORT_W | CASTLE_LONG_W));
		if (FROM(move) == 60) cstflag &= (~(CASTLE_SHORT_B | CASTLE_LONG_B));
		if (FROM(move) == 0) cstflag &= (~CASTLE_LONG_W);
		if (FROM(move) == 7) cstflag &= (~CASTLE_SHORT_W);
		if (FROM(move) == 56) cstflag &= (~CASTLE_LONG_B);
		if (FROM(move) == 63) cstflag &= (~CASTLE_SHORT_B);
		if (TO(move) == 0) cstflag &= (~CASTLE_LONG_W);
		if (TO(move) == 7) cstflag &= (~CASTLE_SHORT_W);
		if (TO(move) == 56) cstflag &= (~CASTLE_LONG_B);
		if (TO(move) == 63) cstflag &= (~CASTLE_SHORT_B);
	}

	hashBoard ^= hashMAP[cstflag][14];
	hashBoard ^= hashMAP[enpflag][15];
	hashBoard ^= hashMAP[0][0];

	upBitboards(move);
	movestack[nstack++] = move;
	hashstack[nstack] = hashBoard;
	enpstack[nstack] = enpflag;
	cststack[nstack] = cstflag;
}

void donullmove() {
	hashBoard ^= hashMAP[enpflag][15];
	enpflag = 0;
	hashBoard ^= hashMAP[0][15];
	hashBoard ^= hashMAP[0][0];
	movestack[nstack++] = 0;
	hashstack[nstack] = hashBoard;
	enpstack[nstack] = enpflag;
	cststack[nstack] = cstflag;
}

void undonullmove() {
	hashBoard = hashstack[--nstack];
	enpflag = enpstack[nstack];
	cstflag = cststack[nstack];
}

void undomove() {
	Move move = movestack[--nstack];

	if (P_WH(move) == KIN_W) {
		rookmoves(FROM(move), TO(move), PIECE(move),0);
		if (ONMOVE(move) == COL_W) kingpos[COL_W] = FROM(move);
		else kingpos[COL_B] = FROM(move);
	}

	board[FROM(move)] = PIECE(move);

	if ((P_TO(move)|1) == ENP_B) {
		board[ONEBACK(TO(move), ONMOVE(move))] = PAWN[ONMOVE(move)^1];
		board[TO(move)] = EMPTY;
	} else {
		board[TO(move)] = P_TO(move);
	}

	material -= PSWAP(move);
	upBitboards(move);
	hashBoard = hashstack[nstack];
	enpflag = enpstack[nstack];
	cstflag = cststack[nstack];
}

int generate_legal_moves(int on_move, int p) {
	int i, n = generate_moves(on_move, p, 0);
	for (i = 0; i < n; i++) {
		domove(movelist[i][p]);
		if (attacked(kingpos[on_move], on_move)) {
			movelist[i--][p]=movelist[--n][p];
		}
		undomove();
	}
	return n;
}

void rookmoves(int from, int to, int p, int domov) {
	Move rookmove = 0;
	if (p == KIN_W && to == 2 && from == 4) {
		rookmove = newmove(0, 3, ROO_W, EMPTY, ROO_W, 0);
	} else if (p == KIN_W && to == 6 && from == 4) {
		rookmove = newmove(7, 5, ROO_W, EMPTY, ROO_W, 0);
	} else if (p == KIN_B && to == 58 && from == 60) {
		rookmove = newmove(56, 59, ROO_B, EMPTY, ROO_B, 0);
	} else if (p == KIN_B && to==62 && from==60) {
		rookmove = newmove(63, 61, ROO_B, EMPTY, ROO_B, 0);
	}
	if (rookmove == 0) return;
	hashBoard ^= hashMAP[0][0];
	if (domov) { 
		domove(rookmove);
		nstack--;
		return;
	}
	movestack[nstack++] = rookmove;
	undomove();
}

int lookUpH(int depth, int *w, Move *move) {
	HashREC *h = hashREC + (hashBoard & hashMask);
	if (h->rec != hashBoard && (++h)->rec != hashBoard) return 0;
	*move = h->move;
	if (h->depth == depth) {
		*w = h->w;
		return h->flag;
	}
	return 0;
}

void storeH(Move move, int depth, int w, int flag) {
	HashREC *h = hashREC + (hashBoard & hashMask);
	if (depth < h->depth) h++; else if (h->move) *(h+1) = *h;

	h->w = (short)w;
	h->depth = (char)depth;
	h->move = move;
	h->flag = (char)flag;
	h->rec = hashBoard;
}

int plookUpH(int *w) {
	HashREC *h = pawnREC + (hashPawn & pawnMask);
	if (h->rec != hashPawn) return 0;
	*w = h->w;
	return 1;
}

void pstoreH(int w) {
	HashREC *h = pawnREC + (hashPawn & pawnMask);
	h->w = (short)w;
	h->rec = hashPawn;
}

int pawneval(u64 pawn, u64 xpawn, int c) {
	u64 bit = pawn;
	int pval = 0;
	int i, f;

	while (bit) {
		i = LSB(bit);
		bit ^= _r000m[i];
		f = (LineT[i] & Bef[c][0][i] & xpawn) ? 3 : 6; // Lola run
		pval += f * pawn_val[i][c];
		if (!(LineD[i] & Bef[c^1][0][i] & pawn)) {  // Last Pawn
			if (!(LineS[i] & xpawn & Bef[c][1][i]))  // ...on free file
				pval += 4 * pawn_val[i][c^1];
			if (!(LineD[i] & pawn & NeiB[ONEFORW(i, c)])) // Lonely
				pval += 2 * pawn_val[i][c^1];
		}
		if (!(LineD[i] & (~Bef[c^1][2][i]) & pawn)) { // Pawn 2 fields for
			pval += pawn_val[i][c^1];
			if (!(LineD[i] & (~Bef[c^1][3][i]) & pawn)) // even 3 fields
				pval += 2*pawn_val[i][c^1];
		}
		if (LineS[i] & Bef[c][1][i] & pawn)  // Double Pawn
			pval += pawn_val[i][c^1];
	}
	return pval;
}

int eval(int on_move, int alpha, int beta) {
	int i, j, c, p, poseval;
	u64 hbit;
	int teval = on_move ? -material : material;
	if (teval - 150 >= beta) return beta;
	if (teval + 150 <= alpha) return alpha;
	neval ++;
	
	if (plookUpH(&poseval) != 1) {
		poseval = pawneval(R000BitB[PAW_W], R000BitB[PAW_B], COL_W);
		poseval += pawneval(R000BitB[PAW_B], R000BitB[PAW_W], COL_B);
		pstoreH(poseval);
	} 
	poseval += material;

	hbit = P000 & ~(R000BitB[PAW_W] | R000BitB[PAW_B]);
	while (hbit) {		
		i = LSB(hbit);
		hbit ^= _r000m[i];
		j = board[i];
		p = j | 1;
		c = j & 1;
			if (p == KNI_B) { poseval += knightmobil[i] * pos_val[j]; continue; }
			if (p == BIS_B) { poseval += bitcount(A045(i) | A135(i)) * pos_val[j]; continue; }
			if (p == ROO_B) { poseval += bitcount(A000(i) | A090(i)) * pos_val[j]; continue; }
			if (p == KIN_B) { poseval -= kingmobil[i] * pos_val[j ^ (!R000BitB[QUEEN[c^1]])]; }
	}

	if (!R000BitB[PAW_W] && poseval > 0) {
		if (!(QuRo & R000BitB[COL_W]) && bitcount(R000BitB[COL_W]) < 3) return 0;
	} else if (!R000BitB[PAW_B] && poseval < 0) {
		if (!(QuRo & R000BitB[COL_B]) && bitcount(R000BitB[COL_B]) < 3) return 0;
	}

	return on_move ? - poseval : poseval;
}

int checkfordraw(int two) {
	int i, j;	
	for (i = nstack - 2; i > 0; i--) {
		if (P_TO(movestack[i]) != EMPTY || P_WH(movestack[i]) == PAW_W) return 0;
		if (nstack - i > 100) return 2;
		if (hashstack[i] == hashBoard) {
			if (two) return 1;
			for (j = i - 1; j > 0; j--) {
				if (P_TO(movestack[j]) != EMPTY || P_WH(movestack[j]) == PAW_W) return 0;
				if (nstack - j > 100) return 2;
				if (hashstack[j] == hashBoard) return 1;
			}
		}
	}
	return 0;
}

Move pick(int ply, int n) {
	Move move;
	int i, imax=0, max = 0;
	for (i = 0; i < n; i++) {
		if (VAL(movelist[i][ply]) >= max) {
			imax = i;
			max = VAL(movelist[i][ply]);
		}
	}
	move = movelist[imax][ply];
	movelist[imax][ply] = movelist[n-1][ply];
	return move;
}

int bioskey() { return 0; }

int search(int on_move, int ply, int depth, int alpha, int beta, int do_null)
{ 
	int n, i, j, w, c, hflag, nolegal;
	Move bestmove = 0, move = 0;
  long ms = get_time();
  
	if ((++nodes & 0x1fff) == 0) { 
		if (bioskey()) inputmove(pmove ? ONMOVE(pmove) : COL_N);
	
    if(!pmove && (get_time()-ms)>mytime) { sabort = 1; return alpha; }		// Timeout stops calculations 
		}
	
	if (ply && depth && checkfordraw(1)) return 0;
	pvlength[ply] = ply;

	c = attacked(kingpos[on_move], on_move);
	if (c) depth++;
	else if (nstack && IS_PROM(movestack[nstack-1])) depth++;
	if (depth < 0) depth = 0;

	hflag = lookUpH(depth, &w, &move);
	if (ply && hflag) {
		if (hflag == 1) if (w >= beta) return beta;
		if (hflag == 2) { do_null = 0; if (w < alpha) return alpha; }
	}

	if (!c && do_null && depth && ply && bitcount(R000BitB[on_move] & (~R000BitB[PAWN[on_move]])) > 3) {
		donullmove();
		w = -search(on_move^1, ply+1, depth - 3 - (depth > 5), -beta, -beta + 1, 0);
		undonullmove();
		if (w >= beta) {
			storeH(0, depth, w, 1);
			return w;
		}
	}

	if (!ply) move = pv[0][0];
	hashmv[ply] = move;
	hflag = move ? 1 : 0;
	nolegal = 1;
	n = 0;

	if (depth == 0 && !c) {
		w = eval(on_move, alpha, beta);
		if (w > alpha) alpha = w;
		if (alpha >= beta) return beta;
		n = generate_moves(on_move, ply, 1);
		if (n == 0) return w;
		hflag = 0;
	} 

	i = 0;
	do {
		if (i == 1) hflag = 0;
		if (n == 0 && !hflag) {
			if (c) 	n = generate_check_esc(on_move, ply, 0);
			else n = generate_moves(on_move, ply, 0);

			if (i == 1) {
					for (j = 0; j < n; j++) {
						if (IDENTMV(movelist[j][ply], hashmv[ply])) {
							movelist[j][ply] = movelist[n-1][ply];
							break;
						}
					}
				}
			if (n == 0) break;
			}
		if (!hflag) move = pick(ply, n-i);
		if (!depth && !c && P_TO(move) != EMPTY && swap(move) < -150) continue;

		domove(move);
		if (!c && attacked(kingpos[on_move], on_move)) { undomove(); continue;}
		w = -search(on_move^1, ply+1, depth-1, -beta, -alpha, 1);
		if (nolegal) nolegal=0;
		undomove();
		if (sabort) break;

		if (w >= beta)  {
			if (P_TO(move) == EMPTY)  { 
			  killer2[ply] = killer[ply];
			  killer[ply] = move;
			}
			history[move & 0xFFFF] += depth;
			storeH(move, depth, w, 1);
			return beta;
		}
		if (w > alpha) {
			pv[ply][ply] = bestmove = move;
			for (j = ply+1; j < pvlength[ply+1]; j++) pv[ply][j] = pv[ply+1][j];
			pvlength[ply] = pvlength[ply+1];			
			alpha = w;
			if (alpha == 32499 - ply) break; 
		} 
	} while (++i < n || hflag);
	if (sabort) return alpha;

	if (nolegal) {
		if (c) return -32500 + ply;
		else if (depth) return 0;
	} else
	storeH(bestmove, depth, alpha, bestmove ? 4 : 2);
	return alpha;
}

Move parseMove(char *s, int on_move, int n)
{
  Move move;
  char c = 0;
  int i, to;
  int piece = -1;
  int from_rank = -1;
  int from_file = -1;
  int to_rank = -1;
  int to_file = -1;
  int promote = -1;

  if (*s >= 'A' && *s <= 'Z') c = lookforP(*s);
  if (c) { piece = c | on_move; s++; }
  else if (*s != 'O') {
     	if (*s > 'h' || *s < 'a') return 0;
  }

  if (*s == 'x') s++;
  else {
    if (*s < 'a' || *s > 'h' || s[1] < '1' || s[1] > '8') {
      	if (*s >= 'a' && *s <= 'h') from_file = *s++ - 'a';
      	else if (*s >= '1' && *s <= '8') from_rank = *s++ - '1';
    }
    if (*s == 'x') s++;
  }
  if (*s >= 'a' && *s <= 'h') to_file = *s++ - 'a';
  if (*s >= '1' && *s <= '8') to_rank = *s++ - '1';
  if (*s == '-' || *s == 'x' || *s == '=') s++;
  if (*s >= 'a' && *s <= 'h') { 
	from_file = to_file; to_file = *s++ - 'a';
    	if (*s >= '1' && *s <= '8') { from_rank = to_rank; to_rank = *s++ - '1';}
  }
  c = lookforP(*s);
  if (c && *s) promote = (c & 0xE) | on_move;

  to = to_file + to_rank*8;

  if (!strncmp(s,"O-O-O",5))
  {
    if (!on_move) { to = 2; piece=KIN_W; }
    else { to = 58; piece = KIN_B; }
  }
  else if (!strncmp(s,"O-O",3))
  {
    if (!on_move) { to = 6; piece=KIN_W; }
    else { to = 62; piece = KIN_B; }
  }

  if (piece == -1 && (from_rank == -1 || from_file == -1)) piece = PAWN[on_move];
  if (promote == -1 && piece >= 0) promote = piece;
  
  if (!n) n = generate_legal_moves(on_move, 63);

  for (i = 0; i < n; i++)
  {
    move = movelist[i][63];
    if (to == TO(move) && (piece == -1 || piece == PIECE(move)) 
		       && (promote == -1 || promote == PROMOTE(move)) ) {
      if (from_rank >= 0 && (FROM(move) / 8) != from_rank) continue;
      if (from_file >= 0 && (FROM(move) & 7) != from_file) continue;

      return move;
    }
  }
  return 0;
}

void parsePGN() {
	int bcolor; Move move;
	char inbuf[1024];
	FILE *fpgn = fopen("book.pgn", "r");
	if (fpgn == 0) return; else printf(" Parsing book...");
	while (!feof(fpgn)) {
		fscanf(fpgn, "%s", inbuf);	
		if (!strncmp(inbuf, "[Result", 7)) {
			fscanf(fpgn, "%s", inbuf);
			bcolor = COL_N;
			if (!strncmp(inbuf, "\"1-0\"", 5)) bcolor = COL_W;
			if (!strncmp(inbuf, "\"0-1\"", 5)) bcolor = COL_B;
			if (bcolor == COL_N) continue;
			nstack = -1;
			while (!feof(fpgn)) {
				fscanf(fpgn, "%s", inbuf);
				if (!strncmp(inbuf, "\"1-0\"", 5)) break;
				if (!strncmp(inbuf, "\"0-1\"", 5)) break;
				if (nstack == -1 && !strncmp(inbuf, "1.", 2)) init_board(boardStr);
				if (nstack >= 0) {
				   fscanf(fpgn, "%s", inbuf);
				   move = parseMove(inbuf, COL_W, 0); 
				   if (!move) break;
				   if (bcolor == COL_W) storeH(move, 99, 0, 8);
				   domove(move);
				   fscanf(fpgn, "%s", inbuf);
				   move = parseMove(inbuf, COL_B, 0);
				   if (!move) break;
				   if (bcolor == COL_B) storeH(move, 99, 0, 8);
				   domove(move);
				   if (nstack > 20) break;
				}
			}
		}
	} 
	fclose(fpgn);
	printf("done\n");
}

int calcmove(int on_move)
{
  Move move; int n = 0, w, depth = 0, alpha, beta;
  long ms = get_time();
	nodes = neval = sabort = 0;

	memset(history, 0, sizeof(history));
	alpha = -32500;
	beta = 32500;
	pv[0][0] = 0;

	if (pmove || lookUpH(99, &n, &move) != 8) move = 0; 

  if (!pmove) move = movelist[0][62];

	if (!move) for (depth = 1; depth <= sd; depth++) {

		w = search(on_move, 0, depth, alpha, beta, 1);
		if (w >= beta) w = search(on_move, 0, depth, w - 100, 32500, 1);
		if (w <= alpha) w = search(on_move, 0, depth, -32500, w + 100, 1);
		if (sabort && pvlength[0] == 0) break; else n = w;

		alpha = n - 70;
		beta = n + 70;
		move = pv[0][0];
		if (dopost) {
			printf("%2d %5d %6d %9d  ",
			depth,
			n,
			(get_time() - ms)/10,
			nodes);
			displaypv(); printf("\n");
		}
		fflush(stdout);
		
		if (sabort || (!pmove && ((get_time()-ms)>mytime))) { sabort = 1; break; }
	}
	ms = get_time()-ms;
	
	if(move && (get_time()-ms)>mytime) sabort = 1;		// Timeout stops calculations 
		
	if (ms<2) ms=2;

	while (pmove && !sabort) inputmove(ONMOVE(pmove));

	if (icmd) {
		if (pmove && icmd < 7) { undomove(); return on_move^1; }
		return on_move;
	}

	if (pmove) {
		undomove();
		domove(pmove);
		pmove = 0;
		return calcmove(on_move);
	}
	domove(move);
	
	printf("%d. ... ", (nstack+1)/2);
	displaym(move); printf("\n");
	fflush(stdout);
	
	if (nodes) nps = nodes/ms * 1000 + 1;
	printf("%s %d(%d) %d nds %d nps %d ms %d evs\n", comp ? "kibitz" : "whisper", 
		n, depth, nodes, nps, ms, neval);

	if (!xboard) displayb();

	pnumber = doponder ? generate_legal_moves(on_move^1, 63) : 0;
	if (pnumber) {
		for (n = 0; n < pnumber; n++) {
			if (IDENTMV(movelist[n][63], pv[0][1])) break;
		}
		if (n == pnumber) { pnumber = 0; return on_move^1; }
		pmove = movelist[n][63];
		domove(pmove);
		printf("%s pondering: ", comp ? "kibitz" : "whisper"); displaym(pmove); printf("\n");
	}
	return on_move^1;
}

int inputmove(int on_move)
{
	Move move; int nm, iedit = COL_N; 
	char buf[256];
	switch (icmd) {
		case 1: return init_board(boardStr);
		case 2: undomove(); undomove(); pmove = 0; return on_move;
		case 3: machine = COL_B; pmove = 0; return COL_W;
		case 4: machine = COL_W; pmove = 0; return COL_B;
		case 5: machine = COL_N; pmove = 0; displayb(); return on_move;
		case 6: machine = on_move; pmove = 0; return on_move;
		case 7: undomove(); pnumber = generate_legal_moves(on_move, 63); return on_move^1;
		case 8: pmove = 0; return on_move;
		case 9: domove(pmove); pmove = 1; pnumber = generate_legal_moves(on_move^1, 63); return on_move;
	}
	do {
		nm = 0;
		fgets(buf,255,stdin);
		if (!strncmp(buf,"?",1)) { if (!pmove) sabort = 1; return on_move; }
		if (!strncmp(buf,".",1)) { if (iedit != COL_N) init_board(0); else return on_move;}
		if (!strncmp(buf,"#",1) && iedit != COL_N) for (nm = 0; nm < 64; nm++) board[nm] = EMPTY;
		if (!strncmp(buf,"c",1) && iedit != COL_N) iedit = COL_B;
		if (strlen(buf) < 2) return on_move;
		
		if (!strncmp(buf,"xboard",6)) { xboard = 1; printf("feature done=1\n"); return on_move; }
		if (!strncmp(buf,"quit",4)) exit(0);
		if (!strncmp(buf,"time",4)) { sscanf(buf+5,"%d",&mytime); nm = 1; }
		if (!strncmp(buf,"hint", 4)) { printf("Hint: "); displaym(pmove); printf("\n"); fflush(stdout); return on_move; }

		if (!strncmp(buf,"computer",8)) { comp = 1; nm = 1; }
		if (!strncmp(buf,"hard",4)) { doponder = 1; nm = 1; }
		if (!strncmp(buf,"easy",4)) { doponder = 0; nm = 1; }
		if (!strncmp(buf,"post",4)) { dopost = 1; nm = 1; }
		if (!strncmp(buf,"nopost",6)) { dopost = 0; nm = 1; }

		if (!strncmp(buf,"protover",8)) nm = 1;
		if (!strncmp(buf,"accepted",8)) nm = 1;
		if (!strncmp(buf,"random",6)) nm = 1;
		if (!strncmp(buf,"level",5)) nm = 1;
		if (!strncmp(buf,"otim",4)) nm = 1;
		if (!strncmp(buf,"result",6)) nm = 1;
		if (!strncmp(buf,"name",4)) nm = 1;
		if (!strncmp(buf,"rating",6)) nm = 1;

		if (!strncmp(buf,"edit",4)) { iedit = COL_W; continue; }
		if (!strncmp(buf,"draw",4)) return on_move;
		if (!strncmp(buf,"analyze",7)) { if (!pmove) pnumber = generate_legal_moves(on_move, 63); pmove = 1; return on_move^1; }

		if (!strncmp(buf,"new",3)) icmd = 1;
		if (!strncmp(buf,"remove",6)) icmd = 2;
		if (!strncmp(buf,"white",2)) icmd = 3;
		if (!strncmp(buf,"black",2)) icmd = 4;
		if (!strncmp(buf,"force",5)) icmd = 5;
		if (!strncmp(buf,"go",2)) icmd = 6;
		if (!strncmp(buf,"undo",4)) icmd = 7;
		if (!strncmp(buf,"exit",4)) icmd = 8;

		if (icmd) { sabort = 1; return on_move; }
		if (iedit != COL_N && strlen(buf) > 2) 
			board[buf[1] - 'a' + (buf[2] - '1')*8] = lookforP(buf[0]) | iedit;

	} while (nm || iedit != COL_N);
	if (on_move == COL_N) return COL_N;

	move = parseMove(buf, on_move, pmove ? pnumber : 0);
	if (move) {
		if (pmove) {
			if (pmove == 1) icmd = 9;
			if (IDENTMV(move, pmove)) pmove = 0;
			else { sabort = 1; pmove = move; }
			return on_move;
		}
		domove(move);
		return on_move^1;
	}
	fprintf(stderr,"Illegal move: %s\n",buf);
	fflush(stderr);
	return on_move;
}

int main(int arg_c, char *arg_v[])
{
	int on_move;
	printf("Chess - OliThink 4.1.1\n");
	signal(SIGINT, SIG_IGN);
	init_arrays();


  doponder = 0;
  dopost = 0;

	parsePGN();
	sd = 7;						/* default should be  sd = 40 */
	mytime = 12000;
	on_move = init_board(boardStr);
	nps = 100000;

    while (nstack < 950) { icmd = 0;
    
    // autoplay for testing...
    //if (on_move != machine) machine = 1-machine;
    
		if (on_move == machine) on_move = calcmove(on_move);
		else on_move = pmove ? calcmove(on_move^1) : inputmove(on_move);

		if (icmd) on_move = inputmove(on_move);

		switch (checkfordraw(0)) {
			case 1: printf("draw\nDrawn by repitition!\n"); break;
			case 2: printf("draw\nDrawn by 50 moves rule!\n");
		}
		fflush (stdout);
	}	
    return 0;
}
