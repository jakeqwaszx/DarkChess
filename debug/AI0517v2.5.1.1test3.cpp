#include <iostream>
#include <cstdlib> /* 亂數相關函數 */
#include <ctime>
#include <fstream>
#include <vector>
#include "memory.h"
#include <time.h>
#include <string>
#include <string.h>
#include <sstream>
#include <math.h>
#include <windows.h>
#include <iomanip> 
//目前版本2.5 使用pmove2遮罩減少搜尋時間 翻子搜尋只到depth=2 depth3以上只搜移動或吃子
//阻止平手 該子全部-999999 有計算movepoint(可吃或被吃加減分) 往中間13 14 17 18時分數+1 
//更新:試圖處理平局部分 減少bug 

//可強化 棋力計算 

typedef unsigned int U32;
U32 LS1B(U32 x){ return x&(-x);}//取得x的最低位元
U32 MS1B(U32 x){ // Most Significant 1 Bit (LS1B)函式
	x |= x >> 32; x |= x >> 16; x |= x >> 8;
	x |= x >> 4; x |= x >> 2; x |= x >> 1;
	return (x >> 1) + 1; //可以取得一串位元中最左邊不為零的位元
}
U32 CGen(int ssrc,U32 toccupied);//返還炮位 
U32 CGenCR(U32 x);
U32 CGenCL(U32 x);
int BitsHash(U32 x){return (x * 0x08ED2BE6) >> 27;}
void initial();//初始化 
void chess(U32 tblack,U32 tred,U32 tpiece[16],int deep);//尋找可用移動
void ai2();//ai ver2.0 讚 使用search 
void readBoard();//讀檔模式 讀取board.txt 把讀入檔案轉成bitboard 還沒倒著存入 
void createMovetxt();//創造move.txt 0走步 1翻棋 
void IndexToBoard(int indexa,int indexb);//把src dst從編號0~31->棋盤編號a1~d4 
int countAva(int pie[14],int deep,U32 curPiece[16]);//呼叫則傳回當前棋版
void SimReveal(int ssrc,int pID,U32 curPiece[16],U32 curRed,U32 curBlack);//模擬翻出位於ssrc的pID 
void UnSimReveal(int ssrc,int pID,U32 curPiece[16],U32 curRed,U32 curBlack);//undo simReveal
int* SimMove(int ssrc,int sdst,U32* curPiece,U32* curRed,U32* curBlack,U32* curOccupied,int* curPie);//模擬走步 
int* UnSimMove(int ssrc,int sdst,U32* curPiece,U32* curRed,U32* curBlack,U32* curOccupied,int* curPie,int copy);//undeo simmove
int search(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta);//搜尋最佳走步 
int onewaySearch(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta,int eat);//單向搜尋 測試 
int searchMove(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta);//先搜尋可走版面
void dynamicPower();//計算動態棋力 
void drawOrNot();//由past_walk判斷是否平手 之後結果輸出給draw 
int findPiece(int place,U32 curPiece[16]);//傳編號 回傳在這個編號的棋子 
using namespace std;

int index32[32] = {31, 0, 1, 5, 2, 16, 27, 6, 3, 14, 17, 19, 28, 11, 7, 21, 30, 4, 15, 26, 13,
18, 10, 20, 29, 25, 12, 9, 24, 8, 23, 22};
int GetIndex(U32 mask){ return index32[BitsHash(mask)];}//輸入遮罩返回棋盤編號
U32 pMoves[32]={0x00000012,0x00000025,0x0000004A,0x00000084,0x00000121,0x00000252,0x000004A4,0x00000848,
0x00001210,0x00002520,0x00004A40,0x00008480,0x00012100,0x00025200,0x0004A400,0x00084800,
0x00121000,0x00252000,0x004A4000,0x00848000,0x01210000,0x02520000,0x04A40000,0x08480000,
0x12100000,0x25200000,0x4A400000,0x84800000,0x21000000,0x52000000,0xA4000000,0x48000000};//棋子移動遮罩
U32 pMoves2[32]={
0x00000116,0x0000022D,0x0000044B,0x00000886,0x00001161,0x000022D2,0x000044B4,0x00008868,
0x00011611,0x00022D22,0x00044B44,0x00088688,0x00116110,0x0022D220,0x0044B440,0x00886880,
0x01161100,0x022D2200,0x044B4400,0x08868800,0x11611000,0x22D22000,0x44B44000,0x88688000,
0x16110000,0x2D220000,0x4B440000,0x86880000,0x61100000,0xD2200000,0xB4400000,0x68800000};//翻棋遮罩
U32 pMoves3[32]={
0x00000136,0x0000027D,0x000004EB,0x000008C6,0x00001363,0x000027D7,0x00004EB4,0x00008C6C,
0x00013631,0x00027D72,0x0004EBE4,0x0008C6C8,0x00136310,0x0027D720,0x004EBE40,0x008C6C80,
0x01363100,0x027D7200,0x04EBE400,0x08C6C800,0x13631000,0x27D72000,0x4EBE4000,0x8C6C8000,
0x36310000,0x7D720000,0xEBE40000,0xC6C80000,0x63100000,0xD7200000,0xBE400000,0x6C800000};//翻棋遮罩
U32 file[4]={0x11111111,0x22222222,0x44444444,0x88888888};//行遮罩 
U32 row[8]={0x0000000F,0x000000F0,0x00000F00,0x0000F000,0x000F0000,0x00F00000,0x0F000000,0xF0000000};//列遮罩 
U32 piece[16]; //0空格- 帥k 士g 相m 車r 馬n 炮c 兵p *2 15未翻x 
U32 red,black,occupied;//紅 黑 有棋子 

string move="a1-a1";//下一步行動 用於背景 
int piece_count[14]={1,2,2,2,2,2,5,1,2,2,2,2,2,5};//剩餘棋子數 0-6 7-13
int DCount[14]={1,2,2,2,2,2,5,1,2,2,2,2,2,5};//剩餘未翻子 
string current_position[32];//盤面狀況總覽 
string history; 
int timeCount;//剩餘時間 
int initailBoard=1;//是否讀取初始版面 
int past_walk[7][2]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};//前七步 用來處理平局問題 
int piPw[14]={6000,2700,900,400,150,2000,100,6000,2700,900,400,150,2000,100};//棋子分數 
int draw=0;//0無平手情況 1有可能進入平手 

U32 testPiece[16];
vector<U32*> test;
vector<int> test2;
vector<vector<vector<U32> > > test3;
vector<vector<U32> > test5;
vector<U32> test4;
int tempdepth = -1;

//int RMcount=13;//讀取模式需要 從13行開始讀取結果 

U32 allEatMove[50][2];//存可吃子的方法 0 src 1 dst
int AEMindex=0;//alleatmove index
U32 allOnlyMove[50][2];//存可移動非吃子的方法 0 src 1 dst
int AOMindex=0;//allonlymove index
U32 EallEatMove[50][2];//存對手可吃子的方法 0 src 1 dst
int EAEMindex=0;//Ealleatmove index
U32 EallOnlyMove[50][2];//存對手可移動非吃子的方法 0 src 1 dst
int EAOMindex=0;//Eallonlymove index

int color;//0 red 1 black
string src,dst;//棋盤編號版 a1~d4
int srci,dsti;//index版 0~31
int maxDepth=4;
int moveMaxDepth=8;
U32 open=0xffffffff;//非未翻棋
U32 ch;//需要search的位置 
int noReDepth=2;

int main()
{
	clock_t start, stop;
	start = clock();
	srand(time(NULL));
	initial();//初始化 
	readBoard();
	drawOrNot();
	dynamicPower();
	U32 onboard=piece[15];
	int a=onboard/2;
	int onboardi=0;//計算場面上未翻棋數量 
	for(int i=0;i<32;i++)
	{
		int a=onboard%2;
		int b=onboard/2;
		if(a==1) 
		{
			onboardi++;
		}
		if(b>0) onboard/=2;
		if(b==1) break;
	}
	if(onboardi<2)maxDepth=8;
	else if(onboardi<4)maxDepth=7;
	else if(onboardi<7)maxDepth=6;
	else if(onboardi<14)maxDepth=5;
	//cout<<hex<<piece[0]<<" "<<piece[1]<<" "<<piece[8]<<" "<<piece[15]<<" "<<red<<" "<<black<<" "<<occupied<<" "<<piece_count[7]<<endl;
	//SimMove(0,1,piece,&red,&black,&occupied,piece_count);
	//cout<<hex<<piece[0]<<" "<<piece[1]<<" "<<piece[8]<<" "<<piece[15]<<" "<<red<<" "<<black<<" "<<occupied<<" "<<piece_count[7]<<endl;
	//UnSimMove(0,1,piece,&red,&black,&occupied,piece_count,8);
	//cout<<hex<<piece[0]<<" "<<piece[1]<<" "<<piece[8]<<" "<<piece[15]<<" "<<red<<" "<<black<<" "<<occupied<<" "<<piece_count[7]<<endl;
	int end=1;//結束則=0
	while(end)
	{
		dst="0";
		ai2();//決定行動 
		stop = clock();
		cout <<" 此步耗時 : " << double(stop - start) / CLOCKS_PER_SEC <<" 秒(精準度0.001秒) "<<endl;
		cout <<" 本方為 : ";
		if(color==0) cout<<"紅 "<<endl;
		else cout<<"黑 "<<endl;
		createMovetxt();
		end=0;
	}
	system("pause");
} 

void ai2()
{
	open=0xffffffff;
	ch=0x00000000;
	open^=piece[15];//非未翻棋
	while(open)
	{
		U32 take=LS1B(open);
		open^=take;
		take=GetIndex(take);
		ch|=pMoves3[take];
	}
	if(piece[15]==0xffffffff)
	{
		IndexToBoard(9,9);
		return;
	}
	chess(black,red,piece,0);
	//int nowbest=countAva(piece_count,0);//至少發生吃子 
	//int movebest=-999999;
	//if((AEMindex+AOMindex)!=0)
		//movebest=searchMove(0,piece,red,black,occupied,piece_count,-999999,999999);//先看走8步會不會更糟 
	//cout<<nowbest<<" "<<movebest<<endl;
	//if(nowbest>movebest||movebest==-999999)
		search(0,piece,red,black,occupied,piece_count,-999999,999999);
	IndexToBoard(srci,dsti);
}

void chess(U32 tblack,U32 tred,U32 tpiece[16],int deep)
{
	U32 dest;//可以吃子的行動
	AEMindex=0;
	AOMindex=0;
	EAEMindex=0;
	EAOMindex=0;
	U32 toccupied=0xFFFFFFFF;
	toccupied^=tpiece[0];
	int ssrc=0;
	//cout<<"-------------------------"<<endl;
	int check=(color+deep)%2;//0red 1black
	if(check==0){//紅 
		//cout<<"Ours available eat:"<<endl;
		for(int i=1; i<8; i++){ //1~7 為帥~兵,src 為棋子起點,dest 為終點。  先算我方 
			U32 p = tpiece[i]; //取得棋子位置
			while(p){ //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				if(i==1) //帥,周圍卒(14)以外的黑子都可以吃。
					dest = pMoves[ssrc] & ( tblack ^ tpiece[14] );
				else if(i==2) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] );
				else if(i==3) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] ^ tpiece[9] );
				else if(i==4) //?,只能吃車(11)、馬、炮、卒。
					dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14] );	
				else if(i==5) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14] );
				else if(i==6) //炮,特殊處理。
					dest = CGen(ssrc,toccupied) & tblack;
				else if(i==7) //兵,只能吃將(8)、卒(14)。
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
				dest = 0;
					while(dest){ //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allEatMove[AEMindex][0]=ssrc;
					allEatMove[AEMindex][1]=result;	
					AEMindex++;
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
				}
			}
		}
		//cout<<"Ours available move:"<<endl;
		for(int i=1; i<8; i++){ //紅方純移動
			U32 p = tpiece[i]; //取得棋子位置
			while(p){ //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
					dest = pMoves[ssrc] & tpiece[0];//只走空格
				while(dest){ //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allOnlyMove[AOMindex][0]=ssrc;
					allOnlyMove[AOMindex][1]=result;
					AOMindex++;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
				}
			}
		}
		//cout<<"Their available eat:"<<endl;
		for(int i=8; i<15; i++){ //再算對手行動 
			U32 p = tpiece[i]; 
			while(p){ 
				U32 mask = LS1B(p); 
				p ^= mask; 
				ssrc = GetIndex(mask); 
				if(i==8) 
					dest = pMoves[ssrc] & ( tred ^ tpiece[7] );
				else if(i==9) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] );
				else if(i==10) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] ^ tpiece[2] );
				else if(i==11) //?,只能吃車(11)、馬、炮、卒。
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7] );	
				else if(i==12) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7] );
				else if(i==13) //炮,特殊處理。
					dest = CGen(ssrc,toccupied) & tred;
				else if(i==14) //兵,只能吃將、卒(14)。
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
				dest = 0;
					while(dest){ //對手行動存入EallEatMove 
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallEatMove[EAEMindex][0]=ssrc;
					EallEatMove[EAEMindex][1]=result;	
					EAEMindex++;
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
				}
			}
		}
		//cout<<"Their available move:"<<endl;
		for(int i=8; i<15; i++){ //對手純移動 
			U32 p = tpiece[i]; //取得棋子位置
			while(p){ //將黑色 8~14 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
					dest = pMoves[ssrc] & tpiece[0];//只走空格
				while(dest){ //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallOnlyMove[EAOMindex][0]=ssrc;
					EallOnlyMove[EAOMindex][1]=result;
					EAOMindex++;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
				}
			}
		}
	}
	else{//黑方移動 
		//cout<<"Our available eat:"<<endl;
		for(int i=8; i<15; i++){ //1~7 為帥~兵,src 為棋子起點,dest 為終點。
			U32 p = tpiece[i]; //取得棋子位置
			while(p){ //將黑色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				if(i==8) //帥,周圍卒(14)以外的黑子都可以吃。
					dest = pMoves[ssrc] & ( tred ^ tpiece[7] );
				else if(i==9) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] );
				else if(i==10) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] ^ tpiece[2] );
				else if(i==11) //?,只能吃車(11)、馬、炮、卒。
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7] );	
				else if(i==12) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7] );
				else if(i==13) //炮,特殊處理。
					dest = CGen(ssrc,toccupied) & tred;
				else if(i==14) //兵,只能吃將、卒(14)。
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
				dest = 0;
					while(dest){ //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allEatMove[AEMindex][0]=ssrc;
					allEatMove[AEMindex][1]=result;	
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
					AEMindex++;
				}
			}
		}
		//cout<<"Our available move:"<<endl;
		for(int i=8; i<15; i++){ //黑方純移動
			U32 p = tpiece[i]; //取得棋子位置
			while(p){ //將黑色 8~14 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
					dest = pMoves[ssrc] & tpiece[0];//只走空格
				while(dest){ //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allOnlyMove[AOMindex][0]=ssrc;
					allOnlyMove[AOMindex][1]=result;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
					AOMindex++;
				}
			}
		}
		//cout<<"Their available eat:"<<endl;
		for(int i=1; i<8; i++){ //計算敵方移動 
			U32 p = tpiece[i]; //取得棋子位置
			while(p){ //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
				if(i==1) //帥,周圍卒(14)以外的黑子都可以吃。
					dest = pMoves[ssrc] & ( tblack ^ tpiece[14] );
				else if(i==2) //仕,周圍將(8)以外的黑子都可以吃。
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] );
				else if(i==3) //相,周圍將、士以外的黑子都可以吃。	
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] ^ tpiece[9] );
				else if(i==4) //?,只能吃車(11)、馬、炮、卒。
						dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14] );	
				else if(i==5) //傌,只能吃馬(12)、炮、卒。
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14] );
				else if(i==6) //炮,特殊處理。
					dest = CGen(ssrc,toccupied) & tblack;
				else if(i==7) //兵,只能吃將(8)、卒(14)。
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
				dest = 0;
					while(dest){ //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallEatMove[EAEMindex][0]=ssrc;
					EallEatMove[EAEMindex][1]=result;	
					EAEMindex++;
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
				}
			}
		}
		//cout<<"Their available move:"<<endl;
		for(int i=1; i<8; i++){ //紅方純移動
			U32 p = tpiece[i]; //取得棋子位置
			while(p){ //將紅色 1~7 號的子都搜尋一遍
				U32 mask = LS1B(p); //如果該棋子在多個位置,先取低位元的位置。
				p ^= mask; //除去位於最低位元的該兵種
				ssrc = GetIndex(mask); //將最低位元的兵種設為走步起點
					dest = pMoves[ssrc] & tpiece[0];//只走空格
				while(dest){ //如果 dest 有多個位置的話,分開存起來。
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallOnlyMove[EAOMindex][0]=ssrc;
					EallOnlyMove[EAOMindex][1]=result;
					EAOMindex++;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
				}
			}
		}
	}
}

U32 CGen(int ssrc,U32 toccupied)
{
	int r=ssrc/4;//列 
	int c=ssrc%4;//行 
	U32 result=0;
	U32 resulta=0;
	U32 x = ( (row[r] & toccupied) ^ (1<<ssrc) ) >> (4*r);//取出跟炮同列 削去炮本身那格
	if(x&&c==0){
		result|=CGenCL(x);
	}
	else if(x&&c==1){
		result|=((x&12)==12) ? 8 : 0;
	}
	else if(x&&c==2){
		result|=((x&3)==3) ? 1 : 0;
	}
	else if(x&&c==3){
		result|=CGenCR(x);
	}
	result=result << (4*r);
	
	x = ( (file[c] & toccupied) ^ (1<<ssrc) ) >> c;//取出跟炮同行 並全部放到第1行
	if(x&&r==0) 
	{
		resulta|=CGenCL(x);
	}
	else if(x&&r==1)
	{
		resulta|=CGenCL(x&0x11111100);
	}
	else if(x&&r==2)
	{
		U32 part = 0;
		if((x&0x00000011)==0x00000011)
		part |= 1;
		resulta |= part | CGenCL(x&0x11111000);
	}
	else if(x&&r==3)
	{
		U32 part = 0;
		part = CGenCR(x&0x00000111);
		resulta |= part | CGenCL(x&0x11110000);
	}
	else if(x&&r==4)
	{
		U32 part = 0;
		part = CGenCR(x&0x00001111);
		resulta |= part | CGenCL(x&0x11100000);
	}
	else if(x&&r==5)
	{
		U32 part = 0;
		part = CGenCR(x&0x00011111);
		resulta |= part | CGenCL(x&0x11000000);
	}
	else if(x&&r==6)
	{
		resulta |= CGenCR(x&0x00111111);
	}
	else if(x&&r==7)
	{
		resulta |= CGenCR(x);
	}
	resulta=resulta<<c;
	U32 re=result|resulta;
	//cout<<hex<<occupied<<" "<<re<<endl;
	//cout<<dec<<r<<" "<<c<<endl;
	return re;
}

U32 CGenCL(U32 x){
	if(x){
		U32 mask = LS1B(x); //mask 為炮架的遮罩位置,讓 x 消去炮架。
		return (x ^= mask) ? LS1B(x) : 0; //狀況 5~8 傳回 LS1B(x),狀況 2~4 傳回 0。
	}else return 0;
}

U32 CGenCR(U32 x){
	if(x){
		U32 mask = MS1B(x); //mask 為炮架的遮罩位置,讓 x 消去炮架。
		return (x ^= mask) ? MS1B(x) : 0; //狀況 5~8 傳回 MS1B(x),狀況 2~4 傳回 0。
	}else return 0;
}

int ccc=0;
int countAva(int pie[14],int deep,U32 curPiece[16])//將士相車馬炮兵
{
	int eat[10];
	int biggest=0;
	//for(int i=0;i<EAEMindex;i++)
	//{
		//eat[i]=findPiece(EallEatMove[i][1],curPiece);//把下一步會被吃得棋子存入eat 0~13
		//if(eat[i]==-1)//找到被吃子的位置 -1為找不到 原因不確定 
		//{
			//U32 p=1<<EallEatMove[i][1];
			//U32 p1=1<<EallEatMove[i][0];
			//for(int l=1;l<=14;l++)
			//{
				//cout<<hex<<curPiece[l]<<dec<<endl;
				//if((curPiece[l]&p)!=0) cout<<"found!!!!!!!!!"<<endl;
			//}
			//cout<<"here"<<endl;
			//cout<<hex<<p1<<" "<<p<<dec<<endl;
			//cout<<"-----------------------"<<endl;
			//eat[i]=0;
		//}
		//else
			//eat[i]=piPw[eat[i]];//把分數存進eat[i] 
		//if(biggest<eat[i]) biggest=eat[i];//找到eat中最大的棋子 
	//}
	if(deep!=maxDepth) biggest=0;
	int power=0;
	if(color==0)//紅 
	{
		for(int i=0;i<7;i++)
			power+=pie[i]*piPw[i];
		for(int i=7;i<14;i++)
			power-=pie[i]*piPw[i];
	}
	else
	{
		for(int i=7;i<14;i++)
			power+=pie[i]*piPw[i];
		for(int i=0;i<7;i++)
			power-=pie[i]*piPw[i];
	}
	//if(ccc==0)
	//{
		//for(int i=0;i<14;i++)
			//cout<<pie[i]<<" ";
		//cout<<endl;
		//ccc=1;
	//}
	int movePoint=0;
	if(deep%2==0)
		movePoint=AEMindex*10+AOMindex-EAEMindex*20-EAOMindex;
	else
		movePoint=EAEMindex*10+EAOMindex-AEMindex*20-AOMindex;
	//cout<<AEMindex<<" "<<AOMindex<<" "<<EAEMindex<<" "<<EAOMindex<<" "<<movePoint+power<<endl;
	//cout<<power;
	//if(deep%2!=0) biggest=-biggest;
	return power+movePoint-deep;
}

int search(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta)
{

	for (int i = 0; i < 16; i++) {
		test4.push_back(curPiece[i]);
	}
	test5.push_back(test4);
	test4.clear();

	/*for (int j = 0; j < test5.size(); j++) {
		cout << "step: " << j << endl;
		for (int i = 0; i < 16; i++) {
			cout << "curpiece= " << i << hex << " " << test5[j][i] << dec << endl;
		}
	}

	cout << "tempdepth: " << tempdepth << endl;
	cout << "depth: " << depth << endl;
	cout << "test5 size: " << test5.size() << endl;
	system("pause");*/

	curRed=curPiece[1]|curPiece[2]|curPiece[3]|curPiece[4]|curPiece[5]|curPiece[6]|curPiece[7];
	curBlack=curPiece[8]|curPiece[9]|curPiece[10]|curPiece[11]|curPiece[12]|curPiece[13]|curPiece[14];
	//for(int ii=1;ii<15;ii++){
		//cout<<hex<<curPiece[ii]<<dec<<endl;
	//}
	//cout<<"b"<<hex<<curBlack<<dec<<endl;
	//cout<<"r"<<hex<<curRed<<dec<<endl;
	//cout<<"next"<<endl;
	//system("Pause");
	chess(curBlack,curRed,curPiece,depth);
	U32 taEM[50][2];//存可吃子的方法 0 src 1 dst 避免被往下搜尋時刷掉
	int tAEMi=AEMindex;//alleatmove index
	U32 taOM[50][2];//存可移動非吃子的方法 0 src 1 dst
	int tAOMi=AOMindex;//allonlymove index
	int etaemi=EAEMindex;
	if(depth%2==1) etaemi=AEMindex;
	memcpy(taEM,allEatMove,sizeof(taEM));
	memcpy(taOM,allOnlyMove,sizeof(taOM));
	int re=countAva(curPie,depth,curPiece);
	int weight[100][3];//計算所有移動與翻棋的得分0src 1dst 2weight 
	int wp = 0;
	int best = -9999999;
	if(depth==maxDepth||(depth>=noReDepth&&(tAEMi+tAOMi)==0))
	{

		test.push_back(curPiece);
		test2.push_back(re);
		/*cout << "weight[wp][2]: " << weight[wp][2] << endl;
		cout << "wp: " << wp << endl;
		cout << "best: " << best << endl;*/
		test3.push_back(test5);
		cout<<"re "<<re<<endl;
		for(int i=0;i<test5.size();i++){
			cout<<endl;
			for(int j=0;j<16;j++){
				cout<<"curpiece= " << j << hex << " " << test5[i][j] << dec << endl;
			}
		}
		return re;
	}
	if(depth%2==1)
		best=9999999;
	if(curPiece[15]!=0&&depth>noReDepth)//可以走空步
	{
		weight[wp][0]=0;weight[wp][1]=0;weight[wp][2]=re;
		wp++;
	}
	if(curPiece[15]!=0&&false)//先試翻棋 做完後call search 
	{
		for(int ssrc=0; ssrc<32; ssrc++){ //搜尋盤面上 32 個位置
			if(curPiece[15] & ( 1 << ssrc ) && ch & ( 1 << ssrc )&&depth<=noReDepth){ //若為未翻子 在未翻子的遮罩內 depth<=3 
				if(depth==0)
				{
					int r=rand()%6;;
					string a[6]={"⊙3⊙","(--;)","(〃ω〃)","(’-_-`)","|ω˙）","(*≧艸≦)"};
					cout<<a[r]+".";

				}
				weight[wp][2]=0;
				//cout<<endl;
				int a=0;
				for(int pID=0; pID<14; pID++){ //搜尋可能會翻出之子
					if(DCount[pID]){ //若該兵種可能被翻出
						a+=DCount[pID];
						int deeper=depth+1;
						U32 c=1<<ssrc;
						int cpID=pID+1;
						if(pID<8)
						{
							curPiece[cpID]|=c;
							//curRed|=c;
							curPiece[15]^=c;
							DCount[pID]--;
						}
						else
						{
							curPiece[cpID]|=c;
							//curBlack|=c;
							curPiece[15]^=c;
							DCount[pID]--;
						}//模擬該兵種翻出來
						//cout<<pID<<" ";
						weight[wp][0] =ssrc;
						weight[wp][1] =ssrc;
						weight[wp][2] += ((DCount[pID]+1)*search(deeper,curPiece,curRed,curBlack,curOccupied,curPie,alpha,beta));
						test5.pop_back();
						ccc=0;
						if(pID<8)
						{
							curPiece[cpID]^=c;
							//curRed^=c;
							curPiece[15]|=c;
							DCount[pID]++;
						}
						else
						{
							curPiece[cpID]^=c;
							//curBlack^=c;
							curPiece[15]|=c;
							DCount[pID]++;
						}//將模擬翻出的子復原
					}
				}
				weight[wp][2]/=a;
				if(depth%2==0)//max
				{
					if(weight[wp][2]>alpha)
					{
						alpha=weight[wp][2];
					}
					if(weight[wp][2]>best)
					{
						best=weight[wp][2];
					}
				}
				else//min
				{
					if(weight[wp][2]<beta)
					{
						beta=weight[wp][2];
					}
					if(weight[wp][2]<best)
					{
						best=weight[wp][2];
					}
				}
				if(beta<=alpha)
				{
					if(depth%2==1)
					{
						return beta;
					}
					else
					{
						return alpha;
					} 
				}
				wp++;
			}
		}
	}
	
	if(tAEMi+tAOMi!=0)//試吃子 
	{
		if(tAEMi>0)
		for(int i=0;i<tAEMi;i++)
		{
			weight[wp][2]=0;
			int deeper=depth+1;
			//SimMove(taEM[i][0],taEM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
	int c1p,c2p=-1;
	U32 c1=1<<taEM[i][0];
	U32 c2=1<<taEM[i][1];
	//cout<<hex<<c2; 
	for(int ii=1;ii<15;ii++){
		U32 check=curPiece[ii]&c1;
		if(check!=0){
			c1p=ii;
			break;
		}
	}
	for(int ii=1;ii<15;ii++){//找c2 清掉 剩餘棋子數更改 
		U32 check=curPiece[ii]&c2;
		if(check!=0){
			c2p=ii;
			break;
		}
	}
	int c2pcopy;
	c2pcopy=c2p;
	curPiece[c1p]^=c1;//清除原位置c1
	curPiece[c1p]|=c2;//移動
	curPiece[0]|=c1;//空格+c1
	curOccupied^=c1;//c1無子
	if(c2p!=-1){//吃子移動 
		curPiece[c2p]^=c2;//清除原位置c2
		int c2ps=c2p-1;
		curPie[c2ps]--;
		//if(8>c1p){//紅吃黑
			//curRed^=c1;
			//curBlack^=c2;
			//curRed|=c2;
		//}
		//else{//黑吃紅 
			//curBlack^=c1;
			//curRed^=c2;
			//curBlack|=c2;
		//**/}
	}
	else//無吃子移動 
	{
		//if(8>c1p){//紅動 
			//curRed^=c1;
			//curRed|=c2;
		//}
		//else{//黑動 
			//curBlack^=c1;
			//curBlack|=c2;
		//}
		curPiece[0]^=c2;//空格-c2
		curOccupied|=c2;//c2有子 
	}
			weight[wp][0] =taEM[i][0];
			weight[wp][1] =taEM[i][1];
			weight[wp][2] =search(deeper,curPiece,curRed,curBlack,curOccupied,curPie,alpha,beta);
			test5.pop_back();
			//if(taEM[i][1]==13||taEM[i][1]==14||taEM[i][1]==17||taEM[i][1]==18) weight[wp][2];//移動到 13 14 17 18時+1; 
			//UnSimMove(taEM[i][0],taEM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie,copy);
	c1p=-1;
	c2p=-1;
	c1=1<<taEM[i][0];
	c2=1<<taEM[i][1];
	//cout<<hex<<c2; 
	for(int ii=1;ii<15;ii++){
		U32 check=curPiece[ii]&c2;
		if(check!=0){
			c1p=ii;
			break;
		}
	}
	c2p=c2pcopy;
	curPiece[c1p]^=c2;//清除原位置c2
	curPiece[0]|=c2;//空格+c2
	curPiece[c1p]|=c1;//移動
	curOccupied|=c1;//c1有子
	curPiece[0]^=c1;//空格-c1
	if(c2p!=-1){//吃子移動 
		curPiece[c2p]|=c2;//回原位置c2
		int c2ps=c2p-1;
		curPie[c2ps]++;
		curPiece[0]^=c2;
		/*if(8>c1p){//紅吃黑
			curRed|=c1;
			curBlack|=c2;
			curRed^=c2;
		}
		else{//黑吃紅 
			curBlack|=c1;
			curRed|=c2;
			curBlack^=c2;
		}
		*/
	}
	else//無吃子移動 
	{
		/*
		if(8>c1p){//紅動 
			curRed|=c1;
			curRed^=c2;
		}
		else{//黑動 
			curBlack|=c1;
			curBlack^=c2;
		}
		*/
		curOccupied^=c2;//c2無子 
	}
			if(depth%2==0)//max
			{
				if(weight[wp][2]>alpha)
				{
					alpha=weight[wp][2];
				}
				if(weight[wp][2]>best)
				{
					best=weight[wp][2];
				}
			}
			else//min
			{
				if(weight[wp][2]<beta)
				{
					beta=weight[wp][2];
				}
				if(weight[wp][2]<best)
				{
					best=weight[wp][2];
				}
			}
			if(beta<=alpha)
			{
				if(depth%2==1)
					return beta;
				else
					return alpha;
			}
			wp++;
		}
		for(int i=0;i<tAOMi;i++)//純移動 
		{
			weight[wp][2]=0;
			int deeper=depth+1;
			//SimMove(taOM[i][0],taOM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
	int c1p,c2p=-1;
	U32 c1=1<<taOM[i][0];
	U32 c2=1<<taOM[i][1];
	//cout<<hex<<c2; 
	for(int ii=1;ii<15;ii++){
		U32 check=curPiece[ii]&c1;
		if(check!=0){
			c1p=ii;
			break;
		}
	}
	for(int ii=1;ii<15;ii++){//找c2 清掉 剩餘棋子數更改 
		U32 check=curPiece[ii]&c2;
		if(check!=0){
			c2p=ii;
			cout<<"err";
		}
	}
	int c2pcopy=0;
	c2pcopy=c2p;
	curPiece[c1p]^=c1;//清除原位置c1
	curPiece[c1p]|=c2;//移動
	curPiece[0]|=c1;//空格+c1
	curOccupied^=c1;//c1無子
	if(c1p>15||c1p<1)
	{
		for(int ii=1;ii<15;ii++){
			cout<<hex<<curPiece[ii]<<dec<<endl;
		}
		cout<<hex<<curBlack<<"b r"<<curRed<<endl;
		cout<<c1<<" "<<c2<<dec<<endl;
		cout<<"next>>"<<endl<<endl;
	}
	if(c2p!=-1){//吃子移動 
		curPiece[c2p]^=c2;//清除原位置c2
		int c2ps=c2p-1;
		curPie[c2ps]--;
		/*
		if(8>c1p){//紅吃黑
			curRed^=c1;
			curBlack^=c2;
			curRed|=c2;
		}
		else{//黑吃紅 
			curBlack^=c1;
			curRed^=c2;
			curBlack|=c2;
		}
		*/
	}
	else//無吃子移動 
	{
		/*
		if(8>c1p){//紅動 
			curRed^=c1;
			curRed|=c2;
		}
		else{//黑動 
			curBlack^=c1;
			curBlack|=c2;
		}
		*/
		curPiece[0]^=c2;//空格-c2
		curOccupied|=c2;//c2有子 
	}
			weight[wp][0] =taOM[i][0];
			weight[wp][1] =taOM[i][1];
			weight[wp][2] =search(deeper,curPiece,curRed,curBlack,curOccupied,curPie,alpha,beta);
			test5.pop_back();
			//if(taEM[i][1]==13||taEM[i][1]==14||taEM[i][1]==17||taEM[i][1]==18) weight[wp][2];//移動到 13 14 17 18時+1; 
			//UnSimMove(taOM[i][0],taOM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie,copy);
	c1p=-1;
	c2p=-1;
	c1=1<<taOM[i][0];
	c2=1<<taOM[i][1];
	//cout<<hex<<c2; 
	for(int ii=1;ii<15;ii++){
		U32 check=curPiece[ii]&c2;
		if(check!=0){
			c1p=ii;
			break;
		}
	}
	c2p=c2pcopy;
	curPiece[c1p]^=c2;//清除原位置c2
	curPiece[0]|=c2;//空格+c2
	curPiece[c1p]|=c1;//移動
	curOccupied|=c1;//c1有子
	curPiece[0]^=c1;//空格-c1
	if(c2p!=-1){//吃子移動 
		curPiece[c2p]|=c2;//回原位置c2
		int c2ps=c2p-1;
		curPie[c2ps]++;
		curPiece[0]^=c2;
		/*
		if(8>c1p){//紅吃黑
			curRed|=c1;
			curBlack|=c2;
			curRed^=c2;
		}
		else{//黑吃紅 
			curBlack|=c1;
			curRed|=c2;
			curBlack^=c2;
		}
		*/
	}
	else//無吃子移動 
	{
		/*
		if(8>c1p){//紅動 
			curRed|=c1;
			curRed^=c2;
		}
		else{//黑動 
			curBlack|=c1;
			curBlack^=c2;
		}
		*/
		curOccupied^=c2;//c2無子 
	}
			if(depth%2==0)//max
			{
				if(weight[wp][2]>best)
				{
					best=weight[wp][2];
				}
				if(weight[wp][2]>alpha)
				{
					alpha=weight[wp][2];
				}
			}
			else//min
			{
				if(weight[wp][2]<best)
				{
					best=weight[wp][2];
				}
				if(weight[wp][2]<beta)
				{
					beta=weight[wp][2];
				}
			}
			if(beta<=alpha)
			{
				if(depth%2==1)
					return beta;
				else
					return alpha;
			}
			wp++;
		}
	}


	if(depth==0)//max
	{
		cout<<endl<<"------------------------------------"<<endl;
		int recordi=0;
		fstream file;
		file.open("debug.txt",ios::app);
		for(int i=wp-1;i>=0;i--)
		{
			if (depth == 0)
			{
				file << weight[i][0] << " " << weight[i][1] << " " << weight[i][2] << endl;
				int temp = 0;
				int q = 0;
				for (int j = 0; j < test2.size(); j++) {
					if (test2[j] == weight[i][2]) {
						temp = j;
						q++;
					}
				}
				file << "q " << q << endl;
				q = 0;
				/*cout << "temp: "<<temp<<endl;
				cout << "test2: " << test2.size() << endl;
				cout << "test3: " << test3.size() << endl;
				cout << "weight: " << wp << endl;*/
				for (int k = 0; k < test3[temp].size(); k++) {
					
					file << "step: " << k << endl;
					file << "curpiece= " << 0 << hex << " " << test3[temp][k][0] << dec << endl;
					file << endl;
					for (int j = 1; j < 8; j++) {
						file << "curpiece= " << j << hex << " " << test3[temp][k][j] << dec << endl;
						/*if (GetIndex(test3[temp][k][j]) != 0) { 
							cout << j <<" "<< GetIndex(test3[temp][k][j]) << endl; 
						}
						for (int i = 0; i < 8; i++) {// print board
							cout << 8 - i;
							for (int j = 0; j < 4; j++) {	// top down	a7-b7-c7-d7
															//			a6-b6-c6-d6

								translate(showboard[j][7 - i]);

							}
							cout << "\n";
						}
						cout << "\ta\tb\tc\td\t" << endl;
						*/
					}
					file << endl;
					for (int j = 8; j < 15; j++) {
						file << "curpiece= " << j << hex << " " << test3[temp][k][j] << dec << endl;
					}
					file << endl;
					file << "curpiece= " << 15 << hex << " " << test3[temp][k][15] << dec << endl;
				}

			}
			if(weight[i][2]==best)
			{
				recordi=i;
				srci=weight[i][0];
				dsti=weight[i][1];
			}
		}
		file.close();
		if(draw==0)
		{
		}
		else if(draw==1)
		{
			if(best<0); //可能輸 故意平手? 
			else if(srci==past_walk[1][1]&&dsti==past_walk[1][0])
			{
				cout<<"draw denied"<<endl;
				weight[recordi][2]-=999999;//可能會贏 選擇不平手? 
				best=weight[0][2];
				for(int ii=0;ii<wp;ii++)
				{
					if(weight[ii][0]==srci)//那顆棋子相關全部都-999999 
					weight[ii][2]-=999999;
				}
				for(int ii=wp-1;ii>=0;ii--)//重新尋找 
				{
					if(weight[ii][2]>best)
					{
						best=weight[ii][2];
						srci=weight[ii][0];
						dsti=weight[ii][1];
					}
				}
			}
		}
		//srci=weight[same[rnd]][0];
		//dsti=weight[same[rnd]][1];
	}
	if(wp==0) best=re;
	if(best==9999999||best==-9999999) best=re;

	//system("pause");
	return best;
}


int* SimMove(int ssrc,int sdst,U32* curPiece,U32* curRed,U32* curBlack,U32* curOccupied,int* curPie)
{
	int c1p,c2p=-1;
	U32 c1=1<<ssrc;
	U32 c2=1<<sdst;
	//cout<<hex<<c2; 
	for(int ii=1;ii<15;ii++){
		U32 check=curPiece[ii]&c1;
		if(check!=0){
			c1p=ii;
			break;
		}
	}
	for(int ii=1;ii<15;ii++){//找c2 清掉 剩餘棋子數更改 
		U32 check=curPiece[ii]&c2;
		if(check!=0){
			c2p=ii;
		}
	}
	//c2pcopy=c2p;
	curPiece[c1p]^=c1;//清除原位置c1
	curPiece[c1p]|=c2;//移動
	curPiece[0]|=c1;//空格+c1
	*curOccupied^=c1;//c1無子
	if(c2p!=-1){//吃子移動 
		curPiece[c2p]^=c2;//清除原位置c2
		int c2ps=c2p-1;
		curPie[c2ps]--;
		if(8>c1p){//紅吃黑
			*curRed^=c1;
			*curBlack^=c2;
			*curRed|=c2;
		}
		else{//黑吃紅 
			*curBlack^=c1;
			*curRed^=c2;
			*curBlack|=c2;
		}
	}
	else//無吃子移動 
	{
		if(8>c1p){//紅動 
			*curRed^=c1;
			*curRed|=c2;
		}
		else{//黑動 
			*curBlack^=c1;
			*curBlack|=c2;
		}
		curPiece[0]^=c2;//空格-c2
		*curOccupied|=c2;//c2有子 
	}
}

int* UnSimMove(int ssrc,int sdst,U32* curPiece,U32* curRed,U32* curBlack,U32* curOccupied,int* curPie,int copy)
{
	int c1p,c2p=-1;
	U32 c1=1<<ssrc;
	U32 c2=1<<sdst;
	//cout<<hex<<c2; 
	for(int ii=1;ii<15;ii++){
		U32 check=curPiece[ii]&c2;
		if(check!=0){
			c1p=ii;
			break;
		}
	}
	c2p=copy;
	curPiece[c1p]^=c2;//清除原位置c2
	curPiece[0]|=c2;//空格+c2
	curPiece[c1p]|=c1;//移動
	*curOccupied|=c1;//c1有子
	curPiece[0]^=c1;//空格-c1
	if(c2p!=-1){//吃子移動 
		curPiece[c2p]|=c2;//回原位置c2
		int c2ps=c2p-1;
		curPie[c2ps]++;
		curPiece[0]^=c2;
		if(8>c1p){//紅吃黑
			*curRed|=c1;
			*curBlack|=c2;
			*curRed^=c2;
		}
		else{//黑吃紅 
			*curBlack|=c1;
			*curRed|=c2;
			*curBlack^=c2;
		}
	}
	else//無吃子移動 
	{
		if(8>c1p){//紅動 
			*curRed|=c1;
			*curRed^=c2;
		}
		else{//黑動 
			*curBlack|=c1;
			*curBlack^=c2;
		}
		*curOccupied^=c2;//c2無子 
	}
}

void SimReveal(int ssrc,int pID,U32 curPiece[16],U32 curRed,U32 curBlack)
{
	U32 c=1<<ssrc;
	if(pID<8)
	{
		curPiece[pID]|=c;
		curRed|=c;
		curPiece[15]^=c;
	}
	else
	{
		curPiece[pID]|=c;
		curBlack|=c;
		curPiece[15]^=c;
	}
}

void UnSimReveal(int ssrc,int pID,U32 curPiece[16],U32 curRed,U32 curBlack)
{
	U32 c=1<<ssrc;
	if(pID<8)
	{
		curPiece[pID]^=c;
		curRed^=c;
		curPiece[15]|=c;
	}
	else
	{
		curPiece[pID]^=c;
		curBlack^=c;
		curPiece[15]|=c;
	}
}

void drawOrNot()
{
	int a=past_walk[0][0];
	int b=past_walk[0][1];
	int c=past_walk[1][1];
	int d=past_walk[1][0];
	//if(a==past_walk[2][1]&&a==past_walk[4][0]&&b==past_walk[2][0]&&b==past_walk[4][1]&&c==past_walk[3][0]&&d==past_walk[3][1])
	//draw=1;
	
	if(a==past_walk[2][1]&&a==past_walk[4][0]&&a==past_walk[6][1]&&b==past_walk[2][0]&&b==past_walk[4][1]&&b==past_walk[6][0]&&c==past_walk[3][0]&&
	c==past_walk[5][1]&&d==past_walk[3][1]&&d==past_walk[5][0])
	draw=1;
	
	cout<<draw<<endl;
}

void dynamicPower()
{
	piPw[0]=6000+piece_count[7]*600+piece_count[8]*270+piece_count[9]*90+piece_count[10]*40+piece_count[11]*15+piece_count[12]*200;
	piPw[1]=2700+piece_count[8]*270+piece_count[9]*90+piece_count[10]*40+piece_count[11]*15+piece_count[12]*200+piece_count[13]*10;
	piPw[2]=900+piece_count[9]*90+piece_count[10]*40+piece_count[11]*15+piece_count[12]*200+piece_count[13]*10;
	piPw[3]=400+piece_count[10]*40+piece_count[11]*15+piece_count[12]*200+piece_count[13]*10;
	piPw[4]=150+piece_count[11]*15+piece_count[12]*200+piece_count[13]*10;
	piPw[5]=2000+piece_count[7]*600+piece_count[8]*270+piece_count[9]*90+piece_count[10]*40+piece_count[11]*15+piece_count[12]*200+piece_count[13]*10;
	piPw[6]=100+piece_count[7]*600+piece_count[13]*10;
	
	piPw[7]=6000+piece_count[0]*600+piece_count[1]*270+piece_count[2]*90+piece_count[3]*40+piece_count[4]*15+piece_count[5]*200;//24300
	piPw[8]=2700+piece_count[1]*270+piece_count[2]*90+piece_count[3]*40+piece_count[4]*15+piece_count[5]*200+piece_count[6]*10;//15500
	piPw[9]=900+piece_count[2]*90+piece_count[3]*40+piece_count[4]*15+piece_count[5]*200+piece_count[6]*10;//8300
	piPw[10]=400+piece_count[3]*40+piece_count[4]*15+piece_count[5]*200+piece_count[6]*10;//6000
	piPw[11]=150+piece_count[4]*15+piece_count[5]*200+piece_count[6]*10;//4950
	piPw[12]=2000+piece_count[0]*600+piece_count[1]*270+piece_count[2]*90+piece_count[3]*40+piece_count[4]*15+piece_count[5]*200+piece_count[6]*10;//20800
	piPw[13]=100+piece_count[0]*600+piece_count[6]*10;//6600
}

int findPiece(int place,U32 curPiece[16])
{
	U32 bplace=1<<place;
	for(int i=1;i<15;i++)//找14種棋中 誰在這個位置上 
	{
		if((curPiece[i]&bplace)!=0)
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
	int cp=0;//current_position計算位址 
	int line=0;//目前讀到的行數 
	ifstream file;
	string str;
	file.open("board.txt",ios::in);
	vector<string> aStr;
	int startmove = 0;
	bool first=true;
	while (getline(file, str))
	{
		aStr.push_back(str);
		//cout<<str<<endl;
		if(line==2)
		{
			for(int i=0;i<=13;i++)
			{
				piece_count[i]=str[i*2+2]-'0';//把剩餘棋子讀入piece_count 
			}
		}
		if(line>=3&&line<=10&&initailBoard==1)
		{
			for(int i=0;i<=3;i++)
			{
				current_position[cp]=str[i*2+2];//讀入初始場上狀況 只讀一次 
				cp++;
			}
			if(line==10)
				initailBoard=0;
		}
		
		if (str.at(2) == 't') {//start after * time 900
			startmove = 1;
		}
		if (str.at(2) == 'C'&&str.at(3)=='o') {//stop before * Comment 0 0
			startmove = 0;
		}
		if (startmove == 1) {
			if (str.length() >= 10 && str.at(2) != 't' && str.length() <= 12) {
				//cout << str.substr(str.length() - 5, 5)<<"\n";
				move.push_back(str.substr(str.length()-5, 5));//a1(k),a1-a2	
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
	else if(!move.empty()){//後手
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
		if (move[i].at(2) == '(') {//record move 'flip' (kgmrncp)
			//string a=move[i].substr(0,2);
			int aa=100-move[i].at(0);
			int bb=4*(56-move[i].at(1));
			U32 cc=aa+bb;//棋盤編號0~31
			cc=1<<cc;
			if(move[i].at(3)=='K') {piece[1]|=cc;red|=cc;piece[15]^=cc;DCount[0]--;}
			if(move[i].at(3)=='G') {piece[2]|=cc;red|=cc;piece[15]^=cc;DCount[1]--;}
			if(move[i].at(3)=='M') {piece[3]|=cc;red|=cc;piece[15]^=cc;DCount[2]--;}
			if(move[i].at(3)=='R') {piece[4]|=cc;red|=cc;piece[15]^=cc;DCount[3]--;}
			if(move[i].at(3)=='N') {piece[5]|=cc;red|=cc;piece[15]^=cc;DCount[4]--;}
			if(move[i].at(3)=='C') {piece[6]|=cc;red|=cc;piece[15]^=cc;DCount[5]--;}
			if(move[i].at(3)=='P') {piece[7]|=cc;red|=cc;piece[15]^=cc;DCount[6]--;}
			if(move[i].at(3)=='k') {piece[8]|=cc;black|=cc;piece[15]^=cc;DCount[7]--;}
			if(move[i].at(3)=='g') {piece[9]|=cc;black|=cc;piece[15]^=cc;DCount[8]--;}
			if(move[i].at(3)=='m') {piece[10]|=cc;black|=cc;piece[15]^=cc;DCount[9]--;}
			if(move[i].at(3)=='r') {piece[11]|=cc;black|=cc;piece[15]^=cc;DCount[10]--;}
			if(move[i].at(3)=='n') {piece[12]|=cc;black|=cc;piece[15]^=cc;DCount[11]--;}
			if(move[i].at(3)=='c') {piece[13]|=cc;black|=cc;piece[15]^=cc;DCount[12]--;}
			if(move[i].at(3)=='p') {piece[14]|=cc;black|=cc;piece[15]^=cc;DCount[13]--;}
			//board[move[i].at(0)-'a'][move[i].at(1)-'1'] = move[i].at(3);	//board[a][1]=k,ai(k)
			//cout << board[move[i].at(0) - 'a'][move[i].at(1) - '1'] ;
		}
		else {//a1-b1之類 
			int c1p,c2p;
			c2p=-1;//如果是-1為移動 
			int aa=100-move[i].at(0);
			int bb=4*(56-move[i].at(1));
			U32 c1=aa+bb;//棋盤編號0~31
			past_walk[6][0]=past_walk[5][0];
			past_walk[5][0]=past_walk[4][0];
			past_walk[4][0]=past_walk[3][0];
			past_walk[3][0]=past_walk[2][0];
			past_walk[2][0]=past_walk[1][0];
			past_walk[1][0]=past_walk[0][0];
			past_walk[0][0]=aa+bb;
			c1=1<<c1;
			//cout<<hex<<c1<<"c1 "; 
			
			int aaa=100-move[i].at(3);
			int bbb=4*(56-move[i].at(4));
			U32 c2=aaa+bbb;//棋盤編號0~31
			past_walk[6][1]=past_walk[5][1];
			past_walk[5][1]=past_walk[4][1];
			past_walk[4][1]=past_walk[3][1];
			past_walk[3][1]=past_walk[2][1];
			past_walk[2][1]=past_walk[1][1];
			past_walk[1][1]=past_walk[0][1];
			past_walk[0][1]=aaa+bbb;
			c2=1<<c2;
			//cout<<hex<<c2; 
			for(int ii=1;ii<15;ii++){
				U32 check=piece[ii]&c1;
				if(check!=0){
					c1p=ii;
					break;
				}
			}
			for(int ii=1;ii<15;ii++){//找c2 清掉 剩餘棋子數更改 
				U32 check=piece[ii]&c2;
				if(check!=0){
					c2p=ii;
				}
			}
			piece[c1p]^=c1;//清除原位置c1
			piece[0]|=c1;//空格+c1
			piece[c1p]|=c2;//移動
			if(c2p!=-1){//吃子移動 
				piece[c2p]^=c2;//清除原位置c2
				int c2ps=c2p-1;
				piece_count[c2ps]--;
				if(8>c1p){//紅吃黑
					red^=c1;
					black^=c2;
					red|=c2;
				}
				else{//黑吃紅 
					black^=c1;
					red^=c2;
					black|=c2;
				}
				occupied^=c1;//c1無子 
			}
			else//無吃子移動 
			{
				if(8>c1p){//紅動 
					red^=c1;
					red|=c2;
				}
				else{//黑動 
					black^=c1;
					black|=c2;
				}
				piece[0]^=c2;//空格-c2
				occupied^=c1;//c1無子 
				occupied|=c2;//c2有子 
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
	file.open("move.txt",ios::out);
	if(src==dst)
		file<<"1"<<endl;
	else
		file<<"0"<<endl;
	if(src==dst)
		file<<src<<endl<<"0"<<endl;
	else
		file<<src<<endl<<dst<<endl;
	cout<<srci<<"."<<src<<":src "<<dsti<<"."<<dst<<":dst "<<endl;
	file.close();
}

void initial()
{
	for(int i=0;i<=14;i++)
		piece[i]=0;
	red=0;
	black=0;
	occupied=0xFFFFFFFF;
	piece[15]=0xFFFFFFFF;
}

void IndexToBoard(int indexa,int indexb)
{
	stringstream ss;
	string aa;
	int a=indexa/4;//0~7
	int b=indexa%4;//0~3 a~d
	if(b==0) aa="d";
	else if(b==1) aa="c";
	else if(b==2) aa="b";
	else if(b==3) aa="a";
	ss << 8-a;
	src=aa+ss.str();
	//cout<<"src : "<<src<<endl;
	
	stringstream sss;
	a=indexb/4;//0~7
	b=indexb%4;//0~3 a~d
	if(b==0) aa="d";
	else if(b==1) aa="c";
	else if(b==2) aa="b";
	else if(b==3) aa="a";
	sss << 8-a;
	dst=aa+sss.str();
	//cout<<"dst : "<<dst<<endl;
}
