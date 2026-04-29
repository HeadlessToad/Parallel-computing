/* <Replace this comment with your ID and Name> */

long decode_c_version(long x, long y, long z) {
    // subq %rdx, %rsi
    long y_minus_z = y - z;
    
    // imulq %rsi, %rdi
    long x_times_y_minus_z = x * y_minus_z;
    
    // movq %rsi, %rax
    long result = y_minus_z;
    
    // salq $63, %rax
    result <<= 63;
    
    // sarq $63, %rax 
    // (Arithmetic right shift propagates the sign bit)
    result >>= 63;
    
    // xorq %rdi, %rax
    return result ^ x_times_y_minus_z;
}
