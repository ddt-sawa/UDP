#pragma once
#include<iostream>
#include"Header.h"
#include<Winsock2.h>
#include<string>
#include <msclr\marshal.h>
#include<queue>

//UDPPacketを格納するキュー
std::queue<UDPPacket> globalQueue;

// CRC-生成多項式 G(x)=X16+X12+X5+1=0x(1)1021
#define CRC_CCITT16 0x1021    

//送受信できるバイト数の上限
#define UDP_LIMIT 1000

//UDPヘッダのサイズ 送信元ポート・宛先ポート・データサイズ・CRC
#define UDP_HEADER_SIZE 64

/**
* 値のビット配列を表示する
* @param integerValue 値
* @author Sawa
* @since 7.27
*/
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

namespace saisinGUI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Threading;
	using namespace msclr::interop;
	using namespace System::Runtime::InteropServices;

	/// <summary>
	/// MyForm の概要
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		//デリゲート
		delegate System::Void myDelegate();

		delegate void stringDelegate(System::String^ stringData);

	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: ここにコンストラクター コードを追加します
			//
		}

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

	public:
		//以下にacceptUDP
		/**
		* スレッド用UDP受信関数
		* @author Sawa
		* @since 10.24
		*/
		void acceptUDP()
		{

			//概要説明
			std::cout << "これはUDP受信用アプリです。\n";

			do {
			//WinSockを使う準備をするクラス(取り合えずバージョンだけWSAStartアップで動かせばよい)
			WSAData wsaData;

			//データを受け取り続ける無限ループ
			

			//ソケット
			SOCKET sock;

			//ソケットのアドレス指定を行う構造体
			struct sockaddr_in addr;

			/*Winsockを初期化する関数
			winsockのバージョン(2,0)を指定し、
			それらの情報を構造体に格納する。
			エラーが発生したときは0以外の値を返す
			*/
			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {

				std::cout << "WSAStartupエラー" << WSAGetLastError();

				exit(EXIT_FAILURE);
			}

			//ソケットクラスにIPアドレス、通信方法、プロトコルを代入
			sock = ::socket(AF_INET, SOCK_DGRAM, 0);

			//ソケットクラスのエラー処理
			if (sock == INVALID_SOCKET) {

				std::cout << "socketエラー" << WSAGetLastError();

				exit(EXIT_FAILURE);

			}

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
			if (::bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {

				//bind関数でエラーが発生した際
				std::cout << "bindエラー" << WSAGetLastError();

				//以上処理
				exit(EXIT_FAILURE);
			}

			//受け取るデータ
			UDPPacket getData;


				//相手方ソケットのアドレス情報を保持する構造体
				struct sockaddr_in sendAddr;

				int sendAddrLen = sizeof(sendAddr);

				//バッファを定義
				char acceptBuffer[UDP_LIMIT] = { 0 };

				/*ソケット上のデータを受信する。送信が来るまでこの関数で待機される。
				socket:ソケット記述子。
				buf:データを受け取るバッファーへのポインター。
				lenbuf:パラメーターが指すバッファーの 長さ (バイト単位)。MSG_CONNTERM フラグが設定されている場合、バッファーの長さはゼロでなければなりません。
				flags:flags パラメーターは、次のフラグを 1 つ以上指定することによって設定されます。複数のフラグを指定する場合は、
				論理 OR 演算子 (|) を使用してフラグを分離する必要があります。MSG_CONNTERM フラグは、他のフラグと相互に排他的です*/

				//recvは送られてきたデータを配列か何かで格納しているっぽい
				//ループ処理にぶち込むと、受け取ったデータを順番に読み込んでくれる
				::recvfrom(sock, acceptBuffer, UDP_LIMIT, 0, (struct sockaddr *)&sendAddr, &sendAddrLen);

				/*
				//recvfrom関数でエラーが発生した際
				std::cout << "recvfrom関数エラー" << WSAGetLastError();

				//異常処理
				exit(EXIT_FAILURE);
				}
				*/

				getData.mSendPortNumber = (reinterpret_cast<uint16_t*>(acceptBuffer)[0]);

				getData.mAcceptPortNumber = (reinterpret_cast<uint16_t*>(acceptBuffer)[1]);

				getData.PacketSize = (reinterpret_cast<uint16_t*>(acceptBuffer)[2]);

				getData.mCRCValue = (reinterpret_cast<uint16_t*>(acceptBuffer)[3]);

				for (int firstCounter = UDP_HEADER_SIZE + 1; firstCounter < getData.PacketSize + 1; firstCounter++) {

					getData.mSendData.push_back(acceptBuffer[firstCounter]);

				}

				//受信したバッファ上の値を出力
				std::cout << "IPアドレス : " << inet_ntoa(sendAddr.sin_addr) << "\nポート番号 : " << ntohs(sendAddr.sin_port) << "\n宛先ポート番号 : " << (getData.mAcceptPortNumber) << "\nデータサイズ : " << getData.PacketSize <<

					"\nCRC値 : " << getData.mCRCValue << "\nメッセージ : " << getData.mSendData << "\n\n";

				globalQueue.push(getData);

				//テキストボックスに表示するためのデリゲート
				myDelegate^ changeBoxDelegate = gcnew myDelegate(this, &MyForm::changeTextBox1);


				BeginInvoke(changeBoxDelegate);

			//ソケットを閉じる
			closesocket(sock);

			//Winsock機能を終了する
			WSACleanup();

			//無限ループ
			} while (true);
		}


	private:

		//コールバック関数で受け取ったパケットのデータを表示する
		System::Void changeTextBox1() {

			System::String^ ListString = gcnew String(globalQueue.back().mSendData.c_str());

			this->textBox1->Text += ListString;
			
		}
	private: System::Windows::Forms::TextBox^  textBox1;
	private: System::Windows::Forms::Button^  button1;
	public:

	private:
		/// <summary>
		/// 必要なデザイナー変数です。
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// デザイナー サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディターで変更しないでください。
		/// </summary>
		void InitializeComponent(void)
		{
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// textBox1
			// 
			this->textBox1->Location = System::Drawing::Point(103, 54);
			this->textBox1->Multiline = true;
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(523, 195);
			this->textBox1->TabIndex = 0;
			this->textBox1->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox1_TextChanged);
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(319, 285);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(75, 23);
			this->button1->TabIndex = 1;
			this->button1->Text = L"受信開始";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 15);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(711, 394);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->textBox1);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void MyForm_Load(System::Object^  sender, System::EventArgs^  e) {
	}
	//ボタンが押された際、UDP受信スレッドを用意
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

		// スレッドを用意
		System::Threading::Thread ^progressThread = gcnew System::Threading::Thread(
			gcnew System::Threading::ThreadStart(this, &MyForm::acceptUDP))
		;

		progressThread->Start();
	}
	private: System::Void textBox1_TextChanged(System::Object^  sender, System::EventArgs^  e) {
	}
};

	
}
