#pragma once
#include "DIA_CLOSE.h"
#include "server_sock.h"

class CTool
{
private:
	inline static DIA_CLOSE dia = NULL;
	unsigned int tid;
public:
	static bool MakeDriveInfo();
	static bool MakeDirInfo();
	static bool RunFile();
	static bool Get_File_Length();
	static bool DownloadFile();
	static bool DoMouseEvent();
	static bool SendScreen();
	static unsigned thread_close_computer(void* arg);
	static bool Close_Computer();
	static bool Open_Computer();
	static bool Delete_File();
	static void Process_Cmd(int cmd);
	static void screen_thread(void* arg);
};

