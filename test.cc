#include "stdshit.h"
#include "win32hlp.h"
#include "cpu-debug.h"

const char progName[] = "test";

byte readcb(void* ctx, int space, int addr)
{
	return addr;
}

int discb(void* ctx, char* buff, byte* data, int addr)
{
	sprintf(buff, "%X, %d", RW(data), addr);
	return 2;
}




int main()
{
	CpuDbgDlg dbg;
	dbg.setSpcAddr(0,0,0xFFFF);
	dbg.readcb = readcb;
	dbg.discb = discb;
	
	
	dbg.create(0);
	dialogMsgLoop();
	




}

