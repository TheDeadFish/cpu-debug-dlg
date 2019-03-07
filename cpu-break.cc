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
	

}

static
INT_PTR CALLBACK brkDlgProc(HWND hwnd, 
	UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	INIT_DLG_CONTEXT(CpuDbgDlg, This->brk_init(hwnd));

	DLGMSG_SWITCH(
	  ON_MESSAGE(WM_MOUSEWHEEL, 
			sendDlgMsg(hwnd, IDC_LIST1, uMsg, wParam))
		ON_MESSAGE(WM_CLOSE, EndDialog(hwnd, 0))
		CASE_COMMAND(
			ON_COMMAND(IDC_CPUDBG_BD, This->brk_create());
		
		,)
	,)
}

void CpuDbgDlg::brk_create()
{
	if(!brkcb) return;
	DialogBoxW(getModuleBase(),
		L"CPUDBGBRK", hwnd, brkDlgProc);
};
