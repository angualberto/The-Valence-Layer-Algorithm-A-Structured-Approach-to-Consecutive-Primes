import os

W = 32
R = 12
B = 16
MOD = 1 << W
MASK = MOD - 1

P = 0xB7E15163
Q = 0x9E3779B9

def rotl(val, r_bits):
    val = val & MASK
    r_bits = r_bits % W
    return ((val << r_bits) | (val >> (W - r_bits))) & MASK

def rc5_setup(key):
    c = B // 4
    L = [0] * c
    for i in range(B):
        L[i // 4] = (L[i // 4] + (key[i] << (8 * (i % 4)))) & MASK
    
    t = 2 * R + 2
    S = [0] * t
    S[0] = P
    for i in range(1, t):
        S[i] = (S[i - 1] + Q) & MASK
        
    i = j = 0
    A = B_val = 0
    loops = 3 * max(t, c)
    for _ in range(loops):
        A = S[i] = rotl((S[i] + A + B_val) & MASK, 3)
        B_val = L[j] = rotl((L[j] + A + B_val) & MASK, (A + B_val) % W)
        i = (i + 1) % t
        j = (j + 1) % c
    return S

def main():
    # The key used for nature & portrait images encryption
    key = b"super_secret_rc5"
    print(f"Original Key: '{key.decode()}'")
    
    # Expand the key to subkeys S
    S = rc5_setup(key)
    print("\nExpanded Subkeys Table S (first few entries):")
    for idx in range(6):
        print(f"  S[{idx}]: 0x{S[idx]:08X}")
        
    # We will attack and recover subkey S[2]
    S2_secret = S[2]
    print(f"\n--- Carry Attack on Subkey S[2] (Secret: 0x{S2_secret:08X}) ---")
    
    S2_reconstructed = 0
    
    print(f"{'Bit':<6} | {'Test input A':<12} | {'Real Output':<12} | {'Zero-Bit Output':<12} | {'Decision':<10} | {'Reconstructed':<12}")
    print("-" * 75)
    
    for bit in range(32):
        # Test input A designed to trigger carry at the target bit
        test_A = (1 << bit) - 1
        
        # Real oracle output (addition with the secret subkey)
        output_real = (test_A + S2_secret) & MASK
        
        # Simulated output assuming the target bit is zero
        output_with_bit_zero = (test_A + S2_reconstructed) & MASK
        
        # Compare target bit
        real_bit = (output_real >> bit) & 1
        model_bit = (output_with_bit_zero >> bit) & 1
        
        decision = "0"
        if real_bit != model_bit:
            # If they differ, the secret bit must be 1 (causing a carry or difference)
            S2_reconstructed |= (1 << bit)
            decision = "1"
            
        print(f"{bit:<6} | 0x{test_A:08X} | 0x{output_real:08X} | 0x{output_with_bit_zero:08X} | Set bit to {decision} | 0x{S2_reconstructed:08X}")
        
    print("\n=== CARRY ATTACK VALIDATION RESULT ===")
    print(f"Secret Subkey S[2]: 0x{S2_secret:08X}")
    print(f"Recovered Subkey:  0x{S2_reconstructed:08X}")
    
    if S2_secret == S2_reconstructed:
        print("SUCCESS: Subkey recovered successfully in exactly 32 additions!")
    else:
        print("FAILURE: Recovery failed.")

if __name__ == "__main__":
    main()
