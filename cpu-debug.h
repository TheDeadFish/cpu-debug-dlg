#pragma once
#include <windows.h>

struct CpuDbgBrk
{
	int flags, spc;
	int addr, size;
};

struct CpuDbgDlg
{
	// creation api	
	CpuDbgDlg() { ZINIT; }
	~CpuDbgDlg() { destroy(); }
	HWND create(HWND hParent);
	void destroy(void) { DestroyWindow(hwnd); }
	bool isAlive() { return hwnd; }
	
	// control api
	void setSpcAddr(int i, int base, int end);
	void setSpcName(int i, const char* name);
	void setSpace(int spc); void setAddr(int addr); 
	
	// callbacks
	void* cbCtx;
	void (*initcb)(void* ctx, int mode);
	byte (*readcb)(void* ctx, int spc, int addr);
	int (*discb)(void* ctx, char* buff, byte* data, int addr);
	
	// debug interface
	int (*brkcb)(void* ctx, int cmd, CpuDbgBrk* brk);
	
	
//private:
	
	// 
	HWND hwnd; int curSpace;
	RECT viewRC; char* text;
	
	struct SpcInfo { int base, end;
		int addr, hexMode; };	
	enum { MAX_SPC = 5 };
	SpcInfo spcInfo[MAX_SPC];
	SpcInfo* sp() { return spcInfo+curSpace; }
	
	
	// buffer read state
	int disMax, rdPos;
	byte data[128];
	
	// message handlers
	void initDlg(HWND hwnd);
	void initScroll(void);
	void onScroll(WPARAM wParam, int delta);
	void onWheel(WPARAM wParam);
	void goAddr(void);
	void update(void); 
	void updateView(void);
	void paint(HWND hwnd);
	void click(LPARAM lParam);
	
	// helper functions
	char* fmtAddr(char* buff, int addr);
	int fmtHex8(char* buff, byte* data);
	void initCb(int x) { if(initcb) initcb(cbCtx, x); }
	byte read(int wrPos);
	
	
	// break dialog functions
	void breakDlg(void);
};
