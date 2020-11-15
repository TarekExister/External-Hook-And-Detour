#pragma once

#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>

HANDLE hProc;
DWORD pID;

//Pointers for functions
typedef void (*ptr_writeBytes)(DWORD, unsigned char[], int);
typedef void (*ptr_readBytes)(DWORD, unsigned char[]);
typedef BOOL(*ptr_attach_process)(char*);
typedef DWORD(*ptr_get_module_address)(char*);
typedef DWORD(*ptr_get_pointer_address)(DWORD, int[], int);
typedef DWORD(*ptr_allocate_memory)(int);
typedef void (*ptr_hook_address)(DWORD, DWORD, int);
typedef void (*ptr_jmp_back)(DWORD, DWORD, int);
typedef unsigned int (*ptr_read_int)(DWORD);

BOOL func_attach_process(char* procName)
{
	PROCESSENTRY32 procEntry32;
	HANDLE hsnapProc;

	procEntry32.dwSize = sizeof(PROCESSENTRY32);
	hsnapProc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hsnapProc == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}


	while (Process32Next(hsnapProc, &procEntry32))
	{
		if (!strcmp(procName, procEntry32.szExeFile))//character set: multibyte
		{
			pID = procEntry32.th32ProcessID;
			hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
			CloseHandle(hsnapProc);
			return TRUE;
		}
	}

	return FALSE;
}

void func_writeBytes(DWORD address, unsigned char bts[], int size)
{
	WriteProcessMemory(hProc, (LPVOID)address, bts, size, 0);
}

void func_readBytes(DWORD address, unsigned char bts[])
{
	ReadProcessMemory(hProc, (LPCVOID)address, (LPVOID)bts, sizeof(bts), 0);
}

unsigned int func_readInt(DWORD address)
{
	unsigned int x = 0;
	ReadProcessMemory(hProc, (LPCVOID)address, (LPVOID)&x, sizeof(int), 0);
	return x;
}

DWORD func_get_module_address(char* module_name)
{
	HANDLE hsnapM;
	MODULEENTRY32 mod;

	mod.dwSize = sizeof(MODULEENTRY32);
	hsnapM = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

	if (hsnapM == INVALID_HANDLE_VALUE) return 0x0;

	while (Module32Next(hsnapM, &mod))
	{
		if (strcmp(mod.szModule, module_name) == 0)
		{
			CloseHandle(hsnapM);
			return (DWORD)mod.modBaseAddr;
		}
	}
}

DWORD func_get_pointer_address(DWORD ptr_base_address, int offsets[], int PointerLevel)
{
	DWORD address = ptr_base_address;
	DWORD temp;
	int x;

	for (x = 0; x < PointerLevel; x++)
	{
		ReadProcessMemory(hProc, (LPCVOID)address, &temp, sizeof(temp), NULL);
		address = temp + offsets[x];
	}
	return address;
}

DWORD func_allocate_memory(int sz)
{
	return (DWORD)VirtualAllocEx(hProc, NULL, sz, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

int jmp(int from, int to)
{
	return (to - from) - 5;
}

void func_hook_address(DWORD address, DWORD allocated_memory, int sz)
{
	int offset = jmp(address, allocated_memory);
	BYTE bytes_to_write[100];
	int x;


	union Myunion
	{
		int _offset;
		BYTE buffer[4];
	};
	union Myunion u;
	bytes_to_write[0] = 0xE9;

	u._offset = offset;
	for (x = 0; x < sz; x++)
	{
		if ((x == 0) || (x == 1) || (x == 2) || (x == 3))
			bytes_to_write[x + 1] = u.buffer[x];
		else bytes_to_write[x + 1] = 0x90;
	}

	WriteProcessMemory(hProc, (LPVOID)address, bytes_to_write, sz, NULL);

}

void func_jmp_back(DWORD allocated_memory_address, DWORD jmp_back_address, int dest)
{
	BYTE jmp_back_buffer[5];
	int offset = jmp(allocated_memory_address + dest, jmp_back_address);
	int x;

	union Myunion
	{
		int _offset;
		BYTE buffer[4];
	};

	union Myunion u;
	u._offset = offset;
	for (x = 0; x < 5; x++)
	{
		if (x == 0) jmp_back_buffer[0] = 0xE9;
		else jmp_back_buffer[x] = u.buffer[x - 1];
	}
	WriteProcessMemory(hProc, (LPVOID)(allocated_memory_address + dest), jmp_back_buffer, 5, NULL);
}

typedef struct _memory
{
	ptr_attach_process attachProcess;
	ptr_readBytes readBytes;
	ptr_writeBytes writeBytes;
	ptr_get_module_address getModuleAddress;
	ptr_get_pointer_address getPointerAddress;
	ptr_allocate_memory allocateMemory;
	ptr_hook_address hookAddress;
	ptr_jmp_back jmpBack;
	ptr_read_int readInt;
}_memory;

_memory memory =
{
	&func_attach_process,
	&func_readBytes,
	&func_writeBytes,
	&func_get_module_address,
	&func_get_pointer_address,
	&func_allocate_memory,
	&func_hook_address,
	&func_jmp_back,
	&func_readInt
};