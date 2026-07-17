#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Birch and Swinnerton-Dyer (BSD) Conjecture: High-Order Derivatives of L-Functions
via the Approximate Functional Equation and Exponential Integrals.

Author: André Gualberto / Antigravity
"""

import sys
import math
from scipy.special import expn

def print_banner():
    banner = """
========================================================================
           BIRCH AND SWINNERTON-DYER (BSD) CONJECTURE TESTER
                 -- High-Order Derivative Analyzer --
========================================================================
This algorithm computes the L-series L(E, s) and its higher derivatives
L^(k)(E, 1) up to k=3 using the approximate functional equation:
  L^(k)(E, 1) = 2 * (-1)^k * sum_{n=1}^N (a_n / n) * E_k(2pi*n / sqrt(N_E))
where E_k is the exponential integral (and E_0(y) = e^-y).
========================================================================
"""
    print(banner)

def count_solutions(a1, a2, a3, a4, a6, p):
    if p == 2:
        sols = 0
        for x in (0, 1):
            for y in (0, 1):
                lhs = (y*y + a1*x*y + a3*y) % 2
                rhs = (x*x*x + a2*x*x + a4*x + a6) % 2
                if lhs == rhs:
                    sols += 1
        return 2 - sols
        
    sols = 0
    for x in range(p):
        B = (a1 * x + a3) % p
        C = (x*x*x + a2*x*x + a4*x + a6) % p
        D = (B * B + 4 * C) % p
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

def L_derivada_ordem(a, conductor, w, ordem, N):
    x = 2.0 * math.pi / math.sqrt(conductor)
    # Symmetry parity check: if w * (-1)^ordem == -1, then L^(ordem)(E, 1) is analytically 0
    if w * ((-1) ** ordem) == -1:
        return 0.0
    
    fator = 2.0 * ((-1) ** ordem)
    if ordem == 0:
        kernel = lambda n: math.exp(-n * x)
    else:
        kernel = lambda n: expn(ordem, n * x)
        
    soma = sum(a[n] * kernel(n) / n for n in range(1, N + 1))
    return fator * soma

def verify_curve_derivatives(name, a1, a2, a3, a4, a6, conductor, w, known_rank, N_terms):
    print(f"Curve: {name} (Known Rank: {known_rank}, Conductor: {conductor}, Sign: {w})")
    print(f"  Weierstrass: y^2 + {a1}xy + {a3}y = x^3 + {a2}x^2 + {a4}x + {a6}")
    print(f"  Generating coefficients (N = {N_terms})...")
    a = generate_a_n(a1, a2, a3, a4, a6, N_terms, conductor)
    
    derivatives = {}
    print("  Computing derivatives L^(k)(E, 1) for k = 0, 1, 2, 3:")
    for k in range(4):
        val = L_derivada_ordem(a, conductor, w, k, N_terms)
        derivatives[k] = val
        print(f"    L^({k})(1) = {val:12.8f}")
        
    # Verify BSD
    # Find order of vanishing
    order_vanishing = 0
    while order_vanishing < 4 and abs(derivatives[order_vanishing]) < 1e-4:
        order_vanishing += 1
        
    if order_vanishing < 4:
        predicted_rank = order_vanishing
        detail = f"First non-vanishing derivative is L^({order_vanishing})(1)"
    else:
        predicted_rank = ">= 4"
        detail = "All computed derivatives L^(0), L^(1), L^(2), L^(3) vanish"
        
    print(f"  BSD Prediction: Rank = {predicted_rank} ({detail})")
    
    if str(predicted_rank) == str(known_rank) or (isinstance(predicted_rank, str) and predicted_rank.startswith(">=") and known_rank >= 4):
        print("  [SUCCESS] Match confirmed!")
    else:
        print("  [WARNING] Mismatch with known rank.")
    print("-" * 72)

def main():
    print_banner()
    
    # 1. 11a1 (rank 0, w=1)
    verify_curve_derivatives("11a1", 0, -1, 1, -10, -20, 11, 1, 0, 1000)
    
    # 2. 37a1 (rank 1, w=-1)
    verify_curve_derivatives("37a1", 0, 0, 1, -1, 0, 37, -1, 1, 5000)
    
    # 3. 389a1 (rank 2, w=1)
    verify_curve_derivatives("389a1", 0, 1, 1, -2, 0, 389, 1, 2, 8000)
    
    # 4. 5077a1 (rank 3, w=-1)
    verify_curve_derivatives("5077a1", 0, 0, 1, -7, 6, 5077, -1, 3, 25000)

if __name__ == '__main__':
    main()
