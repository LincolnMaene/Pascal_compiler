#include <iostream>
using namespace std;
char DataSegment[65536];
int main()
{
	_asm{
		pushad         // Assembler header for all programs (Top)
		lea ebp, DataSegment
		jmp kmain       // Assembler header for all programs (Bottom)


		kmain:

		
		mov EAX,0
		
		mov [ebp+8],EAX
		
		mov EAX,10
		
		mov [ebp+0],EAX
		
		WHILE1:
		
	
		
		mov EAX,[ebp+0]
		
		mov EBX,0
		CMP EAX,EBX
		
		JG w1top
		
		JMP w1end
		
		w1top:
		
		mov EAX,0
		
		mov [ebp+4],EAX
		
		WHILE2:
		
	
		
		mov EAX,[ebp+4]
		
		mov EBX,10
		CMP EAX,EBX
		
		JL w2top
		
		JMP w2end
		
		w2top:
		
		mov EAX,[ebp+4]
		
		mov EBX,1
		add EAX,EBX
		
		mov [ebp+4],EAX
		
		mov EAX,[ebp+8]
		
		mov EBX,1
		add EAX,EBX
		
		mov [ebp+8],EAX
		
		JMP WHILE2
		
		w2end:
		
		mov EAX,0
		
		mov [ebp+4],EAX
		
		WHILE3:
		
	
		
		mov EAX,[ebp+4]
		
		mov EBX,10
		CMP EAX,EBX
		
		JL w3top
		
		JMP w3end
		
		w3top:
		
		mov EAX,[ebp+4]
		
		mov EBX,1
		add EAX,EBX
		
		mov [ebp+4],EAX
		
		mov EAX,[ebp+8]
		
		mov EBX,1
		add EAX,EBX
		
		mov [ebp+8],EAX
		
		JMP WHILE3
		
		w3end:
		
		mov EAX,[ebp+0]
		
		mov EBX,1
		sub EAX,EBX
		
		mov [ebp+0],EAX
		
		JMP WHILE1
		
		w1end:
		

		popad    // Assembler footer for all programs (Bottom)
	}
return 0;
}
