#include "memory.h"
#include <conio.h>

void set_block(DWORD _mov_addr, DWORD _cmp_addr, unsigned char block_bytes[20], int offset_to_code)
{
	int x, y;
	typedef union _cmp_union
	{
		DWORD address;
		byte buffer[4];
	}_cmp_union;

	typedef union _mov_union
	{
		DWORD address;
		byte buffer[4];
	}_mov_union;

	typedef union _jmp_union
	{
		int offset;
		byte buffer[4];
	}_jmp_union;

	_cmp_union cu;
	_mov_union mu;
	_jmp_union ju;


	cu.address = _cmp_addr;
	mu.address = _mov_addr;
	ju.offset = offset_to_code;

	for (x = 0; x < 7; x++)
	{
		if (x == 0) block_bytes[x] = 0x83;
		else if (x == 1) block_bytes[x] = 0x3D;
		else if (x < 6) block_bytes[x] = cu.buffer[x - 2];
		else block_bytes[x] = 0;
	}//x=7

	block_bytes[x] = 0x75;
	x++;
	block_bytes[x] = 0x0B;
	x++;

	for (; x < 15; x++)
	{
		if (x == 9) block_bytes[x] = 0x89;
		else if (x == 10) block_bytes[x] = 0x35;
		else block_bytes[x] = mu.buffer[x - 11];
	}//x = 15
	block_bytes[x] = 0xE9;
	x++;

	for (y = 0; y < 4; y++)
	{
		block_bytes[x] = ju.buffer[y];
		x++;
	}

}

void get_all_enemies(DWORD enemies_list[11])
{
	int x, y, z, a, index = 0;
	DWORD new_list[11] = { 0 };

	for (x = 0; x < 11; x++)
	{
		for (y = x + 1; y < 11; y++)
		{
			if (enemies_list[x] == enemies_list[y]) enemies_list[x] = 0;
		}
	}

	for (z = 0; z < 11; z++)
	{
		if (enemies_list[z] != 0)
		{
			new_list[index] = enemies_list[z];
			index++;
		}
	}

	for (a = 0; a < 11; a++)
	{
		enemies_list[a] = new_list[a];
	}

}

void set_all_enemies_list(DWORD variables_list, DWORD enemies_list[11])
{
	int x = 0;
	int next = 0;
	for (x = 0; x < 11; x++, next += 4)
	{
		enemies_list[x] = memory.readInt(variables_list + next);
	}
}

void hook_and_set_asm(DWORD variables_list, DWORD allocated_memory_address, DWORD jmp_back_address, DWORD address_to_hook)
{
	int _offset = 20;
	int block_bytes = 0;
	int var_dest = 0;
	int x1;
	unsigned char xmm2_bytes[5] = { 0xF3 ,0x0F ,0x10 ,0x56 ,0x38 };
	unsigned char bytes_to_write[11][20];

	for (x1 = 0; x1 < 11; x1++, var_dest += 4, _offset += 20)
	{
		set_block(variables_list + var_dest, variables_list + var_dest, bytes_to_write[x1], 220 - _offset);
		memory.writeBytes(allocated_memory_address + (_offset - 20), bytes_to_write[x1], 20);
	}

	memory.writeBytes(allocated_memory_address + 220, xmm2_bytes, 5);
	memory.jmpBack(allocated_memory_address, jmp_back_address, 225);
	memory.hookAddress(address_to_hook, allocated_memory_address, 5);
}

void clean_memory(DWORD variables_list_address)
{
	unsigned char clear[44] = { 0 };
	memory.writeBytes(variables_list_address, clear, 44);
}


int main()
{
	DWORD enemies_list[11] = { 0 };
	int loop, x;
	BOOL attach_process = memory.attachProcess("WindowsEntryPoint.Windows.exe");
	if (!attach_process) exit(EXIT_SUCCESS);
	DWORD module_address = memory.getModuleAddress("WindowsEntryPoint.Windows.exe");
	DWORD allocated_memory_address = (DWORD)memory.allocateMemory(300);
	DWORD address_to_hook = module_address + 0x39C1F5;
	DWORD jmp_back_address = module_address + 0x39C1FA;
	DWORD variables_list = memory.allocateMemory(44);
	hook_and_set_asm(variables_list, allocated_memory_address, jmp_back_address, address_to_hook);

	while (TRUE)
	{
		clean_memory(variables_list);                   
		Sleep(80);                                       
		set_all_enemies_list(variables_list, enemies_list);
		Sleep(80);
		get_all_enemies(enemies_list);

		system("cls");
		//print all player coordinates to console screen
		for (x = 0; x < 11; x++)
		{
			//enemies_list[x]+0x38 = x_cords , +0x3C = ycoords , +0x40 = zcoords
			printf("address%d: 0x%X\n", x + 1, enemies_list[x]);
		}
		Sleep(100);
	}

	return 0;
}