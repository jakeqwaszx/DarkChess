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
#include <map>
#include "score.h"

using namespace std;
/*
	½�ѥ���� �L�P����
*/


unsigned int LS1B(unsigned int x) { return x & (-x); }//���ox���̧C�줸
unsigned int MS1B(unsigned int x) { // Most Significant 1 Bit (LS1B)�禡
	x |= x >> 32; x |= x >> 16; x |= x >> 8;
	x |= x >> 4; x |= x >> 2; x |= x >> 1;
	return (x >> 1) + 1; //�i�H���o�@��줸���̥��䤣���s���줸
}
unsigned int CGen(int ssrc, unsigned int toccupied);//���٬��� 
unsigned int CGenCR(unsigned int x);
unsigned int CGenCL(unsigned int x);
int BitsHash(unsigned int x) { return (x * 0x08ED2BE6) >> 27; }
void initial();//��l�� 
void chess(vector<unsigned int> tpiece, int deep, int& AEMindex, int& AOMindex, int& EAEMindex, int& EAOMindex, int allEatMove[50][2], int allOnlyMove[50][2]);//�M��i�β���
void readBoard();//Ū�ɼҦ� Ū��board.txt ��Ū�J�ɮ��নbitboard �٨S�˵ۦs�J 
void createMovetxt(string src, string dst, int srci, int dsti);//�гymove.txt 0���B 1½�� 
void IndexToBoard(int indexa, int indexb, string& src, string& dst);//��src dst�q�s��0~31->�ѽL�s��a1~d4 
int countAva(vector<int> pie, int deep, vector<unsigned int> curPiece, int AEMindex, int AOMindex, int EAEMindex, int EAOMindex);//�I�s�h�Ǧ^��e�Ѫ�
int search(int depth, vector<unsigned int> curPiece, vector<int> curPie, int alpha, int beta, vector<int> lDCount, unsigned int hashvalue);//�j�M�̨Ψ��B
void drawOrNot();//��past_walk�P�_�O�_���� ���ᵲ�G��X��draw 
int findPiece(int place, unsigned int curPiece[16]);//�ǽs�� �^�Ǧb�o�ӽs�����Ѥl 
unsigned int myhash(unsigned int tpiece[16]);//�ǤJ�L���A�Ǧ^hash��

int index32[32] = { 31, 0, 1, 5, 2, 16, 27, 6, 3, 14, 17, 19, 28, 11, 7, 21, 30, 4, 15, 26, 13,
18, 10, 20, 29, 25, 12, 9, 24, 8, 23, 22 };
int GetIndex(unsigned int mask) { return index32[BitsHash(mask)]; }//��J�B�n��^�ѽL�s��
unsigned int pMoves[32] = { 0x00000012,0x00000025,0x0000004A,0x00000084,0x00000121,0x00000252,0x000004A4,0x00000848,
0x00001210,0x00002520,0x00004A40,0x00008480,0x00012100,0x00025200,0x0004A400,0x00084800,
0x00121000,0x00252000,0x004A4000,0x00848000,0x01210000,0x02520000,0x04A40000,0x08480000,
0x12100000,0x25200000,0x4A400000,0x84800000,0x21000000,0x52000000,0xA4000000,0x48000000 };//�Ѥl���ʾB�n
unsigned int pMoves2[32] = {
0x00000116,0x0000022D,0x0000044B,0x00000886,0x00001161,0x000022D2,0x000044B4,0x00008868,
0x00011611,0x00022D22,0x00044B44,0x00088688,0x00116110,0x0022D220,0x0044B440,0x00886880,
0x01161100,0x022D2200,0x044B4400,0x08868800,0x11611000,0x22D22000,0x44B44000,0x88688000,
0x16110000,0x2D220000,0x4B440000,0x86880000,0x61100000,0xD2200000,0xB4400000,0x68800000 };//½�ѾB�n
unsigned int pMoves3[32] = {
0x00000136,0x0000027D,0x000004EB,0x000008C6,0x00001363,0x000027D7,0x00004EB4,0x00008C6C,
0x00013631,0x00027D72,0x0004EBE4,0x0008C6C8,0x00136310,0x0027D720,0x004EBE40,0x008C6C80,
0x01363100,0x027D7200,0x04EBE400,0x08C6C800,0x13631000,0x27D72000,0x4EBE4000,0x8C6C8000,
0x36310000,0x7D720000,0xEBE40000,0xC6C80000,0x63100000,0xD7200000,0xBE400000,0x6C800000 };//½�ѾB�n
unsigned int file[4] = { 0x11111111,0x22222222,0x44444444,0x88888888 };//��B�n 
unsigned int row[8] = { 0x0000000F,0x000000F0,0x00000F00,0x0000F000,0x000F0000,0x00F00000,0x0F000000,0xF0000000 };//�C�B�n 
unsigned int piece[16]; //0�Ů�- ��k �hg ��m ��r ��n ��c �Lp *2 15��½x 
unsigned int red, black, occupied;//�� �� ���Ѥl 
int influenceValue[7][7] = {
	120,108,60,36,24,48,0,
	0,106,60,36,24,48,12,
	0,0,58,36,24,48,12,
	0,0,0,34,24,48,12,
	0,0,0,0,22,48,12,
	0,0,0,0,0,0,0,
	108,0,0,0,0,0,10
};

string move = "a1-a1";//�U�@�B��� �Ω�I�� 
int piece_count[14] = { 1,2,2,2,2,2,5,1,2,2,2,2,2,5 };//�Ѿl�Ѥl�� 0-6 7-13
int DCount[14] = { 1,2,2,2,2,2,5,1,2,2,2,2,2,5 };//�Ѿl��½�l 
string current_position[32];//�L�����p�`�� 
string history;
int timeCount;//�Ѿl�ɶ� 
int initailBoard = 1;//�O�_Ū����l���� 
int past_walk[7][2] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 };//�e�C�B �ΨӳB�z�������D 
int draw = 0;//0�L���ⱡ�p 1���i��i�J���� 
//int RMcount=13;//Ū���Ҧ��ݭn �q13��}�lŪ�����G 

int color;//0 red 1 black
int maxDepth = 6;
unsigned int open = 0xffffffff;//�D��½��
unsigned int ch;//�ݭnsearch����m 
int noReDepth = 1;
unsigned int randtable[15][32]; //0~13 ������L�� 14 ��½

struct hashdata {
	int count; int depth; unsigned int curPiece[16];
	unsigned int NextCurPiece[4]; unsigned int NextHashvalue; int MaxDepth; int NextCurPie;
};
unordered_map<unsigned int, hashdata > hashtable;
unordered_map<unsigned int, hashdata > hashtablef;
int TotalSearch = 0;
int HashHit = 0;
int RealHit = 0;
int HashHitSameTurn = 0;
int MoreDepth = 0;
int LessDepth = 0;
int NoUseHash = 0;

int Power[2][3][3][3][3][3][6][7];//all possible power
int DistanceP[32][32][7][7];

clock_t start, stop;//�{������ɶ�
int TimeOut;//�O�_�W��
int TimeLimit;//�ɶ�����

int test = 0;
int test1 = 0;
int test2 = 0;
int test3 = 0;
int test4 = 0;
int test5 = 0;
int test6 = 0;
int test7 = 0;
int test8 = 0;


int main() {
	start = clock();
	srand(time(NULL));
	initial();//��l�� 
	readBoard();
	drawOrNot();
	int onboardi = 0;//�p������W��½�Ѽƶq 
	int onboardpi = 0;//�p������W��½�Ѻ��� 
	int LivePieces = 0;//�p������W���Ѽƶq 
	int complex = 0;
	for (int i = 0; i < 14; i++)
	{
		onboardi += DCount[i];
		LivePieces += piece_count[i];
		if (DCount[i])
			onboardpi++;
	}
	if (onboardi == 0)maxDepth = 10;
	complex = onboardi * onboardpi + LivePieces;
	//if (onboardi <=27)maxDepth = (1000 - complex) / 100;
	open = 0xffffffff;
	ch = 0x00000000;
	open ^= piece[15];//�D��½��
	if (piece[15] == 0xffffffff)
	{
		string a = "a";
		IndexToBoard(9, 9, a, a);
		createMovetxt(a, a, 0, 0);
		return 0;
	}
	while (open)
	{
		unsigned int take = LS1B(open);
		open ^= take;
		take = GetIndex(take);
		ch |= pMoves3[take];
	}
	unsigned int hashvalue = myhash(piece);
	for (TimeLimit = 1000000; (double(stop - start) < TimeLimit && maxDepth < 21); maxDepth++) {
		TotalSearch = 0;
		HashHit = 0;
		HashHitSameTurn = 0;
		RealHit = 0;
		vector<unsigned int> lcurPiece;
		vector<int> lcurPie;
		vector<int> lDCount;
		lcurPiece.assign(piece, piece + 16);
		lcurPie.assign(piece_count, piece_count + 14);
		lDCount.assign(DCount, DCount + 14);
		search(0, lcurPiece, lcurPie, -999999, 999999, lDCount, hashvalue);
		stop = clock();
		cout << " ���B�Ӯ� : " << double(stop - start) / CLOCKS_PER_SEC << " ��(��ǫ�0.001��) " << endl;
		cout << " �`�� : " << maxDepth << endl;
		cout << " ���謰 : ";
		if (color == 0) cout << "�� " << endl;
		else cout << "�� " << endl;
		cout << "total search: " << TotalSearch << endl;
		cout << "hash hit: " << HashHit << endl;
		cout << "hash hit & same turn: " << HashHitSameTurn << endl;
		cout << "real hit: " << RealHit << endl;
		cout << "complex: " << complex << endl;
		cout << "MoreDepth: " << MoreDepth << endl;
		cout << "LessDepth: " << LessDepth << endl;
		cout << "NoUseHash: " << NoUseHash << endl;
		//cout << "maxdepth: " << maxDepth << endl;
		TimeLimit = 1;
	}
}

void chess(vector<unsigned int> tpiece, int deep, int& AEMindex, int& AOMindex, int& EAEMindex, int& EAOMindex, int allEatMove[50][2], int allOnlyMove[50][2])
{
	unsigned int tred, tblack;
	unsigned int dest;//�i�H�Y�l�����
	AEMindex = 0;
	AOMindex = 0;
	EAEMindex = 0;
	EAOMindex = 0;
	unsigned int toccupied = 0xFFFFFFFF;
	toccupied ^= tpiece[0];
	tred = tpiece[1] | tpiece[2] | tpiece[3] | tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7];
	tblack = tpiece[8] | tpiece[9] | tpiece[10] | tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14];
	int ssrc = 0;
	int check = (color + deep) % 2;//0red 1black
	if (check == 0) {//�� 
		//cout<<"Ours available eat:"<<endl;
		for (int i = 1; i < 8; i++) { //1~7 ����~�L,src ���Ѥl�_�I,dest �����I�C  ����ڤ� 
			unsigned int p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
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
				else if (i == 6) //��,�S��B�z�C
					dest = CGen(ssrc, toccupied) & tblack;
				else if (i == 7) //�L,�u��Y�N(8)�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
					dest = 0;
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					allEatMove[AEMindex][0] = ssrc;
					allEatMove[AEMindex][1] = result;
					AEMindex++;
				}
			}
		}
		for (int i = 1; i < 8; i++) { //����²���
			unsigned int p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					allOnlyMove[AOMindex][0] = ssrc;
					allOnlyMove[AOMindex][1] = result;
					AOMindex++;
				}
			}
		}
		for (int i = 8; i < 15; i++) { //�A������ 
			unsigned int p = tpiece[i];
			while (p) {
				unsigned int mask = LS1B(p);
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
				else if (i == 13) //��,�S��B�z�C
					dest = CGen(ssrc, toccupied) & tred;
				else if (i == 14) //�L,�u��Y�N�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
					dest = 0;
				while (dest) { //����ʦs�JEallEatMove 
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					//EallEatMove[EAEMindex][0] = ssrc;
					//EallEatMove[EAEMindex][1] = result;
					EAEMindex++;
				}
			}
		}
		for (int i = 8; i < 15; i++) { //���²��� 
			unsigned int p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N�¦� 8~14 �����l���j�M�@�M
				unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					//EallOnlyMove[EAOMindex][0] = ssrc;
					//EallOnlyMove[EAOMindex][1] = result;
					EAOMindex++;
				}
			}
		}
	}
	else {//�¤貾�� 
		for (int i = 8; i < 15; i++) { //1~7 ����~�L,src ���Ѥl�_�I,dest �����I�C
			unsigned int p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N�¦� 1~7 �����l���j�M�@�M
				unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
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
				else if (i == 13) //��,�S��B�z�C
					dest = CGen(ssrc, toccupied) & tred;
				else if (i == 14) //�L,�u��Y�N�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
					dest = 0;
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					allEatMove[AEMindex][0] = ssrc;
					allEatMove[AEMindex][1] = result;
					AEMindex++;
				}
			}
		}
		for (int i = 8; i < 15; i++) { //�¤�²���
			unsigned int p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N�¦� 8~14 �����l���j�M�@�M
				unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					allOnlyMove[AOMindex][0] = ssrc;
					allOnlyMove[AOMindex][1] = result;
					AOMindex++;
				}
			}
		}
		for (int i = 1; i < 8; i++) { //�p��Ĥ貾�� 
			unsigned int p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
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
				else if (i == 6) //��,�S��B�z�C
					dest = CGen(ssrc, toccupied) & tblack;
				else if (i == 7) //�L,�u��Y�N(8)�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
					dest = 0;
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					//EallEatMove[EAEMindex][0] = ssrc;
					//EallEatMove[EAEMindex][1] = result;
					EAEMindex++;
				}
			}
		}
		for (int i = 1; i < 8; i++) { //����²���
			unsigned int p = tpiece[i]; //���o�Ѥl��m
			while (p) { //�N���� 1~7 �����l���j�M�@�M
				unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while (dest) { //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
					unsigned int mask2 = LS1B(dest);
					dest ^= mask2;
					int result = GetIndex(mask2);
					//EallOnlyMove[EAOMindex][0] = ssrc;
					//EallOnlyMove[EAOMindex][1] = result;
					EAOMindex++;
				}
			}
		}
	}
}

unsigned int CGen(int ssrc, unsigned int toccupied)
{
	int r = ssrc / 4;//�C 
	int c = ssrc % 4;//�� 
	unsigned int result = 0;
	unsigned int resulta = 0;
	unsigned int x = ((row[r] & toccupied) ^ (1 << ssrc)) >> (4 * r);//���X�򬶦P�C �d�h����������
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
		unsigned int part = 0;
		if ((x & 0x00000011) == 0x00000011)
			part |= 1;
		resulta |= part | CGenCL(x & 0x11111000);
	}
	else if (x && r == 3)
	{
		unsigned int part = 0;
		part = CGenCR(x & 0x00000111);
		resulta |= part | CGenCL(x & 0x11110000);
	}
	else if (x && r == 4)
	{
		unsigned int part = 0;
		part = CGenCR(x & 0x00001111);
		resulta |= part | CGenCL(x & 0x11100000);
	}
	else if (x && r == 5)
	{
		unsigned int part = 0;
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
	unsigned int re = result | resulta;
	//cout<<hex<<occupied<<" "<<re<<endl;
	//cout<<dec<<r<<" "<<c<<endl;
	return re;
}

unsigned int CGenCL(unsigned int x) {
	if (x) {
		unsigned int mask = LS1B(x); //mask �����[���B�n��m,�� x ���h���[�C
		return (x ^= mask) ? LS1B(x) : 0; //���p 5~8 �Ǧ^ LS1B(x),���p 2~4 �Ǧ^ 0�C
	}
	else return 0;
}

unsigned int CGenCR(unsigned int x) {
	if (x) {
		unsigned int mask = MS1B(x); //mask �����[���B�n��m,�� x ���h���[�C
		return (x ^= mask) ? MS1B(x) : 0; //���p 5~8 �Ǧ^ MS1B(x),���p 2~4 �Ǧ^ 0�C
	}
	else return 0;
}

int countAva(vector<int> pie, int deep, vector<unsigned int> curPiece, int AEMindex, int AOMindex, int EAEMindex, int EAOMindex)//�N�h�ۨ������L
{
	int eat[10];
	int power = 0;
	int redleft = 0;
	int blackleft = 0;
	if (color == 0)//�� 
	{
		for (int i = 0; i < 7; i++)
		{
			power += pie[i] * Power[pie[7]][pie[8]][pie[9]][pie[10]][pie[11]][pie[12]][pie[13]][i];
			redleft += pie[i];
		}
		for (int i = 7; i < 14; i++)
		{
			power -= pie[i] * Power[pie[0]][pie[1]][pie[2]][pie[3]][pie[4]][pie[5]][pie[6]][i - 7];
			blackleft += pie[i];
		}
		if (redleft == 0) power = -100000 + deep * 1000;
		if (blackleft == 0) power = 100000 - deep * 1000;
	}
	else
	{
		for (int i = 7; i < 14; i++)
		{
			power += pie[i] * Power[pie[0]][pie[1]][pie[2]][pie[3]][pie[4]][pie[5]][pie[6]][i - 7];
			blackleft += pie[i];
		}
		for (int i = 0; i < 7; i++)
		{
			power -= pie[i] * Power[pie[7]][pie[8]][pie[9]][pie[10]][pie[11]][pie[12]][pie[13]][i];
			redleft += pie[i];
		}
		if (blackleft == 0) power = -100000 + deep * 1000;
		if (redleft == 0) power = 100000 - deep * 1000;
	}
	//------------------------------------------------------�P�_�Z�� 
	int distanceScore = 0;
	if (curPiece[15] == 0) {
		if (color == 0)
		{
			for (int i = 1; i < 8; i++)
			{
				if (curPiece[i] && i != 6)//curp!=0 �����W�s�b
				{
					unsigned int tempcpa = curPiece[i];
					while (tempcpa)
					{
						unsigned int a = LS1B(tempcpa);//�@�Ӥ@�ӳB�z 
						tempcpa = tempcpa ^ a;
						for (int ii = i + 7; ii < 15; ii++)
						{
							if (i == 1 && ii == 14) continue;
							if (curPiece[ii])
							{
								unsigned int tempcpb = curPiece[ii];
								while (tempcpb)
								{
									unsigned int b = LS1B(tempcpb);//�@�Ӥ@�ӳB�z 
									tempcpb = tempcpb ^ b;

									int ia = GetIndex(a);//�o��a��index 0~31 
									int ib = GetIndex(b);//�o��b��index 

									int tempS = DistanceP[ia][ib][i - 1][ii - 8];
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
				if (curPiece[i] && i != 13)//curp!=0 �����W�s�b
				{
					unsigned int tempcpa = curPiece[i];
					while (tempcpa)
					{
						unsigned int a = LS1B(tempcpa);//�@�Ӥ@�ӳB�z 
						tempcpa = tempcpa ^ a;
						for (int ii = i - 7; ii < 8; ii++)
						{
							if (i == 8 && ii == 7) continue;
							if (curPiece[ii])
							{
								unsigned int tempcpb = curPiece[ii];
								while (tempcpb)
								{
									unsigned int b = LS1B(tempcpb);//�@�Ӥ@�ӳB�z 
									tempcpb = tempcpb ^ b;
									int ia = GetIndex(a);//�o��a��index 0~31
									int ib = GetIndex(b);//�o��b��index

									int tempS = DistanceP[ia][ib][i - 8][ii - 1];
									distanceScore += tempS;
								}
							}
						}
					}
				}
			}
		}
	}
	//------------------------------------------------------���ʼ� 
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
	//------------------------------------------------------�[�`
	return power + movePoint - deep + distanceScore;

}

int search(int depth, vector<unsigned int>curPiece, vector<int> curPie, int alpha, int beta, vector<int> lDCount, unsigned int hashvalue)
{
	/*if (double(stop - start) > TimeLimit) {
		TimeOut = 1;
		return 0;
	}*/
	stop = clock();
	TotalSearch++;
	unsigned int hashindex = hashvalue;
	int tempscore = 0;
	int HashCheck = 0;
	unsigned int HashTempC1;
	unsigned int HashTempC2;
	/*if (hashtable[hashindex].count != 0) {
		HashHit++;
		if (hashtable[hashindex].depth % 2 == depth % 2) {
			HashHitSameTurn++;
			int temp = 0;
			for (int i = 0; i < 16; i++) {
				if (curPiece[i] ^ hashtable[hashindex].curPiece[i]) {
					temp++;
					break;
				}
			}
			if (temp == 0) {
				RealHit++;
				if (((hashtable[hashindex].MaxDepth - hashtable[hashindex].depth) > (maxDepth - depth))
					) {
					if (hashtable[hashindex].MaxDepth % 2 == maxDepth % 2) {
						LessDepth++;
						return hashtable[hashindex].count - (depth - hashtable[hashindex].depth);
					}
					else {
						NoUseHash++;
					}
				}
				else {

					if (depth > noReDepth && hashtable[hashindex].NextHashvalue) {
						MoreDepth++;
						if (0) {
							test++;
						}
						else {
							test1++;
							HashCheck = 1;

							int tempNextCurPie = hashtable[hashindex].NextCurPie;
							int tempc1p = hashtable[hashindex].NextCurPiece[2];
							int tempc2p = hashtable[hashindex].NextCurPiece[3];
							if (tempNextCurPie !=-1) {//sim
								curPiece[hashtable[hashindex].NextCurPiece[2]] ^= hashtable[hashindex].NextCurPiece[0];//�M�����mc1
								curPiece[hashtable[hashindex].NextCurPiece[2]] |= hashtable[hashindex].NextCurPiece[1];//����
								curPiece[0] |= hashtable[hashindex].NextCurPiece[0];//�Ů�+c1
								curPiece[hashtable[hashindex].NextCurPiece[3]] ^= hashtable[hashindex].NextCurPiece[1];//�M�����mc2
								curPie[hashtable[hashindex].NextCurPie]--;
							}
							else {
								curPiece[hashtable[hashindex].NextCurPiece[2]] ^= hashtable[hashindex].NextCurPiece[0];//�M�����mc1
								curPiece[hashtable[hashindex].NextCurPiece[2]] |= hashtable[hashindex].NextCurPiece[1];//����
								curPiece[0] |= hashtable[hashindex].NextCurPiece[0];//�Ů�+c1
								curPiece[0] ^= hashtable[hashindex].NextCurPiece[1];//�Ů�-c2
							}

							HashTempC1 = hashtable[hashindex].NextCurPiece[0];
							HashTempC2 = hashtable[hashindex].NextCurPiece[1];
							tempscore = search(depth + 1, curPiece, curPie, alpha, beta, hashtable[hashindex].NextHashvalue);

							if (tempNextCurPie != -1) {//unsim
								curPiece[tempc1p] ^= HashTempC2;//�M�����mc2
								curPiece[tempc1p] |= HashTempC1;//����
								curPiece[0] ^= HashTempC1;//�Ů�-c1
								curPiece[tempc2p] |= HashTempC2;//�^���mc2
								curPie[tempNextCurPie]++;
							}
							else {
								curPiece[tempc1p] ^= HashTempC2;//�M�����mc2
								curPiece[0] |= HashTempC2;//�Ů�+c2
								curPiece[tempc1p] |= HashTempC1;//����
								curPiece[0] ^= HashTempC1;//�Ů�-c1
							}

							if (depth % 2 == 0)//max
							{
								if (tempscore > alpha)
								{
									alpha = tempscore;
								}

							}
							else//min
							{
								if (tempscore < beta)
								{
									beta = tempscore;
								}
							}
							if (beta <= alpha)
							{
								hashtable[hashindex].depth = depth;
								hashtable[hashindex].MaxDepth = maxDepth;
								if (depth % 2 == 0) {//max
									hashtable[hashindex].count = alpha;
									return alpha;
								}
								else {//min
									hashtable[hashindex].count = beta;
									return beta;
								}
							}
						}
					}
					else {
						NoUseHash++;
					}
				}
			}
			else {//hash collision
				hashtable[hashindex].NextHashvalue = 0;
				memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
			}
		}
		else {//�h�Ƥ��P
		hashtable[hashindex].NextHashvalue = 0;
		memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
		}
	}
	else {//�s�L��
		memcpy(hashtable[hashindex].curPiece, curPiece, sizeof(hashtable[hashindex].curPiece));
	}*/
	int taEM[50][2];//�s�i�Y�l����k 0 src 1 dst �קK�Q���U�j�M�ɨ걼
	int taOM[50][2];//�s�i���ʫD�Y�l����k 0 src 1 dst
	int tAEMi;//alleatmove index
	int tAOMi;//allonlymove index
	int tEAEMi;//allonlymove index
	int tEAOMi;//allonlymove index
	chess(curPiece, depth, tAEMi, tAOMi, tEAEMi, tEAOMi, taEM, taOM);//void chess(unsigned int tpiece[16], int deep,int AEMindex,int AOMindex ,int EAEMindex ,int EAOMindex , unsigned int allEatMove[50][2] , unsigned int allOnlyMove[50][2])

	vector<int> weights;//�p��Ҧ����ʻP½�Ѫ���msrc
	vector<int> weightd;//�p��Ҧ����ʻP½�Ѫ���mdst
	vector<int> weight;//�p��Ҧ����ʻP½�Ѫ��o��
	int wp = 0;
	int best = -9999999;
	if (depth % 2 == 1) {
		best = 9999999;
	}
	if (HashCheck) {
		weight.push_back(tempscore);
		best = tempscore;
		weights.push_back(100);
		weightd.push_back(100);
		wp++;
	}
	for (int i = 0; i < tAEMi; i++)//�Y�l
	{
		int deeper = depth + 1;
		int c1p, c2p = -1;
		unsigned int c1 = 1 << taEM[i][0];
		unsigned int c2 = 1 << taEM[i][1];
		for (int ii = 1; ii < 15; ii++) {//���c1 ��Jc1p  
			unsigned int check = curPiece[ii] & c1;
			if (check != 0) {
				c1p = ii;
				break;
			}
		}
		for (int ii = 1; ii < 15; ii++) {//��c2
			unsigned int check = curPiece[ii] & c2;
			if (check != 0) {
				c2p = ii;
				break;
			}
		}
		if (c2p == -1 || c1p == -1) {//errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
			cout << endl;
			cout << "err1 c1: " << hex << c1 << " c2: " << c2 << endl;
			for (int j = 0; j < 16; j++) {
				cout << hex << curPiece[j] << dec << endl;
			}
			cout << "depth: " << depth << endl;
			cout << endl;
		}
		curPiece[c1p] ^= c1;//�M�����mc1
		curPiece[c1p] |= c2;//����
		curPiece[0] |= c1;//�Ů�+c1
		curPiece[c2p] ^= c2;//�M�����mc2
		curPie[c2p - 1]--;
		unsigned int tempc1 = c1;
		unsigned int tempc2 = c2;
		unsigned int tempc1p = c1p;
		unsigned int tempc2p = c2p;
		int tempNextCurPie = c2p - 1;
		unsigned int tempNextHashvalue = hashvalue ^ randtable[c1p - 1][GetIndex(c1)] ^ randtable[c2p - 1][GetIndex(c2)] ^ randtable[c1p - 1][GetIndex(c2)];

		/*if (HashCheck == 1) {
			if (c1 == HashTempC1 && c2 == HashTempC2) {
				curPiece[c1p] ^= c2;//�M�����mc2
				curPiece[c1p] |= c1;//����
				curPiece[0] ^= c1;//�Ů�-c1
				curPiece[c2p] |= c2;//�^���mc2
				curPie[c2p - 1]++;
				continue;
			}
			else {
			}
		}*/
		weights.push_back(taEM[i][0]);
		weightd.push_back(taEM[i][1]);
		vector<unsigned int> tcurPiece;
		vector<int> tcurPie;
		vector<int> tlDCount;
		tcurPiece.assign(curPiece.begin(), curPiece.end());
		tcurPie.assign(curPie.begin(), curPie.end());
		tlDCount.assign(lDCount.begin(), lDCount.end());
		int tempweight = 0;
		tempweight = search(deeper, tcurPiece, tcurPie, alpha, beta, tlDCount, hashvalue ^ randtable[c1p - 1][GetIndex(c1)] ^ randtable[c2p - 1][GetIndex(c2)] ^ randtable[c1p - 1][GetIndex(c2)]);
		weight.push_back(tempweight);
		if (depth == 0)
		{
			int r = rand() % 6;;
			string a[6] = { "��3��e","(--;)e","(���s��)e","(��-_-`)e","|�s���^e","(*���J��)e" };
			cout << a[r] + ".";
		}
		curPiece[c1p] ^= c2;//�M�����mc2
		curPiece[c1p] |= c1;//����
		curPiece[0] ^= c1;//�Ů�-c1
		curPiece[c2p] |= c2;//�^���mc2
		curPie[c2p - 1]++;

		if (depth % 2 == 0)//max
		{
			if (weight[wp] > alpha)
			{
				alpha = weight[wp];
			}
			if (weight[wp] > best)
			{
				best = weight[wp];
				/*hashtable[hashindex].NextCurPiece[0] = tempc1;
				hashtable[hashindex].NextCurPiece[1] = tempc2;
				hashtable[hashindex].NextCurPiece[2] = tempc1p;
				hashtable[hashindex].NextCurPiece[3] = tempc2p;
				hashtable[hashindex].NextCurPie = tempNextCurPie;
				hashtable[hashindex].NextHashvalue = tempNextHashvalue;*/
			}
		}
		else//min
		{
			if (weight[wp] < beta)
			{
				beta = weight[wp];
			}
			if (weight[wp] < best)
			{
				best = weight[wp];
				/*hashtable[hashindex].NextCurPiece[0] = tempc1;
				hashtable[hashindex].NextCurPiece[1] = tempc2;
				hashtable[hashindex].NextCurPiece[2] = tempc1p;
				hashtable[hashindex].NextCurPiece[3] = tempc2p;
				hashtable[hashindex].NextCurPie = tempNextCurPie;
				hashtable[hashindex].NextHashvalue = tempNextHashvalue;*/
			}
		}
		if (beta <= alpha)
		{
			//hashtable[hashindex].depth = depth;
			//hashtable[hashindex].MaxDepth = maxDepth;
			if (depth % 2 == 0) {//max
				//hashtable[hashindex].count = alpha;
				return alpha;
			}
			else {//min
				//hashtable[hashindex].count = beta;
				return beta;
			}
		}
		wp++;
	}

	if (depth >= maxDepth)
	{
		int re = countAva(curPie, depth, curPiece, tAEMi, tAOMi, tEAEMi, tEAOMi);
		/*hashtable[hashindex].count = re;
		hashtable[hashindex].depth = depth;
		hashtable[hashindex].MaxDepth = maxDepth;*/
		return re;
	}
	for (int i = 0; i < tAOMi; i++)//�²��� --------------------------------------------------------------------------------------------------------
	{
		int deeper = depth + 1;
		int c1p = -1;
		unsigned int c1 = 1 << taOM[i][0];
		unsigned int c2 = 1 << taOM[i][1];
		for (int ii = 1; ii < 15; ii++) {//��L��
			unsigned int check = curPiece[ii] & c1;
			if (check != 0) {
				c1p = ii;
				break;
			}
		}
		if (c1p == -1) {//errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
			cout << endl;
			cout << "err2 c1: " << hex << c1 << " c2: " << c2 << endl;
			for (int j = 0; j < 16; j++) {
				cout << hex << curPiece[j] << dec << endl;
			}
			cout << "depth: " << depth << endl;
			cout << endl;
		}

		curPiece[c1p] ^= c1;//�M�����mc1
		curPiece[c1p] |= c2;//����
		curPiece[0] |= c1;//�Ů�+c1
		curPiece[0] ^= c2;//�Ů�-c2

		unsigned int tempc1 = c1;
		unsigned int tempc2 = c2;
		unsigned int tempc1p = c1p;
		int tempNextCurPie = -1;
		unsigned int tempNextHashvalue = hashvalue ^ randtable[c1p - 1][GetIndex(c1)] ^ randtable[c1p - 1][GetIndex(c2)];

		/*if (HashCheck == 1) {
			if (c1 == HashTempC1 && c2 == HashTempC2) {
				curPiece[c1p] ^= c2;//�M�����mc2
				curPiece[0] |= c2;//�Ů�+c2
				curPiece[c1p] |= c1;//����
				curPiece[0] ^= c1;//�Ů�-c1
				continue;
			}
			else {
			}
		}*/

		weights.push_back(taOM[i][0]);
		weightd.push_back(taOM[i][1]);
		vector<unsigned int> tcurPiece;
		vector<int> tcurPie;
		vector<int> tlDCount;
		tcurPiece.assign(curPiece.begin(), curPiece.end());
		tcurPie.assign(curPie.begin(), curPie.end());
		tlDCount.assign(lDCount.begin(), lDCount.end());
		int tempweight = 0;
		tempweight = search(deeper, tcurPiece, tcurPie, alpha, beta, tlDCount, hashvalue ^ randtable[c1p - 1][GetIndex(c1)] ^ randtable[c1p - 1][GetIndex(c2)]);
		weight.push_back(tempweight);
		if (depth == 0)
		{
			int r = rand() % 6;;
			string a[6] = { "��3��m","(--;)m","(���s��)m","(��-_-`)m","|�s���^m","(*���J��)m" };
			cout << a[r] + ".";
		}
		curPiece[c1p] ^= c2;//�M�����mc2
		curPiece[0] |= c2;//�Ů�+c2
		curPiece[c1p] |= c1;//����
		curPiece[0] ^= c1;//�Ů�-c1



		if (depth % 2 == 0)//max
		{
			if (weight[wp] > best)
			{
				best = weight[wp];
				/*hashtable[hashindex].NextCurPiece[0] = tempc1;
				hashtable[hashindex].NextCurPiece[1] = tempc2;
				hashtable[hashindex].NextCurPiece[2] = tempc1p;
				hashtable[hashindex].NextCurPie = tempNextCurPie;
				hashtable[hashindex].NextHashvalue = tempNextHashvalue;*/
			}
			if (weight[wp] > alpha)
			{
				alpha = weight[wp];
			}
		}
		else//min
		{
			if (weight[wp] < best)
			{
				best = weight[wp];
				/*hashtable[hashindex].NextCurPiece[0] = tempc1;
				hashtable[hashindex].NextCurPiece[1] = tempc2;
				hashtable[hashindex].NextCurPiece[2] = tempc1p;
				hashtable[hashindex].NextCurPie = tempNextCurPie;
				hashtable[hashindex].NextHashvalue = tempNextHashvalue;*/
			}
			if (weight[wp] < beta)
			{
				beta = weight[wp];
			}
		}
		if (beta <= alpha)
		{
			//hashtable[hashindex].depth = depth;
			//hashtable[hashindex].MaxDepth = maxDepth;
			if (depth % 2 == 0) {//max
				//hashtable[hashindex].count = alpha;
				return alpha;
			}
			else {//min
				//hashtable[hashindex].count = beta;
				return beta;
			}
		}
		wp++;
	}

	if (curPiece[15] != 0)//����½�� ������call search 
	{
		for (int ssrc = 0; ssrc < 32; ssrc++) { //�j�M�L���W 32 �Ӧ�m


			if (curPiece[15] & (1 << ssrc) && ch & (1 << ssrc) && depth <= noReDepth) { //�Y����½�l �b��½�l���B�n�� depth<=noReDepth 
				if (depth == 0)
				{
					int r = rand() % 6;;
					string a[6] = { "��3��f","(--;)f","(���s��)f","(��-_-`)f","|�s���^f","(*���J��)f" };
					cout << a[r] + ".";
				}
				int a = 0;
				int tempweight = 0;
				weights.push_back(ssrc);
				weightd.push_back(ssrc);
#pragma omp parallel  for default(none) firstprivate(curPiece) reduction(+:tempweight,a)     
				for (int pID = 0; pID < 14; pID++) { //�j�M�i��|½�X���l
					if (lDCount[pID]) { //�Y�ӧL�إi��Q½�X
						a += lDCount[pID];
						int deeper = depth + 1;
						unsigned int c = 1 << ssrc;
						int cpID = pID + 1;

						curPiece[cpID] |= c;//�����ӧL��½�X��
						curPiece[15] ^= c;

						vector<unsigned int> tcurPiece;
						vector<int> tcurPie;
						vector<int> tlDCount;
						tcurPiece.assign(curPiece.begin(), curPiece.end());
						tcurPie.assign(curPie.begin(), curPie.end());
						tlDCount.assign(lDCount.begin(), lDCount.end());

						int ttempweight = 0;

						ttempweight = ((lDCount[pID]) * search(deeper, tcurPiece, tcurPie, -999999, 999999, tlDCount, hashvalue ^ randtable[pID][ssrc] ^ randtable[14][ssrc]));
						tempweight += ttempweight;

						curPiece[cpID] ^= c;//�N����½�X���l�_��
						curPiece[15] |= c;
					}
				}
				//if (weights.back() > 32)cout << weights.back() << endl;
				//if (weightd.back() > 32)cout << weightd.back() << endl;

				tempweight /= a;
				weight.push_back(tempweight);

				if (depth % 2 == 0)//max
				{
					if (weight[wp] > alpha)
					{
						alpha = weight[wp];
					}
					if (weight[wp] > best)
					{
						best = weight[wp];
					}
				}
				else//min
				{
					if (weight[wp] < beta)
					{
						beta = weight[wp];
					}
					if (weight[wp] < best)
					{
						best = weight[wp];
					}
				}
				if (beta <= alpha)
				{
					//hashtable[hashindex].depth = depth;
					//hashtable[hashindex].MaxDepth = maxDepth;
					if (depth % 2 == 0) {//max
						//hashtable[hashindex].count = alpha;
						return alpha;
					}
					else {//min
						//hashtable[hashindex].count = beta;
						return beta;
					}
				}
				wp++;
			}
		}
	}
	if (curPiece[15] != 0 && depth > noReDepth)//�i�H���ŨB
	{
		vector<unsigned int> tcurPiece;
		vector<int> tcurPie;
		vector<int> tlDCount;
		tcurPiece.assign(curPiece.begin(), curPiece.end());
		tcurPie.assign(curPie.begin(), curPie.end());
		tlDCount.assign(lDCount.begin(), lDCount.end());
		weights.push_back(0);
		weightd.push_back(0);
		int tempweight = 0;
		tempweight = search(depth + 1, tcurPiece, tcurPie, alpha, beta, tlDCount, hashvalue);
		weight.push_back(tempweight);
		wp++;
	}

	/*for (int i = 0; i < 32; i++) {
		if (weightU[wp][0] > 32)cout << weightU[wp][0] << endl;
	}*/

	if (depth == 0)//root
	{
		string src, dst;
		int srci, dsti;
		if (TimeOut == 1) {
			return 0;
		}
		cout << endl << "------------------------------------" << endl;
		int recordi = 0;
		for (int i = wp - 1; i >= 0; i--)
		{
			if (depth == 0)
			{
				cout << weights[i] << " " << weightd[i] << " " << weight[i] << endl;
			}
			if (weight[i] == best)
			{
				recordi = i;
				srci = weights[i];
				dsti = weightd[i];
			}
		}

		if (draw == 1)
		{
			int loseOne = 0;
			unsigned int c1 = 1 << srci;
			for (int ii = 1; ii < 15; ii++) {//��L��
				unsigned int check = curPiece[ii] & c1;
				if (check != 0) {
					if (ii < 8) {
						loseOne = Power[curPie[7]][curPie[8]][curPie[9]][curPie[10]][curPie[11]][curPie[12]][curPie[13]][ii - 1];
					}
					else {
						loseOne = Power[curPie[0]][curPie[1]][curPie[2]][curPie[3]][curPie[4]][curPie[5]][curPie[6]][ii - 8];
					}
					break;
				}
			}

			if (best - loseOne < 0); //�i��� �G�N����? 
			else if (srci == past_walk[1][1] && dsti == past_walk[1][0])
			{
				cout << "draw denied" << endl;
				weight[recordi] -= 999999;//�i��|Ĺ ��ܤ�����? 
				best = weight[0];

				for (int ii = wp - 1; ii >= 0; ii--)//���s�M�� 
				{
					if (weight[ii] > best)
					{
						best = weight[ii];
						srci = weights[ii];
						dsti = weightd[ii];
					}
				}
			}
		}
		IndexToBoard(srci, dsti, src, dst);
		createMovetxt(src, dst, srci, dsti);
	}
	if (wp == 0) best = countAva(curPie, depth, curPiece, tAEMi, tAOMi, tEAEMi, tEAOMi);
	if (best == 9999999 || best == -9999999) best = countAva(curPie, depth, curPiece, tAEMi, tAOMi, tEAEMi, tEAOMi);
	/*hashtable[hashindex].count = best;
	hashtable[hashindex].depth = depth;
	hashtable[hashindex].MaxDepth = maxDepth;*/
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

/*void dynamicPower(int curPie[14])
{
	piPw[0] = 6000 + curPie[7] * 600 + curPie[8] * 270 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200;
	piPw[1] = 2700 + curPie[8] * 270 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[2] = 900 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[3] = 400 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[4] = 150 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[5] = 2000 + curPie[7] * 600 + curPie[8] * 270 + curPie[9] * 90 + curPie[10] * 40 + curPie[11] * 15 + curPie[12] * 200 + curPie[13] * 10;
	piPw[6] = 100 + curPie[7] * 600 + curPie[13] * 10;

	piPw[7] = 6000 + curPie[0] * 600 + curPie[1] * 270 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200;//7830
	piPw[8] = 2700 + curPie[1] * 270 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//3980
	piPw[9] = 900 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//1640
	piPw[10] = 400 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//960
	piPw[11] = 150 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//630
	piPw[12] = 2000 + curPie[0] * 600 + curPie[1] * 270 + curPie[2] * 90 + curPie[3] * 40 + curPie[4] * 15 + curPie[5] * 200 + curPie[6] * 10;//3880
	piPw[13] = 100 + curPie[0] * 600 + curPie[6] * 10;//750
	//total max 33760
}*/

int findPiece(int place, unsigned int curPiece[16])
{
	unsigned int bplace = 1 << place;
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
			break;
		}
		if (startmove == 1) {
			if (str.length() >= 10 && str.at(2) != 't' && str.length() <= 15) {
				//cout << str.substr(str.length() - 5, 5)<<"\n";
				if (str.at(str.length() - 1) == ' ') {
					move.push_back(str.substr(str.length() - 6, 5));
				}
				else {
					move.push_back(str.substr(str.length() - 5, 5));//a1(k),a1-a2	
				}
				first = false;
			}
			if (str.length() >= 16) {
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
		//cout<<i<<move[i].at(0)<<move[i].at(1)<<move[i].at(2)<<endl;
		if (move[i].at(2) == '(') {//record move 'flip' (kgmrncp)
			//string a=move[i].substr(0,2);
			int aa = 100 - move[i].at(0);
			int bb = 4 * (56 - move[i].at(1));
			unsigned int cc = aa + bb;//�ѽL�s��0~31
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
			unsigned int c1 = aa + bb;//�ѽL�s��0~31
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
			unsigned int c2 = aaa + bbb;//�ѽL�s��0~31
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
				unsigned int check = piece[ii] & c1;
				if (check != 0) {
					c1p = ii;
					break;
				}
			}
			for (int ii = 1; ii < 15; ii++) {//��c2 �M�� �Ѿl�Ѥl�Ƨ�� 
				unsigned int check = piece[ii] & c2;
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

void createMovetxt(string src, string dst, int srci, int dsti)//�I���Ҧ� 
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
	uniform_int_distribution<unsigned int> dis(pow(2, 31), pow(2, 32) - 1);

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
	score score;
	score.dynamicPower(Power);
	score.CreateDistanceP(DistanceP);
}

void IndexToBoard(int indexa, int indexb, string& src, string& dst)
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

unsigned int myhash(unsigned int tpiece[16]) {
	unsigned int hashvalue = 0;
	for (int i = 1; i < 16; i++) { //1~14 ������L�� 0�Ů� 15 ��½
		unsigned int p = tpiece[i]; //���o�Ѥl��m
		while (p) { //�N1~15 �����l���j�M�@�M
			unsigned int mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
			p ^= mask; //���h���̧C�줸���ӧL��
			hashvalue ^= randtable[i - 1][GetIndex(mask)]; //�N�̧C�줸���L�ئ�m���H����xor hashtable
		}
	}
	return hashvalue;
}