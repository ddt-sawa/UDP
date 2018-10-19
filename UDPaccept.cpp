#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include<iostream>
#include<string>
#include <winsock2.h>
#include"UDPPacket.h"

//送受信できるバイト数の上限
#define UDP_LIMIT 1000

using namespace std;

int main()
{
	//概要説明
	cout << "これはUDP受信用アプリです。\n";

	//WinSockを使う準備をするクラス(取り合えずバージョンだけWSAStartアップで動かせばよい)
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


	/*接続先のIPアドレスやポート番号の情報を保持するために，sockaddr_in構造体が 用意されており，
	各ソケットは，bindシステムコールによって sockaddr_in構造体のデータと関連づけられる．sockaddr_in構造体は次のように 定義されている．

/usr/include/netinet/in.h:
   struct in_addr {
      u_int32_t s_addr;
   };

   struct sockaddr_in {
      u_char  sin_len;    （このメンバは古いOSでは存在しない）
      u_char  sin_family;    （アドレスファミリ．今回はAF_INETで固定）
      u_short sin_port;    （ポート番号）
      struct  in_addr sin_addr;    （IPアドレス）
      char    sin_zero[8];    （無視してもよい．「詰め物」のようなもの）
   };
ポート番号やIPアドレスはネットワークバイトオーダー (big endian) になって いないといけない．
このため，整数をネットワークバイトオーダーに変換する htons関数を用いる．*/
	
	//アドレスファミリ(プロトコル集の名前空間)
	addr.sin_family = AF_INET;

	//相手方ポートの指定
	addr.sin_port = htons(12345);

	/*例えばサーバープログラムを作る場合、どのアドレスからの接続でも受け入れるように待ち受ける(ことが多い)、
	つまり接続を受けるネットワークインターフェイスがどれでもいいので、bind()の引数にINADDR_ANYが指定される。
	*/
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	/*bind() は、ファイルディスクリプター sockfd で参照されるソケットに addr で指定されたアドレスを割り当てる。
	addrlen には addr が指すアドレス構造体のサイズをバイト単位で指定する。 
	伝統的にこの操作は 「ソケットに名前をつける」 と呼ばれる。*/
	bind(sock, (struct sockaddr *)&addr, sizeof(addr));

	//受け取るデータ
	UDPPacket getData;

	//データを受け取り続ける無限ループ
	do {
		//相手方ソケットのアドレス情報を保持する構造体
		struct sockaddr_in sendAddr;

		int sendAddrLen = sizeof(sendAddr);

		//バッファを定義
		char acceptBuffer[UDP_LIMIT] = {0};

		/*ソケット上のデータを受信する。送信が来るまでこの関数で待機される。
		socket:ソケット記述子。
		buf:データを受け取るバッファーへのポインター。
		lenbuf:パラメーターが指すバッファーの 長さ (バイト単位)。MSG_CONNTERM フラグが設定されている場合、バッファーの長さはゼロでなければなりません。
		flags:flags パラメーターは、次のフラグを 1 つ以上指定することによって設定されます。複数のフラグを指定する場合は、
		論理 OR 演算子 (|) を使用してフラグを分離する必要があります。MSG_CONNTERM フラグは、他のフラグと相互に排他的です*/

		//recvは送られてきたデータを配列か何かで格納しているっぽい
		//ループ処理にぶち込むと、受け取ったデータを順番に読み込んでくれる
		recvfrom(sock, reinterpret_cast<char*>(&getData), UDP_LIMIT, 0, (struct sockaddr *)&sendAddr, &sendAddrLen);

		//受信したバッファ上の値を出力
		cout << "IPアドレス : " << inet_ntoa(sendAddr.sin_addr) << "\nポート番号 : " << ntohs(sendAddr.sin_port) << "\n宛先ポート番号 : " << ntohs(getData.mAcceptPortNumber) << "\nデータサイズ : " << getData.PacketSize <<

			"\nメッセージ : " << getData.mSendData << "\n\n";

	//無限ループ
	} while (true);

	//ソケットを閉じる
	closesocket(sock);

	//Winsock機能を終了する
	WSACleanup();

	//正常終了
	return 0;
}
