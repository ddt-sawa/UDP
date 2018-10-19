#ifndef ___UDPPacketStruct
#define ___UDPPacketStruct

#include<iostream>

//UDPPacket構造体
struct UDPPacket {

//公開メンバ
public:

	//送信元ポート番号
	uint16_t mSendPortNumber;

	//送信先ポート番号
	uint16_t mAcceptPortNumber;

	//パケットサイズ(バイト)
	uint16_t PacketSize;

	//チェックサム
	uint16_t mCheckSum;

	//データ本体
	char mSendData[1000];

};

#endif