import multiprocessing, time, math, sys

def gerar_primos_ate(limite):
    crivo = bytearray(b'\x01')*(limite+1)
    crivo[0:2] = b'\x00\x00'
    for p in range(2, int(limite**0.5)+1):
        if crivo[p]:
            start = p*p
            crivo[start:limite+1:p] = b'\x00'*(((limite-start)//p)+1)
    return [i for i, v in enumerate(crivo) if v]

def busca_paralela(id_proc, N, primos, res):
    for p in primos:
        if res.value != 0: return
        if N % p == 0:
            res.value = p
            print(f"[+] Processo {id_proc} achou fator: {p} x {N//p}")
            return

if __name__ == "__main__":
    print("Gerando primos ate 2 milhoes...")
    sys.stdout.flush()
    primos = gerar_primos_ate(2000000)
    print(f"Total: {len(primos)} primos")

    p_segredo = 1999993
    q_segredo = 1999973
    N = p_segredo * q_segredo
    print(f"\nALVO N = {N} (13 digitos)")
    print(f"  fatores: {p_segredo} x {q_segredo}")
    print()

    n_procs = multiprocessing.cpu_count()
    fatia = len(primos) // n_procs
    res = multiprocessing.Value('i', 0)
    procs = []

    print(f"Lancando {n_procs} processos em paralelo...")
    t0 = time.time()
    for i in range(n_procs):
        inicio = i * fatia
        fim = (i+1)*fatia if i < n_procs-1 else len(primos)
        p = multiprocessing.Process(target=busca_paralela, args=(i, N, primos[inicio:fim], res))
        procs.append(p)
        p.start()
    for p in procs: p.join()
    t = time.time() - t0

    if res.value:
        print(f"\nRSA QUEBRADO em {t:.4f}s!")
    else:
        print(f"\nNao encontrou apos {t:.4f}s")

    print("\n--- Projecao para RSA real ---")
    print("RSA-2048: sqrt(N) ~ 10^308")
    print("Primos ate sqrt(N): ~10^308 / ln(10^308) ~ 10^306")
    print(f"Com {n_procs} nucleos e esse metodo:")
    print("  tempo ~ bilhoes de anos")
    print("  conclusao: RSA-2048 continua seguro")
