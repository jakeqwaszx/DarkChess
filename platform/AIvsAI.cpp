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
int first;// 0=first:creatroom , second:enterroom
int turn;// how many turn to play
int changeorder;// 1:yes, 0:no
//*******************************//

vector<char> flip;
char showboard[4][8];
int color = -1;// 0:create is b,enter is R; 1:create is R,enter is b
int firstflip = 0;// 1 first time flip(use to determine color);
int err = 0;
int totalturn;
double timeout[2];//0:create's time; 1:enter's time
vector<double> createtime;
vector<double> entertime;

//this are for creatroom
//*******************************//
int win = 0;
int lose = 0;
int draw = 0;
//*******************************//

//record win,lose in realtime.txt ex."2019 3 31 14 31 50.txt"
//record move in realtimemove.txt ex."2019 3 31 14 31 50 move.txt"
//*******************************//
time_t now = time(0);
tm *ltm = localtime(&now);
string formattime = to_string(1900 + ltm->tm_year) + " " + to_string(1 + ltm->tm_mon) + " " + to_string(ltm->tm_mday) + " " + to_string(ltm->tm_hour) + " " + to_string(ltm->tm_min) + " " + to_string(ltm->tm_sec);
string winrecord = formattime + ".txt";
string winrecordmove = formattime + " move.txt";
vector<string> recordmove;// use to record all move in one game
//*******************************//
 
void init() {
	for (int i = 0; i < 8; i++) {// initboard[4][8]
		for (int j = 0; j < 4; j++) {
			showboard[j][i] = 'X';
		}
	}
	string initboard = "* 8:59am March 31, 2019\n* Player1 vs Player2\n* 1 2 2 2 2 2 5 1 2 2 2 2 2 5\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* first 2\n* time 900\n* Comment 0 0\n";
	flip = { 'K','G','G','M','M','R','R','N','N','C','C','P','P' ,'P' ,'P' ,'P'
				,'k','g','g','m','m','r','r','n','n','c','c','p','p' ,'p' ,'p' ,'p' };
	firstflip = 1;
	color = -1;
	for (int i = 0; i < 2; i++) {
		timeout[i] = 0;
	}
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
	ofstream foutrc(winrecord, ios::app);
	ofstream foutrcm(winrecordmove, ios::app);
	foutrc << totalturn << " ply cycle \n" << " ## Win : " << win << " ,   Lose : " << lose << " ,  Draw : " << draw << " ,  PlayTime : " << totalturn - turn + 1 << "  ##\n";
	foutrcm << "PlayTime: " << totalturn - turn + 1 << endl;
	for (int i = 0; i < recordmove.size(); i++) {
		foutrcm << recordmove[i] << endl;
	}
	for (int i = 0; i < recordmove.size(); i+=2) {
		if (i < createtime.size())
			foutrcm << i << " ." << createtime[i] << endl;
		if(i<entertime.size())
			foutrcm << i+1 << " ." << entertime[i] << endl;
	}
	foutrcm << "createroom total time use: " << timeout[0] << endl;
	foutrcm << "enterroom total time use: " << timeout[1] << endl;
	foutrc.close();
	foutrcm.close();
}

void translate(char a) { //k to 將
	if (a - 96 > 0) {
		set_console_color(10);
	}
	if (a - 64 > 0 && a - 88 < 0) {
		set_console_color(12);
	}
	if (a == 'k') {//將
		printf("\t將");
	}
	else if (a == 'g') {//士
		printf("\t士");
	}
	else if (a == 'm') {//象
		printf("\t象");
	}
	else if (a == 'r') {//車
		printf("\t車");

	}
	else if (a == 'n') {//馬
		printf("\t馬");

	}
	else if (a == 'c') {//砲
		printf("\t砲");

	}
	else if (a == 'p') {//卒
		printf("\t卒");

	}
	else if (a == 'K') {//帥
		printf("\t帥");

	}
	else if (a == 'G') {//仕
		printf("\t仕");

	}
	else if (a == 'M') {//相
		printf("\t相");

	}
	else if (a == 'R') {//車
		printf("\t車");

	}
	else if (a == 'N') {//傌
		printf("\t傌");

	}
	else if (a == 'C') {//炮
		printf("\t炮");

	}
	else if (a == 'P') {//兵
		printf("\t兵");

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
		cout << "turn: " << totalturn - turn + 1 <<endl;
		if (color) {
			cout << "create is R,enter is b" << endl;
		}
		else {
			cout << "create is b,enter is R" << endl;
		}
	}
	else {
		cout << "enterroom first" << endl;
		cout << "turn: " << totalturn - turn + 1 << endl;
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
		}
		else {
			cout << "creatroom win!" << endl;
			win++;
		}
		return true;
	}
	else if (b == 0) {
		if (color) {
			cout << "creatroom win!" << endl;
			win++;
		}
		else {
			cout << "enterroom win!" << endl;
			lose++;
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

	if (a == 'k') {//將
		if (b == 'p') {
			return false;
		}
		else return true;
	}
	else if (a == 'g') {//士
		if (b == 'k') {
			return false;
		}
		else return true;
	}
	else if (a == 'm') {//象
		if (b == 'k' || b == 'g') {
			return false;
		}
		else return true;
	}
	else if (a == 'r') {//車
		if (b == 'k' || b == 'g' || b == 'm') {
			return false;
		}
		else return true;
	}
	else if (a == 'n') {//馬
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

bool checktimeout(double start, double end, int whostime) {
	double timeuse = (end - start) / CLOCKS_PER_SEC;
	cout << "use:" << timeuse << " seconds\n";
	if (whostime) {
		timeout[whostime] += timeuse;
		entertime.push_back(timeuse);
		if (timeout[whostime] >=900) {
			cout << "enterroom timeout\n";
			win++;
			return true;
		}
	}
	else {
		timeout[whostime] += timeuse;
		createtime.push_back(timeuse);
		if (timeout[whostime] >=900) {
			cout << "createroom timeout\n";
			lose++;
			return true;
		}
	}
	return false;
}

bool docreate() {
	vector<string> board;
	string str;
	vector<string> move;// move in move.txt
	vector<string> readmove;// move in board.txt

	double START, END;
	START = clock();
	system("cd CreateRoom\\Search && search.exe");// excute creatroom's search.exe
	END = clock();
	if (checktimeout(START, END, 0)) { return true; }
	ifstream finc("CreateRoom\\Search\\board.txt", ios::in);
	int startmove = 0;
	while (getline(finc, str)) {// read creatroom's board.txt
		board.push_back(str);
	}
	finc.close();

	ifstream fincm("CreateRoom\\Search\\move.txt", ios::in);// read creatroom's move.txt
	int moveline = 0;
	while (getline(fincm, str)) {
		move.push_back(str);
	}
	fincm.close();

	ofstream foute("EnterRoom\\Search\\board.txt", ios::out);
	for (int i = 0; i < board.size(); i++) {// copy to enterroom's board.txt
		if (first) {
			if (board[i].length() >= 10 && board[i].at(2) != 't' && board[i].length() <= 12) {// plus move made by creatroom's search.exe
				if (move[0].at(0) == '1') {// flip
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					flip.erase(flip.begin() + f);
					cout << "creat move: " << move[1] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == lose
						lose++;
						cout << "creat made wrong flip!" << endl;
						recordmove.push_back("creat made wrong flip!");
						
						return true;
					}
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = temp.at(3);
				}
				else {//move
					string temp = move[1] + "-" + move[2];
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					cout << "creat move: " << move[1] << "-" << move[2] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						lose++;
						cout << "creat made wrong move(take blank)!" << endl;
						recordmove.push_back("creat made wrong move(take blank)!");
						
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == lost
						lose++;
						cout << "creat made wrong move(cant eat)!" << endl;
						recordmove.push_back("creat made wrong move(cant eat)!");
						
						return true;
					}
					if (color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// red but take black
							lose++;
							cout << "creat made wrong move(red but take black)!" << endl;
							recordmove.push_back("creat made wrong move(red but take black)!");
							
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// black but take red
							lose++;
							cout << "creat made wrong move(black but take red)!" << endl;
							recordmove.push_back("creat made wrong move(black but take red)!");
							
							return true;
						}
					}
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}
			}
		}
		else {
			if (board[i].at(2) == 'C'&&board[i].at(3) == 'o') {// create new line of move before comment
				if (move[0].at(0) == '1') {// flip
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					flip.erase(flip.begin() + f);
					cout << "creat move: " << move[1] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == lose
						lose++;
						cout << "creat made wrong flip!" << endl;
						recordmove.push_back("creat made wrong flip!");
						
						return true;
					}
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
					string temp = move[1] + "-" + move[2];
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					cout << "creat move: " << move[1] << "-" << move[2] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						lose++;
						cout << "creat made wrong move(take blank)!" << endl;
						recordmove.push_back("creat made wrong move(take blank)!");
						
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == lost
						lose++;
						cout << "creat made wrong move(cant eat)!" << endl;
						recordmove.push_back("creat made wrong move(cant eat)!");
						
						return true;
					}
					if (color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// red but take black
							lose++;
							cout << "creat made wrong move(red but take black)!" << endl;
							recordmove.push_back("creat made wrong move(red but take black)!");
							
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// black but take red
							lose++;
							cout << "creat made wrong move(black but take red)!" << endl;
							recordmove.push_back("creat made wrong move(black but take red)!");
							
							return true;
						}
					}
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}

			}
		}
		foute << board[i] << "\n";
		
	}
	foute.close();
	return false;
}

bool doenter() {
	vector<string> board;
	string str;
	vector<string> move;// move in move.txt
	vector<string> readmove;// move in board.txt

	double START, END;
	START = clock();
	system("cd EnterRoom\\Search && search.exe");// excute enterroom's search.exe
	END = clock();
	if (checktimeout(START, END, 1)) { return true; }

	ifstream finc("EnterRoom\\Search\\board.txt", ios::in);
	int startmove = 0;
	while (getline(finc, str)) {// read creatroom's board.txt
		board.push_back(str);
	}
	finc.close();

	ifstream fincm("EnterRoom\\Search\\move.txt", ios::in);// read enterroom's move.txt
	int moveline = 0;
	while (getline(fincm, str)) {
		move.push_back(str);
	}
	fincm.close();

	ofstream foute("CreateRoom\\Search\\board.txt", ios::out);
	for (int i = 0; i < board.size(); i++) {// copy to creatroom's board.txt
		if (!first) {
			if (board[i].length() >= 10 && board[i].at(2) != 't' && board[i].length() <= 12) {// plus move made by enterroom's search.exe at the col only have one move
				if (move[0].at(0) == '1') {// flip
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					flip.erase(flip.begin() + f);
					cout << "enter move: " << move[1] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == win
						win++;
						cout << "enter made wrong flip!" << endl;
						recordmove.push_back("enter made wrong flip!");
						
						return true;
					}
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = temp.at(3);
				}
				else {//move
					string temp = move[1] + "-" + move[2];
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
					cout << "enter move: " << move[1] << "-" << move[2] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						win++;
						cout << "enter made wrong move(take blank)!" << endl;
						recordmove.push_back("enter made wrong move(take blank)!");
						
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == win
						win++;
						cout << "enter made wrong move(cant eat)!" << endl;
						recordmove.push_back("enter made wrong move(cant eat)!");
						
						return true;
					}
					if (!color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// black but take red
							win++;
							cout << "enter made wrong move(black but take red)!" << endl;
							recordmove.push_back("enter made wrong move(black but take red)!");
							
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// red but take black
							win++;
							cout << "enter made wrong move(red but take black)!" << endl;
							recordmove.push_back("enter made wrong move(red but take black)!");
							
							return true;
						}
					}
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}
			}
		}
		else {
			if (board[i].at(2) == 'C'&&board[i].at(3) == 'o') {// create new line of move before comment
				if (move[0].at(0) == '1') {// flip
					srand(time(NULL));
					int f = rand() % flip.size();
					string temp = move[1] + "(" + flip[f] + ")";
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					flip.erase(flip.begin() + f);
					cout << "enter move: " << move[1] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] != 'X') {// wrong flip == win
						win++;
						cout << "enter made wrong flip!" << endl;
						recordmove.push_back("enter made wrong flip!");
						
						return true;
					}
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = temp.at(3);
					if (firstflip) {
						if (temp.at(3) - 96 > 0) {// black
							color = 1;
							firstflip = 0;

						}
						else {// red 
							color = 0;
							firstflip = 0;
						}
					}
				}
				else {//move
					string temp = move[1] + "-" + move[2];
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
					cout << "enter move: " << move[1] << "-" << move[2] << endl;
					if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] == '-') {// take blank
						win++;
						cout << "enter made wrong move(take blank)!" << endl;
						recordmove.push_back("enter made wrong move(take blank)!");
						
						return true;
					}
					if (!eat(showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'], showboard[move[2].at(0) - 'a'][move[2].at(1) - '1']) &&
						!showboard[move[2].at(0) - 'a'][move[2].at(1) - '1' == '-']) {// wrong move(cant eat) == win
						win++;
						cout << "enter made wrong move(cant eat)!" << endl;
						recordmove.push_back("enter made wrong move(cant eat)!");
						
						return true;
					}
					if (!color) {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 96 > 0) {// red but take black
							win++;
							cout << "enter made wrong move!" << endl;
							recordmove.push_back("enter made wrong move!");
							
							return true;
						}
					}
					else {
						if (showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 64 > 0 && showboard[move[1].at(0) - 'a'][move[1].at(1) - '1'] - 88 < 0) {// black but take red
							win++;
							cout << "enter made wrong move!" << endl;
							recordmove.push_back("enter made wrong move!");
							
							return true;
						}
					}
					showboard[temp.at(3) - 'a'][temp.at(4) - '1'] = showboard[temp.at(0) - 'a'][temp.at(1) - '1'];	// board[a][2]=k, a1-a2
					showboard[temp.at(0) - 'a'][temp.at(1) - '1'] = '-';												// board[a][1]='-', a1-a2
				}

			}
		}
		foute << board[i] << "\n";

	}
	foute.close();
	return false;
}

bool repetition() {
	int size = recordmove.size();
	if (size > 180) {
		draw++;
		return true;
	}
	for (int i = 0; i < size-5; i++) {
		if (recordmove[i].substr(recordmove[i].length() - 11, 11) == recordmove[i + 2].substr(recordmove[i].length() - 11, 11) &&
			recordmove[i].substr(recordmove[i].length() - 11, 11) == recordmove[i + 4].substr(recordmove[i].length() - 11, 11) &&
			recordmove[i + 1].substr(recordmove[i].length() - 11, 11) == recordmove[i + 3].substr(recordmove[i].length() - 11, 11) &&
			recordmove[i + 1].substr(recordmove[i].length() - 11, 11) == recordmove[i + 5].substr(recordmove[i].length() - 11, 11)) {
			draw++;
			return true;
		}
	}
	return false;
}

int main() {
	cout << "how many turn" << endl;
	cin >> turn;
	cout << "0:creatroom first 1:enterroom first" << endl;
	cin >> first;
	cout << "change order? 1:YES 0:NO" << endl;
	cin >> changeorder;
	totalturn = turn;
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
				if (docreate()) { err++; printwinrecord(); break; }
				if (check()) { printwinrecord(); break; }
				//system("pause");
				cout << "enter step: " << step << endl;
				if (doenter()) { err++;  printwinrecord(); break; }
				if (check()) { printwinrecord(); break; }
				if (repetition()) { printwinrecord(); break; }
				//system("pause");

			}
			else {// first:enterroom , second:creatroom
				cout << "enter step: " << step << endl;
				if (doenter()) { err++;  printwinrecord(); break; }
				if (check()) { printwinrecord(); break; }
				//system("pause");
				cout << "creat step: " << step << endl;
				if (docreate()) { err++;  printwinrecord(); break; }
				if (check()) { printwinrecord(); break; }
				if (repetition()) { printwinrecord(); break; }
				//system("pause");
			}
			step++;
		}
		if (changeorder)
			first = 1 - first;
		//system("pause");
	}
	cout << "err: " << err << endl;
	system("pause");
	return 0;
}

