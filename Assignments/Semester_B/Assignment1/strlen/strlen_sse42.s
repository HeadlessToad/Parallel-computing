.section .text
.globl strlen_sse42
.type strlen_sse42, @function

strlen_sse42:
    movq %rdi, %rax           # Move string pointer to %rax (current position)
    pxor %xmm0, %xmm0         # Zero out %xmm0 (creates a null-byte vector)

.L_strlen_loop:
    # pcmpistri compares 16 bytes at (%rax) with %xmm0.
    # $0x08 configures it for "Equal Each" on unsigned bytes.
    # It sets the Zero Flag (ZF) if a null byte is found in the memory operand.
    # %rcx will contain the index of the matching null byte.
    pcmpistri $0x08, (%rax), %xmm0
    
    jz .L_strlen_found        # If ZF is set, we found the end of the string
    
    addq $16, %rax            # Otherwise, advance the pointer by 16 bytes
    jmp .L_strlen_loop        # and repeat

.L_strlen_found:
    addq %rcx, %rax           # Add the null byte index to the current pointer
    subq %rdi, %rax           # Subtract the start pointer to get the length
    ret

# Tell the linker we do not need an executable stack
.section .note.GNU-stack,"",@progbits