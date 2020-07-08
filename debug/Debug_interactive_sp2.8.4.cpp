#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <memory.h>
#include <time.h>
#include <string>
#include <sstream>
#include <cassert>
#include <random>
#include <unordered_map>
#include <stack>
#include <windows.h>

typedef unsigned int U32;
using namespace std;

struct node
{
	int count;
	int realcount;
	int depth;
	U32 piece[16];
	node* parent;
	vector<node*> child;
	int move[2];
	int chessmove[2];
	bool visit;
	int alpha;
	int beta;
};
node* currentParent;
node* currentChild;

U32 LS1B(U32 x) { return x & (-x); }//取得x的最低位元
U32 MS1B(U32 x) { // Most Significant 1 Bit (LS1B)函式
	x |= x >> 32; x |= x >> 16; x |= x >> 8;
	x |= x >> 4; x |= x >> 2; x |= x >> 1;
	return (x >> 1) + 1; //可以取得一串位元中最左邊不為零的位元
}
U32 CGen(int ssrc, U32 toccupied);//返還炮位 
U32 CGenCR(U32 x);
U32 CGenCL(U32 x);
int BitsHash(U32 x) { return (x * 0x08ED2BE6) >> 27; }
void initial();//初始化 
void chess(U32 tpiece[16], int deep);//尋找可用移動
void ai2();//ai ver2.0 讚 使用search 
void readBoard();//讀檔模式 讀取board.txt 把讀入檔案轉成bitboard 還沒倒著存入 
void createMovetxt();//創造move.txt 0走步 1翻棋 
void IndexToBoard(int indexa, int indexb);//把src dst從編號0~31->棋盤編號a1~d4 
int countAva(int pie[14], int deep, U32 curPiece[16]);//呼叫則傳回當前棋版
int search(int depth, U32 curPiece[16], int curPie[14], int alpha, int beta, U32 hashvalue);//搜尋最佳走步 
void dynamicPower(int curPie[14]);//計算動態棋力 
void drawOrNot();//由past_walk判斷是否平手 之後結果輸出給draw 
int findPiece(int place, U32 curPiece[16]);//傳編號 回傳在這個編號的棋子 
void printDebug();
void translate(int a);
void set_console_color(unsigned short color_index);
string index(int indexa);
U32 myhash(U32 tpiece[16]);//傳入盤面，傳回hash值

int index32[32] = { 31, 0, 1, 5, 2, 16, 27, 6, 3, 14, 17, 19, 28, 11, 7, 21, 30, 4, 15, 26, 13,
18, 10, 20, 29, 25, 12, 9, 24, 8, 23, 22 };
int GetIndex(U32 mask) { return index32[BitsHash(mask)]; }//輸入遮罩返回棋盤編號
U32 pMoves[32] = { 0x00000012,0x00000025,0x0000004A,0x00000084,0x00000121,0x00000252,0x000004A4,0x00000848,
0x00001210,0x00002520,0x00004A40,0x00008480,0x00012100,0x00025200,0x0004A400,0x00084800,
0x00121000,0x00252000,0x004A4000,0x00848000,0x01210000,0x02520000,0x04A40000,0x08480000,
0x12100000,0x25200000,0x4A400000,0x84800000,0x21000000,0x52000000,0xA4000000,0x48000000 };//棋子移動遮罩
U32 pMoves2[32] = {
0x00000116,0x0000022D,0x0000044B,0x00000886,0x00001161,0x000022D2,0x000044B4,0x00008868,
0x00011611,0x00022D22,0x00044B44,0x00088688,0x00116110,0x0022D220,0x0044B440,0x00886880,
0x01161100,0x022D2200,0x044B4400,0x08868800,0x11611000,0x22D22000,0x44B44000,0x88688000,
0x16110000,0x2D220000,0x4B440000,0x86880000,0x61100000,0xD2200000,0xB4400000,0x68800000 };//翻棋遮罩
U32 pMoves3[32] = {
0x00000136,0x0000027D,0x000004EB,0x000008C6,0x00001363,0x000027D7,0x00004EB4,0x00008C6C,
0x00013631,0x00027D72,0x0004EBE4,0x0008C6C8,0x00136310,0x0027D720,0x004EBE40,0x008C6C80,
0x01363100,0x027D7200,0x04EBE400,0x08C6C800,0x13631000,0x27D72000,0x4EBE4000,0x8C6C8000,
0x36310000,0x7D720000,0xEBE40000,0xC6C80000,0x63100000,0xD7200000,0xBE400000,0x6C800000 };//翻棋遮罩
U32 file[4] = { 0x11111111,0x22222222,0x44444444,0x88888888 };//行遮罩 
U32 row[8] = { 0x0000000F,0x000000F0,0x00000F00,0x0000F000,0x000F0000,0x00F00000,0x0F000000,0xF0000000 };//列遮罩 
U32 piece[16]; //0空格- 帥k 士g 相m 車r 馬n 炮c 兵p *2 15未翻x 
U32 red, black, occupied;//紅 黑 有棋子 
int influenceValue[7][7] = {
	120,108,60,36,24,48,0,
	0,106,60,36,24,48,12,
	0,0,58,36,24,48,12,
	0,0,0,34,24,48,12,
	0,0,0,0,22,48,12,
	0,0,0,0,0,0,0,
	108,0,0,0,0,0,10
};

string move = "a1-a1";//下一步行動 用於背景 
int piece_count[14] = { 1,2,2,2,2,2,5,1,2,2,2,2,2,5 };//剩餘棋子數 0-6 7-13
int DCount[14] = { 1,2,2,2,2,2,5,1,2,2,2,2,2,5 };//剩餘未翻子 
string current_position[32];//盤面狀況總覽 
string history;
int timeCount;//剩餘時間 
int initailBoard = 1;//是否讀取初始版面 
int past_walk[7][2] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 };//前七步 用來處理平局問題 
int piPw[14] = { 6000,2700,900,400,150,2000,100,6000,2700,900,400,150,2000,100 };//棋子分數 
int draw = 0;//0無平手情況 1有可能進入平手 
//int RMcount=13;//讀取模式需要 從13行開始讀取結果 

vector<vector<node*>> nodes;
vector<int> finalScore;
vector<int> finalsrc;
vector<int> finaldst;
vector<int> depthrecord;
int nodemove[2] = { -1, -1 };

U32 allEatMove[50][2];//存可吃子的方法 0 src 1 dst 可執行 
int AEMindex = 0;//alleatmove index
U32 allOnlyMove[50][2];//存可移動非吃子的方法 0 src 1 dst
int AOMindex = 0;//allonlymove index
U32 EallEatMove[50][2];//存對手可吃子的方法 0 src 1 dst 下回合才有可能執行 
int EAEMindex = 0;//Ealleatmove index
U32 EallOnlyMove[50][2];//存對手可移動非吃子的方法 0 src 1 dst
int EAOMindex = 0;//Eallonlymove index

int color;//0 red 1 black
string src, dst;//棋盤編號版 a1~d4
int srci, dsti;//index版 0~31
int maxDepth = 4;
int moveMaxDepth = 8;
U32 open = 0xffffffff;//非未翻棋
U32 ch;//需要search的位置 
int noReDepth = 1;
U32 randtable[15][32]; //0~13 為雙方兵種 14 未翻
struct hashdata {
	int count; int depth; U32 curPiece[16];
};
unordered_map<unsigned int, hashdata > hashtable;
unordered_map<unsigned int, hashdata > hashtablef;
int test = 0;
int testa = 0;
int testb = 0;

int main()
{
	clock_t start, stop;
	start = clock();
	srand(time(NULL));
	initial();//初始化 
	readBoard();
	drawOrNot();
	int onboardi = 0;//計算場面上未翻棋數量 
	for (int i = 0; i < 14; i++)
	{
		onboardi += DCount[i];
	}
	if (onboardi == 0)maxDepth = 10;
	//else if (onboardi < 27)maxDepth = 6;
	nodes.resize(maxDepth + 1);
	int end = 1;//結束則=0
	while (end)
	{
		dst = "0";
		ai2();//決定行動 
		stop = clock();
		cout << " 此步耗時 : " << double(stop - start) / CLOCKS_PER_SEC << " 秒(精準度0.001秒) " << endl;
		cout << " 本方為 : ";
		if (color == 0) cout << "紅 " << endl;
		else cout << "黑 " << endl;
		createMovetxt();
		end = 0;
	}
	int temp = 0;
	for (int i = 0; i < nodes.size(); i++)
	{
		for (int k = 0; k < nodes[i].size(); k++)
		{
			if (nodes[i][k]->depth == 0)
				temp++;
		}
	}
	printDebug();
	cout << test << endl;
	cout << testa << endl;
	cout << testb << endl;
}

void ai2()
{
	open = 0xffffffff;
	ch = 0x00000000;
	open ^= piece[15];//非未翻棋
	while (open)
	{
		U32 take = LS1B(open);
		open ^= take;
		take = GetIndex(take);
		ch |= pMoves3[take];
	}
	if (piece[15] == 0xffffffff)
	{
		IndexToBoard(9, 9);
		return;
	}
	chess(piece, 0);
	U32 hashvalue = myhash(piece);
	search(0, piece, piece_count, -999999, 999999, hashvalue);
	IndexToBoard(srci, dsti);
}

void chess(U32 tpiece[16], int deep)
{
	U32 tred, tblack;
	U32 dest;//可以吃子的行動
	AEMindex = 0;
	AOMindex = 0;
	EAEMindex = 0;
	EAOMindex = 0;
	U32 toccupied = 0xFFFFFFFF;
	toccupied ^= tpiece[0];
	tred = tpiece[1] | tpiece[2] | tpiece[3] | tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7];
	tblack = tpiece[8] | tpiece[9] | tpiece[10] | tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14];
	int ssrc = 0;
	int check = (color + deep) % 2;//0red 1black
	if (check == 0) {//紅 
		//cout<<"Ours available eat:"<<endl;
		for (int i = 1; i < 8; i++) { //1~7 為帥~兵,src 為棋子起點,dest 為終點。  先算我方 
			U32 p = tpiece[i]; //取得棋子位置
			while (p) { //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				if (i == 1) //帥,周圍卒(14)以外的黑子都可以吃。
					dest = pMoves[ssrc] & (tblack ^ tpiece[14]);
				else if (i == 2) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & (tblack ^ tpiece[8]);
				else if (i == 3) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & (tblack ^ tpiece[8] ^ tpiece[9]);
				else if (i == 4) //?,只能吃車(11)、馬、炮、卒。
					dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 5) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 6) //炮,特殊處理。
					dest = CGen(ssrc, toccupied) & tblack;
				else if (i == 7) //兵,只能吃將(8)、卒(14)。
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
					dest = 0;
				while (dest) { //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allEatMove[AEMindex][0] = ssrc;
					allEatMove[AEMindex][1] = result;
					AEMindex++;
				}
			}
		}
		for (int i = 1; i < 8; i++) { //紅方純移動
			U32 p = tpiece[i]; //取得棋子位置
			while (p) { //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				dest = pMoves[ssrc] & tpiece[0];//只走空格
				while (dest) { //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allOnlyMove[AOMindex][0] = ssrc;
					allOnlyMove[AOMindex][1] = result;
					AOMindex++;
				}
			}
		}
		for (int i = 8; i < 15; i++) { //再算對手行動 
			U32 p = tpiece[i];
			while (p) {
				U32 mask = LS1B(p);
				p ^= mask;
				ssrc = GetIndex(mask);
				if (i == 8)
					dest = pMoves[ssrc] & (tred ^ tpiece[7]);
				else if (i == 9) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & (tred ^ tpiece[1]);
				else if (i == 10) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & (tred ^ tpiece[1] ^ tpiece[2]);
				else if (i == 11) //?,只能吃車(11)、馬、炮、卒。
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 12) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 13) //炮,特殊處理。
					dest = CGen(ssrc, toccupied) & tred;
				else if (i == 14) //兵,只能吃將、卒(14)。
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
					dest = 0;
				while (dest) { //對手行動存入EallEatMove 
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallEatMove[EAEMindex][0] = ssrc;
					EallEatMove[EAEMindex][1] = result;
					EAEMindex++;
				}
			}
		}
		for (int i = 8; i < 15; i++) { //對手純移動 
			U32 p = tpiece[i]; //取得棋子位置
			while (p) { //將黑色 8~14 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				dest = pMoves[ssrc] & tpiece[0];//只走空格
				while (dest) { //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallOnlyMove[EAOMindex][0] = ssrc;
					EallOnlyMove[EAOMindex][1] = result;
					EAOMindex++;
				}
			}
		}
	}
	else {//黑方移動 
		for (int i = 8; i < 15; i++) { //1~7 為帥~兵,src 為棋子起點,dest 為終點。
			U32 p = tpiece[i]; //取得棋子位置
			while (p) { //將黑色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				if (i == 8) //帥,周圍卒(14)以外的黑子都可以吃。
					dest = pMoves[ssrc] & (tred ^ tpiece[7]);
				else if (i == 9) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & (tred ^ tpiece[1]);
				else if (i == 10) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & (tred ^ tpiece[1] ^ tpiece[2]);
				else if (i == 11) //?,只能吃車(11)、馬、炮、卒。
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 12) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 13) //炮,特殊處理。
					dest = CGen(ssrc, toccupied) & tred;
				else if (i == 14) //兵,只能吃將、卒(14)。
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
					dest = 0;
				while (dest) { //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allEatMove[AEMindex][0] = ssrc;
					allEatMove[AEMindex][1] = result;
					AEMindex++;
				}
			}
		}
		for (int i = 8; i < 15; i++) { //黑方純移動
			U32 p = tpiece[i]; //取得棋子位置
			while (p) { //將黑色 8~14 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				dest = pMoves[ssrc] & tpiece[0];//只走空格
				while (dest) { //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allOnlyMove[AOMindex][0] = ssrc;
					allOnlyMove[AOMindex][1] = result;
					AOMindex++;
				}
			}
		}
		for (int i = 1; i < 8; i++) { //計算敵方移動 
			U32 p = tpiece[i]; //取得棋子位置
			while (p) { //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				if (i == 1) //帥,周圍卒(14)以外的黑子都可以吃。
					dest = pMoves[ssrc] & (tblack ^ tpiece[14]);
				else if (i == 2) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & (tblack ^ tpiece[8]);
				else if (i == 3) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & (tblack ^ tpiece[8] ^ tpiece[9]);
				else if (i == 4) //?,只能吃車(11)、馬、炮、卒。
					dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 5) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 6) //炮,特殊處理。
					dest = CGen(ssrc, toccupied) & tblack;
				else if (i == 7) //兵,只能吃將(8)、卒(14)。
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
					dest = 0;
				while (dest) { //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallEatMove[EAEMindex][0] = ssrc;
					EallEatMove[EAEMindex][1] = result;
					EAEMindex++;
				}
			}
		}
		for (int i = 1; i < 8; i++) { //紅方純移動
			U32 p = tpiece[i]; //取得棋子位置
			while (p) { //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				dest = pMoves[ssrc] & tpiece[0];//只走空格
				while (dest) { //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallOnlyMove[EAOMindex][0] = ssrc;
					EallOnlyMove[EAOMindex][1] = result;
					EAOMindex++;
				}
			}
		}
	}
}

U32 CGen(int ssrc, U32 toccupied)
{
	int r = ssrc / 4;//列 
	int c = ssrc % 4;//行 
	U32 result = 0;
	U32 resulta = 0;
	U32 x = ((row[r] & toccupied) ^ (1 << ssrc)) >> (4 * r);//取出跟炮同列 削去炮本身那格
	if (x && c == 0) {
		result |= CGenCL(x);
	}
	else if (x && c == 1) {
		result |= ((x & 12) == 12) ? 8 : 0;
	}
	else if (x && c == 2) {
		result |= ((x & 3) == 3) ? 1 : 0;
	}
	else if (x && c == 3) {
		result |= CGenCR(x);
	}
	result = result << (4 * r);

	x = ((file[c] & toccupied) ^ (1 << ssrc)) >> c;//取出跟炮同行 並全部放到第1行
	if (x && r == 0)
	{
		resulta |= CGenCL(x);
	}
	else if (x && r == 1)
	{
		resulta |= CGenCL(x & 0x11111100);
	}
	else if (x && r == 2)
	{
		U32 part = 0;
		if ((x & 0x00000011) == 0x00000011)
			part |= 1;
		resulta |= part | CGenCL(x & 0x11111000);
	}
	else if (x && r == 3)
	{
		U32 part = 0;
		part = CGenCR(x & 0x00000111);
		resulta |= part | CGenCL(x & 0x11110000);
	}
	else if (x && r == 4)
	{
		U32 part = 0;
		part = CGenCR(x & 0x00001111);
		resulta |= part | CGenCL(x & 0x11100000);
	}
	else if (x && r == 5)
	{
		U32 part = 0;
		part = CGenCR(x & 0x00011111);
		resulta |= part | CGenCL(x & 0x11000000);
	}
	else if (x && r == 6)
	{
		resulta |= CGenCR(x & 0x00111111);
	}
	else if (x && r == 7)
	{
		resulta |= CGenCR(x);
	}
	resulta = resulta << c;
	U32 re = result | resulta;
	//cout<<hex<<occupied<<" "<<re<<endl;
	//cout<<dec<<r<<" "<<c<<endl;
	return re;
}

U32 CGenCL(U32 x) {
	if (x) {
		U32 mask = LS1B(x); //mask 為炮架的遮罩位置,讓 x 消去炮架。
		return (x ^= mask) ? LS1B(x) : 0; //狀況 5~8 傳回 LS1B(x),狀況 2~4 傳回 0。
	}
	else return 0;
}

U32 CGenCR(U32 x) {
	if (x) {
		U32 mask = MS1B(x); //mask 為炮架的遮罩位置,讓 x 消去炮架。
		return (x ^= mask) ? MS1B(x) : 0; //狀況 5~8 傳回 MS1B(x),狀況 2~4 傳回 0。
	}
	else return 0;
}

int ccc = 0;
int countAva(int pie[14], int deep, U32 curPiece[16])//將士相車馬炮兵
{
	int eat[10];
	int power = 0;
	int redleft = 0;
	int blackleft = 0;
	dynamicPower(pie);
	if (color == 0)//紅 
	{
		for (int i = 0; i < 7; i++)
		{
			power += pie[i] * piPw[i];
			redleft += pie[i];
		}
		for (int i = 7; i < 14; i++)
		{
			power -= pie[i] * piPw[i];
			blackleft += pie[i];
		}
		if (redleft == 0) power = -100000 + deep * 1000;
		if (blackleft == 0) power = 100000 - deep * 1000;
	}
	else
	{
		for (int i = 7; i < 14; i++)
		{
			power += pie[i] * piPw[i];
			blackleft += pie[i];
		}
		for (int i = 0; i < 7; i++)
		{
			power -= pie[i] * piPw[i];
			redleft += pie[i];
		}
		if (blackleft == 0) power = -100000 + deep * 1000;
		if (redleft == 0) power = 100000 - deep * 1000;
	}
	//------------------------------------------------------判斷距離 
	int distanceScore = 0;
	if (curPiece[15] == 0) {
		if (color == 0)
		{
			for (int i = 1; i < 8; i++)
			{
				if (curPiece[i] && i != 6)//curp!=0 版面上存在
				{
					U32 tempcpa = curPiece[i];
					while (tempcpa)
					{
						U32 a = LS1B(tempcpa);//一個一個處理 
						tempcpa = tempcpa ^ a;
						for (int ii = i + 7; ii < 15; ii++)
						{
							if (i == 1 && ii == 14) continue;
							if (curPiece[ii])
							{
								U32 tempcpb = curPiece[ii];
								while (tempcpb)
								{
									U32 b = LS1B(tempcpb);//一個一個處理 
									tempcpb = tempcpb ^ b;
									int ia = GetIndex(a);//得到a的index 0~31 
									int ib = GetIndex(b);//得到b的index 
									ia -= ib;//ia成為兩者距離
									ia = abs(ia);
									int x = ia / 4;
									int y = ia % 4;
									x += y;//x為兩者距離
									int tempS = influenceValue[i-1][ii - 8];
									if (x == 1) tempS /= 2;
									else
									{
										tempS = tempS * pow(2, 2 - x);
									}
									distanceScore += tempS;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			for (int i = 8; i < 15; i++)
			{
				if (curPiece[i] && i != 13)//curp!=0 版面上存在
				{
					U32 tempcpa = curPiece[i];
					while (tempcpa)
					{
						U32 a = LS1B(tempcpa);//一個一個處理 
						tempcpa = tempcpa ^ a;
						for (int ii = i - 7; ii < 8; ii++)
						{
							if (i == 8 && ii == 7) continue;
							if (curPiece[ii])
							{
								U32 tempcpb = curPiece[ii];
								while (tempcpb)
								{
									U32 b = LS1B(tempcpb);//一個一個處理 
									tempcpb = tempcpb ^ b;
									int ia = GetIndex(a);//得到a的index 0~31 
									int ib = GetIndex(b);//得到b的index 
									ia -= ib;//ia成為兩者距離
									ia = abs(ia);
									int x = ia / 4;
									int y = ia % 4;
									x += y;//x為兩者距離
									int tempS = influenceValue[i - 8][ii-1];
									if (x == 1) tempS /= 2;
									else
									{
										tempS = tempS * pow(2, 2 - x);
									}
									distanceScore += tempS;
								}
							}
						}
					}
				}
			}
		}
	}
	//------------------------------------------------------移動數 
	int movePoint = 0;
	if (curPiece[15] != 0) {
		if (deep % 2 == 0)
		{
			movePoint = AEMindex * 20 + AOMindex - EAEMindex * 50 - EAOMindex;
		}
		else
		{
			movePoint = EAEMindex * 20 + EAOMindex - AEMindex * 50 - AOMindex;
		}
	}
	else {
		if (deep % 2 == 0)
		{
			movePoint = AOMindex - EAEMindex * 80 - EAOMindex;
		}
		else
		{
			movePoint = EAOMindex - AEMindex * 80 - AOMindex;
		}
	}
	//------------------------------------------------------加總
	return power + movePoint - deep + distanceScore;
}

int search(int depth, U32 curPiece[16], int curPie[14], int alpha, int beta, U32 hashvalue)
{
	int depthcheck = 0;
	for (int i = 0; i < depthrecord.size(); i++)
	{
		if (depthrecord[i] == depth)
			depthcheck = 1;
	}
	if (depthcheck == 0)
	{
		depthrecord.push_back(depth);
	}
	node* curNode = new node();
	curNode->depth = depth;
	curNode->visit = false;
	curNode->move[0] = nodemove[0];
	curNode->move[1] = nodemove[1];
	curNode->alpha = alpha;
	curNode->beta = beta;
	for (int i = 0; i < 16; i++)
	{
		curNode->piece[i] = curPiece[i];
	}
	nodes[depth].push_back(curNode);
	if (depth == 0)
	{
		currentParent = NULL;
	}
	curNode->parent = currentParent;
	test++;
	U32 hashindex = hashvalue;
	if (hashtable[hashindex].count != 0) {
		testa++;
		if (hashtable[hashindex].depth % 2 == depth % 2) {
			int temp = 0;
			for (int i = 0; i < 16; i++) {
				if (curPiece[i] != hashtable[hashindex].curPiece[i]) {
					temp++;
					break;
				}
			}
			if (temp == 0) {
				testb++;
				if (hashtable[hashindex].depth < depth) {
					curNode->count = hashtable[hashindex].count - (depth - hashtable[hashindex].depth);
					curNode->realcount = hashtable[hashindex].count - (depth - hashtable[hashindex].depth);
					currentChild = curNode;
					return hashtable[hashindex].count-(depth- hashtable[hashindex].depth);
				}
				else {

				}
			}
		}
	}
	hashtable[hashindex].count = 0;
	chess(curPiece, depth);
	U32 taEM[50][2];//存可吃子的方法 0 src 1 dst 避免被往下搜尋時刷掉
	int tAEMi = AEMindex;//alleatmove index
	U32 taOM[50][2];//存可移動非吃子的方法 0 src 1 dst
	int tAOMi = AOMindex;//allonlymove index
	int etaemi = EAEMindex;
	if (depth % 2 == 1) etaemi = AEMindex;
	memcpy(taEM, allEatMove, sizeof(taEM));
	memcpy(taOM, allOnlyMove, sizeof(taOM));
	int re = countAva(curPie, depth, curPiece);

	if (depth == maxDepth)
	{
		int re = countAva(curPie, depth, curPiece);
		hashtable[hashindex].depth = depth;
		hashtable[hashindex].count = re;
		memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
		curNode->count = re;
		curNode->realcount = re;
		currentChild = curNode;
		return re;
	}
	int weight[100][3];//計算所有移動與翻棋的得分0src 1dst 2weight 
	int wp = 0;
	int best = -9999999;
	if (depth % 2 == 1)
		best = 9999999;
	if (curPiece[15] != 0 && depth > noReDepth)//可以走空步
	{
		weight[wp][0] = 0; weight[wp][1] = 0; weight[wp][2] = search(depth + 1, curPiece, curPie, alpha, beta, hashvalue);
		wp++;
	}

	if (tAEMi + tAOMi != 0)
	{
		if (tAEMi > 0)//試吃子 
			for (int i = 0; i < tAEMi; i++)
			{
				weight[wp][2] = 0;
				int deeper = depth + 1;
				int c1p, c2p = -1;
				U32 c1 = 1 << taEM[i][0];
				U32 c2 = 1 << taEM[i][1];
				for (int ii = 1; ii < 15; ii++) {//找到c1 放入c1p  
					U32 check = curPiece[ii] & c1;
					if (check != 0) {
						c1p = ii;
						break;
					}
				}
				for (int ii = 1; ii < 15; ii++) {//找c2 清掉 剩餘棋子數更改 
					U32 check = curPiece[ii] & c2;
					if (check != 0) {
						c2p = ii;
						break;
					}
				}
				if (c2p == -1 || c1p == -1) cout << "err 405";//++++++++++++++++++++++++++++++++++++++++++++
				int c2pcopy;
				c2pcopy = c2p;
				curPiece[c1p] ^= c1;//清除原位置c1
				curPiece[c1p] |= c2;//移動
				curPiece[0] |= c1;//空格+c1
				if (c2p != -1) {//吃子移動 
					curPiece[c2p] ^= c2;//清除原位置c2
					int c2ps = c2p - 1;
					curPie[c2ps]--;
				}
				else//無吃子移動 此處不會發生 只有吃子移動 
				{

					curPiece[0] ^= c2;//空格-c2
					cout << "err 303";
				}
				weight[wp][0] = taEM[i][0];
				weight[wp][1] = taEM[i][1];
				currentParent = curNode;
				nodemove[0] = weight[wp][0];
				nodemove[1] = weight[wp][1];
				weight[wp][2] = search(deeper, curPiece, curPie, alpha, beta, hashvalue ^ randtable[c1p - 1][GetIndex(c1)] ^ randtable[c2p - 1][GetIndex(c2)] ^ randtable[c1p - 1][GetIndex(c2)]);
				if (depth == 0)
				{
					int r = rand() % 6;;
					string a[6] = { "⊙3⊙","(--;)","(〃ω〃)","(’-_-`)","|ω˙）","(*≧艸≦)" };
					cout << a[r] + ".";
				}
				c1p = -1;
				c2p = -1;
				c1 = 1 << taEM[i][0];
				c2 = 1 << taEM[i][1];
				for (int ii = 1; ii < 15; ii++) {
					U32 check = curPiece[ii] & c2;
					if (check != 0) {
						c1p = ii;
						break;
					}
				}
				c2p = c2pcopy;//curpiece找不到c2p 所以要在上一輪記下來 
				curPiece[c1p] ^= c2;//清除原位置c2
				curPiece[0] |= c2;//空格+c2
				curPiece[c1p] |= c1;//移動
				curPiece[0] ^= c1;//空格-c1
				if (c2p != -1) {//吃子移動 
					curPiece[c2p] |= c2;//回原位置c2
					int c2ps = c2p - 1;
					curPie[c2ps]++;
					curPiece[0] ^= c2;
				}

				if (depth % 2 == 0)//max
				{
					if (weight[wp][2] > alpha)
					{
						alpha = weight[wp][2];
						curNode->alpha = alpha;
						curNode->beta = beta;
					}
					if (weight[wp][2] > best)
					{
						best = weight[wp][2];
					}
				}
				else//min
				{
					if (weight[wp][2] < beta)
					{
						beta = weight[wp][2];
						curNode->alpha = alpha;
						curNode->beta = beta;
					}
					if (weight[wp][2] < best)
					{
						best = weight[wp][2];
					}
				}
				if (beta <= alpha)
				{
					if (depth % 2 == 0) {//min
						curNode->count = alpha;
						curNode->realcount = alpha;
						currentChild = curNode;
						hashtable[hashindex].depth = depth;
						hashtable[hashindex].count = alpha;
						memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
						return alpha;
					}
					else {//max
						curNode->count = beta;
						curNode->realcount = beta;
						currentChild = curNode;
						hashtable[hashindex].depth = depth;
						hashtable[hashindex].count = beta;
						memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
						return beta;
					}
				}
				wp++;
			}
		for (int i = 0; i < tAOMi; i++)//純移動 --------------------------------------------------------------------------------------------------------
		{
			weight[wp][2] = 0;
			int deeper = depth + 1;
			int c1p, c2p = -1;
			U32 c1 = 1 << taOM[i][0];
			U32 c2 = 1 << taOM[i][1];
			for (int ii = 1; ii < 15; ii++) {
				U32 check = curPiece[ii] & c1;
				if (check != 0) {
					c1p = ii;
					break;
				}
			}
			for (int ii = 1; ii < 15; ii++) {//找c2 清掉 剩餘棋子數更改 
				U32 check = curPiece[ii] & c2;
				if (check != 0) {
					c2p = ii;
					break;
				}
			}
			int c2pcopy = 0;
			c2pcopy = c2p;
			curPiece[c1p] ^= c1;//清除原位置c1
			curPiece[c1p] |= c2;//移動
			curPiece[0] |= c1;//空格+c1

			if (c2p != -1) {//吃子移動 
				curPiece[c2p] ^= c2;//清除原位置c2
				int c2ps = c2p - 1;
				curPie[c2ps]--;

			}
			else//無吃子移動 
			{

				curPiece[0] ^= c2;//空格-c2
			}
			weight[wp][0] = taOM[i][0];
			weight[wp][1] = taOM[i][1];
			currentParent = curNode;
			nodemove[0] = weight[wp][0];
			nodemove[1] = weight[wp][1];
			weight[wp][2] = search(deeper, curPiece, curPie, alpha, beta, hashvalue ^ randtable[c1p - 1][GetIndex(c1)] ^ randtable[c1p - 1][GetIndex(c2)]);
			if (depth == 0)
			{
				int r = rand() % 6;;
				string a[6] = { "⊙3⊙","(--;)","(〃ω〃)","(’-_-`)","|ω˙）","(*≧艸≦)" };
				cout << a[r] + ".";
			}
			c1p = -1;
			c2p = -1;
			c1 = 1 << taOM[i][0];
			c2 = 1 << taOM[i][1];
			for (int ii = 1; ii < 15; ii++) {
				U32 check = curPiece[ii] & c2;
				if (check != 0) {
					c1p = ii;
					break;
				}
			}
			c2p = c2pcopy;
			curPiece[c1p] ^= c2;//清除原位置c2
			curPiece[0] |= c2;//空格+c2
			curPiece[c1p] |= c1;//移動
			curPiece[0] ^= c1;//空格-c1
			if (c2p != -1) {//吃子移動 
				curPiece[c2p] |= c2;//回原位置c2
				int c2ps = c2p - 1;
				curPie[c2ps]++;
				curPiece[0] ^= c2;
			}

			if (depth % 2 == 0)//max
			{
				if (weight[wp][2] > best)
				{
					best = weight[wp][2];
				}
				if (weight[wp][2] > alpha)
				{
					alpha = weight[wp][2];
					curNode->alpha = alpha;
					curNode->beta = beta;
				}
			}
			else//min
			{
				if (weight[wp][2] < best)
				{
					best = weight[wp][2];
				}
				if (weight[wp][2] < beta)
				{
					beta = weight[wp][2];
					curNode->alpha = alpha;
					curNode->beta = beta;
				}
			}
			if (beta <= alpha)
			{
				if (depth % 2 == 0) {//min
					curNode->count = alpha;
					curNode->realcount = alpha;
					currentChild = curNode;
					hashtable[hashindex].depth = depth;
					hashtable[hashindex].count = alpha;
					memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
					return alpha;
				}
				else {//max
					curNode->count = beta;
					curNode->realcount = beta;
					currentChild = curNode;
					hashtable[hashindex].depth = depth;
					hashtable[hashindex].count = beta;
					memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
					return beta;
				}
			}
			wp++;
		}
	}

	if (curPiece[15] != 0)//先試翻棋 做完後call search 
	{
		for (int ssrc = 0; ssrc < 32; ssrc++) { //搜尋盤面上 32 個位置
			if (curPiece[15] & (1 << ssrc) && ch & (1 << ssrc) && depth <= noReDepth) { //若為未翻子 在未翻子的遮罩內 depth<=noReDepth 
				if (depth == 0)
				{
					int r = rand() % 6;;
					string a[6] = { "⊙3⊙","(--;)","(〃ω〃)","(’-_-`)","|ω˙）","(*≧艸≦)" };
					cout << a[r] + ".";
				}
				weight[wp][2] = 0;
				int a = 0;
				for (int pID = 0; pID < 14; pID++) { //搜尋可能會翻出之子
					if (DCount[pID]) { //若該兵種可能被翻出
						a += DCount[pID];
						int deeper = depth + 1;
						U32 c = 1 << ssrc;
						int cpID = pID + 1;
						if (pID < 8)
						{
							curPiece[cpID] |= c;
							curPiece[15] ^= c;
							DCount[pID]--;
						}
						else
						{
							curPiece[cpID] |= c;
							curPiece[15] ^= c;
							DCount[pID]--;
						}//模擬該兵種翻出來
						weight[wp][0] = ssrc;
						weight[wp][1] = ssrc;
						currentParent = curNode;
						nodemove[0] = weight[wp][0];
						nodemove[1] = weight[wp][1];
						currentChild->realcount = ((DCount[pID] + 1) * search(deeper, curPiece, curPie, -999999, 999999, hashvalue ^ randtable[pID][ssrc] ^ randtable[14][ssrc]));
						weight[wp][2] += currentChild->realcount;
						currentChild->chessmove[0] = pID;
						currentChild->chessmove[1] = pID;
						curNode->child.push_back(currentChild);
						ccc = 0;
						if (pID < 8)
						{
							curPiece[cpID] ^= c;
							curPiece[15] |= c;
							DCount[pID]++;
						}
						else
						{
							curPiece[cpID] ^= c;
							curPiece[15] |= c;
							DCount[pID]++;
						}//將模擬翻出的子復原
					}
				}
				weight[wp][2] /= a;
				for (int i = 0; i < curNode->child.size(); i++)
				{
					if (curNode->child[i]->move[0] == curNode->child[i]->move[1])
						curNode->child[i]->count = weight[wp][2];
				}
				curNode->child.clear();
				if (depth % 2 == 0)//max
				{
					if (weight[wp][2] > alpha)
					{
						alpha = weight[wp][2];
						curNode->alpha = alpha;
						curNode->beta = beta;
					}
					if (weight[wp][2] > best)
					{
						best = weight[wp][2];
					}
				}
				else//min
				{
					if (weight[wp][2] < beta)
					{
						beta = weight[wp][2];
						curNode->alpha = alpha;
						curNode->beta = beta;
					}
					if (weight[wp][2] < best)
					{
						best = weight[wp][2];
					}
				}
				if (beta <= alpha)
				{
					if (depth % 2 == 0) {//min
						curNode->count = alpha;
						curNode->realcount = alpha;
						currentChild = curNode;
						hashtable[hashindex].depth = depth;
						hashtable[hashindex].count = alpha;
						memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
						return alpha;
					}
					else {//max
						curNode->count = beta;
						curNode->realcount = beta;
						currentChild = curNode;
						hashtable[hashindex].depth = depth;
						hashtable[hashindex].count = beta;
						memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
						return beta;
					}
				}
				wp++;
			}
		}
	}

	if (depth == 0)//max
	{
		cout << endl << "------------------------------------" << endl;
		int recordi = 0;
		int temp2 = 0;
		for (int i = wp - 1; i >= 0; i--)
		{
			int temp = 0;
			if (depth == 0)
			{
				cout << weight[i][0] << " " << weight[i][1] << " " << weight[i][2] << endl;
				for (int k = 0; k < nodes[1].size(); k++)
				{
					if (nodes[1][k]->count == weight[i][2])
					{
						//cout << "get " << k << endl;
						temp++;
						//for (int m = 0; m < 16; m++) {
						//	cout << m << " piece[m] " << hex << nodes[1][k]->piece[m] << dec << endl;
						//}
					}
				}
			}
			if (weight[i][2] == best)
			{
				recordi = i;
				srci = weight[i][0];
				dsti = weight[i][1];
			}
			if (temp != 0)
			{
				temp2++;
			}
		}
		if (temp2 != wp)
		{
			cout << "oops " << wp - temp2 << endl;
		}
		cout << temp2 << endl;
		if (draw == 0)
		{
		}
		else if (draw == 1)
		{
			if (best < 0); //可能輸 故意平手? 
			else if (srci == past_walk[1][1] && dsti == past_walk[1][0])
			{
				cout << "draw denied" << endl;
				weight[recordi][2] -= 999999;//可能會贏 選擇不平手? 
				best = weight[0][2];
				for (int ii = 0; ii < wp; ii++)
				{
					if (weight[ii][0] == srci)//那顆棋子相關全部都-999999 
						weight[ii][2] -= 999999;
				}
				for (int ii = wp - 1; ii >= 0; ii--)//重新尋找 
				{
					if (weight[ii][2] > best)
					{
						best = weight[ii][2];
						srci = weight[ii][0];
						dsti = weight[ii][1];
					}
				}
			}
		}
	}
	if (wp == 0) best = re;
	if (best == 9999999 || best == -9999999) best = re;
	hashtable[hashindex].depth = depth;
	hashtable[hashindex].count = best;
	memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
	curNode->count = best;
	curNode->realcount = best;
	currentChild = curNode;
	return best;
}

void drawOrNot()
{
	int a = past_walk[0][0];
	int b = past_walk[0][1];
	int c = past_walk[1][1];
	int d = past_walk[1][0];
	//if(a==past_walk[2][1]&&a==past_walk[4][0]&&b==past_walk[2][0]&&b==past_walk[4][1]&&c==past_walk[3][0]&&d==past_walk[3][1])
	//draw=1;

	if (a == past_walk[2][1] && a == past_walk[4][0] && a == past_walk[6][1] && b == past_walk[2][0] && b == past_walk[4][1] && b == past_walk[6][0] && c == past_walk[3][0] &&
		c == past_walk[5][1] && d == past_walk[3][1] && d == past_walk[5][0])
		draw = 1;

	cout << draw << endl;
}

void dynamicPower(int curPie[14])
{
	piPw[0] = 6000 + curPie[7] * 600 + curPie[8] * 270 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200;
	piPw[1] = 2700 + curPie[8] * 270 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[2] = 900 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[3] = 400 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[4] = 150 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[5] = 2000 + curPie[7] * 600 + curPie[8] * 270 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[6] = 100 + curPie[7] * 600 + curPie[13] * 10;

	piPw[7] = 6000 + curPie[0] * 600 + curPie[1] * 270 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200;//7830
	piPw[8] = 2700 + curPie[1] * 270 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//3930
	piPw[9] = 900 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//1590
	piPw[10] = 400 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//910
	piPw[11] = 150 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//580
	piPw[12] = 2000 + curPie[0] * 600 + curPie[1] * 270 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//3880
	piPw[13] = 100 + curPie[0] * 600 + curPie[6] * 10;//660
	//total max 32910
}

int findPiece(int place, U32 curPiece[16])
{
	U32 bplace = 1 << place;
	for (int i = 1; i < 15; i++)//找14種棋中 誰在這個位置上 
	{
		if ((curPiece[i] & bplace) != 0)
		{
			i--;
			return i;//找到棋子回傳 
		}
	}
	return -1;//錯誤 
}

void readBoard()//讀檔模式 讀取board.txt專用 
{
	vector<string> move;
	int cp = 0;//current_position計算位址 
	int line = 0;//目前讀到的行數 
	ifstream file;
	string str;
	file.open("board.txt", ios::in);
	vector<string> aStr;
	int startmove = 0;
	bool first = true;
	while (getline(file, str))
	{
		aStr.push_back(str);
		//cout<<str<<endl;
		if (line == 2)
		{
			for (int i = 0; i <= 13; i++)
			{
				piece_count[i] = str[i * 2 + 2] - '0';//把剩餘棋子讀入piece_count 
			}
		}
		if (line >= 3 && line <= 10 && initailBoard == 1)
		{
			for (int i = 0; i <= 3; i++)
			{
				current_position[cp] = str[i * 2 + 2];//讀入初始場上狀況 只讀一次 
				cp++;
			}
			if (line == 10)
				initailBoard = 0;
		}

		if (str.at(2) == 't') {//start after * time 900
			startmove = 1;
		}
		if (str.at(2) == 'C' && str.at(3) == 'o') {//stop before * Comment 0 0
			startmove = 0;
		}
		if (startmove == 1) {
			if (str.length() >= 10 && str.at(2) != 't' && str.length() <= 12) {
				//cout << str.substr(str.length() - 5, 5)<<"\n";
				move.push_back(str.substr(str.length() - 5, 5));//a1(k),a1-a2	
				first = false;
			}
			if (str.length() >= 16 && str.length() <= 18) {
				move.push_back(str.substr(str.length() - 11, 5));//a1(k),a1-a2
				//cout << str.substr(str.length() - 11, 5) << "\n";
				move.push_back(str.substr(str.length() - 5, 5));//a1(k),a1-a2
				//cout << str.substr(str.length() - 5, 5) << "\n";
			}
		}
		line++;

	}
	file.close();
	if (first && !move.empty()) {//先手 處理我方顏色 
		if (move[0].at(3) - 91 > 0)//小寫
			color = 1;//黑
		else
			color = 0;//紅
	}
	else if (!move.empty()) {//後手
		if (move[0].at(3) - 91 > 0)//小寫
			color = 0;//紅
		else
			color = 1;//黑
	}
	else {
		color = 0;
	}
	//cout << "color(b0 r1 u-1): " << color<<"\n";
	for (int i = 0; i < move.size(); i++) {//record in char board[4][8] 處理history 產生現在棋版 
		//cout<<i<<move[i].at(0)<<move[i].at(1)<<move[i].at(2)<<endl;
		if (move[i].at(2) == '(') {//record move 'flip' (kgmrncp)
			//string a=move[i].substr(0,2);
			int aa = 100 - move[i].at(0);
			int bb = 4 * (56 - move[i].at(1));
			U32 cc = aa + bb;//棋盤編號0~31
			cc = 1 << cc;
			if (move[i].at(3) == 'K') { piece[1] |= cc; red |= cc; piece[15] ^= cc; DCount[0]--; }
			if (move[i].at(3) == 'G') { piece[2] |= cc; red |= cc; piece[15] ^= cc; DCount[1]--; }
			if (move[i].at(3) == 'M') { piece[3] |= cc; red |= cc; piece[15] ^= cc; DCount[2]--; }
			if (move[i].at(3) == 'R') { piece[4] |= cc; red |= cc; piece[15] ^= cc; DCount[3]--; }
			if (move[i].at(3) == 'N') { piece[5] |= cc; red |= cc; piece[15] ^= cc; DCount[4]--; }
			if (move[i].at(3) == 'C') { piece[6] |= cc; red |= cc; piece[15] ^= cc; DCount[5]--; }
			if (move[i].at(3) == 'P') { piece[7] |= cc; red |= cc; piece[15] ^= cc; DCount[6]--; }
			if (move[i].at(3) == 'k') { piece[8] |= cc; black |= cc; piece[15] ^= cc; DCount[7]--; }
			if (move[i].at(3) == 'g') { piece[9] |= cc; black |= cc; piece[15] ^= cc; DCount[8]--; }
			if (move[i].at(3) == 'm') { piece[10] |= cc; black |= cc; piece[15] ^= cc; DCount[9]--; }
			if (move[i].at(3) == 'r') { piece[11] |= cc; black |= cc; piece[15] ^= cc; DCount[10]--; }
			if (move[i].at(3) == 'n') { piece[12] |= cc; black |= cc; piece[15] ^= cc; DCount[11]--; }
			if (move[i].at(3) == 'c') { piece[13] |= cc; black |= cc; piece[15] ^= cc; DCount[12]--; }
			if (move[i].at(3) == 'p') { piece[14] |= cc; black |= cc; piece[15] ^= cc; DCount[13]--; }
			//board[move[i].at(0)-'a'][move[i].at(1)-'1'] = move[i].at(3);	//board[a][1]=k,ai(k)
			//cout << board[move[i].at(0) - 'a'][move[i].at(1) - '1'] ;
		}
		else {//a1-b1之類 
			int c1p, c2p;
			c2p = -1;//如果是-1為移動 
			int aa = 100 - move[i].at(0);
			int bb = 4 * (56 - move[i].at(1));
			U32 c1 = aa + bb;//棋盤編號0~31
			past_walk[6][0] = past_walk[5][0];
			past_walk[5][0] = past_walk[4][0];
			past_walk[4][0] = past_walk[3][0];
			past_walk[3][0] = past_walk[2][0];
			past_walk[2][0] = past_walk[1][0];
			past_walk[1][0] = past_walk[0][0];
			past_walk[0][0] = aa + bb;
			c1 = 1 << c1;
			//cout<<hex<<c1<<"c1 "; 

			int aaa = 100 - move[i].at(3);
			int bbb = 4 * (56 - move[i].at(4));
			U32 c2 = aaa + bbb;//棋盤編號0~31
			past_walk[6][1] = past_walk[5][1];
			past_walk[5][1] = past_walk[4][1];
			past_walk[4][1] = past_walk[3][1];
			past_walk[3][1] = past_walk[2][1];
			past_walk[2][1] = past_walk[1][1];
			past_walk[1][1] = past_walk[0][1];
			past_walk[0][1] = aaa + bbb;
			c2 = 1 << c2;
			//cout<<hex<<c2; 
			for (int ii = 1; ii < 15; ii++) {
				U32 check = piece[ii] & c1;
				if (check != 0) {
					c1p = ii;
					break;
				}
			}
			for (int ii = 1; ii < 15; ii++) {//找c2 清掉 剩餘棋子數更改 
				U32 check = piece[ii] & c2;
				if (check != 0) {
					c2p = ii;
				}
			}
			piece[c1p] ^= c1;//清除原位置c1
			piece[0] |= c1;//空格+c1
			piece[c1p] |= c2;//移動
			if (c2p != -1) {//吃子移動 
				piece[c2p] ^= c2;//清除原位置c2
				int c2ps = c2p - 1;
				piece_count[c2ps]--;
				if (8 > c1p) {//紅吃黑
					red ^= c1;
					black ^= c2;
					red |= c2;
				}
				else {//黑吃紅 
					black ^= c1;
					red ^= c2;
					black |= c2;
				}
				occupied ^= c1;//c1無子 
			}
			else//無吃子移動 
			{
				if (8 > c1p) {//紅動 
					red ^= c1;
					red |= c2;
				}
				else {//黑動 
					black ^= c1;
					black |= c2;
				}
				piece[0] ^= c2;//空格-c2
				occupied ^= c1;//c1無子 
				occupied |= c2;//c2有子 
			}
			//board[move[i].at(3) - 'a'][move[i].at(4) - '1'] = board[move[i].at(0) - 'a'][move[i].at(1) - '1'];	//board[a][2]=k, a1-a2
			//board[move[i].at(0) - 'a'][move[i].at(1) - '1'] = '-';												//board[a][1]='-', a1-a2
			//cout << board[move[i].at(3) - 'a'][move[i].at(4) - '1'];
		}
	}
	//cout<<hex<<piece[9];
}

void createMovetxt()//背景模式 
{
	fstream file;
	file.open("move.txt", ios::out);
	if (src == dst)
		file << "1" << endl;
	else
		file << "0" << endl;
	if (src == dst)
		file << src << endl << "0" << endl;
	else
		file << src << endl << dst << endl;
	cout << srci << "." << src << ":src " << dsti << "." << dst << ":dst " << endl;
	file.close();
}

void initial()
{
	random_device rd;
	default_random_engine gen = default_random_engine(rd());
	uniform_int_distribution<U32> dis(0, pow(2, 32) - 1);

	for (int i = 0; i <= 14; i++)
		piece[i] = 0;
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 32; j++) {
			randtable[i][j] = dis(gen);
			while (1) {
				int same = 0;
				for (int k = 0; k < i; k++) {
					for (int l = 0; l < i; l++) {
						if (randtable[i][j] == randtable[k][l]) {
							randtable[i][j] = dis(gen);
							same++;
							cout << "randtable repeat" << endl;
						}
					}
				}
				if (!same)
					break;
			}
		}
	}
	red = 0;
	black = 0;
	occupied = 0xFFFFFFFF;
	piece[15] = 0xFFFFFFFF;
}

void IndexToBoard(int indexa, int indexb)
{
	stringstream ss;
	string aa;
	int a = indexa / 4;//0~7
	int b = indexa % 4;//0~3 a~d
	if (b == 0) aa = "d";
	else if (b == 1) aa = "c";
	else if (b == 2) aa = "b";
	else if (b == 3) aa = "a";
	ss << 8 - a;
	src = aa + ss.str();
	//cout<<"src : "<<src<<endl;

	stringstream sss;
	a = indexb / 4;//0~7
	b = indexb % 4;//0~3 a~d
	if (b == 0) aa = "d";
	else if (b == 1) aa = "c";
	else if (b == 2) aa = "b";
	else if (b == 3) aa = "a";
	sss << 8 - a;
	dst = aa + sss.str();
	//cout<<"dst : "<<dst<<endl;
}

U32 myhash(U32 tpiece[16]) {
	U32 hashvalue = 0;
	for (int i = 1; i < 16; i++) { //1~14 為雙方兵種 0空格 15 未翻
		U32 p = tpiece[i]; //取得棋子位置
		while (p) { //將1~15 號的子都搜尋一遍
			U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
			p ^= mask; //除去位於最低位元的該兵種
			hashvalue ^= randtable[i - 1][GetIndex(mask)]; //將最低位元的兵種位置的隨機值xor hashtable
		}
	}
	return hashvalue;
}

void printDebug()
{
	int depth = 1;
	int s = -1;
	stack<node*> historyHit;
	vector<node*> select;
	node* hit = nodes[0][0];
	historyHit.push(nodes[0][0]);
	for (int i = 0; i < nodes[depth].size(); i++)
	{
		if (nodes[depth][i]->parent == hit)
		{
			select.push_back(nodes[depth][i]);
			s++;
			if (nodes[depth][i]->move[0] == nodes[depth][i]->move[1])
			{
				translate(nodes[depth][i]->chessmove[0]);
			}
			cout << s << " " << index(nodes[depth][i]->move[0]) << " " << index(nodes[depth][i]->move[1]) << " " << nodes[depth][i]->realcount 
				<<" "<< nodes[depth][i] ->alpha<<" "<< nodes[depth][i] ->beta<< endl;
		}
	}
	while (1)
	{
		cout << "current depth: " << depth << " enter two int(undo:-1): " << endl;
		int in1 = 0;
		cin >> in1;
		if (in1 == -1)
		{ //undo
			depth--;
			historyHit.pop();
			hit = historyHit.top();
			for (int i = 0; i < nodes[depth].size(); i++)
			{
				if (nodes[depth][i]->parent == hit)
				{
					select.push_back(nodes[depth][i]);
					s++;
					if (nodes[depth][i]->move[0] == nodes[depth][i]->move[1])
					{
						translate(nodes[depth][i]->chessmove[0]);
					}
					cout << s << " " << index(nodes[depth][i]->move[0]) << " " << index(nodes[depth][i]->move[1]) << " " << nodes[depth][i]->realcount
						<< " " << nodes[depth][i]->alpha << " " << nodes[depth][i]->beta << endl;
				}
			}
		}
		else if (depth < maxDepth)
		{ //deeper
			for (int i = 0; i < nodes[depth].size(); i++)
			{
				if (select[in1] == nodes[depth][i])
				{
					hit = nodes[depth][i];
					historyHit.push(hit);
					select.clear();
					s = -1;
					break;
				}
			}
			depth++;
			for (int i = 0; i < nodes[depth].size(); i++)
			{
				if (nodes[depth][i]->parent == hit)
				{
					select.push_back(nodes[depth][i]);
					s++;
					if (nodes[depth][i]->move[0] == nodes[depth][i]->move[1])
					{
						translate(nodes[depth][i]->chessmove[0]);
					}
					cout << s << " " << index(nodes[depth][i]->move[0]) << " " << index(nodes[depth][i]->move[1]) << " " << nodes[depth][i]->realcount
						<< " " << nodes[depth][i]->alpha << " " << nodes[depth][i]->beta << endl;
				}
			}
		}
		else
		{ //err
			cout << "too depth";
		}
		int high = -100000;
		int low = 100000;
		if (depth % 2 == 1)
		{
			for (int i = 0; i < select.size(); i++)
			{
				if (select[i]->count > high)
					high = select[i]->count;
			}
			for (int i = 0; i < select.size(); i++)
			{
				if (select[i]->count == high)
					cout << "No. " << i << " is highest." << endl;
			}
		}
		else
		{
			for (int i = 0; i < select.size(); i++)
			{
				if (select[i]->count < low)
					low = select[i]->count;
			}
			for (int i = 0; i < select.size(); i++)
			{
				if (select[i]->count == low)
					cout << "No. " << i << " is lowest." << endl;
			}
		}
		high = -100000;
		low = 100000;
		select.clear();
		s = -1;
	}
}

void translate(int a)
{ //k to 將
	if (a > 6)
	{
		set_console_color(10);
	}
	else
	{
		set_console_color(12);
	}
	if (a == 7)
	{ //將
		printf("將");
	}
	else if (a == 8)
	{ //士
		printf("士");
	}
	else if (a == 9)
	{ //象
		printf("象");
	}
	else if (a == 10)
	{ //車
		printf("車");
	}
	else if (a == 11)
	{ //馬
		printf("馬");
	}
	else if (a == 12)
	{ //砲
		printf("砲");
	}
	else if (a == 13)
	{ //卒
		printf("卒");
	}
	else if (a == 0)
	{ //帥
		printf("帥");
	}
	else if (a == 1)
	{ //仕
		printf("仕");
	}
	else if (a == 2)
	{ //相
		printf("相");
	}
	else if (a == 3)
	{ //車
		printf("車");
	}
	else if (a == 4)
	{ //傌
		printf("傌");
	}
	else if (a == 5)
	{ //炮
		printf("炮");
	}
	else if (a == 6)
	{ //兵
		printf("兵");
	}
	else
	{
		printf("F");
	}
	set_console_color(7);
}

void set_console_color(unsigned short color_index) // control collor
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_index);
}

string index(int indexa)
{
	stringstream ss;
	string aa;
	int a = indexa / 4; //0~7
	int b = indexa % 4; //0~3 a~d
	if (b == 0)
		aa = "d";
	else if (b == 1)
		aa = "c";
	else if (b == 2)
		aa = "b";
	else if (b == 3)
		aa = "a";
	ss << 8 - a;
	return aa + ss.str();
}

