#include "stdshit.h"
#include "win32hlp.h"
#include "cpu-debug-res.h"
#include "cpu-debug.h"

#define DISASM_MAX 13
#define OPSIZE_MAX 8

static
WNDPROC subclass_window(HWND hwnd, WNDPROC proc, void* This)
{
	SetWindowLongPtrW(hwnd, GWL_USERDATA, (DWORD_PTR)This);
	return (WNDPROC)SetWindowLongPtrW(hwnd, GWL_WNDPROC, (INT_PTR)proc);
}

static 
WNDPROC subclass_control(HWND hwnd, int ctrlID, WNDPROC proc, void* This) {
	return subclass_window(GetDlgItem(hwnd, ctrlID), proc, This); }

int dlgScroll_setPos(HWND hwnd, int ctrlID, int pos)
{
	HWND hScroll = GetDlgItem(hwnd, ctrlID);
	SCROLLINFO si = {sizeof(si), SIF_POS};
	si.nPos = pos;
	SetScrollInfo(hScroll, SB_CTL, &si, TRUE);
	GetScrollInfo(hScroll, SB_CTL, &si);
	return si.nPos;
}

static WNDPROC listWndProc;
LRESULT CALLBACK listWndProcSC(
	HWND   hwnd,	UINT   uMsg,
	WPARAM wParam, LPARAM lParam
) {
	GET_WND_CONTEXT(CpuDbgDlg);

	if(uMsg == WM_MOUSEWHEEL) {
		int count = short(HIWORD(wParam)) / -30;
		This->onScroll(SB_LINEDOWN, count);
		return 0; }
	
	return listWndProc(hwnd, uMsg, wParam, lParam);
}

byte CpuDbgDlg::read(int wrPos)
{
	byte rdat = -1;
	int addr = sp()->addr + wrPos;
	if(inRng1(addr, sp()->base, sp()->end))
		rdat = readcb(cbCtx, curSpace, addr);
	return data[wrPos] = rdat;
}

int CpuDbgDlg::fmtHex8(char* buff, byte* data)
{
	sprintf(buff, "%.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X ",	data[0], 
		data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
	byte tmp[8]; for(int i = 0; i < 8; i++) {
		tmp[i] = data[i] ? data[i] : '.'; }
	sprintf(buff+18, "%.8s", tmp); return 8;
}

void CpuDbgDlg::updateView(void)
{
	// read buffer
	int wrPos = 0; if(rdPos) {
		while(wrPos < rdPos) { byte prev = data[wrPos];
			if(prev != read(wrPos++)) goto DATA_CHANGE;
		} return; DATA_CHANGE: rdPos = 0;
	}
	
	listBox_reset(hwnd, IDC_CPU_DISAM);
	
	char buff[256];
	for(int i = 0; i < DISASM_MAX; i++) {
	
		// perform read
		int reqPos = rdPos + 8;
		for(;wrPos < reqPos; wrPos++) 
			read(wrPos);
			
		int addr = sp()->addr + rdPos;
		if(addr > sp()->end) break;
		char* pos = fmtAddr(buff, addr);
		rdPos += sp()->hexMode ? fmtHex8(pos, data+rdPos) 
			: discb(cbCtx, pos, data+rdPos, addr);
		
		listBox_addStr(hwnd, IDC_CPU_DISAM, buff);
	}
}

void CpuDbgDlg::initDlg(HWND hwnd)
{
	this->hwnd = hwnd;
	listWndProc = subclass_control(hwnd, 
		IDC_CPU_DISAM, listWndProcSC, this);
	
	// setup control fonts
	HFONT hFont = (HFONT) GetStockObject(OEM_FIXED_FONT);
	sendDlgMsg(hwnd, IDC_CPU_DISAM,  WM_SETFONT, (WPARAM)hFont, TRUE);
	
	setSpace(0); SetTimer(hwnd, 1, 10, 0);
	this->initCb(1);
}

void CpuDbgDlg::update()
{
	if(readcb && (discb || sp()->hexMode))
		this->updateView();
}

char* CpuDbgDlg::fmtAddr(char* buff, int addr)
{
	int count = 1; 
	for(unsigned value = sp()->end;
		value >>= 4; count++);
		
	buff += sprintf(buff, "%0.*X ", count, addr);
	return buff;
}

void CpuDbgDlg::setAddr(int pos)
{
	// update scrollbar position
	pos = dlgScroll_setPos(hwnd, IDC_CPU_SCROLL, pos);
	this->sp()->addr = pos; rdPos = 0;
	
	// update address edit
	char buff[32]; fmtAddr(buff, pos);
	SetDlgItemTextA(hwnd, IDC_ADDR, buff);
}

void CpuDbgDlg::onScroll(WPARAM wParam, int delta)
{
	SCROLLINFO si = {sizeof(si), SIF_ALL};
	GetScrollInfo(GetDlgItem(hwnd,
		IDC_CPU_SCROLL), SB_CTL, &si);
	delta *= sp()->hexMode ? 8 : 1;
	
	int nPos = si.nPos;
	switch(LOWORD (wParam)) { 
	case SB_PAGEUP: nPos -= si.nPage; break; 
	case SB_PAGEDOWN: nPos += si.nPage; break; 
	case SB_LINEUP: nPos -= delta; break;
	case SB_LINEDOWN: nPos += delta; break; 
	case SB_THUMBTRACK: nPos = si.nTrackPos;
	}
		
	if(sp()->hexMode) {
		if(nPos < si.nPos) nPos += 7;
		nPos &= ~7; }
	this->setAddr(nPos);
}

void CpuDbgDlg::goAddr(void)
{
	char buff[32]; char* endPtr;
	GetDlgItemTextA(hwnd, IDC_ADDR, buff, 32);
	unsigned addr = strtoul(buff, &endPtr, 16);
	if(endPtr != buff) sp()->addr = addr;
	this->setAddr(sp()->addr);
}


void CpuDbgDlg::initScroll(void)
{
	rdPos = 0;
	sp()->hexMode = IsDlgButtonChecked(hwnd, IDC_HEXMODE);
	
	SCROLLINFO si = {sizeof(si), SIF_RANGE | SIF_PAGE,
		sp()->base, sp()->end, DISASM_MAX};
	if(sp()->hexMode) si.nPage *= 8;
	SetScrollInfo(GetDlgItem(hwnd, IDC_CPU_SCROLL),
		SB_CTL, &si, TRUE);
}

void CpuDbgDlg::setSpace(int space)
{
	this->curSpace = space;
	CheckDlgRadio(hwnd, IDC_ADDRSPC1+space);
	CheckDlgButton(hwnd, IDC_HEXMODE, sp()->hexMode);
	this->initScroll(); this->setAddr(sp()->addr);	
}

void CpuDbgDlg::setSpcName(int i, const char* name)
{
	HWND hItem = GetDlgItem(hwnd, IDC_ADDRSPC1+i);
	SetWindowTextA(hItem, name);
	ShowWindow(hItem,name ? SW_SHOW : SW_HIDE);
	if(!name && (i == curSpace)); setSpace(0);
}

int CpuDbgDlg::getSpcName(int i, char* name)
{
	return GetDlgItemTextA(hwnd, IDC_ADDRSPC1+i, name, 32);
}

void CpuDbgDlg::setSpcAddr(int i, int base, int end)
{
	spcInfo[i].base = base; spcInfo[i].end = end;
	if(i == curSpace) this->setAddr(sp()->addr);
}

static
INT_PTR CALLBACK mainDlgProc(
	HWND hwnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	INIT_DLG_CONTEXT(CpuDbgDlg, This->initDlg(hwnd));

	DLGMSG_SWITCH(
	  ON_MESSAGE(WM_MOUSEWHEEL, 
			sendDlgMsg(hwnd, IDC_CPU_DISAM, uMsg, wParam))
		ON_MESSAGE(WM_DESTROY, This->initCb(0); This->hwnd = 0)
		ON_MESSAGE(WM_TIMER, This->update())
		ON_MESSAGE(WM_VSCROLL, This->onScroll(wParam, 1))
		CASE_COMMAND(
			// debugger commands
		  ON_COMMAND(IDC_CPUDBG_SI, This->brk_cmd(CpuDbgBrk::STEPI));
			ON_COMMAND(IDC_CPUDBG_SO, This->brk_cmd(CpuDbgBrk::STEPO));
			ON_COMMAND(IDC_CPUDBG_RR, This->brk_cmd(CpuDbgBrk::RRET));
			ON_COMMAND(IDC_CPUDBG_CO, This->brk_cmd(CpuDbgBrk::CONT));
			ON_COMMAND(IDC_CPUDBG_ST, This->brk_cmd(CpuDbgBrk::STOP));
			ON_COMMAND(IDC_CPUDBG_BR, This->brk_once());
			ON_COMMAND(IDC_CPUDBG_BD, This->brk_create());
		  
			ON_COMMAND(IDC_HEXMODE, This->initScroll());
			ON_COMMAND(IDCANCEL, DestroyWindow(hwnd));
	 
		ON_COMMAND(IDC_ADDRGO, This->goAddr())
		ON_RADIO_RNG(IDC_ADDRSPC1, IDC_ADDRSPC5,
			This->setSpace(index))
	 ,)
	,)
}

HWND CpuDbgDlg::create(HWND hParent)
{
	if(isAlive()) { ShowWindow(hwnd,
		SW_SHOW); return hwnd; }
	return CreateDialogParamW(getModuleBase(),
		L"CPUDBGDLG", hParent, mainDlgProc, (LPARAM)this);		
}
