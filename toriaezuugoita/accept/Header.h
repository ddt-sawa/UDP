#pragma once
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
	uint16_t mCRCValue;

	//データ本体
	std::string mSendData;

};

//GUI表示用構造体
struct  GUIPacket {

	//公開メンバ
public:

	//送信元IPアドレス
	uint16_t mSendIP;

	//送信元ポート番号
	uint16_t mSendPortNumber;

	//データ本体
	std::string mSendData;
};

#endif