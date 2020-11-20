# External hook and detour steps:
- allocate some memory in the target app with virtualalloc and store it's address in a variable
- again allocate more memory and store it's address in a variable , this time let's call this variable y, and the other x .
- start writing your asm code to that location of x , this asm code should compare and move (eax as example) to the array y (_asm keyword and naked function is useless in this step)
- after finish writing your asm code now write the original bytes that we need to hook at the end of x
- then write the jmp back
- hook the target address , to jmp to address of x
- then we need to clear the array of bytes of y every 120+ms , so those addresses will update every time
- use readprocessMemory to get the array of bytes of y , every 4bytes in y , is an address was inside eax .
- after storing all those addresses make a function to remove the duplicated addresses .
