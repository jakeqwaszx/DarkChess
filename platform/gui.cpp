// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <fstream> 
#include <string> 
#include <vector>
#include <iostream>
#include <math.h>
#include <time.h>
#include <cstdlib>
#include <list>
#include "stdio.h"
#include <omp.h>
#include <map>
using namespace std;
// Global variables
#define WM_MY_MESSAGE1 WM_USER + 1
//change here
//*******************************//
int first;// 0=first:creatroom , second:enterroom
int turn;// how many turn to play
int changeorder;// 1:yes, 0:no
//*******************************//

vector<char> flip;
static char showboard[4][8] = { 'X','X','X','X',
								'X','X','X','X',
								'X','X','X','X',
								'X','X','X','X',
								'X','X','X','X',
								'X','X','X','X',
								'X','X','X','X',
								'X','X','X','X' };
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
#pragma warning(suppress : 4996)
tm* ltm = localtime(&now);
string formattime = to_string(1900 + ltm->tm_year) + " " + to_string(1 + ltm->tm_mon) + " " + to_string(ltm->tm_mday) + " " + to_string(ltm->tm_hour) + " " + to_string(ltm->tm_min) + " " + to_string(ltm->tm_sec);
string winrecord = formattime + ".txt";
string winrecordmove = formattime + " move.txt";
vector<string> recordmove;// use to record all move in one game
int specialSeeds = 0;
int globalspecialSeeds = 0;//specialSeeds for all play (seeds[i]=rand())
vector<int> seeds;// generate rand seeds as many as turn in main 
map<int, char> flipboard;
//*******************************//

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("暗棋平台");
static int out = 0;
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
void init();
int boardtoindex(string board);
void set_console_color(unsigned short color_index);
void printwinrecord(int state);
TCHAR* translate(char a);
bool check();
bool eat(char a, char b);
bool checktimeout(double start, double end, int whostime);
bool docreate();
bool doenter();
bool repetition();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(_In_ HINSTANCE hInstance,_In_ HINSTANCE hPrevInstance,_In_ LPSTR lpCmdLine,_In_ int nCmdShow)
{
	DisableProcessWindowsGhosting();
	WNDCLASSEX wcex;
	AllocConsole();
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,_T("Call to RegisterClassEx failed!"),_T("Windows Desktop Guided Tour"),NULL);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	HWND hWnd = CreateWindow(szWindowClass,szTitle,WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, CW_USEDEFAULT,400, 800,NULL,NULL,hInstance,NULL);

	if (!hWnd)
	{
		MessageBox(NULL,_T("Call to CreateWindow failed!"),_T("Windows Desktop Guided Tour"),NULL);

		return 1;
	}

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd,
		nCmdShow);
	UpdateWindow(hWnd);
	SetTimer(hWnd, 1, 1000, nullptr);

	MSG msg;
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), TEXT("how many turn\n"), wcslen(TEXT("how many turn\n")), NULL, NULL);
	char buffer[10];
	DWORD read;
	ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), &read, nullptr);
	turn = atoi(buffer);
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), TEXT("0:creatroom first 1:enterroom first\n"), wcslen(TEXT("0:creatroom first 1:enterroom first\n")), NULL, NULL);
	ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), &read, nullptr);
	first = atoi(buffer);
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), TEXT("change order? 1:YES 0:NO\n"), wcslen(TEXT("change order? 1:YES 0:NO\n")), NULL, NULL);
	ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), &read, nullptr);
	changeorder = atoi(buffer);
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), TEXT("special seeds? 0:NO\n"), wcslen(TEXT("special seeds? 0:NO\n")), NULL, NULL);
	ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), &read, nullptr);
	specialSeeds = atoi(buffer);
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), TEXT("special Seeds All? 0:NO\n"), wcslen(TEXT("special Seeds All? 0:NO\n")), NULL, NULL);
	ReadConsoleA(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), &read, nullptr);
	globalspecialSeeds = atoi(buffer);
	totalturn = turn;
	if (globalspecialSeeds == 0) {
		srand(time(NULL));
		globalspecialSeeds = rand();
	}
	srand(globalspecialSeeds);
	for (int i = 0; i < turn; i++) {
		seeds.push_back(rand());
	}
	printwinrecord(0);
	for (; turn > 0; turn--) {
		if (specialSeeds != 0) {
			srand(specialSeeds);
		}
		else {
			srand(seeds[totalturn - turn]);
		}
		int step = 0;
		init();
		printwinrecord(1);
		while (1) {
			if (!first) {// first:creatroom , second:enterroom
				cout << "creat step: " << step << endl;
				if (docreate()) { err++; printwinrecord(2); break; }
				out++;
				SendMessage(hWnd, WM_MY_MESSAGE1, 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (check()) { break; }
				//system("pause");
				cout << "enter step: " << step << endl;
				if (doenter()) { err++;  printwinrecord(2); break; }
				out++;
				SendMessage(hWnd, WM_MY_MESSAGE1, 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				printwinrecord(2);
				if (check()) { break; }
				if (repetition()) { break; }
				//system("pause");

			}
			else {// first:enterroom , second:creatroom
				cout << "enter step: " << step << endl;
				if (doenter()) { err++; printwinrecord(2); break; }
				out++;
				SendMessage(hWnd, WM_MY_MESSAGE1, 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (check()) { break; }
				//system("pause");
				cout << "creat step: " << step << endl;
				if (docreate()) { err++; printwinrecord(2); break; }
				out++;
				SendMessage(hWnd, WM_MY_MESSAGE1, 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				printwinrecord(2);
				if (check()) { break; }
				if (repetition()) { break; }
				//system("pause");
			}
			step++;
		}
		printwinrecord(3);
		if (changeorder)
			first = 1 - first;
		//system("pause");
	}
	cout << "err: " << err << endl;
	system("pause");

	// Main message loop:
	//MSG msg;
	/*while (1) {
		out++;
		SendMessage(hWnd, WM_MY_MESSAGE1, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		Sleep(5000);
	}*/
	/*while (GetMessage(&msg, NULL, 0, 0))
	{
		out++;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		SendMessage(hWnd, WM_MY_MESSAGE1, 0, 0);
	}*/
	
	return (int)msg.wParam;
}

void init() {
	for (int i = 0; i < 8; i++) {// initboard[4][8]
		for (int j = 0; j < 4; j++) {
			showboard[j][i] = 'X';
		}
	}
	string initboard = "* 8:59am March 31, 2019\n* Player1 vs Player2\n* 1 2 2 2 2 2 5 1 2 2 2 2 2 5\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* X X X X\n* first 2\n* time 900\n* Comment 0 0\n";
	flip = { 'K','G','G','M','M','R','R','N','N','C','C','P','P' ,'P' ,'P' ,'P'
				,'k','g','g','m','m','r','r','n','n','c','c','p','p' ,'p' ,'p' ,'p' };
	for (int i = 0; i < 32; i++) {
		int f = rand() % flip.size();
		flipboard[i] = flip[f];
		flip.erase(flip.begin() + f);
	}
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

int boardtoindex(string board) {
	int c = 100 - board.at(0);
	int i = 4 * (56 - board.at(1));
	return c + i;//棋盤編號0~31
}

void set_console_color(unsigned short color_index)// control collor
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_index);
}

void printwinrecord(int state) {
	ofstream foutrc(winrecord, ios::app);
	ofstream foutrcm(winrecordmove, ios::app);
	if (state == 0) {
		foutrcm << "globalspecialSeeds: " << globalspecialSeeds << endl;
	}
	else if (state == 1) {
		foutrcm << "PlayTime: " << totalturn - turn + 1 << " Seeds: " << seeds[totalturn - turn] << endl;
	}
	else if (state == 2) {

		foutrcm << recordmove[recordmove.size() - 1] << endl;

	}
	else {
		foutrc << totalturn << " ply cycle \n" << " ## Win : " << win << " ,   Lose : " << lose << " ,  Draw : " << draw << " ,  PlayTime : " << totalturn - turn + 1 << "  ##\n";
		if (recordmove.size() % 2 == 1) {
			foutrcm << recordmove[recordmove.size() - 1] << endl;
		}
		for (int i = 0; i < recordmove.size(); i += 2) {
			if (i < createtime.size())
				foutrcm << i << " ." << createtime[i] << endl;
			if (i < entertime.size())
				foutrcm << i + 1 << " ." << entertime[i] << endl;
		}
		foutrcm << "createroom total time use: " << timeout[0] << endl;
		foutrcm << "enterroom total time use: " << timeout[1] << endl;
	}
	foutrc.close();
	foutrcm.close();
}

TCHAR* translate(char a) { //k to 將
	if (a == 'k') {//將
		TCHAR szText[20] = TEXT("將");
		return szText;
	}
	else if (a == 'g') {//士
		TCHAR szText[20] = TEXT("士");
		return szText;
	}
	else if (a == 'm') {//象
		TCHAR szText[20] = TEXT("象");
		return szText;
	}
	else if (a == 'r') {//車
		TCHAR szText[20] = TEXT("車");
		return szText;

	}
	else if (a == 'n') {//馬
		TCHAR szText[20] = TEXT("馬");
		return szText;

	}
	else if (a == 'c') {//砲
		TCHAR szText[20] = TEXT("砲");
		return szText;

	}
	else if (a == 'p') {//卒
		TCHAR szText[20] = TEXT("卒");
		return szText;

	}
	else if (a == 'K') {//帥
		TCHAR szText[20] = TEXT("帥");
		return szText;

	}
	else if (a == 'G') {//仕
		TCHAR szText[20] = TEXT("仕");
		return szText;

	}
	else if (a == 'M') {//相
		TCHAR szText[20] = TEXT("相");
		return szText;

	}
	else if (a == 'R') {//車
		TCHAR szText[20] = TEXT("車");
		return szText;

	}
	else if (a == 'N') {//傌
		TCHAR szText[20] = TEXT("傌");
		return szText;

	}
	else if (a == 'C') {//炮
		TCHAR szText[20] = TEXT("炮");
		return szText;

	}
	else if (a == 'P') {//兵
		TCHAR szText[20] = TEXT("兵");
		return szText;

	}
	else if (a == 'X') {
		TCHAR szText[20] = TEXT("X");
		return szText;

	}
	else if (a == '-') {
		TCHAR szText[20] = TEXT("-");
		return szText;

	}
}

bool check() {
	cout << "\n";
	if (!first) {
		cout << "creatroom first" << endl;
		cout << "turn: " << totalturn - turn + 1 << endl;
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
			if (flipboard.empty()) {
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
	if (!flipboard.empty()) {
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
		if (timeout[whostime] >= 900) {
			cout << "enterroom timeout\n";
			win++;
			return true;
		}
	}
	else {
		timeout[whostime] += timeuse;
		createtime.push_back(timeuse);
		if (timeout[whostime] >= 900) {
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

	int done = 0;
	double START, END = 0.0;
	START = clock();
#pragma omp parallel sections
	{
#pragma omp section
		{
			system("cd CreateRoom\\Search && search.exe");// excute creatroom's search.exe
			done = 1;
		}
#pragma omp section
		{
			while (!done) {
				END = clock();
				if ((timeout[0] + (END - START) / CLOCKS_PER_SEC) >= 900) {
					system("taskkill /F /T /IM search.exe");
				}
			}
		}
	}

	if (checktimeout(START, END, 0)) {
		return true;
	}
	Sleep(500);
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
					string temp = move[1] + "(" + flipboard[boardtoindex(move[1])] + ")";
					flipboard.erase(boardtoindex(move[1]));
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
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
			if (board[i].at(2) == 'C' && board[i].at(3) == 'o') {// create new line of move before comment
				if (move[0].at(0) == '1') {// flip
					string temp = move[1] + "(" + flipboard[boardtoindex(move[1])] + ")";
					flipboard.erase(boardtoindex(move[1]));
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
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

	int done = 0;
	double START, END = 0.0;
	START = clock();
#pragma omp parallel sections
	{
#pragma omp section
		{
			system("cd EnterRoom\\Search && search.exe");// excute enterroom's search.exe
			done = 1;
		}
#pragma omp section
		{
			while (!done) {
				END = clock();
				if ((timeout[1] + (END - START) / CLOCKS_PER_SEC) >= 900) {
					system("taskkill /F /T /IM search.exe");
				}
			}
		}
	}
	if (checktimeout(START, END, 1)) {
		return true;
	}
	Sleep(500);
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
					string temp = move[1] + "(" + flipboard[boardtoindex(move[1])] + ")";
					flipboard.erase(boardtoindex(move[1]));
					board[i] += " " + temp;
					recordmove[recordmove.size() - 1] += " " + temp;
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
			if (board[i].at(2) == 'C' && board[i].at(3) == 'o') {// create new line of move before comment
				if (move[0].at(0) == '1') {// flip
					string temp = move[1] + "(" + flipboard[boardtoindex(move[1])] + ")";
					flipboard.erase(boardtoindex(move[1]));
					foute << "* " << i - 12 << ". " << temp << "\n";
					recordmove.push_back("* " + to_string(i - 12) + ". " + temp);
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
	for (int i = 0; i < size - 5; i++) {
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hdcStatic;
	RECT rect;
	static HFONT hFont;
	static HWND hBtn;
	static HWND a8;
	static HWND a7;
	static HWND a6;
	static HWND a5;
	static HWND a4;
	static HWND a3;
	static HWND a2;
	static HWND a1;
	static HWND b8;
	static HWND b7;
	static HWND b6;
	static HWND b5;
	static HWND b4;
	static HWND b3;
	static HWND b2;
	static HWND b1;
	static HWND c8;
	static HWND c7;
	static HWND c6;
	static HWND c5;
	static HWND c4;
	static HWND c3;
	static HWND c2;
	static HWND c1;
	static HWND d8;
	static HWND d7;
	static HWND d6;
	static HWND d5;
	static HWND d4;
	static HWND d3;
	static HWND d2;
	static HWND d1;
	static HWND la;
	static HWND lb;
	static HWND lc;
	static HWND ld;
	static HWND l1;
	static HWND l2;
	static HWND l3;
	static HWND l4;
	static HWND l5;
	static HWND l6;
	static HWND l7;
	static HWND l8;
	TCHAR szUsername[100];
	TCHAR szPassword[100];
	TCHAR szUserInfo[200];
	TCHAR szTexta8[20];
	TCHAR szTexta7[20];
	TCHAR szTexta6[20];
	TCHAR szTexta5[20];
	TCHAR szTexta4[20];
	TCHAR szTexta3[20];
	TCHAR szTexta2[20];
	TCHAR szTexta1[20];
	TCHAR szTextb8[20];
	TCHAR szTextb7[20];
	TCHAR szTextb6[20];
	TCHAR szTextb5[20];
	TCHAR szTextb4[20];
	TCHAR szTextb3[20];
	TCHAR szTextb2[20];
	TCHAR szTextb1[20];
	TCHAR szTextc8[20];
	TCHAR szTextc7[20];
	TCHAR szTextc6[20];
	TCHAR szTextc5[20];
	TCHAR szTextc4[20];
	TCHAR szTextc3[20];
	TCHAR szTextc2[20];
	TCHAR szTextc1[20];
	TCHAR szTextd8[20];
	TCHAR szTextd7[20];
	TCHAR szTextd6[20];
	TCHAR szTextd5[20];
	TCHAR szTextd4[20];
	TCHAR szTextd3[20];
	TCHAR szTextd2[20];
	TCHAR szTextd1[20];
	TCHAR szText[20];
	static HBRUSH hbrBkgnd;
	DWORD ctrlID;
	switch (message) {
	case  WM_CREATE:
		hFont = CreateFont(
			-15, -7.5, 0, 0, 700,
			FALSE, FALSE, FALSE,
			DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
			DEFAULT_QUALITY,
			FF_DONTCARE,
			L"微软雅黑"
		);

		SendMessage(
			hBtn,
			WM_SETFONT,
			(WPARAM)hFont,
			NULL
		);
		a8 = CreateWindow(L"static", translate(showboard[0][7]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 0, 50, 50, hWnd, (HMENU)8, hInst, NULL);
		a7 = CreateWindow(L"static", translate(showboard[0][6]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 50, 50, 50, hWnd, (HMENU)7, hInst, NULL);
		a6 = CreateWindow(L"static", translate(showboard[0][5]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 100, 50, 50, hWnd, (HMENU)6, hInst, NULL);
		a5 = CreateWindow(L"static", translate(showboard[0][4]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 150, 50, 50, hWnd, (HMENU)5, hInst, NULL);
		a4 = CreateWindow(L"static", translate(showboard[0][3]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 200, 50, 50, hWnd, (HMENU)4, hInst, NULL);
		a3 = CreateWindow(L"static", translate(showboard[0][2]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 250, 50, 50, hWnd, (HMENU)3, hInst, NULL);
		a2 = CreateWindow(L"static", translate(showboard[0][1]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 300, 50, 50, hWnd, (HMENU)2, hInst, NULL);
		a1 = CreateWindow(L"static", translate(showboard[0][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 350, 50, 50, hWnd, (HMENU)1, hInst, NULL);
		b8 = CreateWindow(L"static", translate(showboard[1][7]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 0, 50, 50, hWnd, (HMENU)18, hInst, NULL);
		b7 = CreateWindow(L"static", translate(showboard[1][6]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 50, 50, 50, hWnd, (HMENU)17, hInst, NULL);
		b6 = CreateWindow(L"static", translate(showboard[1][5]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 100, 50, 50, hWnd, (HMENU)16, hInst, NULL);
		b5 = CreateWindow(L"static", translate(showboard[1][4]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 150, 50, 50, hWnd, (HMENU)15, hInst, NULL);
		b4 = CreateWindow(L"static", translate(showboard[1][3]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 200, 50, 50, hWnd, (HMENU)14, hInst, NULL);
		b3 = CreateWindow(L"static", translate(showboard[1][2]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 250, 50, 50, hWnd, (HMENU)13, hInst, NULL);
		b2 = CreateWindow(L"static", translate(showboard[1][1]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 300, 50, 50, hWnd, (HMENU)12, hInst, NULL);
		b1 = CreateWindow(L"static", translate(showboard[1][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 350, 50, 50, hWnd, (HMENU)11, hInst, NULL);
		c8 = CreateWindow(L"static", translate(showboard[2][7]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 0, 50, 50, hWnd, (HMENU)28, hInst, NULL);
		c7 = CreateWindow(L"static", translate(showboard[2][6]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 50, 50, 50, hWnd, (HMENU)27, hInst, NULL);
		c6 = CreateWindow(L"static", translate(showboard[2][5]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 100, 50, 50, hWnd, (HMENU)26, hInst, NULL);
		c5 = CreateWindow(L"static", translate(showboard[2][4]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 150, 50, 50, hWnd, (HMENU)25, hInst, NULL);
		c4 = CreateWindow(L"static", translate(showboard[2][3]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 200, 50, 50, hWnd, (HMENU)24, hInst, NULL);
		c3 = CreateWindow(L"static", translate(showboard[2][2]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 250, 50, 50, hWnd, (HMENU)23, hInst, NULL);
		c2 = CreateWindow(L"static", translate(showboard[2][1]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 300, 50, 50, hWnd, (HMENU)22, hInst, NULL);
		c1 = CreateWindow(L"static", translate(showboard[2][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 350, 50, 50, hWnd, (HMENU)21, hInst, NULL);
		d8 = CreateWindow(L"static", translate(showboard[3][7]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 0, 50, 50, hWnd, (HMENU)38, hInst, NULL);
		d7 = CreateWindow(L"static", translate(showboard[3][6]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 50, 50, 50, hWnd, (HMENU)37, hInst, NULL);
		d6 = CreateWindow(L"static", translate(showboard[3][5]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 100, 50, 50, hWnd, (HMENU)36, hInst, NULL);
		d5 = CreateWindow(L"static", translate(showboard[3][4]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 150, 50, 50, hWnd, (HMENU)35, hInst, NULL);
		d4 = CreateWindow(L"static", translate(showboard[3][3]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 200, 50, 50, hWnd, (HMENU)34, hInst, NULL);
		d3 = CreateWindow(L"static", translate(showboard[3][2]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 250, 50, 50, hWnd, (HMENU)33, hInst, NULL);
		d2 = CreateWindow(L"static", translate(showboard[3][1]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 300, 50, 50, hWnd, (HMENU)32, hInst, NULL);
		d1 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 350, 50, 50, hWnd, (HMENU)31, hInst, NULL);
		la = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 0, 400, 50, 50, hWnd, (HMENU)41, hInst, NULL);
		lb = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 50, 400, 50, 50, hWnd, (HMENU)42, hInst, NULL);
		lc = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 100, 400, 50, 50, hWnd, (HMENU)43, hInst, NULL);
		ld = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 150, 400, 50, 50, hWnd, (HMENU)44, hInst, NULL);
		l1 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 350, 50, 50, hWnd, (HMENU)51, hInst, NULL);
		l2 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 300, 50, 50, hWnd, (HMENU)52, hInst, NULL);
		l3 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 250, 50, 50, hWnd, (HMENU)53, hInst, NULL);
		l4 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 200, 50, 50, hWnd, (HMENU)54, hInst, NULL);
		l5 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 150, 50, 50, hWnd, (HMENU)55, hInst, NULL);
		l6 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 100, 50, 50, hWnd, (HMENU)56, hInst, NULL);
		l7 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 50, 50, 50, hWnd, (HMENU)57, hInst, NULL);
		l8 = CreateWindow(L"static", translate(showboard[3][0]), WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, 200, 0, 50, 50, hWnd, (HMENU)58, hInst, NULL);
		break;
	case WM_PAINT:
	
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CTLCOLORSTATIC:
		hdcStatic = (HDC)wParam;
		ctrlID = GetDlgCtrlID((HWND)lParam);
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 8; j++) {
				if (ctrlID == i*10+j+1)
				{
					if (showboard[i][j] - 96 > 0) {
						SetTextColor(hdcStatic, RGB(0, 0, 0));
						SetBkColor(hdcStatic, RGB(255, 255, 255));
						hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
					}
					else if(showboard[i][j]=='X'){
						SetTextColor(hdcStatic, RGB(0, 0, 0));
						SetBkColor(hdcStatic, RGB(0, 0, 0));
						hbrBkgnd = CreateSolidBrush(RGB(0, 0, 0));
					}
					else if (showboard[i][j] == '-') {
						SetTextColor(hdcStatic, RGB(255, 255, 255));
						SetBkColor(hdcStatic, RGB(255, 255, 255));
						hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
					}
					else {
						SetTextColor(hdcStatic, RGB(255, 0, 0));
						SetBkColor(hdcStatic, RGB(255, 255, 255));
						hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
					}
				}
			}		
		}
		if (ctrlID > 40) {
			SetTextColor(hdcStatic, RGB(0, 0, 0));
			SetBkColor(hdcStatic, RGB(255, 255, 255));
			hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
		}
		if (hbrBkgnd == NULL)
		{
			hbrBkgnd = CreateSolidBrush(RGB(255, 255, 255));
		}
		return (INT_PTR)hbrBkgnd;
	
	case WM_MY_MESSAGE1:
		hFont = CreateFont(-30, -7.5, 0, 0, 700,FALSE, FALSE, FALSE,DEFAULT_CHARSET,OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,L"微软雅黑");
		SendMessage(a1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(a2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(a3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(a4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(a5, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(a6, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(a7, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(a8, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b5, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b6, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b7, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(b8, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c5, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c6, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c7, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(c8, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d1, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d2, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d3, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d4, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d5, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d6, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d7, WM_SETFONT, (WPARAM)hFont, NULL);
		SendMessage(d8, WM_SETFONT, (WPARAM)hFont, NULL);
		wsprintf(szTexta8, translate(showboard[0][7]));
		wsprintf(szTexta7, translate(showboard[0][6]));
		wsprintf(szTexta6, translate(showboard[0][5]));
		wsprintf(szTexta5, translate(showboard[0][4]));
		wsprintf(szTexta4, translate(showboard[0][3]));
		wsprintf(szTexta3, translate(showboard[0][2]));
		wsprintf(szTexta2, translate(showboard[0][1]));
		wsprintf(szTexta1, translate(showboard[0][0]));
		wsprintf(szTextb8, translate(showboard[1][7]));
		wsprintf(szTextb7, translate(showboard[1][6]));
		wsprintf(szTextb6, translate(showboard[1][5]));
		wsprintf(szTextb5, translate(showboard[1][4]));
		wsprintf(szTextb4, translate(showboard[1][3]));
		wsprintf(szTextb3, translate(showboard[1][2]));
		wsprintf(szTextb2, translate(showboard[1][1]));
		wsprintf(szTextb1, translate(showboard[1][0]));
		wsprintf(szTextc8, translate(showboard[2][7]));
		wsprintf(szTextc7, translate(showboard[2][6]));
		wsprintf(szTextc6, translate(showboard[2][5]));
		wsprintf(szTextc5, translate(showboard[2][4]));
		wsprintf(szTextc4, translate(showboard[2][3]));
		wsprintf(szTextc3, translate(showboard[2][2]));
		wsprintf(szTextc2, translate(showboard[2][1]));
		wsprintf(szTextc1, translate(showboard[2][0]));
		wsprintf(szTextd8, translate(showboard[3][7]));
		wsprintf(szTextd7, translate(showboard[3][6]));
		wsprintf(szTextd6, translate(showboard[3][5]));
		wsprintf(szTextd5, translate(showboard[3][4]));
		wsprintf(szTextd4, translate(showboard[3][3]));
		wsprintf(szTextd3, translate(showboard[3][2]));
		wsprintf(szTextd2, translate(showboard[3][1]));
		wsprintf(szTextd1, translate(showboard[3][0]));
		SetWindowText(a8, szTexta8);
		SetWindowText(a7, szTexta7);
		SetWindowText(a6, szTexta6);
		SetWindowText(a5, szTexta5);
		SetWindowText(a4, szTexta4);
		SetWindowText(a3, szTexta3);
		SetWindowText(a2, szTexta2);
		SetWindowText(a1, szTexta1);
		SetWindowText(b8, szTextb8);
		SetWindowText(b7, szTextb7);
		SetWindowText(b6, szTextb6);
		SetWindowText(b5, szTextb5);
		SetWindowText(b4, szTextb4);
		SetWindowText(b3, szTextb3);
		SetWindowText(b2, szTextb2);
		SetWindowText(b1, szTextb1);
		SetWindowText(c8, szTextc8);
		SetWindowText(c7, szTextc7);
		SetWindowText(c6, szTextc6);
		SetWindowText(c5, szTextc5);
		SetWindowText(c4, szTextc4);
		SetWindowText(c3, szTextc3);
		SetWindowText(c2, szTextc2);
		SetWindowText(c1, szTextc1);
		SetWindowText(d8, szTextd8);
		SetWindowText(d7, szTextd7);
		SetWindowText(d6, szTextd6);
		SetWindowText(d5, szTextd5);
		SetWindowText(d4, szTextd4);
		SetWindowText(d3, szTextd3);
		SetWindowText(d2, szTextd2);
		SetWindowText(d1, szTextd1);
		SetWindowText(la, TEXT("a"));
		SetWindowText(lb, TEXT("b"));
		SetWindowText(lc, TEXT("c"));
		SetWindowText(ld, TEXT("d"));
		SetWindowText(l1, TEXT("1"));
		SetWindowText(l2, TEXT("2"));
		SetWindowText(l3, TEXT("3"));
		SetWindowText(l4, TEXT("4"));
		SetWindowText(l5, TEXT("5"));
		SetWindowText(l6, TEXT("6"));
		SetWindowText(l7, TEXT("7"));
		SetWindowText(l8, TEXT("8"));
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
