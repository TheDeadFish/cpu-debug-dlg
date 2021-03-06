#include "stdshit.h"
#include "win32hlp.h"
#include "cpu-debug.h"

const char progName[] = "test";

byte readcb(void* ctx, int space, unsigned addr)
{
	return addr;
}

int discb(void* ctx, char* buff, byte* data, unsigned addr)
{
	sprintf(buff, "%X, %d", RW(data), addr);
	return 2;
}


CpuDbgBrkLst brkLst;


int brkcb(void* ctx, int cmd, CpuDbgBrk* brk)
{
	if(brkLst.isCmd(cmd)) {
		printf("cmd: %d\n", cmd); return 0; }
	return brkLst.add(cmd, brk);
}

int main()
{
	CpuDbgDlg* dbg = cpuDbgDlg_create_(NULL);
	cpuDbgDlg_initSpc(dbg,0,0,0xFFFF,0);
	
	dbg->readcb = readcb;
	dbg->discb = discb;
	dbg->brkcb = brkcb;	

	dialogMsgLoop();
	




}

