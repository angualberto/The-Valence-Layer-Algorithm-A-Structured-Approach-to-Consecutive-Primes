import os
import sys

# Ensure PIL is installed or try to import it
try:
    from PIL import Image
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "pillow"])
    from PIL import Image

# RC5-32/12/16 implementation
W = 32  # word size in bits
R = 12  # number of rounds
B = 16  # key size in bytes

MOD = 1 << W
MASK = MOD - 1

P = 0xB7E15163
Q = 0x9E3779B9

def rotl(val, r_bits):
    val = val & MASK
    r_bits = r_bits % W
    return ((val << r_bits) | (val >> (W - r_bits))) & MASK

def rotr(val, r_bits):
    val = val & MASK
    r_bits = r_bits % W
    return ((val >> r_bits) | (val << (W - r_bits))) & MASK

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

def rc5_encrypt_block(pt_A, pt_B, S):
    A = (pt_A + S[0]) & MASK
    B_val = (pt_B + S[1]) & MASK
    for i in range(1, R + 1):
        A = (rotl(A ^ B_val, B_val % W) + S[2 * i]) & MASK
        B_val = (rotl(B_val ^ A, A % W) + S[2 * i + 1]) & MASK
    return A, B_val

def rc5_decrypt_block(ct_A, ct_B, S):
    A = ct_A
    B_val = ct_B
    for i in range(R, 0, -1):
        B_val = rotr((B_val - S[2 * i + 1]) & MASK, A % W) ^ A
        A = rotr((A - S[2 * i]) & MASK, B_val % W) ^ B_val
    B_val = (B_val - S[1]) & MASK
    A = (A - S[0]) & MASK
    return A, B_val

def pad_data(data):
    # RC5 works on 8-byte blocks (two 32-bit words)
    pad_len = 8 - (len(data) % 8)
    if pad_len == 8:
        # We always pad at least 1 byte
        pad_len = 8
    # PKCS#7 padding
    return data + bytes([pad_len] * pad_len)

def unpad_data(data):
    pad_len = data[-1]
    if pad_len < 1 or pad_len > 8:
        return data
    # Verify padding
    for i in range(len(data) - pad_len, len(data)):
        if data[i] != pad_len:
            return data
    return data[:-pad_len]

def encrypt_image_cbc(input_bmp_path, output_bmp_path, key):
    S = rc5_setup(key)
    with open(input_bmp_path, "rb") as f:
        bmp_data = f.read()
    
    # BMP Header is typically 54 bytes
    # But let's find the exact pixel data offset from the BMP header (bytes 10-13)
    pixel_offset = int.from_bytes(bmp_data[10:14], byteorder="little")
    header = bmp_data[:pixel_offset]
    pixel_data = bmp_data[pixel_offset:]
    
    padded_pixels = pad_data(pixel_data)
    encrypted_pixels = bytearray()
    
    # CBC Initialization Vector (IV)
    iv_A, iv_B = 0x12345678, 0x9ABCDEF0
    
    for i in range(0, len(padded_pixels), 8):
        block = padded_pixels[i:i+8]
        pt_A = int.from_bytes(block[0:4], byteorder="little")
        pt_B = int.from_bytes(block[4:8], byteorder="little")
        
        # CBC XOR
        pt_A ^= iv_A
        pt_B ^= iv_B
        
        ct_A, ct_B = rc5_encrypt_block(pt_A, pt_B, S)
        
        # Next IV is the ciphertext
        iv_A, iv_B = ct_A, ct_B
        
        encrypted_pixels.extend(ct_A.to_bytes(4, byteorder="little"))
        encrypted_pixels.extend(ct_B.to_bytes(4, byteorder="little"))
        
    with open(output_bmp_path, "wb") as f:
        f.write(header + encrypted_pixels)
    print(f"Encrypted pixels saved to {output_bmp_path}")

def decrypt_image_cbc(input_bmp_path, output_bmp_path, key):
    S = rc5_setup(key)
    with open(input_bmp_path, "rb") as f:
        bmp_data = f.read()
        
    pixel_offset = int.from_bytes(bmp_data[10:14], byteorder="little")
    header = bmp_data[:pixel_offset]
    pixel_data = bmp_data[pixel_offset:]
    
    decrypted_pixels = bytearray()
    
    iv_A, iv_B = 0x12345678, 0x9ABCDEF0
    
    for i in range(0, len(pixel_data), 8):
        block = pixel_data[i:i+8]
        ct_A = int.from_bytes(block[0:4], byteorder="little")
        ct_B = int.from_bytes(block[4:8], byteorder="little")
        
        dec_A, dec_B = rc5_decrypt_block(ct_A, ct_B, S)
        
        # CBC XOR
        pt_A = dec_A ^ iv_A
        pt_B = dec_B ^ iv_B
        
        # Next IV is the ciphertext
        iv_A, iv_B = ct_A, ct_B
        
        decrypted_pixels.extend(pt_A.to_bytes(4, byteorder="little"))
        decrypted_pixels.extend(pt_B.to_bytes(4, byteorder="little"))
        
    unpadded_pixels = unpad_data(decrypted_pixels)
    
    with open(output_bmp_path, "wb") as f:
        f.write(header + unpadded_pixels)
    print(f"Decrypted pixels saved to {output_bmp_path}")

def main():
    base_dir = r"C:\Users\Andre\.gemini\antigravity\brain\1eea3146-6482-4f3e-ba79-bde90cce8fad"
    jpg_path = os.path.join(base_dir, "test_image_to_encrypt_1784316363751.jpg")
    
    # 1. Convert JPEG to BMP
    bmp_orig = os.path.join(base_dir, "test_image_orig.bmp")
    img = Image.open(jpg_path)
    img.save(bmp_orig)
    print("Converted JPEG to original BMP.")
    
    # 2. Key
    key = b"antigravity_key!" # 16 bytes
    
    # 3. Encrypt
    bmp_enc = os.path.join(base_dir, "test_image_encrypted.bmp")
    encrypt_image_cbc(bmp_orig, bmp_enc, key)
    
    # 4. Decrypt
    bmp_dec = os.path.join(base_dir, "test_image_decrypted.bmp")
    decrypt_image_cbc(bmp_enc, bmp_dec, key)
    
    # 5. Convert encrypted and decrypted BMPs to JPEGs for viewing
    img_enc = Image.open(bmp_enc)
    # Note: Pillow might complain about opening a BMP with encrypted pixels,
    # but since the header is valid, it works or we can save it as JPEG.
    # If the image format is weird, it might not render properly, but it's a valid BMP.
    # Let's save them as JPEGs.
    jpg_enc = os.path.join(base_dir, "test_image_encrypted.jpg")
    img_enc.convert("RGB").save(jpg_enc)
    
    img_dec = Image.open(bmp_dec)
    jpg_dec = os.path.join(base_dir, "test_image_decrypted.jpg")
    img_dec.convert("RGB").save(jpg_dec)
    
    # 6. Verify integrity
    with open(bmp_orig, "rb") as f1, open(bmp_dec, "rb") as f2:
        orig_bytes = f1.read()
        dec_bytes = f2.read()
        
    if orig_bytes == dec_bytes:
        print("SUCCESS: Decrypted image bytes match the original perfectly!")
    else:
        print("FAILURE: Bytes do not match.")
        
if __name__ == "__main__":
    main()
