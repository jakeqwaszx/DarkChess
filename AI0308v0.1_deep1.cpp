#include <iostream>
#include <cstdlib> /* �üƬ������ */
#include <ctime>
#include <fstream>
#include <vector>
#include "memory.h"
#include <time.h>
#include <string>
#include <string.h>
#include <sstream>
#include <math.h>

typedef unsigned int U32;
U32 LS1B(U32 x){ return x&(-x);}//���ox���̧C�줸
U32 MS1B(U32 x){ // Most Significant 1 Bit (LS1B)�禡
	x |= x >> 32; x |= x >> 16; x |= x >> 8;
	x |= x >> 4; x |= x >> 2; x |= x >> 1;
	return (x >> 1) + 1; //�i�H���o�@��줸���̥��䤣���s���줸
}
U32 CGen(int ssrc);//���٬��� 
U32 CGenCR(U32 x);
U32 CGenCL(U32 x);
int BitsHash(U32 x){return (x * 0x08ED2BE6) >> 27;}
int initial();//��l�� 
int openchess();//½�� 
int chess(U32 tblack,U32 tred,U32 tpiece[16]);//�M��i�β���
int ai();//�M�w���  �H��? 
int ai2();//ai ver2.0 �g �ϥ�search 
int readBoard();//Ū�ɼҦ� Ū��board.txt ��Ū�J�ɮ��নbitboard �٨S�˵ۦs�J 
int createMovetxt();//�гymove.txt 0���B 1½�� 
int IndexToBoard(int indexa,int indexb);//��src dst�q�s��0~31->�ѽL�s��a1~d4 
int countAva(int pie[14]);//�I�s�h�Ǧ^��e�Ѫ� �ڤ貾��-�Ĥ貾�� 
void Reveal(int turn);
void SimReveal(int ssrc,int pID,U32 curPiece[16],U32 curRed,U32 curBlack);//����½�X���ssrc��pID 
void UnSimReveal(int ssrc,int pID,U32 curPiece[16],U32 curRed,U32 curBlack);//undo simReveal
int* SimMove(int ssrc,int sdst,U32* curPiece,U32* curRed,U32* curBlack,U32* curOccupied,int* curPie);
int* UnSimMove(int ssrc,int sdst,U32* curPiece,U32* curRed,U32* curBlack,U32* curOccupied,int* curPie);
int search(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14]);
void Search();
using namespace std;

int index32[32] = {31, 0, 1, 5, 2, 16, 27, 6, 3, 14, 17, 19, 28, 11, 7, 21, 30, 4, 15, 26, 13,
18, 10, 20, 29, 25, 12, 9, 24, 8, 23, 22};
int GetIndex(U32 mask){ return index32[BitsHash(mask)];}//��J�B�n��^�ѽL�s��
U32 pMoves[32]={0x00000012,0x00000025,0x0000004A,0x00000084,0x00000121,0x00000252,0x000004A4,0x00000848,
0x00001210,0x00002520,0x00004A40,0x00008480,0x00012100,0x00025200,0x0004A400,0x00084800,
0x00121000,0x00252000,0x004A4000,0x00848000,0x01210000,0x02520000,0x04A40000,0x08480000,
0x12100000,0x25200000,0x4A400000,0x84800000,0x21000000,0x52000000,0xA4000000,0x48000000};//�X�l���ʾB�n
U32 file[4]={0x11111111,0x22222222,0x44444444,0x88888888};//��B�n 
U32 rank[8]={0x0000000F,0x000000F0,0x00000F00,0x0000F000,0x000F0000,0x00F00000,0x0F000000,0xF0000000};//�C�B�n 
U32 piece[16]; //0�Ů�- ��k �hg ��m ��r ��n ��c �Lp *2 15��½x 
U32 red,black,occupied;//�� �� ���Ѥl 

string move="a1-a1";//�U�@�B��� �Ω�I�� 
int piece_count[14]={1,2,2,2,2,2,5,1,2,2,2,2,2,5};//�Ѿl�Ѥl�� 0-6 7-13
int DCount[14]={1,2,2,2,2,2,5,1,2,2,2,2,2,5};//�Ѿl��½�l 
string current_position[32];//�L�����p�`�� 
string history; 
int timeCount;//�Ѿl�ɶ� 
int initailBoard=1;//�O�_Ū����l���� 
//int RMcount=13;//Ū���Ҧ��ݭn �q13��}�lŪ�����G 

U32 allEatMove[1000][2];//�s�i�Y�l����k 0 src 1 dst
int AEMindex=0;//alleatmove index
U32 allOnlyMove[1000][2];//�s�i���ʫD�Y�l����k 0 src 1 dst
int AOMindex=0;//allonlymove index
U32 EallEatMove[1000][2];//�s���i�Y�l����k 0 src 1 dst
int EAEMindex=0;//Ealleatmove index
U32 EallOnlyMove[1000][2];//�s���i���ʫD�Y�l����k 0 src 1 dst
int EAOMindex=0;//Eallonlymove index

bool color;//0 red 1 black
string src,dst;//�ѽL�s���� a1~d4
int srci,dsti;//index�� 0~31
int maxDepth=1;
int revealWeight[32];

int main()
{
	srand(time(NULL));
	initial();//��l�� 
	readBoard();
	int end=1;//�����h=0
	IndexToBoard(0,31);
	while(end)
	{
		dst="0";
		ai2();//�M�w��� 
		createMovetxt();
		end=0;
	}
} 

int ai2()
{
	search(0,piece,red,black,occupied,piece_count);
	IndexToBoard(srci,dsti);
}

int ai()
{
	chess(black,red,piece);//��X�Ҧ��ħڦ�� 
	if(AEMindex!=0)//��Y���Y �H��
	{
		int a=rand()%AEMindex;
		srci=allEatMove[a][0];
		dsti=allEatMove[a][1];
	}
	else if(piece[15]!=0)
	{
		int run[100];
		int runi=0;
		int eat[100];
		int eati=0;
		if(EAEMindex!=0)
		{
			for(int i=0;i<EAEMindex;i++)//�ݬO�_�Y�N�Q�Y 
			{
				for(int ii=0;ii<AEMindex;ii++)
				{
					if(EallEatMove[i][1]==allEatMove[ii][0])//�Y�N���k�]
					{
						eat[eati]=ii;
						eati++;
					}
				}
				for(int ii=0;ii<AOMindex;ii++)
				{
					if(EallEatMove[i][1]==allOnlyMove[ii][0])//�k�]
					{
						run[runi]=ii;
						runi++;
					}
				}
			}
			if(eati!=0)
			{
				int r=rand()%eati;
				srci=allEatMove[eat[r]][0];
				dsti=allEatMove[eat[r]][1];
				eat[r];
			}
			else if(runi!=0)
			{
				int r=rand()%runi;
				srci=allOnlyMove[run[r]][0];
				dsti=allOnlyMove[run[r]][1];
				run[r];
			}
			else
			{
				openchess();
			}
		}
		else
			openchess();
	}
	else
	{
		int run[100];
		int runi=0;
		int eat[100];
		int eati=0;
		if(EAEMindex!=0)
		{
			for(int i=0;i<EAEMindex;i++)//�ݬO�_�Y�N�Q�Y 
			{
				for(int ii=0;ii<AEMindex;ii++)
				{
					if(EallEatMove[i][1]==allEatMove[ii][0])//�Y�N���k�]
					{
						eat[eati]=ii;
						eati++;
					}
				}
				for(int ii=0;ii<AOMindex;ii++)
				{
					if(EallEatMove[i][1]==allOnlyMove[ii][0])//�k�]
					{
						run[runi]=ii;
						runi++;
					}
				}
			}
			if(eati!=0)
			{
				int r=rand()%eati;
				srci=allEatMove[eat[r]][0];
				dsti=allEatMove[eat[r]][1];
				eat[r];
			}
			else if(runi!=0)
			{
				int r=rand()%runi;
				srci=allOnlyMove[run[r]][0];
				dsti=allOnlyMove[run[r]][1];
				run[r];
			}
			else
			{
				int a=rand()%AOMindex;
				srci=allOnlyMove[a][0];
				dsti=allOnlyMove[a][1];
			}
		}
		else
		{
			int a=rand()%AOMindex;
			srci=allOnlyMove[a][0];
			dsti=allOnlyMove[a][1];
		}
	}
	if(srci!=dsti)
	{
		IndexToBoard(srci,dsti);
	}
}

int chess(U32 tblack,U32 tred,U32 tpiece[16])
{
	U32 dest;//�i�H�Y�l�����
	AEMindex=0;
	AOMindex=0;
	EAEMindex=0;
	EAOMindex=0;
	int ssrc=0;
	//cout<<"-------------------------"<<endl;
	if(!color){//�� 
		//cout<<"Ours available eat:"<<endl;
		for(int i=1; i<8; i++){ //1~7 ����~�L,src ���Ѥl�_�I,dest �����I�C  ����ڤ� 
			U32 p = tpiece[i]; //���o�Ѥl��m
			while(p){ //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				if(i==1) //��,�P���(14)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & ( tblack ^ tpiece[14] );
				else if(i==2) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] );
				else if(i==3) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] ^ tpiece[9] );
				else if(i==4) //?,�u��Y��(11)�B���B���B��C
					dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14] );	
				else if(i==5) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14] );
				else if(i==6) //��,�S��B�z�C
					dest = CGen(ssrc) & tblack;
				else if(i==7) //�L,�u��Y�N(8)�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
				dest = 0;
					while(dest){ //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
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
		for(int i=1; i<8; i++){ //����²���
			U32 p = tpiece[i]; //���o�Ѥl��m
			while(p){ //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
					dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while(dest){ //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
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
		for(int i=8; i<15; i++){ //�A������ 
			U32 p = tpiece[i]; 
			while(p){ 
				U32 mask = LS1B(p); 
				p ^= mask; 
				ssrc = GetIndex(mask); 
				if(i==8) 
					dest = pMoves[ssrc] & ( tred ^ tpiece[7] );
				else if(i==9) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] );
				else if(i==10) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] ^ tpiece[2] );
				else if(i==11) //?,�u��Y��(11)�B���B���B��C
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7] );	
				else if(i==12) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7] );
				else if(i==13) //��,�S��B�z�C
					dest = CGen(ssrc) & tred;
				else if(i==14) //�L,�u��Y�N�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
				dest = 0;
					while(dest){ //����ʦs�JEallEatMove 
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
		for(int i=8; i<15; i++){ //���²��� 
			U32 p = tpiece[i]; //���o�Ѥl��m
			while(p){ //�N�¦� 8~14 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
					dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while(dest){ //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
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
	else{//�¤貾�� 
		//cout<<"Our available eat:"<<endl;
		for(int i=8; i<15; i++){ //1~7 ����~�L,src ���Ѥl�_�I,dest �����I�C
			U32 p = tpiece[i]; //���o�Ѥl��m
			while(p){ //�N�¦� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				if(i==8) //��,�P���(14)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & ( tred ^ tpiece[7] );
				else if(i==9) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] );
				else if(i==10) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & ( tred ^ tpiece[1] ^ tpiece[2] );
				else if(i==11) //?,�u��Y��(11)�B���B���B��C
					dest = pMoves[ssrc] & (tpiece[4] | tpiece[5] | tpiece[6] | tpiece[7] );	
				else if(i==12) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[5] | tpiece[6] | tpiece[7] );
				else if(i==13) //��,�S��B�z�C
					dest = CGen(ssrc) & tred;
				else if(i==14) //�L,�u��Y�N�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[1] | tpiece[7]);
				else
				dest = 0;
					while(dest){ //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
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
		for(int i=8; i<15; i++){ //�¤�²���
			U32 p = tpiece[i]; //���o�Ѥl��m
			while(p){ //�N�¦� 8~14 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
					dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while(dest){ //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
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
		for(int i=1; i<8; i++){ //�p��Ĥ貾�� 
			U32 p = tpiece[i]; //���o�Ѥl��m
			while(p){ //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
				if(i==1) //��,�P���(14)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & ( tblack ^ tpiece[14] );
				else if(i==2) //�K,�P��N(8)�H�~���¤l���i�H�Y�C
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] );
				else if(i==3) //��,�P��N�B�h�H�~���¤l���i�H�Y�C	
					dest = pMoves[ssrc] & ( tblack ^ tpiece[8] ^ tpiece[9] );
				else if(i==4) //?,�u��Y��(11)�B���B���B��C
						dest = pMoves[ssrc] & (tpiece[11] | tpiece[12] | tpiece[13] | tpiece[14] );	
				else if(i==5) //�X,�u��Y��(12)�B���B��C
					dest = pMoves[ssrc] & (tpiece[12] | tpiece[13] | tpiece[14] );
				else if(i==6) //��,�S��B�z�C
					dest = CGen(ssrc) & tblack;
				else if(i==7) //�L,�u��Y�N(8)�B��(14)�C
					dest = pMoves[ssrc] & (tpiece[8] | tpiece[14]);
				else
				dest = 0;
					while(dest){ //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
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
		for(int i=1; i<8; i++){ //����²���
			U32 p = tpiece[i]; //���o�Ѥl��m
			while(p){ //�N���� 1~7 �����l���j�M�@�M
				U32 mask = LS1B(p); //�p�G�ӴѤl�b�h�Ӧ�m,�����C�줸����m�C
				p ^= mask; //���h���̧C�줸���ӧL��
				ssrc = GetIndex(mask); //�N�̧C�줸���L�س]�����B�_�I
					dest = pMoves[ssrc] & tpiece[0];//�u���Ů�
				while(dest){ //�p�G dest ���h�Ӧ�m����,���}�s�_�ӡC
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

U32 CGen(int ssrc)
{
	int r=ssrc/4;//�C 
	int c=ssrc%4;//�� 
	U32 result=0;
	U32 resulta=0;
	U32 x = ( (rank[r] & occupied) ^ (1<<ssrc) ) >> (4*r);//���X�򬶦P�C �d�h����������
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
	
	x = ( (file[c] & occupied) ^ (1<<ssrc) ) >> c;//���X�򬶦P�� �å�������1��
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
		U32 mask = LS1B(x); //mask �����[���B�n��m,�� x ���h���[�C
		return (x ^= mask) ? LS1B(x) : 0; //���p 5~8 �Ǧ^ LS1B(x),���p 2~4 �Ǧ^ 0�C
	}else return 0;
}

U32 CGenCR(U32 x){
	if(x){
		U32 mask = MS1B(x); //mask �����[���B�n��m,�� x ���h���[�C
		return (x ^= mask) ? MS1B(x) : 0; //���p 5~8 �Ǧ^ MS1B(x),���p 2~4 �Ǧ^ 0�C
	}else return 0;
}

int countAva(int pie[14])//�N�h�ۨ������L 
{
	int piPw[14]={6000,2700,900,400,150,2000,100,6000,2700,900,400,150,2000,100};
	int power=0;
	if(!color)//�� 
	{
		//if(pie[12]==0&&pie[13]==0) piPw[4]=0;
		//if(pie[11]==0&&pie[12]==0&&pie[13]==0) piPw[3]=0;
		//if(pie[10]==0&&pie[11]==0&&pie[12]==0&&pie[13]==0) piPw[2]=0;
		//if(pie[9]==0&&pie[10]==0&&pie[11]==0&&pie[12]==0&&pie[13]==0) piPw[1]=0;
		for(int i=0;i<7;i++)
			power+=pie[i]*piPw[i];
		for(int i=7;i<14;i++)
			power-=pie[i]*piPw[i];
	}
	else
	{
		//if(pie[5]==0&&pie[6]==0) piPw[4]=0;
		//if(pie[4]==0&&pie[5]==0&&pie[6]==0) piPw[3]=0;
		//if(pie[3]==0&&pie[4]==0&&pie[5]==0&&pie[6]==0) piPw[2]=0;
		//if(pie[2]==0&&pie[3]==0&&pie[4]==0&&pie[5]==0&&pie[6]==0) piPw[1]=0;
		for(int i=7;i<14;i++)
			power+=pie[i]*piPw[i];
		for(int i=0;i<7;i++)
			power-=pie[i]*piPw[i];
	}
	int movePoint=AEMindex*10+AOMindex-EAEMindex*10-EAOMindex;
	//cout<<AEMindex<<" "<<AOMindex<<" "<<EAEMindex<<" "<<EAOMindex<<endl;
	//cout<<power;
	return movePoint+power;
}

int openchess()
{
	for(int i=0;i<32;i++)
	{
		U32 s1=1<<i&piece[15];
		if(s1!=0)
		{
		}
	}
	stringstream ss;
	U32 a=rand()%32;//0~31
	U32 aa=1<<a;
	U32 b=aa&piece[15];//�T�{�O�_��½
	while(b==0)//�S��쥼½�� �B �٦���½�� 
	{
		a=rand()%32;//0~31
		aa=1<<a;
		b=aa&piece[15];
	}
	string aaa;
	U32 a1=a%4;
	U32 a2=a/4;
	if(a1==3) aaa="a";
	else if(a1==2) aaa="b";
	else if(a1==1) aaa="c";
	else if(a1==0) aaa="d";
	ss << 8-a2;
	src=aaa+ss.str();
	dst=src;
	//cout<<aa+ss.str()<<endl;
	//cout<<hex<<d<<" ";
}

int search(int depth,U32 curPiece[16],U32 curRed,U32 curBlack,U32 curOccupied,int curPie[14])
{
	chess(curBlack,curRed,curPiece);
	U32 taEM[1000][2];//�s�i�Y�l����k 0 src 1 dst
	int tAEMi=0;//alleatmove index
	U32 taOM[1000][2];//�s�i���ʫD�Y�l����k 0 src 1 dst
	int tAOMi=0;//allonlymove index
	memcpy(taEM,allEatMove,sizeof(taEM));
	tAEMi=AEMindex;
	memcpy(taOM,allOnlyMove,sizeof(taOM));
	tAOMi=AOMindex;
	
	if(depth==maxDepth)
	{
		return countAva(curPie);
	}
	int weight[1000][3];//�p��Ҧ����ʻP½�Ѫ��o��0src 1dst 2weight 
	int wp=0;
	int bestsrc;
	int bestdst;
	int count=0;
	if(piece[15]!=0)//����½�� ������call search 
	{
		for(int ssrc=0; ssrc<32; ssrc++){ //�j�M�L���W 32 �Ӧ�m
			if(curPiece[15] & ( 1 << ssrc )){ //�Y����½�l
				weight[wp][3]=0;
				//cout<<endl;
				int a=0;
				for(int pID=0; pID<14; pID++){ //�j�M�i��|½�X���l
					if(DCount[pID]){ //�Y�ӧL�إi��Q½�X
						a+=DCount[pID];
						int deeper=depth+1;
						U32 c=1<<ssrc;
						int cpID=pID+1;
						if(pID<8)
						{
							curPiece[cpID]|=c;
							curRed|=c;
							curPiece[15]^=c;
							DCount[pID]--;
						}
						else
						{
							curPiece[cpID]|=c;
							curBlack|=c;
							curPiece[15]^=c;
							DCount[pID]--;
						}//�����ӧL��½�X��
						//cout<<pID<<" ";
						weight[wp][0] =ssrc;
						weight[wp][1] =ssrc;
						weight[wp][2] += (DCount[pID]+1)*search(deeper,curPiece,curRed,curBlack,curOccupied,curPie);
						if(pID<8)
						{
							curPiece[cpID]^=c;
							curRed^=c;
							curPiece[15]|=c;
							DCount[pID]++;
						}
						else
						{
							curPiece[cpID]^=c;
							curBlack^=c;
							curPiece[15]|=c;
							DCount[pID]++;
						}//�N����½�X���l�_��
					}
				}
				weight[wp][2]/=a;
				wp++;
			}
			else{
				revealWeight[ssrc] = -999999;
			}
		}
	}
	if(tAEMi+tAOMi!=0)//�A�ղ��� 
	{
		if(tAEMi>0)
		for(int i=0;i<tAEMi;i++)
		{
			int deeper=depth+1;
			SimMove(taEM[i][0],taEM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
			weight[wp][0] =taEM[i][0];
			weight[wp][1] =taEM[i][1];
			weight[wp][2] =search(deeper,curPiece,curRed,curBlack,curOccupied,curPie);
			UnSimMove(taEM[i][0],taEM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
			wp++;
		}
		for(int i=0;i<tAOMi;i++)
		{
			int deeper=depth+1;
			SimMove(taOM[i][0],taOM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
			weight[wp][0] =taOM[i][0];
			weight[wp][1] =taOM[i][1];
			weight[wp][2] =search(deeper,curPiece,curRed,curBlack,curOccupied,curPie);
			UnSimMove(taOM[i][0],taOM[i][1],curPiece,&curRed,&curBlack,&curOccupied,curPie);
			wp++;
		}
	}
	int best=-999999;
	if(depth%2==0)//max
	{
		for(int i=0;i<wp;i++)
		{
			cout<<weight[i][0]<<" "<<weight[i][1]<<" "<<weight[i][2]<<endl;
			if(weight[i][2]>best)
			{
				bestsrc=weight[i][0];
				bestdst=weight[i][1];
				best=weight[i][2];
			}
		}
	}
	else//min
	{
		best=999999;
		for(int i=0;i<wp;i++)
		{
			if(weight[i][2]<best)
			{
				bestsrc=weight[i][0];
				bestdst=weight[i][1];
				best=weight[i][2];
			}
		}
	}
	
	if(depth==0)
	{
		srci=bestsrc;
		dsti=bestdst;
	}
	else
	{
		return best;
	}
}

void Reveal(int turn)//�ѼƬ����֤U,�@�謰 0,�t�@�謰 1�C
{
	int maxWeight = -99999999, maxReveal = 0;
	//�`�@�|���X�ؤl�Q½�X��(DCountSum),�ثe���X����½�l(dCounter)�C
	int DCountSum = 0, dCounter = 0;
	U32 dark = piece[15];
	while(dark){ //���p��L���W���X����½�l
		U32 mask = LS1B(dark);
		dark ^= mask;
		dCounter++;
	}
	if(dCounter >= 31){ //�̷ӭӤH�g��,�Ĥ@��½�o�|�Ӧ�m���n�C
		revealWeight[9] = 10;
		revealWeight[10] = 10;
		revealWeight[21] = 10;
		revealWeight[22] = 10;
	}
	
	turn = 1 - turn; //�洫�^�X
	//int factor = (turn == color) ? 1 : -1;
	//for(int pID=1; pID<15; pID++){
	//	if(DCount[pID])
	//		DCountSum += DCount[pID];
	//}
	
	for(int ssrc=0; ssrc<32; ssrc++){ //�j�M�L���W 32 �Ӧ�m
		if(piece[15] & ( 1 << ssrc )){ //�Y����½�l
			for(int pID=1; pID<15; pID++){ //�j�M�i��|½�X���l
				if(piece[pID]){ //�Y�ӧL�إi��Q½�X
					//SimReveal(ssrc, pID); //�����ӧL��½�X��
					//revealWeight[ssrc] += -( Search(turn) );
					//UnSimReveal(ssrc, pID); //�N����½�X���l�_��
				}
			}
		}
		else{
			revealWeight[ssrc] = -99999999;
		}
	}

	for(int ssrc=0; ssrc<32; ssrc++){ //��X�v���̤j�����Ӧ�m
		if(maxWeight < revealWeight[ssrc]){
			maxWeight = revealWeight[ssrc];
			maxReveal = ssrc;
		}
	}
		turn = 1 - turn;; //���F�O�I�_��,�N�^�X���^�ӡC
		srci = maxReveal; //½�_���Ƴ̰�������
}

int Search(int turn)//½�ѫ�u�j�M���ʪ��i�� 
{
	
}
int c2pcopy;
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
	for(int ii=1;ii<15;ii++){//��c2 �M�� �Ѿl�Ѥl�Ƨ�� 
		U32 check=curPiece[ii]&c2;
		if(check!=0){
			c2p=ii;
		}
	}
	cout<<c1p<<" "<<sdst<<" ";
	c2pcopy=c2p;
	curPiece[c1p]^=c1;//�M�����mc1
	curPiece[0]|=c1;//�Ů�+c1
	curPiece[c1p]|=c2;//����
	if(c2p!=-1){//�Y�l���� 
		curPiece[c2p]^=c2;//�M�����mc2
		int c2ps=c2p-1;
		curPie[c2ps]--;
		if(8>c1p){//���Y��
			*curRed^=c1;
			*curBlack^=c2;
			*curRed|=c2;
		}
		else{//�¦Y�� 
			*curBlack^=c1;
			*curRed^=c2;
			*curBlack|=c2;
		}
		*curOccupied^=c1;//c1�L�l 
	}
	else//�L�Y�l���� 
	{
		if(8>c1p){//���� 
			*curRed^=c1;
			*curRed|=c2;
		}
		else{//�°� 
			*curBlack^=c1;
			*curBlack|=c2;
		}
		curPiece[0]^=c2;//�Ů�-c2
		*curOccupied^=c1;//c1�L�l 
		*curOccupied|=c2;//c2���l 
	}
}

int* UnSimMove(int ssrc,int sdst,U32* curPiece,U32* curRed,U32* curBlack,U32* curOccupied,int* curPie)
{
	int c1p,c2p;
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
	c2p=c2pcopy;
	curPiece[c1p]|=c1;//�M�����mc1
	curPiece[0]^=c1;//�Ů�+c1
	curPiece[c1p]^=c2;//����
	if(c2p!=0){//�Y�l���� 
		curPiece[c2p]|=c2;//�M�����mc2
		int c2ps=c2p-1;
		curPie[c2ps]++;
		if(8>c1p){//���Y��
			*curRed|=c1;
			*curBlack|=c2;
			*curRed^=c2;
		}
		else{//�¦Y�� 
			*curBlack|=c1;
			*curRed|=c2;
			*curBlack^=c2;
		}
		*curOccupied|=c1;//c1�L�l 
	}
	else//�L�Y�l���� 
	{
		if(8>c1p){//���� 
			*curRed|=c1;
			*curRed^=c2;
		}
		else{//�°� 
			*curBlack|=c1;
			*curBlack^=c2;
		}
		curPiece[0]|=c2;//�Ů�-c2
		*curOccupied|=c1;//c1�L�l 
		*curOccupied^=c2;//c2���l 
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

int readBoard()//Ū�ɼҦ� Ū��board.txt�M�� 
{
	vector<string> move;
	int cp=0;//current_position�p���} 
	int line=0;//�ثeŪ�쪺��� 
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
				piece_count[i]=str[i*2+2]-'0';//��Ѿl�ѤlŪ�Jpiece_count 
			}
		}
		if(line>=3&&line<=10&&initailBoard==1)
		{
			for(int i=0;i<=3;i++)
			{
				current_position[cp]=str[i*2+2];//Ū�J��l���W���p �uŪ�@�� 
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
	if (first && !move.empty()) {//���� �B�z�ڤ��C�� 
		if (move[0].at(3) - 91 > 0)//�p�g
			color = 1;//��
		else
			color = 0;//��
	}
	else if(!move.empty()){//���
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
			int aa=100-move[i].at(0);
			int bb=4*(56-move[i].at(1));
			U32 cc=aa+bb;//�ѽL�s��0~31
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
		else {//a1-b1���� 
			int c1p,c2p;
			c2p=-1;//�p�G�O-1������ 
			int aa=100-move[i].at(0);
			int bb=4*(56-move[i].at(1));
			U32 c1=aa+bb;//�ѽL�s��0~31
			c1=1<<c1;
			//cout<<hex<<c1<<"c1 "; 
			
			int aaa=100-move[i].at(3);
			int bbb=4*(56-move[i].at(4));
			U32 c2=aaa+bbb;//�ѽL�s��0~31
			c2=1<<c2;
			//cout<<hex<<c2; 
			for(int ii=1;ii<15;ii++){
				U32 check=piece[ii]&c1;
				if(check!=0){
					c1p=ii;
					break;
				}
			}
			for(int ii=1;ii<15;ii++){//��c2 �M�� �Ѿl�Ѥl�Ƨ�� 
				U32 check=piece[ii]&c2;
				if(check!=0){
					c2p=ii;
				}
			}
			piece[c1p]^=c1;//�M�����mc1
			piece[0]|=c1;//�Ů�+c1
			piece[c1p]|=c2;//����
			if(c2p!=-1){//�Y�l���� 
				piece[c2p]^=c2;//�M�����mc2
				int c2ps=c2p-1;
				piece_count[c2ps]--;
				if(8>c1p){//���Y��
					red^=c1;
					black^=c2;
					red|=c2;
				}
				else{//�¦Y�� 
					black^=c1;
					red^=c2;
					black|=c2;
				}
				occupied^=c1;//c1�L�l 
			}
			else//�L�Y�l���� 
			{
				if(8>c1p){//���� 
					red^=c1;
					red|=c2;
				}
				else{//�°� 
					black^=c1;
					black|=c2;
				}
				piece[0]^=c2;//�Ů�-c2
				occupied^=c1;//c1�L�l 
				occupied|=c2;//c2���l 
			}
			//board[move[i].at(3) - 'a'][move[i].at(4) - '1'] = board[move[i].at(0) - 'a'][move[i].at(1) - '1'];	//board[a][2]=k, a1-a2
			//board[move[i].at(0) - 'a'][move[i].at(1) - '1'] = '-';												//board[a][1]='-', a1-a2
			//cout << board[move[i].at(3) - 'a'][move[i].at(4) - '1'];
		}
	}
	//cout<<hex<<piece[9];
}

int createMovetxt()//�I���Ҧ� 
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
	cout<<src<<":src "<<dst<<":dst "<<endl;
}

int initial()
{
	for(int i=0;i<=14;i++)
		piece[i]=0;
	occupied=0xFFFFFFFF;
	piece[15]=0xFFFFFFFF;
}

int IndexToBoard(int indexa,int indexb)
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
