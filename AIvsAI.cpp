#include <fstream> 
#include <string> 
#include <vector>
#include <iostream>
#include <math.h>
#include <time.h>
#include <cstdlib>
#include <list>
#include "stdio.h"
#include "windows.h"
using namespace std;


//change here
//*******************************//
int first = 0;// 0=first:creatroom , second:enterroom
int turn = 2;// how many turn to play
int changeorder = 1;// 1:yes, 0:no
//*******************************//

vector<char> flip;
char showboard[4][8];
int color = -1;// 0:create is b,enter is R; 1:create is R,enter is b
string initboard = "* 8:59am March 31, 2019\n* Player1 vs Player2\n* 1 2 2 2 2 2 5 1 2 2 2 2 2 5\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* first 2\n* time 900\n* Comment 0 0\n";
int firstflip = 0;// 1 first time flip(use to determine color);

//this are for creatroom
//*******************************//
int win = 0;
int lose = 0;
int draw = 0;
//*******************************//

int totalturn = turn;

//record win,lose in realtime.txt ex."2019 3 31 14 31 50.txt"
//record move in realtimemove.txt ex."2019 3 31 14 31 50 move.txt"
//*******************************//
time_t now = time(0);
tm *ltm = localtime(&now);
string formattime = to_string(1900 + ltm->tm_year) + " " + to_string(1 + ltm->tm_mon) + " " + to_string(ltm->tm_mday) + " " + to_string(ltm->tm_hour) + " " + to_string(ltm->tm_min) + " " + to_string(ltm->tm_sec);
string winrecord = formattime + ".txt";
string winrecordmove = formattime + " move.txt";
ofstream foutrc(winrecord, ios::ate);
ofstream foutrcm(winrecordmove, ios::ate);
vector<string> recordmove;// use to record all move in one game
//*******************************//

void init() {
	for (int i = 0; i < 8; i++) {// initboard[4][8]
		for (int j = 0; j < 4; j++) {
			showboard[j][i] = 'X';
		}
	}
	flip = { 'K','G','G','M','M','R','R','N','N','C','C','P','P' ,'P' ,'P' ,'P'
				,'k','g','g','m','m','r','r','n','n','c','c','p','p' ,'p' ,'p' ,'p' };
	firstflip = 1;
	color = -1;
	ofstream foutc("CreateRoom\\Search\\board.txt", ios::out);
	ofstream foute("EnterRoom\\Search\\board.txt", ios::out);
	recordmove.clear();
	foutc << initboard;
	foute << initboard;
	foutc.close();
	foute.close();
}

void set_console_color(unsigned short color_index)// control collor
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_index);
}

void printwinrecord() {
	foutrc << totalturn << " ply cycle " << " enterroom win! \n" << " ## Win : " << win << " ,   Lose : " << lose << " ,  Draw : " << draw << " ,  PlayTime : " << totalturn - turn + 1 << "  ##\n";
	foutrcm << "PlayTime: " << totalturn - turn + 1 << endl;
	for (int i = 0; i < recordmove.size(); i++) {
		foutrcm << recordmove[i] << endl;
	}
}

void translate(char a) { //k to �N
	if (a - 96 > 0) {
		set_console_color(1);
	}
	if (a - 64 > 0 && a - 88 < 0) {
		set_console_color(4);
	}
	if (a == 'k') {//�N
		printf("\t�N");
	}
	else if (a == 'g') {//�h
		printf("\t�h");
	}
	else if (a == 'm') {//�H
		printf("\t�H");
	}
	else if (a == 'r') {//��
		printf("\t��");

	}
	else if (a == 'n') {//��
		printf("\t��");

	}
	else if (a == 'c') {//��
		printf("\t��");

	}
	else if (a == 'p') {//��
		printf("\t��");

	}
	else if (a == 'K') {//��
		printf("\t��");

	}
	else if (a == 'G') {//�K
		printf("\t�K");

	}
	else if (a == 'M') {//��
		printf("\t��");

	}
	else if (a == 'R') {//��
		printf("\t��");

	}
	else if (a == 'N') {//�X
		printf("\t�X");

	}
	else if (a == 'C') {//��
		printf("\t��");

	}
	else if (a == 'P') {//�L
		printf("\t�L");

	}
	else if (a == 'X') {
		printf("\tX");

	}
	else if (a == '-') {
		printf("\t-");

	}
	else {
		printf("F");

	}
	set_console_color(7);
}

bool check() {
	cout << "\n";
	if (!first) {
		cout << "creatroom first" << endl;
		if (color) {
			cout << "create is R,enter is b" << endl;
		}
		else {
			cout << "create is b,enter is R" << endl;
		}
	}
	else {
		cout << "enterroom first" << endl;
		if (color) {
			cout << "create is R,enter is b" << endl;
		}
		else {
			cout << "create is b,enter is R" << endl;
		}
	}
	int r = 0;// red chess amount
	int b = 0;// black chess amount
	for (int i = 0; i < 8; i++) {// print board
		cout << 8 - i;
		for (int j = 0; j < 4; j++) {	// top down	a7-b7-c7-d7
										//			a6-b6-c6-d6

			translate(showboard[j][7 - i]);
			if (flip.empty()) {
				if (showboard[j][7 - i] - 96 > 0) {// how many black chess left
					b++;

				}
				if (showboard[j][7 - i] - 64 > 0 && showboard[j][7 - i] - 88 < 0) {// how many red chess left
					r++;

				}
			}
		}
		cout << "\n";
	}
	cout << "\ta\tb\tc\td\t" << endl;
	if (!flip.empty()) {
		return false;
	}
	else if (r == 0) {
		if (color) {
			cout << "enterroom win!" << endl;
			lose++;
			printwinrecord();
		}
		else {
			cout << "creatroom win!" << endl;
			win++;
			printwinrecord();
		}
		return true;
	}
	else if (b == 0) {
		if (color) {
			cout << "creatroom win!" << endl;
			win++;
			printwinrecord();
		}
		else {
			cout << "enterroom win!" << endl;
			lose++;
			printwinrecord();
		}
		return true;
	}
	else {
		return false;
	}
}

bool eat(char a, char b) { //a eat b
	if (a - 97 < 0)a = a + 32;
	if (b - 97 < 0)b = b + 32;

	if (a == 'k') {//�N
		if (b == 'p') {
			return false;
		}
		else return true;
	}
	else if (a == 'g') {//�h
		if (b == 'k') {
			return false;
		}
		else return true;
	}
	else if (a == 'm') {//�H
		if (b == 'k' || b == 'g') {
			return false;
		}
		else return true;
	}
	else if (a == 'r') {//��
		if (b == 'k' || b == 'g' || b == 'm') {
			return false;
		}
		else return true;
	}
	else if (a == 'n') {//��
		if (b == 'c' || b == 'p') {
			return true;
		}
		else return false;
	}
	else if (a == 'c') {
		return false;
	}
	else if (a == 'p') {
		if (b == 'k' || b == 'p') {
			return true;
		}
		else return false;
	}
	else
		return false;
}

bool docreate() {
	vector<string> board;
	string str;
	vector<string> move;// move in move.txt
	vector<string> readmove;// move in board.txt
	system("cd CreateRoom\\Search && search.exe");// excute creatroom's search.exe
	ifstream finc("CreateRoom\\Search\\board.txt", ios::in);
	int startmove = 0;
	while (getline(finc, str)) {// read creatroom's board.txt
		board.push_back(str);
		/*if (str.at(2) == 't') {// start after * time 900
			startmove = 1;
		}
		if (str.at(2) == 'C'&&str.at(3) == 'o') {// stop before * Comment 0 0
			startmove = 0;
		}
		if (startmove == 1) {
			if (str.length() >= 10 && str.at(2) != 't' && str.length() <= 12) {
				readmove.push_back(str.substr(str.length() - 5, 5));// a1(k),a1-a2
			}
			if (str.length() >= 16 && str.length() <= 18) {
				readmove.push_back(str.substr(str.length() - 11, 5));// a1(k),a1-a2
				readmove.push_back(str.substr(str.length() - 5, 5));// a1(k),a1-a2
			}
		}*/
	}

	/*for (int i = 0; i < readmove.size()&& !readmove.empty(); i++) {// record in char board[4][8]
		if (readmove[i].at(2) == '(') {// record move 'flip'
			showboard[readmove[i].at(0) - 'a'][readmove[i].at(1) - '1'] = readmove[i].at(3);										// board[a][1]=k,ai(k)
		}
		else {
			showboard[readmove[i].at(3) - 'a'][readmove[i].at(4) - '1'] = showboard[readmove[i].at(0) - 'a'][readmove[i].at(1) - '1'];	// board[a][2]=k, a1-a2
			showboard[readmove[i].at(0) - 'a'][readmove[i].at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
		}
	}*/

	ifstream fincm("CreateRoom\\Search\\move.txt", ios::in);// read creatroom's move.txt
	int moveline = 0;
	while (getline(fincm, str)) {
		move.push_back(str);
	}

	ofstream foute("EnterRoom\\Search\\board.txt", ios::out);
	for (int i = 0; i < board.size(); i++) {// copy to enterroom's board.txt
		if (first) {
			if (board[i].length() >= 10 && board[i].at(2) != 't' && board[i].length() <= 12) {// plus move made by creatroom's search.exe
				if (move[0].at(0) == '1') {// flip
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == lose
						lose++;
						cout << "creat made wrong flip!";
						printwinrecord();
						return true;
					}
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					flip.erase(flip.begin() + f);
					cout << "creat move: " << move[1] << endl;
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = temp.at(3);
				}
				else {//move
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						lose++;
						cout << "creat made wrong move(take blank)!";
						printwinrecord();
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == lost
						lose++;
						cout << "creat made wrong move(cant eat)!";
						printwinrecord();
						return true;
					}
					if (color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// red but take black
							lose++;
							cout << "creat made wrong move(red but take black)!";
							printwinrecord();
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// black but take red
							lose++;
							cout << "creat made wrong move(black but take red)!";
							printwinrecord();
							return true;
						}
					}
					string temp = move[1] + "-" + move[2];
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					recordmove.push_back(board[i]);
					cout << "creat move: " << move[1] << "-" << move[2] << endl;
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}
			}
		}
		else {
			if (board[i].at(2) == 'C'&&board[i].at(3) == 'o') {// create new line of move before comment
				if (move[0].at(0) == '1') {// flip
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == lose
						lose++;
						cout << "creat made wrong flip!";
						printwinrecord();
						return true;
					}
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					flip.erase(flip.begin() + f);
					cout << "creat move: " << move[1] << endl;
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = temp.at(3);
					if (firstflip) {
						if (temp.at(3) - 96 > 0) {// black chess
							color = 0;
							firstflip = 0;

						}
						else {// red chess
							color = 1;
							firstflip = 0;
						}
					}
				}
				else {//move
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						lose++;
						cout << "creat made wrong move(take blank)!";
						printwinrecord();
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == lost
						lose++;
						cout << "creat made wrong move(cant eat)!";
						printwinrecord();
						return true;
					}
					if (color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// red but take black
							lose++;
							cout << "creat made wrong move(red but take black)!";
							printwinrecord();
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// black but take red
							lose++;
							cout << "creat made wrong move(black but take red)!";
							printwinrecord();
							return true;
						}
					}
					string temp = move[1] + "-" + move[2];
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					cout << "creat move: " << move[1] << "-" << move[2] << endl;
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}

			}
		}
		foute << board[i] << "\n";

	}
	return false;
}

bool doenter() {
	vector<string> board;
	string str;
	vector<string> move;// move in move.txt
	vector<string> readmove;// move in board.txt

	system("cd EnterRoom\\Search && search.exe");// excute enterroom's search.exe
	ifstream finc("EnterRoom\\Search\\board.txt", ios::in);

	int startmove = 0;
	while (getline(finc, str)) {// read creatroom's board.txt
		board.push_back(str);
		/*if (str.at(2) == 't') {// start after * time 900
			startmove = 1;
		}
		if (str.at(2) == 'C'&&str.at(3) == 'o') {// stop before * Comment 0 0
			startmove = 0;
		}
		if (startmove == 1) {
			if (str.length() >= 10 && str.at(2) != 't' && str.length() <= 12) {
				readmove.push_back(str.substr(str.length() - 5, 5));// a1(k),a1-a2
			}
			if (str.length() >= 16 && str.length() <= 18) {
				readmove.push_back(str.substr(str.length() - 11, 5));// a1(k),a1-a2
				readmove.push_back(str.substr(str.length() - 5, 5));// a1(k),a1-a2
			}
		}*/
	}
	/*for (int i = 0; i < readmove.size()&& !readmove.empty(); i++) {// record in char board[4][8]
		if (readmove[i].at(2) == '(') {// record move 'flip'
			showboard[readmove[i].at(0) - 'a'][readmove[i].at(1) - '1'] = readmove[i].at(3);										// board[a][1]=k,ai(k)
		}
		else {
			showboard[readmove[i].at(3) - 'a'][readmove[i].at(4) - '1'] = showboard[readmove[i].at(0) - 'a'][readmove[i].at(1) - '1'];	// board[a][2]=k, a1-a2
			showboard[readmove[i].at(0) - 'a'][readmove[i].at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
		}
	}*/

	ifstream fincm("EnterRoom\\Search\\move.txt", ios::in);// read enterroom's move.txt
	int moveline = 0;
	while (getline(fincm, str)) {
		move.push_back(str);
	}

	ofstream foute("CreateRoom\\Search\\board.txt", ios::out);
	for (int i = 0; i < board.size(); i++) {// copy to creatroom's board.txt
		if (!first) {
			if (board[i].length() >= 10 && board[i].at(2) != 't' && board[i].length() <= 12) {// plus move made by enterroom's search.exe at the col only have one move
				if (move[0].at(0) == '1') {// flip
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == win
						win++;
						cout << "enter made wrong flip!";
						printwinrecord();
						return true;
					}
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					flip.erase(flip.begin() + f);
					cout << "enter move: " << move[1] << endl;
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = temp.at(3);
				}
				else {//move
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						win++;
						cout << "creat made wrong move(take blank)!";
						printwinrecord();
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == win
						win++;
						cout << "enter made wrong move(cant eat)!";
						printwinrecord();
						return true;
					}
					if (!color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// black but take red
							win++;
							cout << "enter made wrong move(black but take red)!";
							printwinrecord();
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// red but take black
							win++;
							cout << "enter made wrong move(red but take black)!";
							printwinrecord();
							return true;
						}
					}
					string temp = move[1] + "-" + move[2];
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					cout << "enter move: " << move[1] << "-" << move[2] << endl;
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}
			}
		}
		else {
			if (board[i].at(2) == 'C'&&board[i].at(3) == 'o') {// create new line of move before comment
				if (move[0].at(0) == '1') {// flip
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == win
						win++;
						cout << "enter made wrong flip!";
						printwinrecord();
						return true;
					}
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					flip.erase(flip.begin() + f);
					cout << "enter move: " << move[1] << endl;
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = temp.at(3);
					if (firstflip) {
						if (temp.at(3) - 96 > 0) {// how many black chess left
							color = 1;
							firstflip = 0;

						}
						else {// how many red chess left
							color = 0;
							firstflip = 0;
						}
					}
				}
				else {//move
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						win++;
						cout << "creat made wrong move(take blank)!";
						printwinrecord();
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == win
						win++;
						cout << "enter made wrong move(cant eat)!";
						printwinrecord();
						return true;
					}
					if (!color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// red but take black
							win++;
							cout << "enter made wrong move!";
							printwinrecord();
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// black but take red
							win++;
							cout << "enter made wrong move!";
							printwinrecord();
							return true;
						}
					}
					string temp = move[1] + "-" + move[2];
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					cout << "enter move: " << move[1] << "-" << move[2] << endl;
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}

			}
		}
		foute << board[i] << "\n";

	}
	return false;
}

int main() {
	/*
	ifstream finc("CreateRoom\\Search\\board.txt", ios::in);
	ofstream foutc("CreateRoom\\Search\\board.txt", ios::out);
	ifstream fincm("CreateRoom\\Search\\move.txt", ios::in);
	ofstream foutcm("CreateRoom\\Search\\move.txt", ios::out);
	ifstream fine("EnterRoom\\Search\\board.txt", ios::in);
	ofstream foute("EnterRoom\\Search\\board.txt", ios::out);
	ifstream finem("EnterRoom\\Search\\move.txt", ios::in);
	ofstream foutem("EnterRoom\\Search\\move.txt", ios::out);
	*/
	for (; turn > 0; turn--) {
		int step = 0;
		init();
		while (1) {
			if (!first) {// first:creatroom , second:enterroom
				cout << "creat step: " << step << endl;
				if (docreate()) { break; }
				if (check()) { break; }
				//system("pause");
				cout << "enter step: " << step << endl;
				if (doenter()) { break; }
				if (check()) { break; }
				//system("pause");

			}
			else {// first:enterroom , second:creatroom
				cout << "enter step: " << step << endl;
				if (doenter()) { break; }
				if (check()) { break; }
				//system("pause");
				cout << "creat step: " << step << endl;
				if (docreate()) { break; }
				if (check()) { break; }
				//system("pause");
			}
			step++;
		}
		if (changeorder)
			first = 1 - first;
		system("pause");
	}
	//system("pause");
	return 0;
}

