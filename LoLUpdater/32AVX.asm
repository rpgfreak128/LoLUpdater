.686p
.xmm
.model FLAT

; CPUID Win32
.code 

; int isAvxSupported();
_isAvxSupported proc
xor eax, eax
cpuid
cmp eax, 1 ; does CPUID support eax = 1?
jb not_supported
mov eax, 1
cpuid
and ecx, 018000000h ;check 27 bit (OS uses XSAVE/XRSTOR)
cmp ecx, 018000000h ; and 28 (AVX supported by CPU)
jne not_supported
xor ecx, ecx ; XFEATURE_ENABLED_MASK/XCR0 register number = 0
xgetbv ; XFEATURE_ENABLED_MASK register is in edx:eax
and eax, 110b
cmp eax, 110b ; check the AVX registers restore at context switch
jne not_supported
mov eax, 1
ret
not_supported:
xor eax, eax
ret
_isAvxSupported endp
END