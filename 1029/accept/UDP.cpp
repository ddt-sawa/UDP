#include"UDP.h"
#include<iostream>
#include<process.h> //スレッド関係のライブラリ
#include<string>
#include<iostream>
#include<Winsock2.h>

//初着タイマーの猶予
#define TIMER_LENGTH 100000

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char szClassNme[] = "ウィンドウクラス・ネーム";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst,
	LPSTR lpszCmdLine, int nCmdShow)
{
	//メッセージ
	MSG msg;
	//ウィンドウの設定
	WNDCLASS myProg;
	if (!hPreInst) {
		//水平・垂直でウィンドウサイズ変更があった時書き直す
		myProg.style = CS_HREDRAW | CS_VREDRAW;
		//ウィンドウプロシージャの名前を決める
		myProg.lpfnWndProc = WndProc;
		//メモリ領域追加しない
		myProg.cbClsExtra = 0;
		//メモリ領域追加しない
		myProg.cbWndExtra = 0;
		//インスタンス登録
		myProg.hInstance = hInstance;
		//アイコン設定なし
		myProg.hIcon = NULL;
		//カーソル設定
		myProg.hCursor = LoadCursor(NULL, IDC_ARROW);
		//背景登録
		myProg.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		//メニュー登録
		myProg.lpszMenuName = NULL;
		//クラス名登録
		myProg.lpszClassName = szClassNme;
		if (!RegisterClass(&myProg))
		//登録が上手くいかなかった場合異常終了
		if (!RegisterClass(&myProg)) {
			return false;
		}
	}
	//ウィンドウハンドルにウィンドウを設定
	hWnd = CreateWindow(

		//クラス名
		szClassNme,
		//タイトル
		"UDP通信",
		//ウィンドウスタイル
		WS_CLIPCHILDREN,
		//x座標
		CW_USEDEFAULT,
		//y座標
		CW_USEDEFAULT,
		//横幅
		CW_USEDEFAULT,
		//高さ
		CW_USEDEFAULT,
		//親ウィンドウハンドル
		NULL,
		//メニューハンドル
		NULL,
		//インスタンスハンドル
		hInstance,
		//ウィンドウ作成データ
		NULL);
	//ウィンドウを開く、通常モードで
	ShowWindow(hWnd, SW_SHOW);
	//プロシージャにWM_PANINメッセージを送る
	UpdateWindow(hWnd);

	//メッセージループ作成
	while (GetMessage(&msg, NULL, 0, 0)) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (msg.wParam);
}


void ShowMyText(HWND hWnd,const char* printedMessage)
{
	HDC hdc;
	PAINTSTRUCT paint;

	hdc = BeginPaint(hWnd, &paint);
	TextOut(hdc, 10, 10, (LPCSTR)printedMessage, strlen(printedMessage));
	EndPaint(hWnd, &paint);
	return;
}

//コールバック関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

	//ウィンドウ作成メッセージが来たら
	case WM_CREATE:

		//パケット受信クラスを作成する
		UDPPacketAcceptClass myClass;
		//パケット受信用のスレッドを作成する
		myClass.openThread();
		//タイマーをセットする
		if (SetTimer(hWnd, ID_MYTIMER,TIMER_LENGTH, NULL) == 0) {
			//タイマーがセットできなかった場合異常終了
			exit(EXIT_FAILURE);
		}
		break;

	//ウィンドウ破棄メッセージが来たら
	case WM_DESTROY:
		//ウィンドウを破棄
		PostQuitMessage(0);
		break;

	//書き直しメッセージが来たら
	case WM_PAINT:
		//書き直しを今のウィンドウに行う
		ShowMyText(hWnd,"UDP通信");
		break;

	//タイマーの時間切れを感知した場合
	case WM_TIMER:
		//タイマーを止める
		KillTimer(hWnd, ID_MYTIMER);
		//メッセージの表示
		MessageBox(hWnd,
			(LPCSTR)"10秒間パケットが届かなかったため、タイムアウトしました。",
			(LPCSTR)"UDP通信",
			MB_OK | MB_ICONEXCLAMATION);
		//UDP受信スレッドを終了する
		myClass.closeThread();
		break;

	//UDPパケットを受信した場合
	case  WM_ACCEPT_UDPPACKET:
		//ダイアログボックスに受け取ったデータ本体を表示する
				MessageBox(hWnd,
			(LPCSTR)&globalQueue.back().mSendData[0],
			(LPCSTR)"UDP通信",
			MB_OK | MB_ICONEXCLAMATION);
		break;

	//以上のいずれにも該当しない場合
	default:
		//デフォルト処理を行う
		return(DefWindowProc(hWnd, msg, wParam, lParam));
	}

	return (0L);
}