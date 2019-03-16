#pragma once
#include <windows.h>

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
	// creation api	
	CpuDbgDlg() { ZINIT; }
	~CpuDbgDlg() { destroy(); }
	HWND create(HWND hParent);
	void destroy(void) { DestroyWindow(hwnd); }
	bool isAlive() { return hwnd; }
	
	// control api
	void setSpcAddr(int i, int base, int end);
	void setSpcName(int i, const char* name);
	void setSpace(int spc); void setAddr(unsigned addr); 
	
	// callbacks
	void* cbCtx;
	void (*initcb)(void* ctx, int mode);
	byte (*readcb)(void* ctx, int spc, unsigned addr);
	int (*discb)(void* ctx, char* buff, byte* data, unsigned addr);
	
	// debug interface
	int (*brkcb)(void* ctx, int cmd, CpuDbgBrk* brk);
	
//private:
	
	// 
	HWND hwnd; int curSpace;
	RECT viewRC; char* text;
	
	struct SpcInfo { 
		unsigned base, end;
		unsigned addr, hexMode; };
	enum { MAX_SPC = 5, STR_MAX = 32 };
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
	char* fmtAddr(char* buff, unsigned addr);
	int fmtHex8(char* buff, byte* data);
	void initCb(int x) { if(initcb) initcb(cbCtx, x); }
	byte read(int wrPos);
	int getSpcName(int i, char* name);
	
	
	
	// break dialog functions
	void brk_create(void);
	void brk_init(HWND hBrk);
	void brk_cmd(int cmd);
	void brk_once(void);
	
	// break dialog handlers
	void brk_add(HWND hBrk);
	void brk_remove(HWND hBrk);
	void brk_update(HWND hBrk);
	void brk_updateView(HWND hBrk);
	int brk_set(HWND hwnd, CpuDbgBrk* brk);
	
	
	
	
	
	
};
