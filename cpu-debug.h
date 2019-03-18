#pragma once

#define CPUDBGDLG_CALL extern "C" __stdcall

// define basic types
#ifndef _WINDOWS_
 typedef void* HWND;
#endif
typedef unsigned char byte;
typedef unsigned int u32;



struct CpuDbgBrk
{
	int flags, spc; unsigned addr, end;
	enum { STEPI = -1, STEPO = -2, 
		RRET = -3, CONT = -4, STOP = -5 };
	enum { EXEC=1, READ=2, WRITE=4, ONCE=8 };
	
	
		
		
		
		
	bool operator==(const CpuDbgBrk& That) {
		return !memcmp(this, &That, sizeof(*this)); }
};

struct CpuDbgBrkLst
{
	enum { BRK_MAX = 8 };
	
	CpuDbgBrk data[BRK_MAX]; 
	int len;

	static bool isCmd(int cmd) { return cmd < 0; }

	//static bool reqValidate(int cmd) {
	//	return (cmd>>16)>0) && (short(cmd)>= 0); }
	
	
	
	
	int add(int cmd, CpuDbgBrk* brk);
	void remove(int index);
};


struct CpuDbgDlg
{
	HWND hwnd; 
	int curSpace;

	// callbacks
	void* cbCtx;
	void (*initcb)(void* ctx, int mode);
	byte (*readcb)(void* ctx, int spc, u32 addr);
	int (*discb)(void* ctx, char* buff, byte* data, u32 addr);
	int (*brkcb)(void* ctx, int cmd, CpuDbgBrk* brk);
};


// creation api
CPUDBGDLG_CALL CpuDbgDlg* cpuDbgDlg_create_(HWND hParent);
CPUDBGDLG_CALL CpuDbgDlg* cpuDbgDlg_create(CpuDbgDlg**, HWND hParent);
CPUDBGDLG_CALL CpuDbgDlg* cpuDbgDlg_toggle(CpuDbgDlg**, HWND hParent);
CPUDBGDLG_CALL void cpuDbgDlg_destroy(CpuDbgDlg**);

// control api
CPUDBGDLG_CALL HWND cpuDbgDlg_alive(CpuDbgDlg*);
CPUDBGDLG_CALL void cpuDbgDlg_initSpc(CpuDbgDlg*, int i, 
	u32 base, u32 end, cch* name);
CPUDBGDLG_CALL void cpuDbgDlg_setSpc(CpuDbgDlg*, int i);
CPUDBGDLG_CALL void cpuDbgDlg_setAddr(CpuDbgDlg*, u32 addr);
