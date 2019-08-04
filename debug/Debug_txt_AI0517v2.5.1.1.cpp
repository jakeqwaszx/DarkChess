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

typedef unsigned int U32;
using namespace std;

struct node {
	int count;
	int depth;
	U32 piece[16];
	node* parent;
	vector<node*> child;
	int move[2];
	bool visit;
};
node* currentParent;
node* currentChild;
U32 LS1B(U32 x) { return x & (-x); }//���ox���̧C�줸
U32 MS1B(U32 x) { // Most Significant 1 Bit (LS1B)�禡
	x |= x >> 32; x |= x >> 16; x |= x >> 8;
	x |= x >> 4; x |= x >> 2; x |= x >> 1;
	return (x >> 1) + 1; //�i�H���o�@��줸���̥��䤣���s���줸
}
U32 CGen(int ssrc, U32 toccupied);//���٬��� 
U32 CGenCR(U32 x);
U32 CGenCL(U32 x);
int BitsHash(U32 x) { return (x * 0x08ED2BE6) >> 27; }
void initial();//��l�� 
void chess(U32 tblack, U32 tred, U32 tpiece[16], int deep);//�M��i�β���
void ai2();//ai ver2.0 �g �ϥ�search 
void readBoard();//Ū�ɼҦ� Ū��board.txt ��Ū�J�ɮ��নbitboard �٨S�˵ۦs�J 
void createMovetxt();//�гymove.txt 0���B 1½�� 
void IndexToBoard(int indexa, int indexb);//��src dst�q�s��0~31->�ѽL�s��a1~d4 
int countAva(int pie[14], int deep, U32 curPiece[16]);//�I�s�h�Ǧ^���e�Ѫ�
void SimReveal(int ssrc, int pID, U32 curPiece[16], U32 curRed, U32 curBlack);//����½�X���ssrc��pID 
void UnSimReveal(int ssrc, int pID, U32 curPiece[16], U32 curRed, U32 curBlack);//undo simReveal
int* SimMove(int ssrc, int sdst, U32 * curPiece, U32 * curRed, U32 * curBlack, U32 * curOccupied, int* curPie);//�������B 
int* UnSimMove(int ssrc, int sdst, U32 * curPiece, U32 * curRed, U32 * curBlack, U32 * curOccupied, int* curPie, int copy);//undeo simmove
int search(int depth, U32 curPiece[16], U32 curRed, U32 curBlack, U32 curOccupied, int curPie[14], int alpha, int beta);//�j�M�̨Ψ��B 
int onewaySearch(int depth, U32 curPiece[16], U32 curRed, U32 curBlack, U32 curOccupied, int curPie[14], int alpha, int beta, int eat);//��V�j�M ���� 
int searchMove(int depth, U32 curPiece[16], U32 curRed, U32 curBlack, U32 curOccupied, int curPie[14], int alpha, int beta);//���j�M�i������
void dynamicPower();//�p��ʺA�ѤO 
void drawOrNot();//��past_walk�P�_�O�_���� ���ᵲ�G��X��draw 
int findPiece(int place, U32 curPiece[16]);//�ǽs�� �^�Ǧb�o�ӽs�����Ѥl 
void printDebug();

int index32[32] = { 31, 0, 1, 5, 2, 16, 27, 6, 3, 14, 17, 19, 28, 11, 7, 21, 30, 4, 15, 26, 13,
18, 10, 20, 29, 25, 12, 9, 24, 8, 23, 22 };
int GetIndex(U32 mask) { return index32[BitsHash(mask)]; }//��J�B�n��^�ѽL�s��
U32 pMoves[32] = { 0x00000012,0x00000025,0x0000004A,0x00000084,0x00000121,0x00000252,0x000004A4,0x00000848,
0x00001210,0x00002520,0x00004A40,0x00008480,0x00012100,0x00025200,0x0004A400,0x00084800,
0x00121000,0x00252000,0x004A4000,0x00848000,0x01210000,0x02520000,0x04A40000,0x08480000,
0x12100000,0x25200000,0x4A400000,0x84800000,0x21000000,0x52000000,0xA4000000,0x48000000 };//�Ѥl���ʾB�n
U32 pMoves2[32] = {
0x00000116,0x0000022D,0x0000044B,0x00000886,0x00001161,0x000022D2,0x000044B4,0x00008868,
0x00011611,0x00022D22,0x00044B44,0x00088688,0x00116110,0x0022D220,0x0044B440,0x00886880,
0x01161100,0x022D2200,0x044B4400,0x08868800,0x11611000,0x22D22000,0x44B44000,0x88688000,
0x16110000,0x2D220000,0x4B440000,0x86880000,0x61100000,0xD2200000,0xB4400000,0x68800000 };//½�ѾB�n
U32 pMoves3[32] = {
0x00000136,0x0000027D,0x000004EB,0x000008C6,0x00001363,0x000027D7,0x00004EB4,0x00008C6C,
0x00013631,0x00027D72,0x0004EBE4,0x0008C6C8,0x00136310,0x0027D720,0x004EBE40,0x008C6C80,
0x01363100,0x027D7200,0x04EBE400,0x08C6C800,0x13631000,0x27D72000,0x4EBE4000,0x8C6C8000,
0x36310000,0x7D720000,0xEBE40000,0xC6C80000,0x63100000,0xD7200000,0xBE400000,0x6C800000 };//½�ѾB�n
U32 file[4] = { 0x11111111,0x22222222,0x44444444,0x88888888 };//��B�n 
U32 row[8] = { 0x0000000F,0x000000F0,0x00000F00,0x0000F000,0x000F0000,0x00F00000,0x0F000000,0xF0000000 };//�C�B�n 
U32 piece[16]; //0�Ů�- ��k �hg ��m ��r ��n ��c �Lp *2 15��½x 
U32 red, black, occupied;//�� �� ���Ѥl 

string move = "a1-a1";//�U�@�B��� �Ω�I�� 
int piece_count[14] = { 1,2,2,2,2,2,5,1,2,2,2,2,2,5 };//�Ѿl�Ѥl�� 0-6 7-13
int DCount[14] = { 1,2,2,2,2,2,5,1,2,2,2,2,2,5 };//�Ѿl��½�l 
string current_position[32];//�L�����p�`�� 
string history;
int timeCount;//�Ѿl�ɶ� 
int initailBoard = 1;//�O�_Ū����l���� 
int past_walk[7][2] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 };//�e�C�B �ΨӳB�z�������D 
int piPw[14] = { 6000,2700,900,400,150,2000,100,6000,2700,900,400,150,2000,100 };//�Ѥl���� 
int draw = 0;//0�L���ⱡ�p 1���i��i�J���� 

vector<vector<node*>> nodes;
vector<int> finalScore;
vector<int> finalsrc;
vector<int> finaldst;
vector<int> depthrecord;
int nodemove[2] = { -1,-1 };



//int RMcount=13;//Ū���Ҧ��ݭn �q13��}�lŪ�����G 

U32 allEatMove[50][2];//�s�i�Y�l����k 0 src 1 dst
int AEMindex = 0;//alleatmove index
U32 allOnlyMove[50][2];//�s�i���ʫD�Y�l����k 0 src 1 dst
int AOMindex = 0;//allonlymove index
U32 EallEatMove[50][2];//�s���i�Y�l����k 0 src 1 dst
int EAEMindex = 0;//Ealleatmove index
U32 EallOnlyMove[50][2];//�s���i���ʫD�Y�l����k 0 src 1 dst
int EAOMindex = 0;//Eallonlymove index

int color;//0 red 1 black
string src, dst;//�ѽL�s���� a1~d4
int srci, dsti;//index�� 0~31
int maxDepth = 4;
int moveMaxDepth = 8;
U32 open = 0xffffffff;//�D��½��
U32 ch;//�ݭnsearch����m 
int noReDepth = 2;

int main()
{
	clock_t start, stop;
	start = clock();
	srand(time(NULL));
	initial();//��l�� 
	readBoard();
	drawOrNot();
	dynamicPower();
	U32 onboard = piece[15];
	int a = onboard / 2;
	int onboardi = 0;//�p������W��½�Ѽƶq 
	for (int i = 0; i < 32; i++)
	{
		int a = onboard % 2;
		int b = onboard / 2;
		if (a == 1)
		{
			onboardi++;
		}
		if (b > 0) onboard /= 2;
		if (b == 1) break;
	}
	if (onboardi < 2)maxDepth = 8;
	else if (onboardi < 4)maxDepth = 7;
	else if (onboardi < 7)maxDepth = 6;
	else if (onboardi < 14)maxDepth = 5;
	nodes.resize(maxDepth + 1);
	//cout<<hex<<piece[0]<<" "<<piece[1]<<" "<<piece[8]<<" "<<piece[15]<<" "<<red<<" "<<black<<" "<<occupied<<" "<<piece_count[7]<<endl;
	//SimMove(0,1,piece,&red,&black,&occupied,piece_count);
	//cout<<hex<<piece[0]<<" "<<piece[1]<<" "<<piece[8]<<" "<<piece[15]<<" "<<red<<" "<<black<<" "<<occupied<<" "<<piece_count[7]<<endl;
	//UnSimMove(0,1,piece,&red,&black,&occupied,piece_count,8);
	//cout<<hex<<piece[0]<<" "<<piece[1]<<" "<<piece[8]<<" "<<piece[15]<<" "<<red<<" "<<black<<" "<<occupied<<" "<<piece_count[7]<<endl;
	int end = 1;//�����h=0
	while (end)
	{
		dst = "0";
		ai2();//�M�w��� 
		stop = clock();
		cout << " ���B�Ӯ� : " << double(stop - start) / CLOCKS_PER_SEC << " ��(��ǫ�0.001��) " << endl;
		cout << " ���謰 : ";
		if (color == 0) cout << "�� " << endl;
		else cout << "�� " << endl;
		createMovetxt();
		end = 0;
	}
	int temp = 0;
	for (int i = 0; i < nodes.size(); i++) {
		for (int k = 0; k < nodes[i].size(); k++) {
			if (nodes[i][k]->depth == 0)
				temp++;
		}
	}
	printDebug();

}

void ai2()
{
	open = 0xffffffff;
	ch = 0x00000000;
	open ^= piece[15];//�D��½��
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
	chess(black, red, piece, 0);
	//int nowbest=countAva(piece_count,0);//�ܤֵo�ͦY�l 
	//int movebest=-999999;
	//if((AEMindex+AOMindex)!=0)
		//movebest=searchMove(0,piece,red,black,occupied,piece_count,-999999,999999);//���ݨ�8�B�|���|���V 
	//cout<<nowbest<<" "<<movebest<<endl;
	//if(nowbest>movebest||movebest==-999999)
	search(0, piece, red, black, occupied, piece_count, -999999, 999999);
	IndexToBoard(srci, dsti);
}

void chess(U32 tblack, U32 tred, U32 tpiece[16], int deep)
{
	U32 dest;//�i�H�Y�l�����
	AEMindex = 0;
	AOMindex = 0;
	EAEMindex = 0;
	EAOMindex = 0;
	U32 toccupied = 0xFFFFFFFF;
	toccupied ^= tpiece[0];
	int ssrc = 0;
	//cout<<"-------------------------"<<endl;
	int check = (color + deep) % 2;//0red 1black
	if (check == 0) {//�� 
		//cout<<"Ours available eat:"<<endl;
		for (int i = 1; i < 8; i++) { //1~7 ����~�L,src ���Ѥl�_�I,dest �����I�C  ����ڤ� 
			U32 p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				if (i == 1) //��,�P���(14)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & (tblack ^ tpiece[14]);
				else if (i == 2) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & (tblack ^ tpiece[8]);
				else if (i == 3) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & (tblack ^ tpiece[8] ^ tpiece[9]);
				else if (i == 4) //?,�u��Y��(11)�B���B���B��C
					dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 5) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 6) //��,�S���B�z�C
					dest = CGen(ssrc, toccupied) & tblack;
				else if (i == 7) //�L,�u��Y�N(8)�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
					dest = 0;
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allEatMove[AEMindex][0] = ssrc;
					allEatMove[AEMindex][1] = result;
					AEMindex++;
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
				}
			}
		}
		//cout<<"Ours available move:"<<endl;
		for (int i = 1; i < 8; i++) { //����²���
			U32 p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allOnlyMove[AOMindex][0] = ssrc;
					allOnlyMove[AOMindex][1] = result;
					AOMindex++;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
				}
			}
		}
		//cout<<"Their available eat:"<<endl;
		for (int i = 8; i < 15; i++) { //�A������ 
			U32 p = tpiece[i];
			while (p) {
				U32 mask = LS1B(p);
				p ^= mask;
				ssrc = GetIndex(mask);
				if (i == 8)
					dest = pMoves[ssrc] & (tred ^ tpiece[7]);
				else if (i == 9) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & (tred ^ tpiece[1]);
				else if (i == 10) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & (tred ^ tpiece[1] ^ tpiece[2]);
				else if (i == 11) //?,�u��Y��(11)�B���B���B��C
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 12) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 13) //��,�S���B�z�C
					dest = CGen(ssrc, toccupied) & tred;
				else if (i == 14) //�L,�u��Y�N�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
					dest = 0;
				while (dest) { //����ʦs�JEallEatMove 
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallEatMove[EAEMindex][0] = ssrc;
					EallEatMove[EAEMindex][1] = result;
					EAEMindex++;
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
				}
			}
		}
		//cout<<"Their available move:"<<endl;
		for (int i = 8; i < 15; i++) { //���²��� 
			U32 p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N�¦� 8~14 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallOnlyMove[EAOMindex][0] = ssrc;
					EallOnlyMove[EAOMindex][1] = result;
					EAOMindex++;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
				}
			}
		}
	}
	else {//�¤貾�� 
		//cout<<"Our available eat:"<<endl;
		for (int i = 8; i < 15; i++) { //1~7 ����~�L,src ���Ѥl�_�I,dest �����I�C
			U32 p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N�¦� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				if (i == 8) //��,�P���(14)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & (tred ^ tpiece[7]);
				else if (i == 9) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & (tred ^ tpiece[1]);
				else if (i == 10) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & (tred ^ tpiece[1] ^ tpiece[2]);
				else if (i == 11) //?,�u��Y��(11)�B���B���B��C
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 12) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7]);
				else if (i == 13) //��,�S���B�z�C
					dest = CGen(ssrc, toccupied) & tred;
				else if (i == 14) //�L,�u��Y�N�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
					dest = 0;
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allEatMove[AEMindex][0] = ssrc;
					allEatMove[AEMindex][1] = result;
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
					AEMindex++;
				}
			}
		}
		//cout<<"Our available move:"<<endl;
		for (int i = 8; i < 15; i++) { //�¤�²���
			U32 p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N�¦� 8~14 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					allOnlyMove[AOMindex][0] = ssrc;
					allOnlyMove[AOMindex][1] = result;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
					AOMindex++;
				}
			}
		}
		//cout<<"Their available eat:"<<endl;
		for (int i = 1; i < 8; i++) { //�p��Ĥ貾�� 
			U32 p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				if (i == 1) //��,�P���(14)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & (tblack ^ tpiece[14]);
				else if (i == 2) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & (tblack ^ tpiece[8]);
				else if (i == 3) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & (tblack ^ tpiece[8] ^ tpiece[9]);
				else if (i == 4) //?,�u��Y��(11)�B���B���B��C
					dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 5) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14]);
				else if (i == 6) //��,�S���B�z�C
					dest = CGen(ssrc, toccupied) & tblack;
				else if (i == 7) //�L,�u��Y�N(8)�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
					dest = 0;
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallEatMove[EAEMindex][0] = ssrc;
					EallEatMove[EAEMindex][1] = result;
					EAEMindex++;
					//cout<<"es:"<<ssrc<<" er:"<<result<<endl;
				}
			}
		}
		//cout<<"Their available move:"<<endl;
		for (int i = 1; i < 8; i++) { //����²���
			U32 p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					U32 mask2 = LS1B(dest);
					dest ^= mask2;
					U32 result = GetIndex(mask2);
					EallOnlyMove[EAOMindex][0] = ssrc;
					EallOnlyMove[EAOMindex][1] = result;
					EAOMindex++;
					//cout<<"os:"<<ssrc<<" or:"<<result<<endl;
				}
			}
		}
	}
}

U32 CGen(int ssrc, U32 toccupied)
{
	int r = ssrc / 4;//�C 
	int c = ssrc % 4;//�� 
	U32 result = 0;
	U32 resulta = 0;
	U32 x = ((row[r] & toccupied) ^ (1 << ssrc)) >> (4 * r);//���X�򬶦P�C �d�h����������
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

	x = ((file[c] & toccupied) ^ (1 << ssrc)) >> c;//���X�򬶦P�� �å�������1��
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
		U32 mask = LS1B(x); //mask �����[���B�n��m,�� x ���h���[�C
		return (x ^= mask) ? LS1B(x) : 0; //���p 5~8 �Ǧ^ LS1B(x),���p 2~4 �Ǧ^ 0�C
	}
	else return 0;
}

U32 CGenCR(U32 x) {
	if (x) {
		U32 mask = MS1B(x); //mask �����[���B�n��m,�� x ���h���[�C
		return (x ^= mask) ? MS1B(x) : 0; //���p 5~8 �Ǧ^ MS1B(x),���p 2~4 �Ǧ^ 0�C
	}
	else return 0;
}

int ccc = 0;
int countAva(int pie[14], int deep, U32 curPiece[16])//�N�h�ۨ������L
{
	int eat[10];
	int biggest = 0;
	//for(int i=0;i<EAEMindex;i++)
	//{
		//eat[i]=findPiece(EallEatMove[i][1],curPiece);//��U�@�B�|�Q�Y�o�Ѥl�s�Jeat 0~13
		//if(eat[i]==-1)//���Q�Y�l����m -1���䤣�� ��]���T�w 
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
			//eat[i]=piPw[eat[i]];//����Ʀs�ieat[i] 
		//if(biggest<eat[i]) biggest=eat[i];//���eat���̤j���Ѥl 
	//}
	if (deep != maxDepth) biggest = 0;
	int power = 0;
	if (color == 0)//�� 
	{
		for (int i = 0; i < 7; i++)
			power += pie[i] * piPw[i];
		for (int i = 7; i < 14; i++)
			power -= pie[i] * piPw[i];
	}
	else
	{
		for (int i = 7; i < 14; i++)
			power += pie[i] * piPw[i];
		for (int i = 0; i < 7; i++)
			power -= pie[i] * piPw[i];
	}
	//if(ccc==0)
	//{
		//for(int i=0;i<14;i++)
			//cout<<pie[i]<<" ";
		//cout<<endl;
		//ccc=1;
	//}
	int movePoint = 0;
	if (deep % 2 == 0)
		movePoint = AEMindex * 10 + AOMindex - EAEMindex * 20 - EAOMindex;
	else
		movePoint = EAEMindex * 10 + EAOMindex - AEMindex * 20 - AOMindex;
	//cout<<AEMindex<<" "<<AOMindex<<" "<<EAEMindex<<" "<<EAOMindex<<" "<<movePoint+power<<endl;
	//cout<<power;
	//if(deep%2!=0) biggest=-biggest;
	return power + movePoint - deep;
}

int search(int depth, U32 curPiece[16], U32 curRed, U32 curBlack, U32 curOccupied, int curPie[14], int alpha, int beta)
{
	int depthcheck = 0;
	for (int i = 0; i < depthrecord.size(); i++) {
		if (depthrecord[i] == depth)
			depthcheck = 1;
	}
	if (depthcheck == 0) {
		depthrecord.push_back(depth);
	}
	node* curNode = new node();
	curNode->depth = depth;
	curNode->visit = false;
	curNode->move[0] = nodemove[0];
	curNode->move[1] = nodemove[1];
	for (int i = 0; i < 16; i++) {
		curNode->piece[i] = curPiece[i];
	}
	nodes[depth].push_back(curNode);
	if (depth == 0) {
		currentParent = NULL;
	}
	curNode->parent = currentParent;
	curRed = curPiece[1] | curPiece[2] | curPiece[3] | curPiece[4] | curPiece[5] | curPiece[6] | curPiece[7];
	curBlack = curPiece[8] | curPiece[9] | curPiece[10] | curPiece[11] | curPiece[12] | curPiece[13] | curPiece[14];

	chess(curBlack, curRed, curPiece, depth);
	U32 taEM[50][2];//�s�i�Y�l����k 0 src 1 dst �קK�Q���U�j�M�ɨ걼
	int tAEMi = AEMindex;//alleatmove index
	U32 taOM[50][2];//�s�i���ʫD�Y�l����k 0 src 1 dst
	int tAOMi = AOMindex;//allonlymove index
	int etaemi = EAEMindex;
	if (depth % 2 == 1) etaemi = AEMindex;
	memcpy(taEM, allEatMove, sizeof(taEM));
	memcpy(taOM, allOnlyMove, sizeof(taOM));
	int re = countAva(curPie, depth, curPiece);
	int weight[100][3];//�p��Ҧ����ʻP½�Ѫ��o��0src 1dst 2weight 
	int wp = 0;
	int best = -9999999;
	if (depth == maxDepth || (depth >= noReDepth && (tAEMi + tAOMi) == 0))
	{
		curNode->count = re;
		currentChild = curNode;
		return re;
	}
	if (depth % 2 == 1)
		best = 9999999;
	if (curPiece[15] != 0 && depth > noReDepth)//�i�H���ŨB
	{
		weight[wp][0] = 0; weight[wp][1] = 0; weight[wp][2] = re;
		wp++;
	}
	if (curPiece[15] != 0)//����½�� ������call search 
	{
		for (int ssrc = 0; ssrc < 32; ssrc++) { //�j�M�L���W 32 �Ӧ�m
			if (curPiece[15] & (1 << ssrc) && ch & (1 << ssrc) && depth <= noReDepth) { //�Y����½�l �b��½�l���B�n�� depth<=3 
				if (depth == 0)
				{
					int r = rand() % 6;;
					string a[6] = { "��3��","(--;)","(���s��)","(��-_-`)","|�s���^","(*���J��)" };
					cout << a[r] + ".";

				}
				weight[wp][2] = 0;
				//cout<<endl;
				int a = 0;
				for (int pID = 0; pID < 14; pID++) { //�j�M�i��|½�X���l
					if (DCount[pID]) { //�Y�ӧL�إi��Q½�X
						a += DCount[pID];
						int deeper = depth + 1;
						U32 c = 1 << ssrc;
						int cpID = pID + 1;
						if (pID < 8)
						{
							curPiece[cpID] |= c;
							//curRed|=c;
							curPiece[15] ^= c;
							DCount[pID]--;
						}
						else
						{
							curPiece[cpID] |= c;
							//curBlack|=c;
							curPiece[15] ^= c;
							DCount[pID]--;
						}//�����ӧL��½�X��
						//cout<<pID<<" ";
						weight[wp][0] = ssrc;
						weight[wp][1] = ssrc;
						currentParent = curNode;
						nodemove[0] = weight[wp][0];
						nodemove[1] = weight[wp][1];
						weight[wp][2] += ((DCount[pID] + 1) * search(deeper, curPiece, curRed, curBlack, curOccupied, curPie, alpha, beta));
						curNode->child.push_back(currentChild);
						ccc = 0;
						if (pID < 8)
						{
							curPiece[cpID] ^= c;
							//curRed^=c;
							curPiece[15] |= c;
							DCount[pID]++;
						}
						else
						{
							curPiece[cpID] ^= c;
							//curBlack^=c;
							curPiece[15] |= c;
							DCount[pID]++;
						}//�N����½�X���l�_��
					}
				}
				weight[wp][2] /= a;
				for (int i = 0; i < curNode->child.size(); i++) {
					curNode->child[i]->count = weight[wp][2];
				}
				curNode->child.clear();
				if (depth % 2 == 0)//max
				{
					if (weight[wp][2] > alpha)
					{
						alpha = weight[wp][2];
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
					}
					if (weight[wp][2] < best)
					{
						best = weight[wp][2];
					}
				}
				if (beta <= alpha)
				{
					if (depth % 2 == 1)
					{
						curNode->count = beta;
						currentChild = curNode;
						return beta;
					}
					else
					{
						curNode->count = alpha;
						currentChild = curNode;
						return alpha;
					}
				}
				wp++;
			}
		}
	}

	if (tAEMi + tAOMi != 0)//�զY�l 
	{
		if (tAEMi > 0)
			for (int i = 0; i < tAEMi; i++)
			{
				weight[wp][2] = 0;
				int deeper = depth + 1;
				//SimMove(taEM[i][0],taEM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
				int c1p, c2p = -1;
				U32 c1 = 1 << taEM[i][0];
				U32 c2 = 1 << taEM[i][1];
				//cout<<hex<<c2; 
				for (int ii = 1; ii < 15; ii++) {
					U32 check = curPiece[ii] & c1;
					if (check != 0) {
						c1p = ii;
						break;
					}
				}
				for (int ii = 1; ii < 15; ii++) {//��c2 �M�� �Ѿl�Ѥl�Ƨ�� 
					U32 check = curPiece[ii] & c2;
					if (check != 0) {
						c2p = ii;
						break;
					}
				}
				int c2pcopy;
				c2pcopy = c2p;
				curPiece[c1p] ^= c1;//�M�����mc1
				curPiece[c1p] |= c2;//����
				curPiece[0] |= c1;//�Ů�+c1
				curOccupied ^= c1;//c1�L�l
				if (c2p != -1) {//�Y�l���� 
					curPiece[c2p] ^= c2;//�M�����mc2
					int c2ps = c2p - 1;
					curPie[c2ps]--;
					//if(8>c1p){//���Y��
						//curRed^=c1;
						//curBlack^=c2;
						//curRed|=c2;
					//}
					//else{//�¦Y�� 
						//curBlack^=c1;
						//curRed^=c2;
						//curBlack|=c2;
					//**/}
				}
				else//�L�Y�l���� 
				{
					//if(8>c1p){//���� 
						//curRed^=c1;
						//curRed|=c2;
					//}
					//else{//�°� 
						//curBlack^=c1;
						//curBlack|=c2;
					//}
					curPiece[0] ^= c2;//�Ů�-c2
					curOccupied |= c2;//c2���l 
				}
				weight[wp][0] = taEM[i][0];
				weight[wp][1] = taEM[i][1];
				currentParent = curNode;
				nodemove[0] = weight[wp][0];
				nodemove[1] = weight[wp][1];
				weight[wp][2] = search(deeper, curPiece, curRed, curBlack, curOccupied, curPie, alpha, beta);
				curNode->count = weight[wp][2];
				//if(taEM[i][1]==13||taEM[i][1]==14||taEM[i][1]==17||taEM[i][1]==18) weight[wp][2];//���ʨ� 13 14 17 18��+1; 
				//UnSimMove(taEM[i][0],taEM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie,copy);
				c1p = -1;
				c2p = -1;
				c1 = 1 << taEM[i][0];
				c2 = 1 << taEM[i][1];
				//cout<<hex<<c2; 
				for (int ii = 1; ii < 15; ii++) {
					U32 check = curPiece[ii] & c2;
					if (check != 0) {
						c1p = ii;
						break;
					}
				}
				c2p = c2pcopy;
				curPiece[c1p] ^= c2;//�M�����mc2
				curPiece[0] |= c2;//�Ů�+c2
				curPiece[c1p] |= c1;//����
				curOccupied |= c1;//c1���l
				curPiece[0] ^= c1;//�Ů�-c1
				if (c2p != -1) {//�Y�l���� 
					curPiece[c2p] |= c2;//�^���mc2
					int c2ps = c2p - 1;
					curPie[c2ps]++;
					curPiece[0] ^= c2;
					/*if(8>c1p){//���Y��
						curRed|=c1;
						curBlack|=c2;
						curRed^=c2;
					}
					else{//�¦Y��
						curBlack|=c1;
						curRed|=c2;
						curBlack^=c2;
					}
					*/
				}
				else//�L�Y�l���� 
				{
					/*
					if(8>c1p){//����
						curRed|=c1;
						curRed^=c2;
					}
					else{//�°�
						curBlack|=c1;
						curBlack^=c2;
					}
					*/
					curOccupied ^= c2;//c2�L�l 
				}
				if (depth % 2 == 0)//max
				{
					if (weight[wp][2] > alpha)
					{
						alpha = weight[wp][2];
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
					}
					if (weight[wp][2] < best)
					{
						best = weight[wp][2];
					}
				}
				if (beta <= alpha)
				{
					if (depth % 2 == 1) {
						currentChild = curNode;
						return beta;
					}
					else {
						curNode->count = alpha;
						return alpha;
					}
				}
				wp++;
			}
		for (int i = 0; i < tAOMi; i++)//�²��� 
		{
			weight[wp][2] = 0;
			int deeper = depth + 1;
			//SimMove(taOM[i][0],taOM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
			int c1p, c2p = -1;
			U32 c1 = 1 << taOM[i][0];
			U32 c2 = 1 << taOM[i][1];
			//cout<<hex<<c2; 
			for (int ii = 1; ii < 15; ii++) {
				U32 check = curPiece[ii] & c1;
				if (check != 0) {
					c1p = ii;
					break;
				}
			}
			for (int ii = 1; ii < 15; ii++) {//��c2 �M�� �Ѿl�Ѥl�Ƨ�� 
				U32 check = curPiece[ii] & c2;
				if (check != 0) {
					c2p = ii;
					cout << "err";
				}
			}
			int c2pcopy = 0;
			c2pcopy = c2p;
			curPiece[c1p] ^= c1;//�M�����mc1
			curPiece[c1p] |= c2;//����
			curPiece[0] |= c1;//�Ů�+c1
			curOccupied ^= c1;//c1�L�l
			if (c1p > 15 || c1p < 1)
			{
				for (int ii = 1; ii < 15; ii++) {
					cout << hex << curPiece[ii] << dec << endl;
				}
				cout << hex << curBlack << "b r" << curRed << endl;
				cout << c1 << " " << c2 << dec << endl;
				cout << "next>>" << endl << endl;
			}
			if (c2p != -1) {//�Y�l���� 
				curPiece[c2p] ^= c2;//�M�����mc2
				int c2ps = c2p - 1;
				curPie[c2ps]--;
				/*
				if(8>c1p){//���Y��
					curRed^=c1;
					curBlack^=c2;
					curRed|=c2;
				}
				else{//�¦Y��
					curBlack^=c1;
					curRed^=c2;
					curBlack|=c2;
				}
				*/
			}
			else//�L�Y�l���� 
			{
				/*
				if(8>c1p){//����
					curRed^=c1;
					curRed|=c2;
				}
				else{//�°�
					curBlack^=c1;
					curBlack|=c2;
				}
				*/
				curPiece[0] ^= c2;//�Ů�-c2
				curOccupied |= c2;//c2���l 
			}
			weight[wp][0] = taOM[i][0];
			weight[wp][1] = taOM[i][1];
			currentParent = curNode;
			nodemove[0] = weight[wp][0];
			nodemove[1] = weight[wp][1];
			weight[wp][2] = search(deeper, curPiece, curRed, curBlack, curOccupied, curPie, alpha, beta);
			curNode->count = weight[wp][2];
			//if(taEM[i][1]==13||taEM[i][1]==14||taEM[i][1]==17||taEM[i][1]==18) weight[wp][2];//���ʨ� 13 14 17 18��+1; 
			//UnSimMove(taOM[i][0],taOM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie,copy);
			c1p = -1;
			c2p = -1;
			c1 = 1 << taOM[i][0];
			c2 = 1 << taOM[i][1];
			//cout<<hex<<c2; 
			for (int ii = 1; ii < 15; ii++) {
				U32 check = curPiece[ii] & c2;
				if (check != 0) {
					c1p = ii;
					break;
				}
			}
			c2p = c2pcopy;
			curPiece[c1p] ^= c2;//�M�����mc2
			curPiece[0] |= c2;//�Ů�+c2
			curPiece[c1p] |= c1;//����
			curOccupied |= c1;//c1���l
			curPiece[0] ^= c1;//�Ů�-c1
			if (c2p != -1) {//�Y�l���� 
				curPiece[c2p] |= c2;//�^���mc2
				int c2ps = c2p - 1;
				curPie[c2ps]++;
				curPiece[0] ^= c2;
				/*
				if(8>c1p){//���Y��
					curRed|=c1;
					curBlack|=c2;
					curRed^=c2;
				}
				else{//�¦Y��
					curBlack|=c1;
					curRed|=c2;
					curBlack^=c2;
				}
				*/
			}
			else//�L�Y�l���� 
			{
				/*
				if(8>c1p){//����
					curRed|=c1;
					curRed^=c2;
				}
				else{//�°�
					curBlack|=c1;
					curBlack^=c2;
				}
				*/
				curOccupied ^= c2;//c2�L�l 
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
				}
			}
			if (beta <= alpha)
			{
				if (depth % 2 == 1) {
					currentChild = curNode;
					return beta;
				}
				else {
					currentChild = curNode;
					return alpha;
				}
			}
			wp++;
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
				finalsrc.push_back(weight[i][0]);
				finaldst.push_back(weight[i][1]);
				finalScore.push_back(weight[i][2]);
				for (int k = 0; k < nodes[1].size(); k++) {
					if (nodes[1][k]->count == weight[i][2]) {
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
			if (temp != 0) {
				temp2++;
			}
		}
		if (temp2 != wp) {
			cout << "oops " << wp - temp2 << endl;
		}
		cout << temp2 << endl;
		if (draw == 0)
		{
		}
		else if (draw == 1)
		{
			if (best < 0); //�i��� �G�N����? 
			else if (srci == past_walk[1][1] && dsti == past_walk[1][0])
			{
				cout << "draw denied" << endl;
				weight[recordi][2] -= 999999;//�i��|Ĺ ��ܤ�����? 
				best = weight[0][2];
				for (int ii = 0; ii < wp; ii++)
				{
					if (weight[ii][0] == srci)//�����Ѥl����������-999999 
						weight[ii][2] -= 999999;
				}
				for (int ii = wp - 1; ii >= 0; ii--)//���s�M�� 
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
		//srci=weight[same[rnd]][0];
		//dsti=weight[same[rnd]][1];
	}
	if (wp == 0) best = re;
	if (best == 9999999 || best == -9999999) best = re;

	//system("pause");
	curNode->count = best;
	currentChild = curNode;
	return best;
}


int* SimMove(int ssrc, int sdst, U32 * curPiece, U32 * curRed, U32 * curBlack, U32 * curOccupied, int* curPie)
{
	int c1p, c2p = -1;
	U32 c1 = 1 << ssrc;
	U32 c2 = 1 << sdst;
	//cout<<hex<<c2; 
	for (int ii = 1; ii < 15; ii++) {
		U32 check = curPiece[ii] & c1;
		if (check != 0) {
			c1p = ii;
			break;
		}
	}
	for (int ii = 1; ii < 15; ii++) {//��c2 �M�� �Ѿl�Ѥl�Ƨ�� 
		U32 check = curPiece[ii] & c2;
		if (check != 0) {
			c2p = ii;
		}
	}
	//c2pcopy=c2p;
	curPiece[c1p] ^= c1;//�M�����mc1
	curPiece[c1p] |= c2;//����
	curPiece[0] |= c1;//�Ů�+c1
	*curOccupied ^= c1;//c1�L�l
	if (c2p != -1) {//�Y�l���� 
		curPiece[c2p] ^= c2;//�M�����mc2
		int c2ps = c2p - 1;
		curPie[c2ps]--;
		if (8 > c1p) {//���Y��
			*curRed ^= c1;
			*curBlack ^= c2;
			*curRed |= c2;
		}
		else {//�¦Y�� 
			*curBlack ^= c1;
			*curRed ^= c2;
			*curBlack |= c2;
		}
	}
	else//�L�Y�l���� 
	{
		if (8 > c1p) {//���� 
			*curRed ^= c1;
			*curRed |= c2;
		}
		else {//�°� 
			*curBlack ^= c1;
			*curBlack |= c2;
		}
		curPiece[0] ^= c2;//�Ů�-c2
		*curOccupied |= c2;//c2���l 
	}
}

int* UnSimMove(int ssrc, int sdst, U32 * curPiece, U32 * curRed, U32 * curBlack, U32 * curOccupied, int* curPie, int copy)
{
	int c1p, c2p = -1;
	U32 c1 = 1 << ssrc;
	U32 c2 = 1 << sdst;
	//cout<<hex<<c2; 
	for (int ii = 1; ii < 15; ii++) {
		U32 check = curPiece[ii] & c2;
		if (check != 0) {
			c1p = ii;
			break;
		}
	}
	c2p = copy;
	curPiece[c1p] ^= c2;//�M�����mc2
	curPiece[0] |= c2;//�Ů�+c2
	curPiece[c1p] |= c1;//����
	*curOccupied |= c1;//c1���l
	curPiece[0] ^= c1;//�Ů�-c1
	if (c2p != -1) {//�Y�l���� 
		curPiece[c2p] |= c2;//�^���mc2
		int c2ps = c2p - 1;
		curPie[c2ps]++;
		curPiece[0] ^= c2;
		if (8 > c1p) {//���Y��
			*curRed |= c1;
			*curBlack |= c2;
			*curRed ^= c2;
		}
		else {//�¦Y�� 
			*curBlack |= c1;
			*curRed |= c2;
			*curBlack ^= c2;
		}
	}
	else//�L�Y�l���� 
	{
		if (8 > c1p) {//���� 
			*curRed |= c1;
			*curRed ^= c2;
		}
		else {//�°� 
			*curBlack |= c1;
			*curBlack ^= c2;
		}
		*curOccupied ^= c2;//c2�L�l 
	}
}

void SimReveal(int ssrc, int pID, U32 curPiece[16], U32 curRed, U32 curBlack)
{
	U32 c = 1 << ssrc;
	if (pID < 8)
	{
		curPiece[pID] |= c;
		curRed |= c;
		curPiece[15] ^= c;
	}
	else
	{
		curPiece[pID] |= c;
		curBlack |= c;
		curPiece[15] ^= c;
	}
}

void UnSimReveal(int ssrc, int pID, U32 curPiece[16], U32 curRed, U32 curBlack)
{
	U32 c = 1 << ssrc;
	if (pID < 8)
	{
		curPiece[pID] ^= c;
		curRed ^= c;
		curPiece[15] |= c;
	}
	else
	{
		curPiece[pID] ^= c;
		curBlack ^= c;
		curPiece[15] |= c;
	}
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

void dynamicPower()
{
	piPw[0] = 6000 + piece_count[7] * 600 + piece_count[8] * 270 + piece_count[9] * 90 + piece_count[10] * 40 + piece_count[11] * 15 + piece_count[12] * 200;
	piPw[1] = 2700 + piece_count[8] * 270 + piece_count[9] * 90 + piece_count[10] * 40 + piece_count[11] * 15 + piece_count[12] * 200 + piece_count[13] * 10;
	piPw[2] = 900 + piece_count[9] * 90 + piece_count[10] * 40 + piece_count[11] * 15 + piece_count[12] * 200 + piece_count[13] * 10;
	piPw[3] = 400 + piece_count[10] * 40 + piece_count[11] * 15 + piece_count[12] * 200 + piece_count[13] * 10;
	piPw[4] = 150 + piece_count[11] * 15 + piece_count[12] * 200 + piece_count[13] * 10;
	piPw[5] = 2000 + piece_count[7] * 600 + piece_count[8] * 270 + piece_count[9] * 90 + piece_count[10] * 40 + piece_count[11] * 15 + piece_count[12] * 200 + piece_count[13] * 10;
	piPw[6] = 100 + piece_count[7] * 600 + piece_count[13] * 10;

	piPw[7] = 6000 + piece_count[0] * 600 + piece_count[1] * 270 + piece_count[2] * 90 + piece_count[3] * 40 + piece_count[4] * 15 + piece_count[5] * 200;//24300
	piPw[8] = 2700 + piece_count[1] * 270 + piece_count[2] * 90 + piece_count[3] * 40 + piece_count[4] * 15 + piece_count[5] * 200 + piece_count[6] * 10;//15500
	piPw[9] = 900 + piece_count[2] * 90 + piece_count[3] * 40 + piece_count[4] * 15 + piece_count[5] * 200 + piece_count[6] * 10;//8300
	piPw[10] = 400 + piece_count[3] * 40 + piece_count[4] * 15 + piece_count[5] * 200 + piece_count[6] * 10;//6000
	piPw[11] = 150 + piece_count[4] * 15 + piece_count[5] * 200 + piece_count[6] * 10;//4950
	piPw[12] = 2000 + piece_count[0] * 600 + piece_count[1] * 270 + piece_count[2] * 90 + piece_count[3] * 40 + piece_count[4] * 15 + piece_count[5] * 200 + piece_count[6] * 10;//20800
	piPw[13] = 100 + piece_count[0] * 600 + piece_count[6] * 10;//6600
}

int findPiece(int place, U32 curPiece[16])
{
	U32 bplace = 1 << place;
	for (int i = 1; i < 15; i++)//��14�شѤ� �֦b�o�Ӧ�m�W 
	{
		if ((curPiece[i] & bplace) != 0)
		{
			i--;
			return i;//���Ѥl�^�� 
		}
	}
	return -1;//���~ 
}

void readBoard()//Ū�ɼҦ� Ū��board.txt�M�� 
{
	vector<string> move;
	int cp = 0;//current_position�p���} 
	int line = 0;//�ثeŪ�쪺��� 
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
				piece_count[i] = str[i * 2 + 2] - '0';//��Ѿl�ѤlŪ�Jpiece_count 
			}
		}
		if (line >= 3 && line <= 10 && initailBoard == 1)
		{
			for (int i = 0; i <= 3; i++)
			{
				current_position[cp] = str[i * 2 + 2];//Ū�J��l���W���p �uŪ�@�� 
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
	if (first && !move.empty()) {//���� �B�z�ڤ��C�� 
		if (move[0].at(3) - 91 > 0)//�p�g
			color = 1;//��
		else
			color = 0;//��
	}
	else if (!move.empty()) {//���
		if (move[0].at(3) - 91 > 0)//�p�g
			color = 0;//��
		else
			color = 1;//��
	}
	else {
		color = 0;
	}
	//cout << "color(b0 r1 u-1): " << color<<"\n";
	for (int i = 0; i < move.size(); i++) {//record in char board[4][8] �B�zhistory ���Ͳ{�b�Ѫ� 
		if (move[i].at(2) == '(') {//record move 'flip' (kgmrncp)
			//string a=move[i].substr(0,2);
			int aa = 100 - move[i].at(0);
			int bb = 4 * (56 - move[i].at(1));
			U32 cc = aa + bb;//�ѽL�s��0~31
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
		else {//a1-b1���� 
			int c1p, c2p;
			c2p = -1;//�p�G�O-1������ 
			int aa = 100 - move[i].at(0);
			int bb = 4 * (56 - move[i].at(1));
			U32 c1 = aa + bb;//�ѽL�s��0~31
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
			U32 c2 = aaa + bbb;//�ѽL�s��0~31
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
			for (int ii = 1; ii < 15; ii++) {//��c2 �M�� �Ѿl�Ѥl�Ƨ�� 
				U32 check = piece[ii] & c2;
				if (check != 0) {
					c2p = ii;
				}
			}
			piece[c1p] ^= c1;//�M�����mc1
			piece[0] |= c1;//�Ů�+c1
			piece[c1p] |= c2;//����
			if (c2p != -1) {//�Y�l���� 
				piece[c2p] ^= c2;//�M�����mc2
				int c2ps = c2p - 1;
				piece_count[c2ps]--;
				if (8 > c1p) {//���Y��
					red ^= c1;
					black ^= c2;
					red |= c2;
				}
				else {//�¦Y�� 
					black ^= c1;
					red ^= c2;
					black |= c2;
				}
				occupied ^= c1;//c1�L�l 
			}
			else//�L�Y�l���� 
			{
				if (8 > c1p) {//���� 
					red ^= c1;
					red |= c2;
				}
				else {//�°� 
					black ^= c1;
					black |= c2;
				}
				piece[0] ^= c2;//�Ů�-c2
				occupied ^= c1;//c1�L�l 
				occupied |= c2;//c2���l 
			}
			//board[move[i].at(3) - 'a'][move[i].at(4) - '1'] = board[move[i].at(0) - 'a'][move[i].at(1) - '1'];	//board[a][2]=k, a1-a2
			//board[move[i].at(0) - 'a'][move[i].at(1) - '1'] = '-';												//board[a][1]='-', a1-a2
			//cout << board[move[i].at(3) - 'a'][move[i].at(4) - '1'];
		}
	}
	//cout<<hex<<piece[9];
}

void createMovetxt()//�I���Ҧ� 
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
	for (int i = 0; i <= 14; i++)
		piece[i] = 0;
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

void printDebug() {
	while (1) {
		cout << "enter two int(all:-1 -1): " << endl;
		int in1 = 0;
		int in2 = 0;
		cin >> in1;
		cin >> in2;

		fstream file;
		if (in1 == -1 && in2 == -1) {
			for (int trees = 0; trees < finalScore.size(); trees++) {
				string filename;
				filename.append(to_string(finalsrc[trees]));
				filename.append(" ");
				filename.append(to_string(finaldst[trees]));
				filename.append(".txt");
				file.open(filename, ios::out);
				node* top;
				queue<node*> toporder;
				file << "depth= " << 1 << endl;
				for (int i = 0; i < nodes[1].size(); i++) {
					if (!nodes[1][i]->visit && nodes[1][i]->count == finalScore[trees]) {
						toporder.push(nodes[1][i]);
						nodes[1][i]->visit = true;
						file << "nodes[1][" << i << "]: " << nodes[1][i]->count
							<< "move: " << nodes[1][i]->move[0] << " " << nodes[1][i]->move[1] << endl;

						for (int j = 0; j < 16; j++) {
							file << "piece[" << j << "]: " << hex << setw(20) << nodes[1][i]->piece[j] << dec
								<< " " << setw(20) << "piece[" << j << "]: " << hex << nodes[1][i]->parent->piece[j] << dec << endl;
						}
					}
				}
				if (!toporder.empty()) {
					top = toporder.front();
					toporder.pop();
				}
				for (int depth = 2; depth < nodes.size(); depth++) {
					file << endl;
					file << "depth= " << depth << endl;

					while (top->depth == (depth - 1)) {
						for (int i = 0; i < nodes[depth].size(); i++) {
							if (!nodes[depth][i]->visit && nodes[depth][i]->parent == top && nodes[depth][i]->count == top->count) {
								toporder.push(nodes[depth][i]);
								nodes[depth][i]->visit = true;
								file << endl;

								file << "nodes[" << depth << "][" << i << "]: " << nodes[depth][i]->count
									<< "move: " << nodes[depth][i]->move[0] << " " << nodes[depth][i]->move[1]
									<< " " << setw(30) << "parent: " << nodes[depth][i]->parent->count
									<< " move: " << nodes[depth][i]->parent->move[0] << " " << nodes[depth][i]->parent->move[1] << endl;

								for (int j = 0; j < 16; j++) {
									file << "piece[" << j << "]: " << hex << setw(20) << nodes[depth][i]->piece[j] << dec
										<< " " << setw(20) << "piece[" << j << "]: " << hex << nodes[depth][i]->parent->piece[j] << dec << endl;
								}

							}
							else if (!nodes[depth][i]->visit && top->move[0] == top->move[1] && nodes[depth][i]->parent == top) {
								toporder.push(nodes[depth][i]);
								nodes[depth][i]->visit = true;
								file << endl;

								file << "nodes[" << depth << "][" << i << "]: " << nodes[depth][i]->count
									<< "move: " << nodes[depth][i]->move[0] << " " << nodes[depth][i]->move[1]
									<< " " << setw(30) << "parent: " << nodes[depth][i]->parent->count
									<< " move: " << nodes[depth][i]->parent->move[0] << " " << nodes[depth][i]->parent->move[1] << endl;

								for (int j = 0; j < 16; j++) {
									file << "piece[" << j << "]: " << hex << setw(20) << nodes[depth][i]->piece[j] << dec
										<< " " << setw(20) << "piece[" << j << "]: " << hex << nodes[depth][i]->parent->piece[j] << dec << endl;
								}

							}
							else {
								;
							}
						}

						if (!toporder.empty()) {
							top = toporder.front();
							toporder.pop();
						}

					}

				}
				file.close();
				cout << "done: " << trees + 1 << "/" << finalScore.size() << endl;
			}
		}
		else {
			int trees;
			for (trees = 0; trees <= finalScore.size(); trees++) {
				if (finalsrc[trees] == in1 && finaldst[trees] == in2)
					break;
			}
			string filename;
			filename.append(to_string(finalsrc[trees]));
			filename.append(" ");
			filename.append(to_string(finaldst[trees]));
			filename.append(".txt");
			file.open(filename, ios::out);
			node* top;
			queue<node*> toporder;
			file << "depth= " << 1 << endl;
			for (int i = 0; i < nodes[1].size(); i++) {
				if (!nodes[1][i]->visit && nodes[1][i]->count == finalScore[trees]) {
					toporder.push(nodes[1][i]);
					nodes[1][i]->visit = true;
					file << "nodes[1][" << i << "]: " << nodes[1][i]->count
						<< "move: " << nodes[1][i]->move[0] << " " << nodes[1][i]->move[1] << endl;

					for (int j = 0; j < 16; j++) {
						file << "piece[" << j << "]: " << hex << setw(20) << nodes[1][i]->piece[j] << dec
							<< " " << setw(20) << "piece[" << j << "]: " << hex << nodes[1][i]->parent->piece[j] << dec << endl;
					}
				}
			}
			if (!toporder.empty()) {
				top = toporder.front();
				toporder.pop();
			}
			for (int depth = 2; depth < nodes.size(); depth++) {
				file << endl;
				file << "depth= " << depth << endl;

				while (top->depth == (depth - 1)) {
					for (int i = 0; i < nodes[depth].size(); i++) {
						if (!nodes[depth][i]->visit && nodes[depth][i]->parent == top && nodes[depth][i]->count == top->count) {
							toporder.push(nodes[depth][i]);
							nodes[depth][i]->visit = true;
							file << endl;

							file << "nodes[" << depth << "][" << i << "]: " << nodes[depth][i]->count
								<< "move: " << nodes[depth][i]->move[0] << " " << nodes[depth][i]->move[1]
								<< " " << setw(30) << "parent: " << nodes[depth][i]->parent->count
								<< " move: " << nodes[depth][i]->parent->move[0] << " " << nodes[depth][i]->parent->move[1] << endl;

							for (int j = 0; j < 16; j++) {
								file << "piece[" << j << "]: " << hex << setw(20) << nodes[depth][i]->piece[j] << dec
									<< " " << setw(20) << "piece[" << j << "]: " << hex << nodes[depth][i]->parent->piece[j] << dec << endl;
							}

						}
						else if (!nodes[depth][i]->visit && top->move[0] == top->move[1] && nodes[depth][i]->parent == top) {
							toporder.push(nodes[depth][i]);
							nodes[depth][i]->visit = true;
							file << endl;

							file << "nodes[" << depth << "][" << i << "]: " << nodes[depth][i]->count
								<< "move: " << nodes[depth][i]->move[0] << " " << nodes[depth][i]->move[1]
								<< " " << setw(30) << "parent: " << nodes[depth][i]->parent->count
								<< " move: " << nodes[depth][i]->parent->move[0] << " " << nodes[depth][i]->parent->move[1] << endl;

							for (int j = 0; j < 16; j++) {
								file << "piece[" << j << "]: " << hex << setw(20) << nodes[depth][i]->piece[j] << dec
									<< " " << setw(20) << "piece[" << j << "]: " << hex << nodes[depth][i]->parent->piece[j] << dec << endl;
							}

						}
						else {
							;
						}
					}

					if (!toporder.empty()) {
						top = toporder.front();
						toporder.pop();
					}

				}

			}
			file.close();

		}
	}
}
