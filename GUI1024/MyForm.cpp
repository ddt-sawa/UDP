#include<Windows.h>
#include "MyForm.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include<iostream>
#include<string>

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

//送受信できるバイト数の上限
#define UDP_LIMIT 1000

using namespace GUI1024;


//エントリポイント指定
[STAThreadAttribute]

int main() {

	//フォーム作成
	MyForm ^GUIForm = gcnew MyForm();

	//フォームスレッド作成
	Threading::Thread^ GUIThread = gcnew Threading::Thread(gcnew Threading::ThreadStart(GUIForm->ShowDialog));

	//フォームスレッド開始
	GUIThread->Start();

	//正常終了
	return 0;

}