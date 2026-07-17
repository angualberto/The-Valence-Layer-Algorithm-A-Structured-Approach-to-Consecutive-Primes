#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Birch and Swinnerton-Dyer (BSD) Conjecture: Visualizing L(E, 1+delta)
near s=1 using the exact approximate functional equation.

Author: André Gualberto / Antigravity
"""

import math
import numpy as np
import matplotlib.pyplot as plt
from scipy.special import gammaincc

# Point counting and coefficient generation functions
def count_solutions(a1, a2, a3, a4, a6, p):
    if p == 2:
        sols = 0
        for x in (0, 1):
            for y in (0, 1):
                if (y*y + a1*x*y + a3*y - x*x*x - a2*x*x - a4*x - a6) % 2 == 0:
                    sols += 1
        return 2 - sols
    sols = 0
    for x in range(p):
        B = (a1 * x + a3) % p
        C = (-(x*x*x + a2*x*x + a4*x + a6)) % p
        D = (B * B - 4 * C) % p
        if D == 0:
            sols += 1
        else:
            leg = pow(D, (p-1)//2, p)
            if leg == 1:
                sols += 2
    return p - sols

def generate_a_n(a1, a2, a3, a4, a6, N, conductor):
    primes = []
    is_p = [True] * (N + 1)
    is_p[0] = is_p[1] = False
    for p in range(2, N + 1):
        if is_p[p]:
            primes.append(p)
            for i in range(p*p, N + 1, p):
                is_p[i] = False
    
    ap_val = {p: count_solutions(a1, a2, a3, a4, a6, p) for p in primes}
    
    a = [0] * (N + 1)
    a[1] = 1
    
    spf = list(range(N + 1))
    for p in range(2, int(N**0.5) + 1):
        if spf[p] == p:
            for i in range(p*p, N + 1, p):
                if spf[i] == i:
                    spf[i] = p
                    
    for n in range(2, N + 1):
        p = spf[n]
        temp = n
        pk = 1
        k = 0
        while temp % p == 0:
            pk *= p
            temp //= p
            k += 1
        m = temp
        
        if k == 1:
            apk_val = ap_val[p]
        else:
            is_bad = (conductor % p == 0)
            prev2 = 1
            prev1 = ap_val[p]
            for j in range(2, k + 1):
                if is_bad:
                    curr = prev1 * ap_val[p]
                else:
                    curr = ap_val[p] * prev1 - p * prev2
                prev2 = prev1
                prev1 = curr
            apk_val = prev1
        
        if m == 1:
            a[n] = apk_val
        else:
            a[n] = apk_val * a[m]
    return a

def L_exact(s, a, conductor, w, N_terms):
    x = 2.0 * math.pi / math.sqrt(conductor)
    factor2 = w * ((conductor / (4.0 * math.pi * math.pi)) ** (1.0 - s)) * (math.gamma(2.0 - s) / math.gamma(s))
    
    sum1 = sum(a[n] * gammaincc(s, n * x) / (n ** s) for n in range(1, N_terms + 1))
    sum2 = sum(a[n] * gammaincc(2.0 - s, n * x) / (n ** (2.0 - s)) for n in range(1, N_terms + 1))
    
    return sum1 + factor2 * sum2

def main():
    print("Generating coefficients for the curves...")
    # Generate coefficients
    a_11 = generate_a_n(0, -1, 1, -10, -20, 1000, 11)
    a_37 = generate_a_n(0, 0, 1, -1, 0, 3000, 37)
    a_389 = generate_a_n(0, 1, 1, -2, 0, 8000, 389)
    a_5077 = generate_a_n(0, 0, 1, -7, 6, 25000, 5077)

    delta_vals = np.linspace(-0.2, 0.2, 100)
    
    print("Calculating L(E, 1+delta) for 11a1 (Rank 0)...")
    L_11 = [L_exact(1.0 + d, a_11, 11, 1, 1000) for d in delta_vals]
    
    print("Calculating L(E, 1+delta) for 37a1 (Rank 1)...")
    L_37 = [L_exact(1.0 + d, a_37, 37, -1, 3000) for d in delta_vals]
    
    print("Calculating L(E, 1+delta) for 389a1 (Rank 2)...")
    L_389 = [L_exact(1.0 + d, a_389, 389, 1, 8000) for d in delta_vals]
    
    print("Calculating L(E, 1+delta) for 5077a1 (Rank 3)...")
    L_5077 = [L_exact(1.0 + d, a_5077, 5077, -1, 25000) for d in delta_vals]

    print("Generating the plot...")
    plt.figure(figsize=(12, 8))
    
    plt.plot(delta_vals, L_11, label='11a1 (Rank 0) - Non-vanishing', color='#FF5722', linewidth=2)
    plt.plot(delta_vals, L_37, label='37a1 (Rank 1) - Simple Zero (Crosses axis)', color='#4CAF50', linewidth=2)
    plt.plot(delta_vals, L_389, label='389a1 (Rank 2) - Double Zero (Parabolic touch)', color='#2196F3', linewidth=2)
    plt.plot(delta_vals, L_5077, label='5077a1 (Rank 3) - Triple Zero (Cubical flat)', color='#9C27B0', linewidth=2)
    
    plt.axhline(y=0, color='#333333', linestyle='--', linewidth=0.8)
    plt.axvline(x=0, color='#d32f2f', linestyle='-', linewidth=1.2, label='s = 1 (Critical Center)')
    
    plt.xlim(-0.2, 0.2)
    plt.ylim(-0.3, 0.5)
    
    plt.xlabel('delta (s = 1 + delta)', fontsize=12)
    plt.ylabel('L(E, s)', fontsize=12)
    plt.title('Birch and Swinnerton-Dyer: Visualizing L(E, 1+delta) around s=1', fontsize=14, fontweight='bold', pad=15)
    plt.legend(fontsize=11, loc='upper left')
    plt.grid(True, linestyle=':', alpha=0.6)
    
    # Save the plot
    output_path = 'demo/bsd_delta_analysis.png'
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Plot saved successfully to {output_path}")

if __name__ == '__main__':
    main()
