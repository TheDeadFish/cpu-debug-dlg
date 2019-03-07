#include "stdshit.h"
#include "win32hlp.h"
#include "cpu-debug-res.h"
#include "cpu-debug.h"

static
INT_PTR CALLBACK brkDlgProc(HWND hwnd, 
	UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CpuDbgDlg* This = (CpuDbgDlg*)
		GetWindowLongPtr(hwnd, DWL_USER);

	DLGMSG_SWITCH(
	  ON_MESSAGE(WM_MOUSEWHEEL, 
			sendDlgMsg(hwnd, IDC_LIST1, uMsg, wParam))
		ON_MESSAGE(WM_CLOSE, EndDialog(hwnd, 0))
	,)
}

void CpuDbgDlg::breakDlg()
{
	DialogBoxW(getModuleBase(),
		L"CPUDBGBRK", hwnd, brkDlgProc);
};
