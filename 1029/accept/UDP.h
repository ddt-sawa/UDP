#ifndef ___UDPClass
#define ___UDPClass
#include"CRC.h"
#include<iostream>
#include<process.h> //スレッド関係のライブラリ
#include<string>
#include<iostream>
#include<Winsock2.h>
#include<queue>
#define WM_ACCEPT_UDPPACKET (WM_APP + 100)

HWND hWnd;


//UDPPacket構造体
struct UDPPacket {

	//公開メンバ
public:

	//送信側ポート番号
	uint16_t mSendPortNumber;

	//受信側ポート番号
	uint16_t mAcceptPortNumber;

	//パケットサイズ(バイト)
	uint16_t PacketSize;

	//チェックサム
	uint16_t mCRCValue;

	//データ本体
	std::string mSendData;

};

//パケットチェッククラスで使用する構造体
struct UDPPacketCheck {

	//UDPPacket構造体
	UDPPacket mUDPPacket;

	//送信側IPアドレス
	uint16_t mSendIPAddress;

};

//GUI表示用構造体
struct  GUIPacket {

	//公開メンバ
public:

	//送信側IPアドレス
	uint16_t mSendIP;

	//送信側ポート番号
	uint16_t mSendPortNumber;

	//データ本体
	std::string mSendData;
};

//UDPPacket構造体を格納するキュー
std::queue<UDPPacket> globalQueue;
//タイマー1のID
# define ID_MYTIMER 1
//タイマー1の猶予
# define TIME_OUT_LENGTH 10000
//UDP通信で送れる限界バイト数（仮）
#define UDP_LIMIT 1500
//UDPヘッダのサイズ
#define UDP_HEADER_SIZE 64 

//UDPPacket受信クラス
class UDPPacketAcceptClass {

private:
	HANDLE mThreadID;

public:

	/**
	* UDPPacket受信クラスのコンストラクタ
	* @param 親ウィンドウハンドル
	* @author Sawa
	* @since 10.29
	*/
	//UDPPacketAcceptClass(HWND hWnd) : mhWnd(hWnd) {};

	/**
	* staticでないメンバ関数acceptUDPをスレッドで呼び出すためのデリゲート
	* 参考サイトhttp://d.hatena.ne.jp/kasei_san/20070612/p1
	* @param agrs _beginthreadが要求する型に合わせるための仮の引数
	* @author Sawa
	* @since 10.29
	*/
	static  unsigned int _stdcall Launcher(void* args) {

		reinterpret_cast<UDPPacketAcceptClass*>(args)->acceptUDP(NULL);

		return 0;

	}

public:
	/**
	* UDP受信関数
	* param args _beginthreadの引数に型を合わせるためのポインタ
	* @author Sawa
	* @since 10.29
	*/
	void acceptUDP(void* args)
	{
		//概要説明
		std::cout << "これはUDP受信用アプリです。\n";

		//UDP受信を行うループ
		do {
			//WinSockを使う準備をするクラス(取り合えずバージョンだけWSAStartアップで動かせばよい)

			WSAData wsaData;

			//ソケット
			SOCKET sock;

			//ソケットのアドレス指定を行う構造体
			struct sockaddr_in addr;

			//受け取るデータ
			UDPPacket getData;

			//DLLの初期化を行い、失敗した場合エラーフローに移行
			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {

				//WASAtartup関数でのエラー。エラー番号表示
				std::cout << "WSAStartupエラー" << WSAGetLastError();

				//強制終了
				exit(EXIT_FAILURE);
			}

			//ソケットクラスにIPアドレス、通信方法、プロトコルを代入。失敗した場合エラーフローに移行
			if ((sock = ::socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {

				//socket関数でのエラー。エラー番号表示
				std::cout << "socketエラー" << WSAGetLastError();

				//強制終了
				exit(EXIT_FAILURE);
			}

			//アドレスファミリ(プロトコル集の名前空間)
			addr.sin_family = AF_INET;

			//送信側ポートの指定(ネットワークバイトオーダーに変換)
			addr.sin_port = htons(12345);

			//送信側ポートの指定（UDPなので誰でもよい）
			addr.sin_addr.S_un.S_addr = INADDR_ANY;

			//ソケットに名前を付ける。失敗した場合エラーフローに移行
			if (::bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {

				//bind関数でエラーが発生した際
				std::cout << "bindエラー" << WSAGetLastError();

				//強制終了
				exit(EXIT_FAILURE);
			}

			//相手方ソケットのアドレス情報を保持する構造体
			struct sockaddr_in sendAddr;


			int sendAddrLen = sizeof(sendAddr);

			//バッファを定義
			char acceptBuffer[UDP_LIMIT] = { 0 };

			//UDPパケット受信(MSG_PEEKは受け取ったデータを削除しないオプション)
			int acceptDataSize = ::recvfrom(sock, acceptBuffer, UDP_LIMIT, MSG_PEEK, (struct sockaddr *)&sendAddr, &sendAddrLen);

			//初着タイマーを止める
			KillTimer(hWnd, ID_MYTIMER);

			//以下、データの妥当性3チェックを行う

			//受信データサイズとUDPパケットヘッダに記載されている送信データサイズが異なっていた場合エラー
			if (acceptDataSize != (reinterpret_cast<uint16_t*>(acceptBuffer)[2])) {

				//データサイズエラーが生じたことを通知
				std::cout << "データサイズエラー" << WSAGetLastError();

				//異常終了
				exit(EXIT_FAILURE);
			}

			//受信データから計算したCRC値とUDPパケットヘッダに記載されているCRC値が異なっていた場合エラー
			if (calculateCRC16(&acceptBuffer[UDP_HEADER_SIZE + 1], acceptDataSize - UDP_HEADER_SIZE) != (reinterpret_cast<uint16_t*>(acceptBuffer)[3])) {

				//CRC値エラーが生じたことを通知
				std::cout << "CRC値エラー" << WSAGetLastError();

				//異常終了
				exit(EXIT_FAILURE);

			}

			//送信側ポート番号を格納
			getData.mSendPortNumber = (reinterpret_cast<uint16_t*>(acceptBuffer)[0]);

			//受信側ポート番号を格納
			getData.mAcceptPortNumber = (reinterpret_cast<uint16_t*>(acceptBuffer)[1]);

			//パケットサイズを格納
			getData.PacketSize = (reinterpret_cast<uint16_t*>(acceptBuffer)[2]);

			//CRC値を格納
			getData.mCRCValue = (reinterpret_cast<uint16_t*>(acceptBuffer)[3]);

			//データ本体を格納するループ
			for (int firstCounter = UDP_HEADER_SIZE + 1; firstCounter < getData.PacketSize + 1; firstCounter++) {

				//一文字ずつ代入
				getData.mSendData.push_back(acceptBuffer[firstCounter]);

			}
			//受け取ったUDPパケットをキューに格納
			globalQueue.push(getData);

			//GUI部にUDPパケットを受け取ったことを通知
			SendMessage(hWnd, WM_ACCEPT_UDPPACKET, 0, 0);

			//ソケットを閉じる
			closesocket(sock);

			//Winsock機能を終了する
			WSACleanup();

			//無限ループ
		} while (true);
	}
public:
	/**
	* UDP受信スレッドを起動する
	* @author Sawa
	* @since 10.29
	*/
	void openThread() {

		//UDP受信スレッド起動
		mThreadID = (HANDLE)_beginthreadex(NULL, 0, &UDPPacketAcceptClass::Launcher, 0, 0, NULL);

	}

public:
	/**
	* UDP受信スレッドを終了する
	* @author Sawa
	* @since 10.29
	*/
	void closeThread() {

		//UDP受信スレッドを終了
		CloseHandle(mThreadID);
	}
};



#endif