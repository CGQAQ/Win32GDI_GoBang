// GoBang.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "GoBang.h"

#pragma comment(lib, "Msimg32.lib")

#define MAX_LOADSTRING 100

#define CHESS_BG_WIDTH		535
#define CHESS_BG_HEIGHT		535
#define CHESS_CHESS_WIDTH	32
#define CHESS_CHESS_HEIGHT  32
#define CHESS_WCHESS		15
#define CHESS_HCHESS		15
#define CHESS_WINCHESSNUM   5
//AI 相关定义----------------------
#define AI_FIVE					1000000		//连五		
#define AI_FOUR					100000		//活四
#define AI_BLOCKED_FOUR			10000		//眠四
#define AI_THREE				1000		//活三
#define AI_BLOCKED_THREE		100			//眠三
#define AI_TWO					100			//活二
#define AI_BLOCKED_TWO			10			//眠二
#define AI_ONE					10			//活一
#define AI_BLOCKED_ONE			1			//眠一


#define HU_FIVE					-1000000	//连五	
#define HU_FOUR					-100000		//活四
#define HU_BLOCKED_FOUR			-10000		//眠四
#define HU_THREE				-1000		//活三
#define HU_BLOCKED_THREE		-100		//眠三
#define HU_TWO					-100		//活二
#define HU_BLOCKED_TWO			-10			//眠二
#define HU_ONE					-10			//活一
#define HU_BLOCKED_ONE			-1			//眠一



#define AI_TimerID			1



// 全局变量: 
HINSTANCE hInst;                                // 当前实例
HWND hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HDC g_hdc;
HDC g_hdcBuf;
HDC g_hdcBG;
HDC g_hdcWhite;
HDC g_hdcBlack;

INT g_intsChess[CHESS_WCHESS][CHESS_HCHESS];
INT g_iCount;
INT g_iRegretMaxStep = 3;
INT g_intsHUScore[CHESS_WCHESS][CHESS_HCHESS];

INT g_AIColor = AIColor_WHITE;					//AI执子
INT g_intsAIScore[CHESS_WCHESS][CHESS_HCHESS];	//AI下棋优先级（点权重）
BOOL g_AITurn = FALSE;


POINT g_ptLastPoint;


VOID CentWindow(HWND hWnd, int nSizeX, int nSizeY);
VOID LoadRES();
VOID InitGame();
VOID PaintGame();
INT GetChessNum(INT x, INT y, Position p, INT chessColor);//
BOOL IsWin(INT x, INT y);
enum FreeLevel FreeCalc(INT x, INT y, Position pos, INT calcColor);
VOID Regret();
VOID CALLBACK AI(HWND, UINT, UINT_PTR, DWORD);
INT MAX(INT[][15]);
INT MIN(INT[][15]);
VOID AICalc();
INT AICalcScore(INT x, INT y, BOOL isAI);
VOID AIControlCenter();
VOID AIMakeDecision();




// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Designer(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GOBANG, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GOBANG));

	MSG msg;

	// 主消息循环: 
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GOBANG));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GOBANG);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindowW(szWindowClass, szTitle, WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// 分析菜单选择: 
			switch (wmId)
			{
				case IDM_ABOUT_VERSION:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_GAME_REGRET:
					Regret();
					PaintGame();
					break;
				case IDM_ABOUT_DESIGNER:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_Designer), hWnd, Designer);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case WM_CREATE:
			CentWindow(hWnd, CHESS_BG_WIDTH, CHESS_BG_HEIGHT);
			break;
		case WM_SHOWWINDOW:
			LoadRES();
			InitGame();
			break;
		case WM_LBUTTONDOWN:
			if (g_AITurn == FALSE)
			{
				g_ptLastPoint.x = LOWORD(lParam) / 35;
				g_ptLastPoint.y = HIWORD(lParam) / 35;
				if (g_ptLastPoint.x < 15 && g_ptLastPoint.y < 15) //棋子在棋盘内
				{
					if (g_intsChess[g_ptLastPoint.x][g_ptLastPoint.y] == 0)  //(x,y)这个位置没有棋子
					{
						g_iCount++;
						g_intsChess[g_ptLastPoint.x][g_ptLastPoint.y] = g_iCount;
						//可以标志这个棋子是第几步下的
						//g_intsChess[x][y] 值为0 表示没有棋子
						//g_intsChess[x][y] 值为奇数， 表示这里有黑棋
						//g_intsChess[x][y] 值为偶数， 表示这里有白棋
						PaintGame();
						if (IsWin(g_ptLastPoint.x, g_ptLastPoint.y))
						{
							MessageBoxW(hWnd, g_intsChess[g_ptLastPoint.x][g_ptLastPoint.y] % 2 ? L"黑棋胜！" : L"白棋胜！", L"游戏结束", MB_OK);
							InitGame();
						}
						else
						{
							g_AITurn = TRUE;
						}
					}
				}
				PaintGame();
			}
			break;
		case WM_PAINT:
		{
			PaintGame();
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: 在此处添加使用 hdc 的任何绘图代码...
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			KillTimer(hWnd, AI_TimerID);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}
// “关于”框的消息处理程序。
INT_PTR CALLBACK Designer(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}


VOID CentWindow(HWND hWnd, int nSizeX, int nSizeY)
{
	int nWinX, nWinY, nClientX, nClientY;
	RECT rect;
	int nScreenX = GetSystemMetrics(SM_CXSCREEN);
	int nScreenY = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(hWnd, &rect);
	nWinX = rect.right - rect.left;
	nWinY = rect.bottom - rect.top;

	GetClientRect(hWnd, &rect);
	nClientX = rect.right - rect.left;
	nClientY = rect.bottom - rect.top;

	nSizeX += (nWinX - nClientX);
	nSizeY += (nWinY - nClientY);

	MoveWindow(hWnd, (nScreenX - nSizeX) / 2, (nScreenY - nSizeY) / 2,
		nSizeX, nSizeY, TRUE);
}

VOID LoadRES()
{
	HBITMAP hBitmap;

	g_hdc = GetDC(hWnd);

	g_hdcBuf = CreateCompatibleDC(g_hdc);
	hBitmap = CreateCompatibleBitmap(g_hdc, CHESS_BG_WIDTH, CHESS_BG_HEIGHT);
	SelectObject(g_hdcBuf, hBitmap);
	DeleteObject(hBitmap);

	g_hdcBG = CreateCompatibleDC(g_hdc);
	hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BG));
	SelectObject(g_hdcBG, hBitmap);
	DeleteObject(hBitmap);

	g_hdcWhite = CreateCompatibleDC(g_hdc);
	hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_WHITE));
	SelectObject(g_hdcWhite, hBitmap);
	DeleteObject(hBitmap);

	g_hdcBlack = CreateCompatibleDC(g_hdc);
	hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BLACK));
	SelectObject(g_hdcBlack, hBitmap);
	DeleteObject(hBitmap);
	return;
}


VOID InitGame()
{
	g_iCount = 0;
	memset(g_intsChess, 0, sizeof(g_intsChess));
	memset(g_intsAIScore, 0, sizeof(g_intsAIScore));
	SetTimer(hWnd, AI_TimerID, 500, &AI);
	return;
}

VOID PaintGame()
{
	BitBlt(g_hdcBuf, 0, 0, CHESS_BG_WIDTH, CHESS_BG_HEIGHT, g_hdcBG, 0, 0, SRCCOPY);
	for (INT x = 0; x < CHESS_WCHESS; x++)
	{
		for (INT y = 0; y < CHESS_HCHESS; y++)
		{
			if (g_intsChess[x][y] % 2 && g_intsChess[x][y]>0)
			{
				TransparentBlt(g_hdcBuf, 35 * x + 5, 35 * y + 5, CHESS_CHESS_WIDTH, CHESS_CHESS_HEIGHT, g_hdcBlack, 0, 0, CHESS_CHESS_WIDTH, CHESS_CHESS_HEIGHT, RGB(0, 0, 0xff));
				continue;
			}
			else if (!(g_intsChess[x][y] % 2) && g_intsChess[x][y]>0)
			{
				TransparentBlt(g_hdcBuf, 35 * x + 5, 35 * y + 5, CHESS_CHESS_WIDTH, CHESS_CHESS_HEIGHT, g_hdcWhite, 0, 0, CHESS_CHESS_WIDTH, CHESS_CHESS_HEIGHT, RGB(0, 0, 0xff));
				continue;
			}

		}
	}

	BitBlt(g_hdc, 0, 0, CHESS_BG_WIDTH, CHESS_BG_HEIGHT, g_hdcBuf, 0, 0, SRCCOPY);
	return;
}

INT GetChessNum(INT x, INT y, Position p, INT chessColor)
{
	int iCount = 0;
	if (p == Position_LEFT)
	{
		if (x > 0)
		{
			while (g_intsChess[x - 1][y] % 2 == chessColor)
			{
				if (g_intsChess[x - 1][y] == 0)
				{
					break;
				}
				iCount++;
				x--;
			}
		}
	}
	else if (p == Position_RIGHT)
	{
		if (x < CHESS_WCHESS - 1)
		{
			while (g_intsChess[x + 1][y] % 2 == chessColor)
			{
				if (g_intsChess[x + 1][y] == 0)
				{
					break;
				}
				iCount++;
				x++;
			}
		}
	}
	else if (p == Position_UP)
	{
		if (y > 0)
		{
			while (g_intsChess[x][y - 1] % 2 == chessColor)
			{
				if (g_intsChess[x][y - 1] == 0)
				{
					break;
				}
				iCount++;
				y--;
			}
		}
	}
	else if (p == Position_DOWN)
	{
		if (y < CHESS_HCHESS - 1)
		{
			while (g_intsChess[x][y + 1] % 2 == chessColor)
			{
				if (g_intsChess[x][y + 1] == 0)
				{
					break;
				}
				iCount++;
				y++;
			}
		}
	}
	else if (p == Position_LEFT_UP)
	{
		if (x > 0 && y > 0)
		{
			while (g_intsChess[x - 1][y - 1] % 2 == chessColor)
			{
				if (g_intsChess[x - 1][y - 1] == 0)
				{
					break;
				}
				iCount++;
				x--;
				y--;
			}
		}
	}
	else if (p == Position_LEFT_DOWN)
	{
		if (x > 0 && y < CHESS_HCHESS - 1)
		{
			while (g_intsChess[x - 1][y + 1] % 2 == chessColor)
			{
				if (g_intsChess[x - 1][y + 1] == 0)
				{
					break;
				}
				iCount++;
				x--;
				y++;
			}
		}
	}
	else if (p == Position_RIGHT_UP)
	{
		if (x < CHESS_WCHESS - 1 && y > 0)
		{
			while (g_intsChess[x + 1][y - 1] % 2 == chessColor)
			{
				if (g_intsChess[x + 1][y - 1] == 0)
				{
					break;
				}
				iCount++;
				x++;
				y--;
			}
		}
	}
	else if (p == Position_RIGHT_DOWN)
	{
		if (x < CHESS_WCHESS - 1 && y < CHESS_HCHESS - 1)
		{
			while (g_intsChess[x + 1][y + 1] % 2 == chessColor)
			{
				if (g_intsChess[x + 1][y + 1] == 0)
				{
					break;
				}
				iCount++;
				x++;
				y++;
			}
		}
	}
	return iCount;
}

BOOL IsWin(INT x, INT y)
{
	if (GetChessNum(x, y, Position_UP, g_intsChess[x][y] % 2) + GetChessNum(x, y, Position_DOWN, g_intsChess[x][y] % 2) + 1 >= CHESS_WINCHESSNUM)
	{
		return TRUE;
	}
	else if (GetChessNum(x, y, Position_LEFT, g_intsChess[x][y] % 2) + GetChessNum(x, y, Position_RIGHT, g_intsChess[x][y] % 2) + 1 >= 5)
	{
		return TRUE;
	}
	else if (GetChessNum(x, y, Position_LEFT_UP, g_intsChess[x][y] % 2) + GetChessNum(x, y, Position_RIGHT_DOWN, g_intsChess[x][y] % 2) + 1 >= 5)
	{
		return TRUE;
	}
	else if (GetChessNum(x, y, Position_LEFT_DOWN, g_intsChess[x][y] % 2) + GetChessNum(x, y, Position_RIGHT_UP, g_intsChess[x][y] % 2) + 1 >= 5)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

enum FreeLevel FreeCalc(INT x, INT y, Position pos, INT calcColor)
{
	INT iFree = 0;
	INT iChessNum;
	iChessNum = GetChessNum(x, y, pos, calcColor);
	if (pos == Position_LEFT)
	{
		if (x > 0)
		{
			if (x + 1 > iChessNum)
			{
				g_intsChess[x - iChessNum - 1][y] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}

	}
	else if (pos == Position_LEFT_UP)
	{
		if (x > 0 && y > 0)
		{
			if (x + 1 > iChessNum && y + 1 > iChessNum)
			{
				g_intsChess[x - iChessNum - 1][y - iChessNum - 1] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}

	}
	else if (pos == Position_UP)
	{
		if (y > 0)
		{
			if (y + 1 > iChessNum)
			{
				g_intsChess[x][y - iChessNum - 1] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}
	}
	else if (pos == Position_LEFT_DOWN)
	{
		if (x > 0 && y < CHESS_HCHESS - 1)
		{
			if (x + 1 > iChessNum && CHESS_HCHESS > iChessNum + y + 1)
			{
				g_intsChess[x + iChessNum + 1][y + iChessNum + 1] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}
	}

	if (pos == Position_LEFT)
	{
		pos = Position_RIGHT;
		iChessNum = GetChessNum(x, y, Position_RIGHT, calcColor);
	}
	else if (pos == Position_UP)
	{
		pos = Position_DOWN;
		iChessNum = GetChessNum(x, y, Position_DOWN, calcColor);
	}
	else if (pos == Position_LEFT_UP)
	{
		pos = Position_RIGHT_DOWN;
		iChessNum = GetChessNum(x, y, Position_RIGHT_DOWN, calcColor);
	}
	else if (pos == Position_LEFT_DOWN)
	{
		pos = Position_RIGHT_UP;
		iChessNum = GetChessNum(x, y, Position_RIGHT_UP, calcColor);
	}


	if (pos == Position_RIGHT_UP)
	{
		if (x < CHESS_WCHESS - 1 && y>0)
		{
			if (y + 1 > iChessNum && CHESS_WCHESS > x + iChessNum + 1 + 1)
			{
				g_intsChess[x - iChessNum - 1][y + iChessNum + 1] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}
	}
	else if (pos == Position_RIGHT)
	{
		if (x < CHESS_WCHESS - 1)
		{
			if (CHESS_WCHESS > iChessNum + x + 1)
			{
				g_intsChess[x + iChessNum + 1][y] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}
	}
	else if (pos == Position_RIGHT_DOWN)
	{
		if (x<CHESS_WCHESS - 1 && y < CHESS_HCHESS - 1)
		{
			if (CHESS_WCHESS > iChessNum + x + 1 && CHESS_HCHESS > iChessNum + y + 1)
			{
				g_intsChess[x + iChessNum + 1][y + iChessNum + 1] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}
	}
	else if (pos == Position_DOWN)
	{
		if (y < CHESS_HCHESS - 1)
		{
			if (CHESS_HCHESS > iChessNum + y + 1)
			{
				g_intsChess[x][y + iChessNum + 1] > 0 ? iFree++ : iFree;
			}
			else
			{
				iFree++;
			}
		}
		else
		{
			iFree++;
		}
	}


	if (iFree == 0)
	{
		return FreeLevel_ALL_FREE;
	}
	else if (iFree == 1)
	{
		return FreeLevel_HALF_FREE;
	}
	else if (iFree == 2)
	{
		return FreeLevel_UN_FREE;
	}
}

VOID Regret()
{
	if (g_iCount > 0)
	{
		for (INT x = 0; x < CHESS_WCHESS; x++)
		{
			for (INT y = 0; y < CHESS_HCHESS; y++)
			{
				if (g_intsChess[x][y] == g_iCount)
				{
					g_intsChess[x][y] = 0;
					g_iCount--;
					return;
				}
			}
		}
	}
	return;
}

VOID CALLBACK AI(HWND, UINT, UINT_PTR, DWORD)
{
	if (g_AITurn)
	{
		g_AITurn = FALSE;
		AIControlCenter();
		PaintGame();
		if (IsWin(g_ptLastPoint.x, g_ptLastPoint.y))
		{
			MessageBoxW(hWnd, g_intsChess[g_ptLastPoint.x][g_ptLastPoint.y] % 2 ? L"你赢了！" : L"你输了，菜逼！", L"游戏结束", MB_OK);
			InitGame();
			PaintGame();
		}

	}
}


//--------------------------------- AI Part ----------------------------------------

//INT AICalcScore(INT x, INT y, BOOL isAI)
//{
//	if (g_intsChess[x][y] > 0)
//	{
//		return 0;
//	}
//
//	INT iAIUpChessNum = GetChessNum(x, y, Position_UP, g_AIColor) + GetChessNum(x, y, Position_DOWN, g_AIColor);
//	INT iAILeftChessNum = GetChessNum(x, y, Position_LEFT, g_AIColor) + GetChessNum(x, y, Position_RIGHT, g_AIColor);
//	INT iAILeftUpChessNum = GetChessNum(x, y, Position_LEFT_UP, g_AIColor) + GetChessNum(x, y, Position_RIGHT_DOWN, g_AIColor);
//	INT iAILeftDownChessNum = GetChessNum(x, y, Position_LEFT_DOWN, g_AIColor) + GetChessNum(x, y, Position_RIGHT_UP, g_AIColor);
//
//	INT iHUUpChessNum = GetChessNum(x, y, Position_UP, g_AIColor ? 0 : 1) + GetChessNum(x, y, Position_DOWN, g_AIColor ? 0 : 1);
//	INT iHULeftChessNum = GetChessNum(x, y, Position_LEFT, g_AIColor ? 0 : 1) + GetChessNum(x, y, Position_RIGHT, g_AIColor ? 0 : 1);
//	INT iHULeftUpChessNum = GetChessNum(x, y, Position_LEFT_UP, g_AIColor ? 0 : 1) + GetChessNum(x, y, Position_RIGHT_DOWN, g_AIColor ? 0 : 1);
//	INT iHULeftDownChessNum = GetChessNum(x, y, Position_LEFT_DOWN, g_AIColor ? 0 : 1) + GetChessNum(x, y, Position_RIGHT_UP, g_AIColor ? 0 : 1);
//
//	INT iAIMax = iAIUpChessNum;
//	INT iHUMax = iHUUpChessNum;
//
//	enum Position posAI = Position_UP;
//	enum Position posHU = Position_UP;
//
//	if (iAIMax < iAILeftChessNum || iHUMax < iHULeftChessNum)
//	{
//		if (iAIMax < iAILeftChessNum)
//		{
//			iAIMax = iAILeftChessNum;
//			posAI = Position_LEFT;
//		}
//		if (iHUMax < iHULeftChessNum)
//		{
//			iHUMax = iHULeftChessNum;
//			posHU = Position_LEFT;
//		}
//	}
//	if (iAIMax < iAILeftUpChessNum || iHUMax < iHULeftUpChessNum)
//	{
//		if (iAIMax < iAILeftUpChessNum)
//		{
//			iAIMax = iAILeftUpChessNum;
//			posAI = Position_LEFT_UP;
//		}
//		if (iHUMax < iHULeftUpChessNum)
//		{
//			iHUMax = iHULeftUpChessNum;
//			posHU = Position_LEFT_UP;
//		}
//	}
//	if (iAIMax < iAILeftDownChessNum || iHUMax < iHULeftDownChessNum)
//	{
//		if (iAIMax < iAILeftDownChessNum)
//		{
//			iAIMax = iAILeftDownChessNum;
//			posAI = Position_LEFT_DOWN;
//		}
//		if (iHUMax < iHULeftDownChessNum)
//		{
//			iHUMax = iHULeftDownChessNum;
//			posHU = Position_LEFT_DOWN;
//		}
//	}
//
//	enum FreeLevel AIFreeLevel = FreeCalc(x, y, posAI, isAI ? g_AIColor : (g_AIColor ? 0 : 1));
//	enum FreeLevel AIUPFreeLevel = FreeCalc(x, y, Position_UP, isAI ? g_AIColor : (g_AIColor ? 0 : 1));
//	enum FreeLevel AILeftFreeLevel = FreeCalc(x, y, Position_LEFT, isAI ? g_AIColor : (g_AIColor ? 0 : 1));
//	enum FreeLevel AILeftUpFreeLevel = FreeCalc(x, y, Position_LEFT_UP, isAI ? g_AIColor : (g_AIColor ? 0 : 1));
//	enum FreeLevel AILeftDownFreeLevel = FreeCalc(x, y, Position_LEFT_DOWN, isAI ? g_AIColor : (g_AIColor ? 0 : 1));
//
//	enum FreeLevel HUFreeLevel = FreeCalc(x, y, posHU, isAI ? (g_AIColor ? 0 : 1) : g_AIColor);
//	enum FreeLevel HUUPFreeLevel = FreeCalc(x, y, Position_UP, isAI ? (g_AIColor ? 0 : 1) : g_AIColor);
//	enum FreeLevel HULeftFreeLevel = FreeCalc(x, y, Position_LEFT, isAI ? (g_AIColor ? 0 : 1) : g_AIColor);
//	enum FreeLevel HULeftUpFreeLevel = FreeCalc(x, y, Position_LEFT_UP, isAI ? (g_AIColor ? 0 : 1) : g_AIColor);
//	enum FreeLevel HULeftDownFreeLevel = FreeCalc(x, y, Position_LEFT_DOWN, isAI ? (g_AIColor ? 0 : 1) : g_AIColor);
//	if (iAIMax >= 5)
//	{
//		return isAI ? AI_CAUTION_1 : HU_CAUTION_1;
//	}
//	else if ((iAIMax < 5 && iAIMax >= 4) || (iHUMax < 5 && iHUMax >= 4))
//	{
//
//		//双死4
//		if (posAI == Position_LEFT || posHU == Position_LEFT)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return  isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftUpFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftDownFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HUUPFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return  isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftUpFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftDownFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//		}
//		else if (posAI == Position_UP || posHU == Position_UP)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftUpFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftDownFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftUpFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftDownFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//		}
//		else if (posAI == Position_LEFT_UP || posHU == Position_LEFT_UP)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftDownFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HUUPFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftDownFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftUpChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//		}
//		else if (posAI == Position_LEFT_DOWN || posHU == Position_LEFT_DOWN)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftDownChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftUpFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftDownChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftDownChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HUUPFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftDownChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftUpFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftDownChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftFreeLevel == FreeLevel_HALF_FREE&&iAIMax == iHULeftDownChessNum)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//		}
//
//		//
//		//死4活3
//		if (posAI == Position_LEFT || posHU == Position_LEFT)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HUUPFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftUpFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftDownFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//		}
//		else if (posAI == Position_UP || posHU == Position_UP)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftUpFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftDownFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//		}
//		else if (posAI == Position_LEFT_UP || posHU == Position_LEFT_UP)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HUUPFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftDownFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//		}
//		else if (posAI == Position_LEFT_DOWN || posHU == Position_LEFT_DOWN)
//		{
//			if (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (AIFreeLevel == FreeLevel_HALF_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//
//			if (HUFreeLevel == FreeLevel_HALF_FREE&&HUUPFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftUpFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//			else if (HUFreeLevel == FreeLevel_HALF_FREE&&HULeftFreeLevel == FreeLevel_ALL_FREE&&iHULeftUpChessNum == 3)
//			{
//				return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//			}
//		}
//		//活4
//		if (AIFreeLevel == FreeLevel_ALL_FREE || HUFreeLevel == FreeLevel_ALL_FREE)
//		{
//			return isAI ? AI_CAUTION_2 : HU_CAUTION_2;
//		}
//		//死4
//		if (AIFreeLevel == FreeLevel_HALF_FREE || HUFreeLevel == FreeLevel_HALF_FREE)
//		{
//			return isAI ? AI_CAUTION_5 : HU_CAUTION_5;
//		}
//
//	}
//	else if ((iAIMax < 4 && iAIMax >= 3) || (iHUMax < 4 && iHUMax >= 3))
//	{
//
//		//双活三
//		if ((posAI == Position_LEFT || posHU == Position_LEFT) && iAIMax == iHUMax)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//
//			if (HUFreeLevel == FreeLevel_ALL_FREE&&HUUPFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftUpFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftDownFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//		}
//		else if (posAI == Position_UP || posHU == Position_UP)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//
//			if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftUpFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftDownFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//		}
//		else if (posAI == Position_LEFT_UP || posHU == Position_LEFT_UP)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//
//			if (HUFreeLevel == FreeLevel_ALL_FREE&&HUUPFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftDownFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//		}
//		else if (posAI == Position_LEFT_DOWN || posHU == Position_LEFT_DOWN)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//
//			if (HUFreeLevel == FreeLevel_ALL_FREE&&HUUPFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftUpFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//			else if (HUFreeLevel == FreeLevel_ALL_FREE&&HULeftFreeLevel == FreeLevel_ALL_FREE)
//			{
//				return isAI ? AI_CAUTION_3 : HU_CAUTION_3;
//			}
//		}
//
//		//死三活三-------------------------------
//		if ((posAI == Position_LEFT || posHU == Position_LEFT) && iAIMax == iHUMax)
//		{
//			if ((AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE && iAIUpChessNum == 3) || (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE && iAIUpChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//		}
//		else if (posAI == Position_UP)
//		{
//			if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//		}
//		else if (posAI == Position_LEFT_UP)
//		{
//			if ((AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE && iAIUpChessNum == 3) || (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE && iAIUpChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//		}
//		else if (posAI == Position_LEFT_DOWN)
//		{
//			if ((AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE && iAIUpChessNum == 3) || (AIFreeLevel == FreeLevel_HALF_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE && iAIUpChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//			else if ((AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 3) || (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 3))
//			{
//				return isAI ? AI_CAUTION_4 : HU_CAUTION_4;
//			}
//		}
//		//活三
//		if (AIFreeLevel == FreeLevel_ALL_FREE || HUFreeLevel == FreeLevel_ALL_FREE)
//		{
//			return isAI ? AI_CAUTION_6 : HU_CAUTION_6;
//		}
//		if (AIFreeLevel == FreeLevel_HALF_FREE || HUFreeLevel == FreeLevel_HALF_FREE)
//		{
//			return isAI ? AI_CAUTION_8 : HU_CAUTION_8;
//		}
//
//	}
//	else if (iAIMax < 3 && iAIMax >= 2)
//	{
//		//双活2
//		if (posAI == Position_LEFT)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE&&iAIUpChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//		}
//		else if (posAI == Position_UP)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//		}
//		else if (posAI == Position_LEFT_UP)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE&&iAIUpChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftDownFreeLevel == FreeLevel_ALL_FREE&&iAILeftDownChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//		}
//		else if (posAI == Position_LEFT_DOWN)
//		{
//			if (AIFreeLevel == FreeLevel_ALL_FREE&&AIUPFreeLevel == FreeLevel_ALL_FREE&&iAIUpChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftUpFreeLevel == FreeLevel_ALL_FREE&&iAILeftUpChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//			else if (AIFreeLevel == FreeLevel_ALL_FREE&&AILeftFreeLevel == FreeLevel_ALL_FREE&&iAILeftChessNum == 2)
//			{
//				return isAI ? AI_CAUTION_7 : HU_CAUTION_7;
//			}
//		}
//		//活2
//		if (AIFreeLevel == FreeLevel_HALF_FREE)
//		{
//			return isAI ? AI_CAUTION_10 : HU_CAUTION_10;
//		}
//		//死2
//		if (AIFreeLevel == FreeLevel_HALF_FREE)
//		{
//			return isAI ? AI_CAUTION_11 : HU_CAUTION_11;
//		}
//	}
//	if (iHUMax == 1)
//	{
//		if (AIFreeLevel == FreeLevel_ALL_FREE)
//		{
//			return isAI ? AI_CAUTION_11 : HU_CAUTION_11;
//		}
//	}
//
//	return 0;
//}

INT AICalcScore(INT x, INT y, BOOL isAI)
{
	INT iScore = 0;

	INT iAIUpChessNum = GetChessNum(x, y, Position_UP, g_AIColor);
	INT iAIDownChessNum = GetChessNum(x, y, Position_DOWN, g_AIColor);
	INT iAILeftChessNum = GetChessNum(x, y, Position_LEFT, g_AIColor);
	INT iAIRightChessNum = GetChessNum(x, y, Position_RIGHT, g_AIColor);
	INT iAILeftUpChessNum = GetChessNum(x, y, Position_LEFT_UP, g_AIColor);
	INT iAILeftDownChessNum = GetChessNum(x, y, Position_LEFT_DOWN, g_AIColor);
	INT iAIRightUpChessNum = GetChessNum(x, y, Position_RIGHT_UP, g_AIColor);
	INT iAIRightDownChessNum = GetChessNum(x, y, Position_RIGHT_DOWN, g_AIColor);

	INT iAIUpState = FreeCalc(x, y, Position_UP, g_AIColor);
	//INT iAIDownState = FreeCalc(x, y, Position_DOWN, g_AIColor);
	INT iAILeftState = FreeCalc(x, y, Position_LEFT, g_AIColor);
	//INT iAIRightState = FreeCalc(x, y, Position_RIGHT, g_AIColor);
	INT iAILeftUpState = FreeCalc(x, y, Position_LEFT_UP, g_AIColor);
	INT iAILeftDownState = FreeCalc(x, y, Position_LEFT_DOWN, g_AIColor);
	//INT iAIRightUpState = FreeCalc(x, y, Position_RIGHT_UP, g_AIColor);
	//INT iAIRightDownState = FreeCalc(x, y, Position_RIGHT_DOWN, g_AIColor);


	INT iHUUpChessNum = GetChessNum(x, y, Position_UP, g_AIColor ? 0 : 1);
	INT iHUDownChessNum = GetChessNum(x, y, Position_DOWN, g_AIColor ? 0 : 1);
	INT iHULeftChessNum = GetChessNum(x, y, Position_LEFT, g_AIColor ? 0 : 1);
	INT iHURightChessNum = GetChessNum(x, y, Position_RIGHT, g_AIColor ? 0 : 1);
	INT iHULeftUpChessNum = GetChessNum(x, y, Position_LEFT_UP, g_AIColor ? 0 : 1);
	INT iHULeftDownChessNum = GetChessNum(x, y, Position_LEFT_DOWN, g_AIColor ? 0 : 1);
	INT iHURightUpChessNum = GetChessNum(x, y, Position_RIGHT_UP, g_AIColor ? 0 : 1);
	INT iHURightDownChessNum = GetChessNum(x, y, Position_RIGHT_DOWN, g_AIColor ? 0 : 1);

	INT iHUUpState = FreeCalc(x, y, Position_UP, g_AIColor ? 0 : 1);
	//INT iHUDownState = FreeCalc(x, y, Position_DOWN, g_AIColor ? 0 : 1);
	INT iHULeftState = FreeCalc(x, y, Position_LEFT, g_AIColor ? 0 : 1);
	//INT iHURightState = FreeCalc(x, y, Position_RIGHT, g_AIColor ? 0 : 1);
	INT iHULeftUpState = FreeCalc(x, y, Position_LEFT_UP, g_AIColor ? 0 : 1);
	INT iHULeftDownState = FreeCalc(x, y, Position_LEFT_DOWN, g_AIColor ? 0 : 1);
	//INT iHURightUpState = FreeCalc(x, y, Position_RIGHT_UP, g_AIColor ? 0 : 1);
	//INT iHURightDownState = FreeCalc(x, y, Position_RIGHT_DOWN, g_AIColor ? 0 : 1);


	if (g_intsChess[x][y] > 0)
	{
		return iScore;
	}
	if (isAI)
	{
		//连五
		if (iAIUpChessNum + iAIDownChessNum + 1 == 5)//|
		{
			iScore += AI_FIVE;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 5)//--
		{
			iScore += AI_FIVE;
		}
		if (iAILeftUpChessNum + iAIRightDownChessNum + 1 == 5)//  /
		{
			iScore += AI_FIVE;
		}
		if (iAILeftDownChessNum + iAIRightUpChessNum + 1 == 5)// \           
		{
			iScore += AI_FIVE;
		}
		//活四
		if (iAIUpChessNum + iAIDownChessNum + 1 == 4 && iAIUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_FOUR;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 4 && iAILeftState == FreeLevel_ALL_FREE)
		{
			iScore += AI_FOUR;
		}
		if (iAILeftUpChessNum + iAIRightDownChessNum + 1 == 4 && iAILeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_FOUR;
		}
		if (iAILeftDownChessNum + iAIRightUpChessNum + 1 == 4 && iAILeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += AI_FOUR;
		}

		//眠四
		if (iAIUpChessNum + iAIDownChessNum + 1 == 4 && iAIUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_FOUR;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 4 && iAILeftState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_FOUR;
		}
		if (iAILeftUpChessNum + iAIRightDownChessNum + 1 == 4 && iAILeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_FOUR;
		}
		if (iAILeftDownChessNum + iAIRightUpChessNum + 1 == 4 && iAILeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_FOUR;
		}

		//活三
		if (iAIUpChessNum + iAIDownChessNum + 1 == 3 && iAIUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_THREE;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 3 && iAILeftState == FreeLevel_ALL_FREE)
		{
			iScore += AI_THREE;
		}
		if (iAILeftUpChessNum + iAIDownChessNum + 1 == 3 && iAILeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_THREE;
		}
		if (iAILeftDownChessNum + iAIDownChessNum + 1 == 3 && iAILeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += AI_THREE;
		}

		//眠三
		if (iAIUpChessNum + iAIDownChessNum + 1 == 3 && iAIUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_THREE;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 3 && iAILeftState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_THREE;
		}
		if (iAILeftUpChessNum + iAIDownChessNum + 1 == 3 && iAILeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_THREE;
		}
		if (iAILeftDownChessNum + iAIDownChessNum + 1 == 3 && iAILeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_THREE;
		}

		//活二
		if (iAIUpChessNum + iAIDownChessNum + 1 == 2 && iAIUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_TWO;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 2 && iAILeftState == FreeLevel_ALL_FREE)
		{
			iScore += AI_TWO;
		}
		if (iAILeftUpChessNum + iAIDownChessNum + 1 == 2 && iAILeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_TWO;
		}
		if (iAILeftDownChessNum + iAIDownChessNum + 1 == 2 && iAILeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += AI_TWO;
		}
		//眠二
		if (iAIUpChessNum + iAIDownChessNum + 1 == 2 && iAIUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_TWO;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 2 && iAILeftState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_TWO;
		}
		if (iAILeftUpChessNum + iAIDownChessNum + 1 == 2 && iAILeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_TWO;
		}
		if (iAILeftDownChessNum + iAIDownChessNum + 1 == 2 && iAILeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_TWO;
		}
		//活一
		if (iAIUpChessNum + iAIDownChessNum + 1 == 1 && iAIUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_ONE;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 1 && iAILeftState == FreeLevel_ALL_FREE)
		{
			iScore += AI_ONE;
		}
		if (iAILeftUpChessNum + iAIDownChessNum + 1 == 1 && iAILeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += AI_ONE;
		}
		if (iAILeftDownChessNum + iAIDownChessNum + 1 == 1 && iAILeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += AI_ONE;
		}

		//眠一
		if (iAIUpChessNum + iAIDownChessNum + 1 == 1 && iAIUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_ONE;
		}
		if (iAILeftChessNum + iAIRightChessNum + 1 == 1 && iAILeftState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_ONE;
		}
		if (iAILeftUpChessNum + iAIDownChessNum + 1 == 1 && iAILeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_ONE;
		}
		if (iAILeftDownChessNum + iAIDownChessNum + 1 == 1 && iAILeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += AI_BLOCKED_ONE;
		}


		//防止ai喜欢在棋盘边下棋
		if (x < 3 || x>11)
		{
			iScore -= 5;
		}
		if (y < 3 || y>11)
		{
			iScore -= 5;
		}
		if (g_iCount < 3)
		{
			if ((x > 5 && x < 9) && (y>5 && y < 9))
			{
				iScore += 5 * AI_ONE;
			}
		}
	}
	else
	{
		//连五
		if (iHUUpChessNum + iHUDownChessNum + 1 == 5)//|
		{
			iScore += HU_FIVE;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 5)//--
		{
			iScore += HU_FIVE;
		}
		if (iHULeftUpChessNum + iHURightDownChessNum + 1 == 5)//  /
		{
			iScore += HU_FIVE;
		}
		if (iHULeftDownChessNum + iHURightUpChessNum + 1 == 5)// \           
		{
			iScore += HU_FIVE;
		}
		//活四
		if (iHUUpChessNum + iHUDownChessNum + 1 == 4 && iHUUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_FOUR;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 4 && iHULeftState == FreeLevel_ALL_FREE)
		{
			iScore += HU_FOUR;
		}
		if (iHULeftUpChessNum + iHURightDownChessNum + 1 == 4 && iHULeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_FOUR;
		}
		if (iHULeftDownChessNum + iHURightUpChessNum + 1 == 4 && iHULeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += HU_FOUR;
		}

		//眠四
		if (iHUUpChessNum + iHUDownChessNum + 1 == 4 && iHUUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_FOUR;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 4 && iHULeftState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_FOUR;
		}
		if (iHULeftUpChessNum + iHURightDownChessNum + 1 == 4 && iHULeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_FOUR;
		}
		if (iHULeftDownChessNum + iHURightUpChessNum + 1 == 4 && iHULeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_FOUR;
		}

		//活三
		if (iHUUpChessNum + iHUDownChessNum + 1 == 3 && iHUUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_THREE;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 3 && iHULeftState == FreeLevel_ALL_FREE)
		{
			iScore += HU_THREE;
		}
		if (iHULeftUpChessNum + iHUDownChessNum + 1 == 3 && iHULeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_THREE;
		}
		if (iHULeftDownChessNum + iHUDownChessNum + 1 == 3 && iHULeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += HU_THREE;
		}

		//眠三
		if (iHUUpChessNum + iHUDownChessNum + 1 == 3 && iHUUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_THREE;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 3 && iHULeftState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_THREE;
		}
		if (iHULeftUpChessNum + iHUDownChessNum + 1 == 3 && iHULeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_THREE;
		}
		if (iHULeftDownChessNum + iHUDownChessNum + 1 == 3 && iHULeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_THREE;
		}

		//活二
		if (iHUUpChessNum + iHUDownChessNum + 1 == 2 && iHUUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_TWO;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 2 && iHULeftState == FreeLevel_ALL_FREE)
		{
			iScore += HU_TWO;
		}
		if (iHULeftUpChessNum + iHUDownChessNum + 1 == 2 && iHULeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_TWO;
		}
		if (iHULeftDownChessNum + iHUDownChessNum + 1 == 2 && iHULeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += AI_TWO;
		}
		//眠二
		if (iHUUpChessNum + iHUDownChessNum + 1 == 2 && iHUUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_TWO;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 2 && iHULeftState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_TWO;
		}
		if (iHULeftUpChessNum + iHUDownChessNum + 1 == 2 && iHULeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_TWO;
		}
		if (iHULeftDownChessNum + iHUDownChessNum + 1 == 2 && iHULeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_TWO;
		}
		//活一
		if (iHUUpChessNum + iHUDownChessNum + 1 == 1 && iHUUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_ONE;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 1 && iHULeftState == FreeLevel_ALL_FREE)
		{
			iScore += HU_ONE;
		}
		if (iHULeftUpChessNum + iHUDownChessNum + 1 == 1 && iHULeftUpState == FreeLevel_ALL_FREE)
		{
			iScore += HU_ONE;
		}
		if (iHULeftDownChessNum + iHUDownChessNum + 1 == 1 && iHULeftDownState == FreeLevel_ALL_FREE)
		{
			iScore += HU_ONE;
		}

		//眠一
		if (iHUUpChessNum + iHUDownChessNum + 1 == 1 && iHUUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_ONE;
		}
		if (iHULeftChessNum + iHURightChessNum + 1 == 1 && iHULeftState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_ONE;
		}
		if (iHULeftUpChessNum + iHUDownChessNum + 1 == 1 && iHULeftUpState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_ONE;
		}
		if (iHULeftDownChessNum + iHUDownChessNum + 1 == 1 && iHULeftDownState == FreeLevel_HALF_FREE)
		{
			iScore += HU_BLOCKED_ONE;
		}


		//防止ai喜欢在棋盘边下棋
		if (x < 3 || x>11)
		{
			iScore += 5;
		}
		if (y < 3 || y>11)
		{
			iScore += 5;
		}
		if (g_iCount < 3)
		{
			if ((x > 5 && x < 9) && (y>5 && y < 9))
			{
				iScore -= 5 * AI_ONE;
			}
		}
	}
	return iScore;
}

INT MAX(INT ints[][15])
{
	INT iMax = 0;
	for (INT i = 0; i < CHESS_WCHESS; i++)
	{
		for (INT j = 0; j < CHESS_HCHESS; j++)
		{
			if (iMax < ints[i][j])
			{
				iMax = ints[i][j];
			}
		}
	}
	return iMax;
}

INT MIN(INT ints[][15])
{
	INT iMin = 0;
	for (INT i = 0; i < CHESS_WCHESS; i++)
	{
		for (INT j = 0; j < CHESS_HCHESS; j++)
		{
			if (iMin > ints[i][j])
			{
				iMin = ints[i][j];
			}
		}
	}
	return iMin;
}

VOID AICalc()
{
	for (INT x = 0; x < CHESS_WCHESS; x++)
	{
		for (INT y = 0; y < CHESS_HCHESS; y++)
		{
			g_intsAIScore[x][y] = AICalcScore(x, y, TRUE);
		}
	}
}

VOID AIMakeDecision()
{
	INT64 iMax = 0;
	for (INT x = 0; x < CHESS_WCHESS; x++)
	{
		for (INT y = 0; y < CHESS_HCHESS; y++)
		{
			if (g_intsAIScore[x][y] >= iMax)
			{
				if (g_intsChess[x][y]>0)
				{
					continue;
				}
				iMax = g_intsAIScore[x][y];
			}
		}
	}
	for (INT x = 0; x < CHESS_WCHESS; x++)
	{
		for (INT y = 0; y < CHESS_HCHESS; y++)
		{
			if (g_intsAIScore[x][y] == iMax)
			{
				g_ptLastPoint = { x,y };
				break;
			}
		}
	}
}

VOID AIControlCenter()
{
	AICalc();
	AIMakeDecision();
	g_intsChess[g_ptLastPoint.x][g_ptLastPoint.y] = g_iCount + 1;
	g_iCount++;
}