#pragma once
#include<iostream>
#include"Source1.h"

// CRC-���������� G(x)=X16+X12+X5+1=0x(1)1021
#define CRC_CCITT16 0x1021    

//����M�ł���o�C�g���̏��
#define UDP_LIMIT 1000

//UDP�w�b�_�̃T�C�Y ���M���|�[�g�E����|�[�g�E�f�[�^�T�C�Y�ECRC
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
	/// MyForm �̊T�v
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: �����ɃR���X�g���N�^�[ �R�[�h��ǉ����܂�
			//
			//��M�X���b�h�쐬
			Threading::Thread^ GUIThread = gcnew Threading::Thread(gcnew Threading::ThreadStart(this,acceptUDP));
		}

	protected:
		/// <summary>
		/// �g�p���̃��\�[�X�����ׂăN���[���A�b�v���܂��B
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}


	public: 
		
		//�ʒm�p�f���Q�[�g
		delegate System::Void MyDeleGate(GUIPacket argumentGUIPacket);


	
	//�R�[���o�b�N�֐��Ŏ󂯎�����p�P�b�g�̃f�[�^��\������
	void changeTextBox1(GUIPacket argumentPacket) {

		//string�^�f�[�^�{�̂�String^�^�ɕϊ�����
		System::String^ DataString = gcnew String(argumentPacket.mSendData.c_str());

		//�e�L�X�g�{�b�N�X�ɕ\��
		this->textBox1->Text = DataString;
	}

	/**
	* �X���b�h�pUDP��M�֐�
	* @author Sawa
	* @since 10.24
	*/
	void acceptUDP()
	{

		//�T�v����
		cout << "�����UDP��M�p�A�v���ł��B\n";

		//WinSock���g������������N���X(��荇�����o�[�W��������WSAStart�A�b�v�œ������΂悢)
		WSAData wsaData;

		//�\�P�b�g
		SOCKET sock;

		//�\�P�b�g�̃A�h���X�w����s���\����
		struct sockaddr_in addr;

		/*Winsock������������֐�
		winsock�̃o�[�W����(2,0)���w�肵�A
		�����̏����\���̂Ɋi�[����B
		�G���[�����������Ƃ���0�ȊO�̒l��Ԃ�
		*/
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {

			cout << "WSAStartup�G���[" << WSAGetLastError();

			exit(EXIT_FAILURE);
		}

		//�\�P�b�g�N���X��IP�A�h���X�A�ʐM���@�A�v���g�R������
		sock = ::socket(AF_INET, SOCK_DGRAM, 0);

		//�\�P�b�g�N���X�̃G���[����
		if (sock == INVALID_SOCKET) {

			cout << "socket�G���[" << WSAGetLastError();

			exit(EXIT_FAILURE);

		}

		/*�ڑ����IP�A�h���X��|�[�g�ԍ��̏���ێ����邽�߂ɁCsockaddr_in�\���̂� �p�ӂ���Ă���C
		�e�\�P�b�g�́Cbind�V�X�e���R�[���ɂ���� sockaddr_in�\���̂̃f�[�^�Ɗ֘A�Â�����Dsockaddr_in�\���͎̂��̂悤�� ��`����Ă���D
		/usr/include/netinet/in.h:
		struct in_addr {
		u_int32_t s_addr;
		};
		struct sockaddr_in {
		u_char  sin_len;    �i���̃����o�͌Â�OS�ł͑��݂��Ȃ��j
		u_char  sin_family;    �i�A�h���X�t�@�~���D�����AF_INET�ŌŒ�j
		u_short sin_port;    �i�|�[�g�ԍ��j
		struct  in_addr sin_addr;    �iIP�A�h���X�j
		char    sin_zero[8];    �i�������Ă��悢�D�u�l�ߕ��v�̂悤�Ȃ��́j
		};
		�|�[�g�ԍ���IP�A�h���X�̓l�b�g���[�N�o�C�g�I�[�_�[ (big endian) �ɂȂ��� ���Ȃ��Ƃ����Ȃ��D
		���̂��߁C�������l�b�g���[�N�o�C�g�I�[�_�[�ɕϊ����� htons�֐���p����D*/

		//�A�h���X�t�@�~��(�v���g�R���W�̖��O���)
		addr.sin_family = AF_INET;

		//������|�[�g�̎w��
		addr.sin_port = htons(12345);

		/*�Ⴆ�΃T�[�o�[�v���O���������ꍇ�A�ǂ̃A�h���X����̐ڑ��ł��󂯓����悤�ɑ҂��󂯂�(���Ƃ�����)�A
		�܂�ڑ����󂯂�l�b�g���[�N�C���^�[�t�F�C�X���ǂ�ł������̂ŁAbind()�̈�����INADDR_ANY���w�肳���B
		*/
		addr.sin_addr.S_un.S_addr = INADDR_ANY;

		/*bind() �́A�t�@�C���f�B�X�N���v�^�[ sockfd �ŎQ�Ƃ����\�P�b�g�� addr �Ŏw�肳�ꂽ�A�h���X�����蓖�Ă�B
		addrlen �ɂ� addr ���w���A�h���X�\���̂̃T�C�Y���o�C�g�P�ʂŎw�肷��B
		�`���I�ɂ��̑���� �u�\�P�b�g�ɖ��O������v �ƌĂ΂��B*/
		if (::bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {

			//bind�֐��ŃG���[������������
			cout << "bind�G���[" << WSAGetLastError();

			//�ȏ㏈��
			exit(EXIT_FAILURE);
		}

		//�󂯎��f�[�^
		UDPPacket getData;

		//�f�[�^���󂯎�葱���閳�����[�v
		do {
			//������\�P�b�g�̃A�h���X����ێ�����\����
			struct sockaddr_in sendAddr;

			int sendAddrLen = sizeof(sendAddr);

			//�o�b�t�@���`
			char acceptBuffer[UDP_LIMIT] = { 0 };

			/*�\�P�b�g��̃f�[�^����M����B���M������܂ł��̊֐��őҋ@�����B
			socket:�\�P�b�g�L�q�q�B
			buf:�f�[�^���󂯎��o�b�t�@�[�ւ̃|�C���^�[�B
			lenbuf:�p�����[�^�[���w���o�b�t�@�[�� ���� (�o�C�g�P��)�BMSG_CONNTERM �t���O���ݒ肳��Ă���ꍇ�A�o�b�t�@�[�̒����̓[���łȂ���΂Ȃ�܂���B
			flags:flags �p�����[�^�[�́A���̃t���O�� 1 �ȏ�w�肷�邱�Ƃɂ���Đݒ肳��܂��B�����̃t���O���w�肷��ꍇ�́A
			�_�� OR ���Z�q (|) ���g�p���ăt���O�𕪗�����K�v������܂��BMSG_CONNTERM �t���O�́A���̃t���O�Ƒ��݂ɔr���I�ł�*/

			//recv�͑����Ă����f�[�^��z�񂩉����Ŋi�[���Ă�����ۂ�
			//���[�v�����ɂԂ����ނƁA�󂯎�����f�[�^�����Ԃɓǂݍ���ł����
			if ((::recvfrom(sock, acceptBuffer, UDP_LIMIT, 0, (struct sockaddr *)&sendAddr, &sendAddrLen)) == SOCKET_ERROR) {


				//recvfrom�֐��ŃG���[������������
				cout << "recvfrom�֐��G���[" << WSAGetLastError();

				//�ُ폈��
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

			//�������[�v
		} while (true);

		//�\�P�b�g�����
		closesocket(sock);

		//Winsock�@�\���I������
		WSACleanup();
	}


	private: System::Windows::Forms::TextBox^  textBox1;
	private: System::Windows::Forms::Button^  button1;
private: System::Windows::Forms::TextBox^  textBox2;
	protected:

	private:
		/// <summary>
		/// �K�v�ȃf�U�C�i�[�ϐ��ł��B
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// �f�U�C�i�[ �T�|�[�g�ɕK�v�ȃ��\�b�h�ł��B���̃��\�b�h�̓��e��
		/// �R�[�h �G�f�B�^�[�ŕύX���Ȃ��ł��������B
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
