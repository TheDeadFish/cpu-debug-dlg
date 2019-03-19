#include "stdshit.h"
#include "win32hlp.h"
#include "cpu-debug-res.h"
#include "cpu-debugInt.h"

#define SATURATE_ADDV(v, x, y) ({ if(__builtin_add_overflow \
	( x, y, &v)) v = std::numeric_limits<__typeof__(v)>::max(); })
#define SATURATE_ADDT(t, x, y) ({ t v;  SATURATE_ADDV(v, x, y); v; })
#define SATURATE_ADD(x, y) SATURATE_ADDT(__typeof__ (x+y), x, y)

#define SATURATE_SUBV(v, x, y) ({ if(__builtin_sub_overflow \
	( x, y, &v)) v = std::numeric_limits<__typeof__(v)>::min(); })
#define SATURATE_SUBT(t, x, y) ({ t v;  SATURATE_SUBV(v, x, y); v; })
#define SATURATE_SUB(x, y) SATURATE_SUBT(__typeof__ (x+y), x, y)

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
	setDlgItemHex(hBrk, IDC_END, brk->end, 4);
	setDlgItemHex(hBrk, IDC_ADDR, brk->addr, 4);
	setDlgItemHex(hBrk, IDC_SIZE1, (brk->end-brk->addr)+1, 1);
	CheckDlgButton(hBrk, rad3, 0);
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
	brk->end = getDlgItemHex(hBrk, IDC_END);
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


void CpuDbgDlgInt::brk_cmd(int cmd)
{
	if(brkcb) brkcb(cbCtx, cmd, NULL);
}

int CpuDbgDlgInt::brk_set(HWND hwnd, CpuDbgBrk* brk)
{
	int ec = brkcb(cbCtx, 0xFFFF, brk);
	if(ec < 0) {
		const char* msg = "out of break slots";
		if(ec == -2) msg = "bad breakpoint";
		MessageBoxA(hwnd, msg, "Breakpoint error", MB_OK);
	}
	
	return ec;
}


void CpuDbgDlgInt::brk_once(void)
{
	if(!brkcb) return;
	CpuDbgBrk brk = {
		CpuDbgBrk::EXEC|CpuDbgBrk::ONCE,
		curSpace, sp()->addr, 1 };
	brk_set(hwnd, &brk);
}

struct CpuDbgDlgBrk : CpuDbgDlgInt
{
	MEMBER_DLGPROC2(CpuDbgDlgBrk, brkDlgProc);
	
	
void brk_selChg(HWND hBrk)
{
	CpuDbgBrk brk;
	int i = listBox_getCurSel(hBrk, IDC_LIST1);
	if((i < 0)||(brkcb(cbCtx, i, &brk) < 0)) {
		brk = {CpuDbgBrk::EXEC, 0, 0, 0};	}
	brk_updateCtrl(hBrk, &brk);
}
	
void brkDlgProcInit(HWND hBrk)
{
	// init spaces list
	char buff[32];
	for(int i = 0; i < MAX_SPC; i++) {
		if(!getSpcName(i, buff)) break;
		dlgCombo_addStr(hBrk, IDC_SPACE, buff);
	}
	
	// set default 
	brk_selChg(hBrk);
	brk_updateView(hBrk);
}

void brk_remove(HWND hBrk)
{
	int sel = listBox_getCurSel(hBrk, IDC_LIST1);
	if((sel < 0) || !brkcb) return;
	brkcb(cbCtx, sel, NULL);
	brk_selChg(hBrk);
	brk_updateView(hBrk);
}

void brk_update(HWND hBrk)
{
	this->brk_remove(hBrk);
	this->brk_add(hBrk);
}

void brk_add(HWND hBrk)
{
	if(!this->brkcb) return;
	
	CpuDbgBrk brk; 
	brk_readCtrl(hBrk, &brk);
	
	int ec = brk_set(hBrk, &brk);
	if(ec >= 0) {	brk_updateView(hBrk);
		listBox_setCurSel(hBrk, IDC_LIST1, ec); }
}



void brk_updateView(HWND hBrk)
{
	CpuDbgBrk brk; char buff[64];
	listBox_reset(hBrk, IDC_LIST1);

	for(int i = 0;; i++) {
		if(brkcb(cbCtx, i, &brk) < 0) break;
		
		
		char* pos = buff + sprintf(buff, "%d. ", i);
		pos += getSpcName(brk.spc, pos);
		pos += sprintf(pos, ", %04X, %04X, ", brk.addr, brk.end);
		
		if(brk.flags & brk.EXEC) *pos++ = 'X';
		if(brk.flags & brk.READ) *pos++ = 'R';
		if(brk.flags & brk.WRITE) *pos++ = 'W';
		if(brk.flags & brk.ONCE) *pos++ = 'O';
		*pos = 0;
		
		listBox_addStr(hBrk, IDC_LIST1, buff);
		
	}
}

void brk_validate(HWND hBrk)
{
	// fetch the sizes
	unsigned addr = getDlgItemHex(hBrk, IDC_ADDR);
	unsigned end = getDlgItemHex(hBrk, IDC_END);
	unsigned size = getDlgItemHex(hBrk, IDC_SIZE1);
	if(size > 0) size -= 1;

	int mode = GetDlgRadioCheck(hBrk, rad1, rad2);
	int sMode = IsDlgButtonChecked(hBrk, rad3);

	// adjust the values
	if(end < addr) { if(mode != 0) 
		addr = end; else end = addr; }
	if(sMode) {	
		if(mode) SATURATE_SUBV(addr, end, size);
		else SATURATE_ADDV(end, addr, size); }
	addr = min_max(addr, sp()->base, sp()->end);
	end = min_max(end, sp()->base, sp()->end);	
	if(sMode == 0) size = end-addr;
	
	// update control state
	setDlgItemHex(hBrk, IDC_ADDR, addr, 4);
	setDlgItemHex(hBrk, IDC_END, end, 4);
	setDlgItemHex(hBrk, IDC_SIZE1, size+1, 1);	
	CheckDlgRadio(hBrk, rad1+mode);
	CheckDlgButton(hBrk, rad3, sMode);
}

};

INT_PTR CpuDbgDlgBrk::brkDlgProc(HWND hwnd, 
	UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLGMSG_SWITCH(
	  ON_MESSAGE(WM_MOUSEWHEEL, 
			sendDlgMsg(hwnd, IDC_LIST1, uMsg, wParam))
			
		CASE_COMMAND(
			// 
			ON_CONTROL(EN_CHANGE, IDC_ADDR, CheckDlgRadio(hwnd, rad1))
			ON_CONTROL(EN_CHANGE, IDC_END, CheckDlgRadio(hwnd, rad2))
			ON_CONTROL(EN_CHANGE, IDC_SIZE1, CheckDlgButton(hwnd, rad3, 1))
			
			
			ON_CONTROL(EN_KILLFOCUS, IDC_ADDR, this->brk_validate(hwnd))
			ON_CONTROL(EN_KILLFOCUS, IDC_END, this->brk_validate(hwnd))
			ON_CONTROL(EN_KILLFOCUS, IDC_SIZE1, this->brk_validate(hwnd))
			ON_COMMAND_RANGE(rad1, rad3, this->brk_validate(hwnd))
			
			
			ON_CONTROL(LBN_SELCHANGE, IDC_LIST1, this->brk_selChg(hwnd));
			
			
		  ON_COMMAND(IDC_CREATE, this->brk_add(hwnd))
		  ON_COMMAND(IDC_DELETE, this->brk_remove(hwnd))
		  ON_COMMAND(IDC_UPDATE, this->brk_update(hwnd))
		  
		
		
		  ON_COMMAND(IDCANCEL, EndDialog(hwnd, 0))
			ON_COMMAND(IDC_CPUDBG_BD, this->brk_create());
		,)
	,)
}

extern const WCHAR resn_CPUDBGBRK[];

void CpuDbgDlgInt::brk_create()
{
	if(!brkcb) return;
	DialogBoxParamW(getModuleBase(), resn_CPUDBGBRK, 
		hwnd, CpuDbgDlgBrk::cbrkDlgProc, (LPARAM)this);
};
