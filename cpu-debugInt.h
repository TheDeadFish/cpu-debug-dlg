#pragma once
#include "cpu-debug.h"

struct CpuDbgDlgInt : CpuDbgDlg
{
	// common variables
	enum { MAX_SPC = 5, STR_MAX = 32 };
	struct SpcInfo { u32 addr;
		u32 base, end, hexMode; };
	SpcInfo spcInfo[MAX_SPC];
	SpcInfo* sp() { return spcInfo+curSpace; }
	
	// main dialog variables
	WNDPROC listWndProc;
	int rdPos; byte data[128];
	
	// common helpers
	int getSpcName(int i, char* name);
	
	// break dialog functions
	void brk_create(void);
	void brk_cmd(int cmd);
	void brk_once(void);
	int brk_set(HWND hwnd, CpuDbgBrk* brk);
	
	
	
	
	
	
	
	
	
	
	
	
};
