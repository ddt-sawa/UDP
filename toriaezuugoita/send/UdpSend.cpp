#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include<iostream>
#include<string>
#include"UdpPacket.h"
#define CRC_CCITT16         0x1021    // CRC-生成多項式 G(x)=X16+X12+X5+1=0x(1)1021

using namespace std;

//UDPヘッダのサイズ 送信元ポート・宛先ポート・データサイズ・CRC
#define UDP_HEADER_SIZE 64

//送信先IPアドレス
const char* acceptIP = "127.0.0.1";

void printBit(char integerValue)
{
	//ビット配列を表示するループ
	for (int firstCounter = 8 - 1; firstCounter >= 0; --firstCounter) {

		//カウンタ変数ぶん右シフトした値が1だった場合'1'を、0だった場合'0'を表示する
		std::cout << ((integerValue >> firstCounter) & 1U ? '1' : '0');
	}
	//改行
	std::cout << '\n';
}

//CRCソースコードhttp://hatenaclang.blogspot.com/2011/03/crc.html
uint16_t calculateCRC16(const char* data, int length)
{
	int i, ii;
	uint16_t crc = 0xFFFF;
	for (i = 0; i<length; i++) {
		crc ^= (uint16_t)(*data++ << 8);
		for (ii = 0; ii<8; ii++) {
			if (crc & 0x8000) { crc <<= 1; crc ^= CRC_CCITT16; }
			else { crc <<= 1; }
		}
	}
	return htons(crc);
}

void setLength(UDPPacket& argumentPacket)
{
	argumentPacket.PacketSize= 0;

	argumentPacket.PacketSize += UDP_HEADER_SIZE;


	argumentPacket.PacketSize += argumentPacket.mSendData.size() + 1;

}

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

	//相手方ポートの指定(ネットワークバイトオーダーに変更)
	addr.sin_port = htons(12345);

	//相手のIPアドレスの指定。inet_addr()は引数をネットワークバイトオーダーに変更する
	addr.sin_addr.S_un.S_addr = inet_addr(acceptIP);

	//
	do {

		//送信するデータの構造体
		UDPPacket samplePacket;

		//送信先ポート番号を格納
		samplePacket.mAcceptPortNumber = htons(addr.sin_port);

		//送信する文字列の入力を促す
		cout << "送信したい文字列を入力してください。";

		//送信する文字列を宣言
		string inputString;

		//文字列を入力
		cin >> inputString;

		//文字列を構造体に格納
		samplePacket.mSendData = inputString;

		//データ本体のcrc値を格納
		samplePacket.mCRCValue = htons(calculateCRC16(samplePacket.mSendData.c_str(), samplePacket.mSendData.size()));

		//パケットサイズ格納
		setLength(samplePacket);


		cout << samplePacket.mSendPortNumber << '\n'

		<< samplePacket.mAcceptPortNumber << '\n'

		<< samplePacket.PacketSize << '\n'

		<< samplePacket.mCRCValue << '\n';

		//ヘッダ＋データ本体を格納するバッファを動的確保
		char* buffer = new char[samplePacket.PacketSize];

		//バッファにヘッダ部分を代入
		memcpy(buffer, reinterpret_cast<const void*>(&samplePacket), UDP_HEADER_SIZE);

		

		//バッファにデータ本体部分を代入
		for (int firstCounter = 0; firstCounter < samplePacket.mSendData.size() + 1; firstCounter++) {
					

			buffer[firstCounter + UDP_HEADER_SIZE + 1] = samplePacket.mSendData[firstCounter];

			printBit(buffer[firstCounter + UDP_HEADER_SIZE + 1]);

			std::cout << "\n";

		}

		cout << samplePacket.mSendData;



		/*sendto()はもう一方のソケットへメッセージを転送するのに使用される
		ソケット名、バッファ、文字サイズ、フラグ、ターゲットアドレス、ターゲットサイズ
		構造体を1バイトずつ送信
		*/
		sendto(sock,buffer, samplePacket.PacketSize, 0, (struct sockaddr *)&addr, sizeof(addr));

		//無限ループ
	} while (true);

	//ソケットを閉じる
	closesocket(sock);

	//Winsock機能を終了する
	WSACleanup();

	//正常終了
	return 0;
}