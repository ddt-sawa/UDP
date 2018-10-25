#pragma once
#include<iostream>
#include"Source1.h"

// CRC-生成多項式 G(x)=X16+X12+X5+1=0x(1)1021
#define CRC_CCITT16 0x1021    

//送受信できるバイト数の上限
#define UDP_LIMIT 1000

//UDPヘッダのサイズ 送信元ポート・宛先ポート・データサイズ・CRC
#define UDP_HEADER_SIZE 64

namespace GUI1024 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace std;

	/// <summary>
	/// MyForm の概要
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: ここにコンストラクター コードを追加します
			//
			//受信スレッド作成
			Threading::Thread^ GUIThread = gcnew Threading::Thread(gcnew Threading::ThreadStart(this,acceptUDP));
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
		
		//通知用デリゲート
		delegate System::Void MyDeleGate(GUIPacket argumentGUIPacket);


	
	//コールバック関数で受け取ったパケットのデータを表示する
	void changeTextBox1(GUIPacket argumentPacket) {

		//string型データ本体をString^型に変換する
		System::String^ DataString = gcnew String(argumentPacket.mSendData.c_str());

		//テキストボックスに表示
		this->textBox1->Text = DataString;
	}

	/**
	* スレッド用UDP受信関数
	* @author Sawa
	* @since 10.24
	*/
	void acceptUDP()
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
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {

			cout << "WSAStartupエラー" << WSAGetLastError();

			exit(EXIT_FAILURE);
		}

		//ソケットクラスにIPアドレス、通信方法、プロトコルを代入
		sock = ::socket(AF_INET, SOCK_DGRAM, 0);

		//ソケットクラスのエラー処理
		if (sock == INVALID_SOCKET) {

			cout << "socketエラー" << WSAGetLastError();

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
			cout << "bindエラー" << WSAGetLastError();

			//以上処理
			exit(EXIT_FAILURE);
		}

		//受け取るデータ
		UDPPacket getData;

		//データを受け取り続ける無限ループ
		do {
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
			if ((::recvfrom(sock, acceptBuffer, UDP_LIMIT, 0, (struct sockaddr *)&sendAddr, &sendAddrLen)) == SOCKET_ERROR) {


				//recvfrom関数でエラーが発生した際
				cout << "recvfrom関数エラー" << WSAGetLastError();

				//異常処理
				exit(EXIT_FAILURE);
			}

			GUIPacket arrangementData;

			arrangementData.mSendData = inet_ntoa(sendAddr.sin_addr);

			arrangementData.mSendPortNumber = (reinterpret_cast<uint16_t*>(acceptBuffer)[1]);

			for (int firstCounter = 0; !acceptBuffer[UDP_HEADER_SIZE + firstCounter + 1]; firstCounter++) {

				arrangementData.mSendData.push_back(acceptBuffer[UDP_HEADER_SIZE + firstCounter + 1]);

			}

			MyDeleGate^ method = gcnew MyDeleGate(this, changeTextBox1);

			BeginInvoke(method);

			//無限ループ
		} while (true);

		//ソケットを閉じる
		closesocket(sock);

		//Winsock機能を終了する
		WSACleanup();
	}


	private: System::Windows::Forms::TextBox^  textBox1;
	private: System::Windows::Forms::Button^  button1;
private: System::Windows::Forms::TextBox^  textBox2;
	protected:

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

		};
private: System::Void InitializeComponent() {
	this->SuspendLayout();
	this->ClientSize = System::Drawing::Size(282, 253);
	this->Name = L"MyForm";
	this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load_1);
	this->ResumeLayout(false);

}
private: System::Void MyForm_Load_1(System::Object^  sender, System::EventArgs^  e) {
}
private: System::Void InitializeComponent() {
	this->SuspendLayout();
	// 
	// MyForm
	// 
	this->ClientSize = System::Drawing::Size(282, 253);
	this->Name = L"MyForm";
	this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
	this->ResumeLayout(false);

}
private: System::Void MyForm_Load(System::Object^  sender, System::EventArgs^  e) {
}
}
