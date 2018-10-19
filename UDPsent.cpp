#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

//送信先IPアドレス
const char* acceptIP = "192.168.1.109";

int main()
{
	//ソケット情報を格納するクラス(取り合えずバージョンだけWSAStartアップで動かせばよい)
	WSAData wsaData;

	//ソケット
	SOCKET sock;

	//ソケットのアドレス指定を行う構造体
	struct sockaddr_in addr;

	/*Winsockを初期化する関数
	winsockのバージョン(2,0)を指定し、
	それらの情報を構造体に格納する。
	エラーが発生したときは0以外の値を返す
	*/
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	//ソケットクラスにIPアドレス、通信方法、プロトコルを代入
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	//アドレスファミリ
	addr.sin_family = AF_INET;

	//相手方ポートの指定
	addr.sin_port = htons(12345);

	//相手のIPアドレスの指定。inet_addr()は引数をネットワークバイトオーダーに変更する
	addr.sin_addr.S_un.S_addr = inet_addr(acceptIP);

	/*
	sendto()はもう一方のソケットへメッセージを転送するのに使用される
	 ソケット名、バッファ、文字サイズ、フラグ、ターゲットアドレス、ターゲットサイズ
	*/
	sendto(sock, "I'm Sawa.", 10, 0, (struct sockaddr *)&addr, sizeof(addr));

	//ソケットを閉じる
	closesocket(sock);

	//Winsock機能を終了する
	WSACleanup();

	//正常終了
	return 0;
}
