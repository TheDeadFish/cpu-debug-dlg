#include "stdshit.h"
#include "win32hlp.h"
#include "cpu-debug-res.h"
#include "cpu-debug.h"

static 
int getDlgItemHex(HWND hwnd, int ctrlID)
{
	char buff[32]; char* end;
	GetDlgItemTextA(hwnd, ctrlID, buff, 32);
	return strtoul(buff, &end, 16);
}

static
void setDlgItemHex(HWND hwnd, 
	int ctrlID, int data, int minLen)
{
	char buff[32];
	sprintf(buff, "%0*X", minLen, data);
	SetDlgItemTextA(hwnd, ctrlID, buff);
}

static
void brk_updateCtrl(HWND hBrk, CpuDbgBrk* brk)
{
	// set check boxes
	dlgButton_setCheck(hBrk, IDC_BRK_EXEC, brk->flags & brk->EXEC);
	dlgButton_setCheck(hBrk, IDC_BRK_ONCE, brk->flags & brk->ONCE);
	dlgButton_setCheck(hBrk, IDC_BRK_READ, brk->flags & brk->READ);
	dlgButton_setCheck(hBrk, IDC_BRK_WRITE, brk->flags & brk->WRITE);
	
	// set other items
	dlgCombo_setSel(hBrk, IDC_SPACE, brk->spc);
	setDlgItemHex(hBrk, IDC_ADDR, brk->addr, 4);
	SetDlgItemInt(hBrk, IDC_SIZE1, brk->size, FALSE);
}

static 
void brk_readCtrl(HWND hBrk, CpuDbgBrk* brk)
{
	memset(brk, 0, sizeof(*brk));
	
	// read check boxes
	if(IsDlgButtonChecked(hBrk, IDC_BRK_EXEC)) brk->flags |= brk->EXEC;
	if(IsDlgButtonChecked(hBrk, IDC_BRK_ONCE)) brk->flags |= brk->ONCE;
	if(IsDlgButtonChecked(hBrk, IDC_BRK_READ)) brk->flags |= brk->READ;
	if(IsDlgButtonChecked(hBrk, IDC_BRK_WRITE)) brk->flags |= brk->WRITE;
	
	// read other items
	brk->spc = dlgCombo_getSel(hBrk, IDC_SPACE);
	brk->addr = getDlgItemHex(hBrk, IDC_ADDR);
	brk->size = GetDlgItemInt(hBrk, IDC_SIZE1, 0, FALSE);
}

void CpuDbgDlg::brk_init(HWND hBrk)
{
	// init spaces list
	char buff[32];
	for(int i = 0; i < MAX_SPC; i++) {
		if(!getSpcName(i, buff)) break;
		dlgCombo_addStr(hBrk, IDC_SPACE, buff);
	}
	
	// set default 
	CpuDbgBrk brk = {CpuDbgBrk::EXEC, 0, 0, 1};
	brk_updateCtrl(hBrk, &brk);
	brk_updateView(hBrk);
}

void CpuDbgDlg::brk_cmd(int cmd)
{
	if(brkcb) brkcb(cbCtx, cmd, NULL);
}

int CpuDbgDlg::brk_set(HWND hwnd, CpuDbgBrk* brk)
{
	int ec = brkcb(cbCtx, 0xFFFF, brk);
	if(ec < 0) {
		const char* msg = "out of break slots";
		if(ec == -2) msg = "bad breakpoint";
		MessageBoxA(hwnd, msg, "Breakpoint error", MB_OK);
	}
	
	return ec;
}


void CpuDbgDlg::brk_once(void)
{
	if(!brkcb) return;
	CpuDbgBrk brk = {
		CpuDbgBrk::EXEC|CpuDbgBrk::ONCE,
		curSpace, sp()->addr, 1 };
	brk_set(hwnd, &brk);
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

void CpuDbgDlg::brk_remove(HWND hBrk)
{
	int sel = listBox_getCurSel(hBrk, IDC_LIST1);
	if((sel < 0) || !brkcb) return;
	brkcb(cbCtx, sel, NULL);
	brk_updateView(hBrk);
}

void CpuDbgDlg::brk_update(HWND hBrk)
{
	this->brk_remove(hBrk);
	this->brk_add(hBrk);
}

void CpuDbgDlg::brk_add(HWND hBrk)
{
	if(!this->brkcb) return;
	
	CpuDbgBrk brk; 
	brk_readCtrl(hBrk, &brk);
	
	int ec = brk_set(hBrk, &brk);
	if(ec >= 0) {	brk_updateView(hBrk);
		listBox_setCurSel(hBrk, IDC_LIST1, ec); }
}



void CpuDbgDlg::brk_updateView(HWND hBrk)
{
	CpuDbgBrk brk; char buff[64];
	listBox_reset(hBrk, IDC_LIST1);

	for(int i = 0;; i++) {
		if(brkcb(cbCtx, i, &brk) < 0) break;
		
		
		char* pos = buff + sprintf(buff, "%d. ", i);
		pos += getSpcName(brk.spc, pos);
		pos += sprintf(pos, ", %04X, %d, ", brk.addr, brk.size);
		
		if(brk.flags & brk.EXEC) *pos++ = 'X';
		if(brk.flags & brk.READ) *pos++ = 'R';
		if(brk.flags & brk.WRITE) *pos++ = 'W';
		if(brk.flags & brk.ONCE) *pos++ = 'O';
		*pos = 0;
		
		listBox_addStr(hBrk, IDC_LIST1, buff);
		
	}
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
		  ON_COMMAND(IDC_CREATE, This->brk_add(hwnd))
		  ON_COMMAND(IDC_DELETE, This->brk_remove(hwnd))
		  ON_COMMAND(IDC_UPDATE, This->brk_update(hwnd))
		  
		
		
		  ON_COMMAND(IDCANCEL, EndDialog(hwnd, 0))
			ON_COMMAND(IDC_CPUDBG_BD, This->brk_create());
		,)
	,)
}

extern const WCHAR resn_CPUDBGBRK[];

void CpuDbgDlg::brk_create()
{
	if(!brkcb) return;
	DialogBoxParamW(getModuleBase(),
		resn_CPUDBGBRK, hwnd, brkDlgProc, (LPARAM)this);
};
