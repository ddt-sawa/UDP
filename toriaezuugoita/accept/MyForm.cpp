#include "MyForm.h"
#include"Header.h"

using namespace saisinGUI;

//エントリポイント指定
[STAThreadAttribute]
int main()
{
	//フォーム作成
	MyForm ^GUIForm = gcnew MyForm();

	std::queue<UDPPacket> globalQueue;

	//フォーム表示
	GUIForm->ShowDialog();

	return 0;
}