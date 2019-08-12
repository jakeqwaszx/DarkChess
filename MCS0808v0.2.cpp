#include <iostream>
#include <cstdlib>
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
#include <queue>
#include <stack>
//目前版本2.6 使用pmove2遮罩減少搜尋時間 翻子搜尋只到depth=2 depth3以上只搜移動或吃子
//阻止平手 該子全部-999999 有計算movepoint(可吃或被吃加減分) 勝負加減分 最後吃或被吃算分完成(停用) 

//由2.6.1改成蒙地卡羅版 測試階段 

typedef unsigned int U32;
using namespace std;

struct node//存整棵樹 用來更新祖先節點 
{
	double UCB;
	double win;
	double round;
	int depth;
	int src;
	int dst;
	U32 piece[16];
	U32 black;
	U32 red;
	int piece_count[14];//算分用 
	int DCount[14];//翻棋用 
	node *parent;
	vector<node *> child;
	bool deeper=true;
};
node *currentParent;
double allRound=0;//總共模擬的次數 
vector<vector<node *> > nodes;//存所有node
int c=1;//控制常數
int simRound=100;//每個點模擬次數 
int simDepth=20;//每個點模擬深度 
vector <int>chMask;//翻棋遮罩 存編號 
int stdScore=0;//儲存標準分數 用來判斷現在盤面是否優勢 

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
void ai3();//執行蒙地卡羅 
int MCSsimulation(int curSimRound,node *SimNode);//模擬走步 傳入模擬次數與模擬盤面 回傳優勢場數 
void readBoard();//讀檔模式 讀取board.txt 把讀入檔案轉成bitboard 還沒倒著存入 
void createMovetxt();//創造move.txt 0走步 1翻棋 
void IndexToBoard(int indexa,int indexb);//把src dst從編號0~31->棋盤編號a1~d4 
int countAva(int pie[14],int deep,U32 curPiece[16]);//呼叫則傳回當前棋版
int search(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta);//搜尋最佳走步 
int onewaySearch(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta,int eat);//單向搜尋 測試 
int searchMove(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta);//先搜尋可走版面
void dynamicPower();//計算動態棋力 
void drawOrNot();//由past_walk判斷是否平手 之後結果輸出給draw 
int findPiece(int place,U32 curPiece[16]);//傳編號 回傳在這個編號的棋子 

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
//int RMcount=13;//讀取模式需要 從13行開始讀取結果 

U32 allEatMove[50][2];//存可吃子的方法 0 src 1 dst 可執行 
int AEMindex=0;//alleatmove index
U32 allOnlyMove[50][2];//存可移動非吃子的方法 0 src 1 dst
int AOMindex=0;//allonlymove index
U32 EallEatMove[50][2];//存對手可吃子的方法 0 src 1 dst 下回合才有可能執行 
int EAEMindex=0;//Ealleatmove index
U32 EallOnlyMove[50][2];//存對手可移動非吃子的方法 0 src 1 dst
int EAOMindex=0;//Eallonlymove index

int color;//0 red 1 black
string src,dst;//棋盤編號版 a1~d4
int srci,dsti;//index版 0~31
int maxDepth=12;
U32 open=0xffffffff;//非未翻棋
U32 ch;//需要search的位置 
int noReDepth=1;

int main()
{
	clock_t start, stop;
	start = clock();
	srand(time(NULL));
	initial();//初始化 
	readBoard();
	drawOrNot();
	dynamicPower();
	U32 onboard2 = piece[15]; //取得棋子位置
	int onboardi=0,onboardi2=0;//計算場面上有棋的數量 
	U32 onboard=(piece[15]|piece[0])^0xffffffff;
	while(onboard2)
	{
		U32 mask = LS1B(onboard2); //如果該棋子在多個位置,先取低位元的位置。
		onboardi2++;
		onboard2 ^= mask;
	}
	while(onboard)
	{
		U32 mask = LS1B(onboard); //如果該棋子在多個位置,先取低位元的位置。
		onboardi++;
		onboard ^= mask;
	}
	int OBP=onboardi+onboardi2*onboardi2;
	/*if(OBP>400)maxDepth=9;
	else if(OBP>350)maxDepth=8;
	else if(OBP>200)maxDepth=7;
	else if(OBP>100)maxDepth=6;
	else if(OBP>80)maxDepth=7;
	else if(OBP>60)maxDepth=8;
	else if(OBP>40)maxDepth=9;
	else if(OBP>20)maxDepth=10;*/
	cout<<" game tree複雜度為 : "<<OBP<<endl; 
	nodes.resize(50);
	int end=1;//結束則=0
	while(end)
	{
		dst="0";
		ai3();//決定行動 
		stop = clock();
		cout <<" 此步耗時 : " << double(stop - start) / CLOCKS_PER_SEC <<" 秒(精準度0.001秒) "<<endl;
		cout <<" 本方為 : ";
		if(color==0) cout<<"紅 "<<endl;
		else cout<<"黑 "<<endl;
		createMovetxt();
		end=0;
	}
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
		search(0,piece,red,black,occupied,piece_count,-999999,999999);
	IndexToBoard(srci,dsti);
}

void ai3()
{
	chess(black,red,piece,0);
	stdScore=countAva(piece_count,0,piece);//test
	cout<<" 目前標準分數 : "<<stdScore<<endl;
	if(piece[15]==0xffffffff)
	{
		IndexToBoard(9,9);
		return;
	}
	//產生遮罩 ------> 產生chMask 
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
	ch&=piece[15];
	while(ch)
	{
		U32 take=LS1B(ch);
		ch^=take;
		chMask.push_back(GetIndex(take));
	}
	//把初始盤面放入depth=0 第0個node 之後depth+1 此為根結點 每次selection由此開始往下找 由FcurNode->child[0~?]中找一下並往下重複 
	node *FcurNode = new node();
	FcurNode->win = 0;
	FcurNode->round = 0;
	FcurNode->depth = 0;
	for (int i = 0; i < 16; i++)
	{
		FcurNode->piece[i] = piece[i];
	}
	FcurNode->red=piece[1]|piece[2]|piece[3]|piece[4]|piece[5]|piece[6]|piece[7];
	FcurNode->black=piece[8]|piece[9]|piece[10]|piece[11]|piece[12]|piece[13]|piece[14];
	for (int i = 0; i < 14; i++)
	{
		FcurNode->piece_count[i] = piece_count[i];
	}
	for (int i = 0; i < 14; i++)
	{
		FcurNode->DCount[i] = DCount[i];
	}
	FcurNode->parent = NULL;
	//FcurNode->UCB = (FcurNode->win / FcurNode->round) + c * sqrt(log10(allRound) / FcurNode->round);
	nodes[0].push_back(FcurNode);
	//最後從node[1]裡面找勝率最高的來當成走步
	for(int curi=1;curi<6;curi++)//跑幾輪 
	{
		//selection 開始時放入現在盤面 ---------------------------------------------- 從根結點開始往下找直到葉節點
		currentParent=FcurNode; 
		int dieway=0;//死路 1直接break; 
		while(currentParent->child.size())
		{
			double bestUCB=0;
			if(currentParent->child[0]->depth % 2==0) bestUCB=999999;//偶數 找最小 跟search相反 
			int bestway=-1;
			for(int i=0;i<currentParent->child.size();i++)
			{
				double UCB=(currentParent->child[i]->win / currentParent->child[i]->round) + c * sqrt(log10(allRound) / currentParent->child[i]->round);
				if(currentParent->child[i]->deeper){
					if(currentParent->child[i]->depth % 2==0)
					{
						if(UCB<bestUCB)
						{
							bestUCB=UCB;
							bestway=i;
						}
					}
					else
					{
						if(UCB>bestUCB)
						{
							bestUCB=UCB;
							bestway=i;
						}
					}
				}
			}
			if(curi!=1 && bestway==-1)
			{
				dieway=1;
				break;
			}
			currentParent=currentParent->child[bestway];
		}
		if(dieway==1) break;
		//expansion 產生走步 --------------------------------------------------------
		chess(currentParent->black,currentParent->red,currentParent->piece,currentParent->depth); 
		vector <vector<U32 > >curAllMove;//存所有走步 先吃子 再走步 再翻棋 
		vector<U32 > temp;
		for(int i=0;i<AEMindex;i++)
		{
			temp.push_back(allEatMove[i][0]);
			temp.push_back(allEatMove[i][1]);
			curAllMove.push_back(temp);
			temp.clear();
		}
		for(int i=0;i<AOMindex;i++)
		{
			temp.push_back(allOnlyMove[i][0]);
			temp.push_back(allOnlyMove[i][1]);
			curAllMove.push_back(temp);
			temp.clear();
		}
		for(int i=0;i<chMask.size();i++)
		{
			temp.push_back(chMask[i]);
			temp.push_back(chMask[i]);
			curAllMove.push_back(temp);
			temp.clear();
		}
		//simulation-----------------------------------------------------------------
		for(int i=0;i<curAllMove.size();i++)
		{
			//模擬走步 
			node *curNode = new node();
			int c1p,c2p=-1;
			U32 c1=1<<curAllMove[i][0];
			U32 c2=1<<curAllMove[i][1];
			if(curAllMove[i][0]==curAllMove[i][1])//翻棋 不作為葉節點 模擬1000次 
			{
				//curNode更新
				curNode->parent = currentParent;
				curNode->depth = curNode->parent->depth;
				curNode->src = curAllMove[i][0];
				curNode->dst = curAllMove[i][1];
				for (int i = 0; i < 16; i++)
				{
					curNode->piece[i] = piece[i];
				}
				curNode->red=red;
				curNode->black=black;
				for (int i = 0; i < 14; i++)
				{
					curNode->piece_count[i] = piece_count[i];
				}
				for (int i = 0; i < 14; i++)
				{
					curNode->DCount[i] = DCount[i];
				}
				int ra=rand()%100;
				curAllMove[i].push_back(MCSsimulation(simRound*10,curNode));//直接下去模擬
				curNode->depth++;//模擬時還沒進行第一步模擬 深度先-1 現在加回來 
				curNode->win = curAllMove[i][2];
				allRound += simRound*10;
				curNode->round = simRound*10;
				curNode->deeper=false;
			}
			else//其他 
			{
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
						break;
					}
				}
				piece[c1p]^=c1;//清除原位置c1
				piece[c1p]|=c2;//移動
				piece[0]|=c1;//空格+c1
				if(c2p!=-1){//吃子移動 
					piece[c2p]^=c2;//清除原位置c2
					piece_count[c2p-1]--;
					if(8>c1p)
					{//紅吃黑
						red^=c1;
						black^=c2;
						red|=c2;
					}
					else
					{//黑吃紅 
						black^=c1;
						red^=c2;
						black|=c2;
					}
				}
				else//無吃子移動 
				{
					piece[0]^=c2;//空格-c2
					if(8>c1p)
					{//紅動 
						red^=c1;
						red|=c2;
					}
					else
					{//黑動 
						black^=c1;
						black|=c2;
					}
				}
				//curNode更新
				curNode->parent = currentParent;
				curNode->depth = curNode->parent->depth+1;
				curNode->src = curAllMove[i][0];
				curNode->dst = curAllMove[i][1];
				for (int i = 0; i < 16; i++)
				{
					curNode->piece[i] = piece[i];
				}
				curNode->red=red;
				curNode->black=black;
				for (int i = 0; i < 14; i++)
				{
					curNode->piece_count[i] = piece_count[i];
				}
				for (int i = 0; i < 14; i++)
				{
					curNode->DCount[i] = DCount[i];
				}
				curAllMove[i].push_back(MCSsimulation(simRound,curNode));//應回傳獲勝場數 此處應下去模擬 simRound 次
				curNode->win = curAllMove[i][2];
				allRound += simRound;
				curNode->round = simRound;
				curNode->deeper=true;
			}
			
			nodes[curNode->depth].push_back(curNode);
			currentParent->child.push_back(curNode);
			//模擬走步取消 
			if(curAllMove[i][0]!=curAllMove[i][1])
			{
				piece[c1p]^=c2;//清除原位置c2
				piece[c1p]|=c1;//移動
				piece[0]^=c1;//空格-c1
				if(c2p!=-1){//吃子移動 
					piece[c2p]|=c2;//回原位置c2
					piece_count[c2p-1]++;
					if(8>c1p){//紅吃黑
						red|=c1;
						black|=c2;
						red^=c2;
					}
					else{//黑吃紅 
						black|=c1;
						red|=c2;
						black^=c2;
					}
				}
				else//無吃子移動 
				{
					piece[0]|=c2;//空格+c2
					if(8>c1p){//紅動 
						red|=c1;
						red^=c2;
					}
					else{//黑動 
						black|=c1;
						black^=c2;
					}
				}
			}
			//propagation 更新祖先節點 --------------------------------------------------1
			node *ancestor=curNode->parent;
			while(ancestor!=NULL)
			{
				ancestor->win+=curNode->win;
				ancestor->round+=curNode->round;
				ancestor=ancestor->parent;
			}
		}
		//-------------------------------------------------------------------------------2
	}
	//選深度1中最高分者
	double chosen=-1;
	for(int i=0 ; i<nodes[1].size();i++)
	{
		float win=nodes[1][i]->win;
		float score=win/nodes[1][i]->round;
		cout<<" "<<nodes[1][i]->src<<" "<<nodes[1][i]->dst<<"	"<<score<<" "<<nodes[1][i]->win<<" "<<nodes[1][i]->round<<endl;
		if(chosen<score)
		{
			chosen=score;
			srci=nodes[1][i]->src;
			dsti=nodes[1][i]->dst;
			IndexToBoard(srci,dsti);
		}
	}
}

int MCSsimulation(int curSimRound,node *SimNode)
{
	int allWin=0;//回傳優勢分數 
	vector <int>ccc=chMask;
	while(curSimRound)
	{
		vector <int>curchMask=chMask;
		node *curSimNode=new node();//不去改變原本盤面
		curSimNode->depth = SimNode->depth;
		for (int i = 0; i < 16; i++)
		{
			curSimNode->piece[i] = SimNode->piece[i];
		}
		for (int i = 0; i < 14; i++)
		{
			curSimNode->piece_count[i] = SimNode->piece_count[i];
		}
		for (int i = 0; i < 14; i++)
		{
			curSimNode->DCount[i] = SimNode->DCount[i];
		}
		curSimNode->black=SimNode->black;
		curSimNode->red=SimNode->red;
		int curDeep=curSimNode->depth;
		for(int deepi = 0; deepi < simDepth; deepi++)//往下20層 
		{
			chess(curSimNode->black,curSimNode->red,curSimNode->piece,curDeep);//depth 奇數是我方 偶數是敵方 跟以前相反 因為存了第一個點 
			vector <vector<U32 > >curAllMove;//存所有走步 先吃子 再走步 再翻棋 
			vector<U32 >temp;
			for(int i=0;i<curchMask.size();i++)
			{
				temp.push_back(curchMask[i]);
				temp.push_back(curchMask[i]);
				curAllMove.push_back(temp);
				temp.clear();
			}
			for(int i=0;i<AEMindex;i++)
			{
				temp.push_back(allEatMove[i][0]);
				temp.push_back(allEatMove[i][1]);
				curAllMove.push_back(temp);
				temp.clear();
			}	
			for(int i=0;i<AOMindex;i++)
			{
				temp.push_back(allOnlyMove[i][0]);
				temp.push_back(allOnlyMove[i][1]);
				curAllMove.push_back(temp);
				temp.clear();
			}
			if(curAllMove.size()==0) continue;//無棋可翻或走 
			int rnd=rand()%curAllMove.size();
			if(curAllMove[rnd][0]==curAllMove[rnd][1])//翻棋
			{
				int allch=0;
				for(int i=0;i<14;i++)
				{
					allch+=curSimNode->DCount[i];
				}
				int rndch=rand()%allch;//14種裡剩下隨機一顆
				for(int i=0;i<14;i++)
				{
					rndch-=curSimNode->DCount[i];
					if(rndch<=0)
					{
						rndch=i;
						break;
					}
				}
				U32 chp=1<<curchMask[rnd];
				curSimNode->DCount[rndch--];
				curSimNode->piece[rndch]|=chp;
				curSimNode->piece[15]^=chp;
				if(chp<8) curSimNode->red|=chp;
				else curSimNode->black|=chp;
				curchMask[rnd]=curchMask[curchMask.size()-1];
				curchMask.pop_back();
			} 
			else//走步 
			{
				int c1p,c2p=-1;
				U32 c1=1<<curAllMove[rnd][0];
				U32 c2=1<<curAllMove[rnd][1];
				for(int ii=1;ii<15;ii++){
					U32 check=curSimNode->piece[ii]&c1;
					if(check!=0){
						c1p=ii;
						break;
					}
				}
				for(int ii=1;ii<15;ii++){//找c2 清掉 剩餘棋子數更改 
					U32 check=curSimNode->piece[ii]&c2;
					if(check!=0){
						c2p=ii;
						break;
					}
				}
				curSimNode->piece[c1p]^=c1;//清除原位置c1
				curSimNode->piece[c1p]|=c2;//移動
				curSimNode->piece[0]|=c1;//空格+c1
				if(c2p!=-1){//吃子移動 
					curSimNode->piece[c2p]^=c2;//清除原位置c2
					curSimNode->piece_count[c2p-1]--;
					if(8>c1p)
					{//紅吃黑
						curSimNode->red^=c1;
						curSimNode->black^=c2;
						curSimNode->red|=c2;
					}
					else
					{//黑吃紅 
						curSimNode->black^=c1;
						curSimNode->red^=c2;
						curSimNode->black|=c2;
					}
				}
				else//無吃子移動 
				{
					curSimNode->piece[0]^=c2;//空格-c2
					if(8>c1p)
					{//紅動 
						curSimNode->red^=c1;
						curSimNode->red|=c2;
					}
					else
					{//黑動 
						curSimNode->black^=c1;
						curSimNode->black|=c2;
					}
				}
			}
			curDeep++;
		}
		chess(curSimNode->black,curSimNode->red,curSimNode->piece,curSimNode->depth);
		int nowScore=countAva(curSimNode->piece_count,curSimNode->depth,curSimNode->piece);
		if(nowScore > stdScore)
		{
			allWin++;
		}
		curSimRound--;//直到0 
	}
	return allWin;
}

void chess(U32 tblack,U32 tred,U32 tpiece[16],int deep) //deep+color%2 0red 1black first 
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

int countAva(int pie[14],int deep,U32 curPiece[16])//將士相車馬炮兵
{
	int eat[10];
	int biggest=0;
	if(deep!=maxDepth) biggest=0;
	int power=0;
	int redleft=0;
	int blackleft=0;
	if(color==0)//紅 
	{
		for(int i=0;i<7;i++)
		{
			power+=pie[i]*piPw[i];
			redleft+=pie[i];
		}
		for(int i=7;i<14;i++)
		{
			power-=pie[i]*piPw[i];
			blackleft+=pie[i];
		}
		if(redleft==0) power=-100000+deep*10000;
		if(blackleft==0) power=100000-deep*10000;
	}
	else
	{
		for(int i=7;i<14;i++)
		{
			power+=pie[i]*piPw[i];
			blackleft+=pie[i];
		}
		for(int i=0;i<7;i++)
		{
			power-=pie[i]*piPw[i];
			redleft+=pie[i];
		}
		if(blackleft==0) power=-100000+deep*10000;
		if(redleft==0) power=100000-deep*10000;
	}
	int movePoint=0;
	//-----------------------------------------------------找誰被吃的函數 
	//int check=0;//如果沒找到誰被吃 
	//int PointMayEat=0;//找到即將吃誰的最大子 
	//for(int i=0; i<AEMindex ;i++)//i所有被吃子 
	//{
		//for(int ii=1; ii<15; ii++)//ii找那個位置為哪個子
		//{
			//U32 eaten=1<<allEatMove[i][1];//allEatMove[i][1]馬上可能被吃的子的位置 
			//if((curPiece[ii]&eaten)!=0)
			//{
				//if(piPw[ii]>PointMayEat)
					//PointMayEat=piPw[ii];//找最大被吃子分數 
				//check=1;
			//}
		//} 
		//if(check==0)
		//{
			//cout<<"err1 ";
		//}
	//}
	//-----------------------------------------------------
	if(deep%2==0)
	{
		movePoint=AEMindex*10+AOMindex-EAEMindex*20-EAOMindex;
	} 
	else
	{
		movePoint=EAEMindex*10+EAOMindex-AEMindex*20-AOMindex;
	} 
	return power+movePoint-deep;
}

int search(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14],int alpha,int beta)
{
	curRed=curPiece[1]|curPiece[2]|curPiece[3]|curPiece[4]|curPiece[5]|curPiece[6]|curPiece[7];
	curBlack=curPiece[8]|curPiece[9]|curPiece[10]|curPiece[11]|curPiece[12]|curPiece[13]|curPiece[14];
	chess(curBlack,curRed,curPiece,depth);
	U32 taEM[50][2];//存可吃子的方法 0 src 1 dst 避免被往下搜尋時刷掉
	int tAEMi=AEMindex;//alleatmove index
	U32 taOM[50][2];//存可移動非吃子的方法 0 src 1 dst
	int tAOMi=AOMindex;//allonlymove index
	memcpy(taEM,allEatMove,sizeof(taEM));
	memcpy(taOM,allOnlyMove,sizeof(taOM));
	int re=countAva(curPie,depth,curPiece);//提前計算好分數 
	
	if(depth==maxDepth||(depth>=noReDepth&&(tAEMi+tAOMi)==0))
	{
		return re;
	}
	if(color==0&&curPiece[15]==0)
	{
		if(curRed==0) return -100000+depth*10000;
		else if(curBlack==0) return 100000-depth*10000;
	}
	else if(curPiece[15]==0) 
	{
		if(curRed==0) return 100000-depth*10000;
		else if(curBlack==0)return -100000+depth*10000;
	}
 	int weight[100][3];//計算所有移動與翻棋的得分0src 1dst 2weight 
	int wp=0;// weight pointer
	int best=-9999999;
	if(depth%2==1)
		best=9999999;
	if(curPiece[15]!=0&&depth>noReDepth)//可以走空步
	{
		int deeper=depth+1;
		weight[wp][0]=0;
		weight[wp][1]=0;
		weight[wp][2]=re;
		wp++;
	}
	if(curPiece[15]!=0)//先試翻棋 做完後call search 
	{
		for(int ssrc=0; ssrc<32; ssrc++){ //搜尋盤面上 32 個位置
			if(curPiece[15] & ( 1 << ssrc ) && ch & ( 1 << ssrc )&&depth<=noReDepth){ //若為未翻子 在未翻子的遮罩內 depth<=2
				if(depth==0)
				{
					int r=rand()%6;;
					string a[6]={"⊙3⊙","(--;)","(〃ω〃)","(’-_-`)","|ω˙）","(*≧艸≦)"};
					cout<<a[r]+".";
				}
				weight[wp][2]=0;
				int a=0;
				for(int pID=0; pID<14; pID++){ //搜尋可能會翻出之子
					if(DCount[pID]){ //若該兵種可能被翻出
						a+=DCount[pID];
						U32 c=1<<ssrc;
						int cpID=pID+1;
						curPiece[cpID]|=c;
						curPiece[15]^=c;
						DCount[pID]--;
						//模擬該兵種翻出來
						weight[wp][0] =ssrc;
						weight[wp][1] =ssrc;
						weight[wp][2] += ((DCount[pID]+1)*search(depth+1,curPiece,curRed,curBlack,curOccupied,curPie,alpha,beta));
						
						curPiece[cpID]^=c;
						curPiece[15]|=c;
						DCount[pID]++;
						//復原翻出的兵種 
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
						return beta;
					else
						return alpha;
				}
				wp++;
			}
		}
	}
	
	if(tAEMi+tAOMi!=0)
	{
		if(tAEMi>0)//試吃子 
		for(int i=0;i<tAEMi;i++)
		{
			weight[wp][2]=0;
			int c1p,c2p=-1;
			U32 c1=1<<taEM[i][0];
			U32 c2=1<<taEM[i][1];
			for(int ii=1;ii<15;ii++){//找到c1 放入c1p  
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
			if(c2p==-1||c1p==-1) cout<<"err 405";
			curPiece[c1p]^=c1;//清除原位置c1
			curPiece[c1p]|=c2;//移動
			curPiece[0]|=c1;//空格+c1
			curOccupied^=c1;//c1無子
			if(c2p!=-1){//吃子移動 
				curPiece[c2p]^=c2;//清除原位置c2
				int c2ps=c2p-1;
				curPie[c2ps]--;
			}
			else//無吃子移動 此處不會發生 只有吃子移動 
			{
				cout<<"err 303";
			}
			weight[wp][0] =taEM[i][0];
			weight[wp][1] =taEM[i][1];
			weight[wp][2] =search(depth+1,curPiece,curRed,curBlack,curOccupied,curPie,alpha,beta);
			
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
			}
			else//無吃子移動
			{
				cout<<"err509";
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
		for(int i=0;i<tAOMi;i++)//純移動 --------------------------------------------------------------------------------------------------------
		{
			weight[wp][2]=0;
			int c1p,c2p=-1;
			U32 c1=1<<taOM[i][0];
			U32 c2=1<<taOM[i][1];
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
			curPiece[c1p]^=c1;//清除原位置c1
			curPiece[c1p]|=c2;//移動
			curPiece[0]|=c1;//空格+c1
			curOccupied^=c1;//c1無子
			if(c1p>15||c1p<1)
			{
				cout<<"err 510"<<endl;
			}
			if(c2p!=-1){//吃子移動 
				cout<<"err 511"<<endl;
			}
			else//無吃子移動 
			{
				curPiece[0]^=c2;//空格-c2
				curOccupied|=c2;//c2有子 
			}
			weight[wp][0] =taOM[i][0];
			weight[wp][1] =taOM[i][1];
			weight[wp][2] =search(depth+1,curPiece,curRed,curBlack,curOccupied,curPie,alpha,beta);
			
			curPiece[c1p]^=c2;//清除原位置c2
			curPiece[0]|=c2;//空格+c2
			curPiece[c1p]|=c1;//移動
			curOccupied|=c1;//c1有子
			curPiece[0]^=c1;//空格-c1
			if(c2p!=-1){//吃子移動 
				cout<<"err 512 "<<endl;
			}
			else//無吃子移動 
			{
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
		for(int i=wp-1;i>=0;i--)
		{
			if(depth==0)
			{
				cout<<weight[i][0]<<" "<<weight[i][1]<<" "<<weight[i][2]<<endl;
			}
			if(weight[i][2]==best)
			{
				recordi=i;
				srci=weight[i][0];
				dsti=weight[i][1];
			}
		}
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
	}
	if(wp==0) best=re;
	if(best==9999999||best==-9999999) best=re;
	return best;
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
		//cout<<i<<move[i].at(0)<<move[i].at(1)<<move[i].at(2)<<endl;
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
