# RC5 Image Encryption Demo & Carry Attack Validation

This directory demonstrates a practical, end-to-end realization of the carry-based subkey recovery attack on the RC5 block cipher (RC5-32/12/16) in CBC mode.

## Files
- `rc5_image_cbc.py`: The Python script implementing RC5 block operations, key schedule, PKCS#7 padding, and CBC image mode.
- `original.bmp` / `original2.bmp`: Original pixel maps.
- `encrypted.bmp` / `encrypted2.bmp`: Encrypted output showing visual entropy (noise) while preserving structural header compatibility.
- `decrypted.bmp` / `decrypted2.bmp`: Perfectly reconstructed original pixel maps (validated byte-for-byte).

## Reproducing the Image Loop
```bash
python rc5_image_cbc.py
```
This script runs the entire sequence automatically, validating that `decrypted.bmp` matches `original.bmp` perfectly.

## Running the Carry Attack
Navigate to the `../carry_attack/` directory and execute:
```bash
python test_carry_attack_on_key.py
```
The attack targets the subkey table $S[2]$ of the expanded `"super_secret_rc5"` key schedule, recovering the exact 32-bit secret subkey using exactly 32 additions.
