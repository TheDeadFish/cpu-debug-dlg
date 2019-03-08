#include "stdshit.h"
#include "win32hlp.h"
#include "cpu-debug-res.h"
#include "cpu-debug.h"

void CpuDbgDlg::brk_init(HWND hBrk)
{
	



}

void CpuDbgDlg::brk_cmd(int cmd)
{
	if(brkcb) brkcb(cbCtx, cmd, NULL);
}

void CpuDbgDlg::brk_once(void)
{
	if(!brkcb) return;
	CpuDbgBrk brk = {
		CpuDbgBrk::EXEC|CpuDbgBrk::ONCE,
		curSpace, sp()->addr, 1 };
	if(brkcb(cbCtx, 0xFFFF, &brk) < 0) 
		Beep(1000, 50);
}

int CpuDbgBrkLst::add(int cmd, CpuDbgBrk* brk)
{
	// valid index
	if(short(cmd) >= 0) {
		if(cmd >= len) return -1;
		if(brk) { *brk = data[cmd]; }
		else { remove(cmd); } return 0;
	}
	
	
	// locate existing bp
	for(int i = 0; i < len; i++) {
		if(data[i] == *brk) return i; }
		
	// create new bp
	if(len == BRK_MAX) return -1;
		data[len] = *brk; return len++;
}

void CpuDbgBrkLst::remove(int index)
{
	len -= 1;
	for(int i = index; i < len; i++) 
		data[i] = data[i+1];
}

static
INT_PTR CALLBACK brkDlgProc(HWND hwnd, 
	UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INIT_DLG_CONTEXT(CpuDbgDlg, This->brk_init(hwnd));

	DLGMSG_SWITCH(
	  ON_MESSAGE(WM_MOUSEWHEEL, 
			sendDlgMsg(hwnd, IDC_LIST1, uMsg, wParam))
			
		CASE_COMMAND(
		  ON_COMMAND(IDCANCEL, EndDialog(hwnd, 0))
			ON_COMMAND(IDC_CPUDBG_BD, This->brk_create());
		
		,)
	,)
}

void CpuDbgDlg::brk_create()
{
	if(!brkcb) return;
	DialogBoxParamW(getModuleBase(),
		L"CPUDBGBRK", hwnd, brkDlgProc, (LPARAM)this);
};
