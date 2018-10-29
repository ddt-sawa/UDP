#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define CRC_CCITT16			0x1021		// CRC-生成多項式 G(x)=X16+X12+X5+1=0x(1)1021
#define UDP_HEADER_SIZE		64			//UDPヘッダのサイズ (送信元ポート・宛先ポート・データサイズ・CRC) 
#include <winsock2.h>
#include<iostream>
#include<string>
#include"UdpPacket.h"
#pragma comment(lib, "ws2_32.lib")

//送信先IPアドレス
const char* acceptIP = "127.0.0.1";

using namespace std;

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
	return crc;
}

/**
* パケットサイズのセッタ
* @param argumentPacket UDPパケット
* @author Sawa
* @since 10.26
*/
void setLength(UDPPacket& argumentPacket)
{
	//パケットサイズを初期化
	argumentPacket.PacketSize= 0;

	//UDPヘッダ部のサイズを加算
	argumentPacket.PacketSize += UDP_HEADER_SIZE;

	//UDPデータ部のサイズを加算
	argumentPacket.PacketSize += argumentPacket.mSendData.size() + 1;

}

/**
* UDPパケットの情報を表示する
* @param argumentPacket UDPパケット
* @author Sawa
* @since 10.26
*/
void printUDPPacket(const UDPPacket& argumetnPacket) {

		//送信側ポート番号
	cout << argumetnPacket.mSendPortNumber << '\n'

		//受信側ポート番号
		<< argumetnPacket.mAcceptPortNumber << '\n'

		//パケットサイズ
		<< argumetnPacket.PacketSize << '\n'

		//CRC値
		<< argumetnPacket.mCRCValue << '\n';
}

int main()
{
	//ソケット情報を格納するクラス(取り合えずバージョンだけWSAStartアップで動かせばよい)
	WSAData wsaData;

	//ソケット
	SOCKET sendSocket;

	//ソケットのアドレス指定を行う構造体
	struct sockaddr_in addr;

	//DLLの初期化を行い、失敗した場合エラーフローに移行
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {

		//WASAtartup関数でのエラー。エラー番号表示
		std::cout << "WSAStartupエラー" << WSAGetLastError();

		//強制終了
		exit(EXIT_FAILURE);
	}
	//ソケットクラスにIPアドレス、通信方法、プロトコルを代入。失敗した場合エラーフローに移行
	if ((sendSocket = ::socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {

		//socket関数でのエラー。エラー番号表示
		std::cout << "socketエラー" << WSAGetLastError();

		//強制終了
		exit(EXIT_FAILURE);
	}
	//アドレスファミリ
	addr.sin_family = AF_INET;

	//相手方ポートの指定(ネットワークバイトオーダーに変更)
	addr.sin_port = htons(12345);

	//相手のIPアドレスの指定。inet_addr()は引数をネットワークバイトオーダーに変更する
	addr.sin_addr.S_un.S_addr = inet_addr(acceptIP);

	//UDPパケットを送信するループ
	do {
		//送信するデータの構造体
		UDPPacket samplePacket;

		//送信する文字列
		string inputString;

		//送信先ポート番号を格納
		samplePacket.mAcceptPortNumber = htons(addr.sin_port);

		//送信するデータ本体の入力を促す
		cout << "送信したい文字列を入力してください。";

		//データ本体を入力
		cin >> inputString;

		//送信先ポート番号を格納
		samplePacket.mAcceptPortNumber = htons(addr.sin_port);

		//データ本体を構造体に格納
		samplePacket.mSendData = inputString;

		//データ本体のcrc値を格納
		samplePacket.mCRCValue = htons(calculateCRC16(samplePacket.mSendData.c_str(), samplePacket.mSendData.size() + 1));

		//パケットサイズ格納
		setLength(samplePacket);

		//ヘッダ＋データ本体を格納するバッファを動的確保して0で初期化
		char* sendBuffer = new char[samplePacket.PacketSize]();

		//バッファにUDPヘッダ部分を格納
		memcpy(sendBuffer, reinterpret_cast<const void*>(&samplePacket), UDP_HEADER_SIZE);

		//バッファにデータ本体部分を格納するループ
		for (int firstCounter = 0; firstCounter < samplePacket.mSendData.size() + 1; firstCounter++) {
					
			//データ本体部分代入
			sendBuffer[firstCounter + UDP_HEADER_SIZE + 1] = samplePacket.mSendData[firstCounter];

		}
		//UDPパケットを送信
		if (sendto(sendSocket, sendBuffer, samplePacket.PacketSize, 0, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {

			//sendto関数でのエラー。エラー番号表示
			std::cout << "sendtoエラー" << WSAGetLastError();

			//強制終了
			exit(EXIT_FAILURE);
		}
	//ソケットを閉じる
	closesocket(sendSocket);

	delete[] sendBuffer;
	//アプリケーションを閉じるまで無限ループ
	} while (true);

	//Winsock機能を終了する
	WSACleanup();

	//正常終了
	return 0;
}