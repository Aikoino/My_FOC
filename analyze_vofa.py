import struct

# 用户收到的数据
user_data = bytes([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80])

print("=== User Received Data Analysis ===\n")
print(f"Total length: {len(user_data)} bytes")
print(f"Hex dump: {' '.join(f'{b:02X}' for b in user_data)}\n")

print("=== Frame 1: Expected VOFA+ packet ===")
print("Structure: [Iu:4][Iv:4][Iw:4][Vbus:4][Tail:4] = 20 bytes")
print(f"First 20 bytes: {' '.join(f'{b:02X}' for b in user_data[:20])}\n")

# 解析前4个float
print("Float parsing (first 20 bytes):")
for i in range(4):
    start = i * 4
    if start + 4 <= 20:
        f = struct.unpack('<f', user_data[start:start+4])[0]
        names = ['Iu', 'Iv', 'Iw', 'Vbus']
        print(f"  {names[i]}: {f:.6f} A or V")

print("\nFrame tail (should be 00 00 80 7F):")
print(f"  Actual:   {user_data[16:20].hex(' ').upper()}")
print(f"  Expected: 00 00 80 7F")

print("\n=== Frame 2: Extra data (11 bytes) ===")
extra = user_data[20:]
print(f"Extra bytes: {' '.join(f'{b:02X}' for b in extra)}")
print(f"Length: {len(extra)} bytes (should be 0)")

print("\n=== Diagnosis ===")
if len(user_data) != 20:
    print(f"[ERROR] Packet length {len(user_data)} != 20 bytes")
    print(f"[ERROR] Extra {len(user_data)-20} bytes detected!")

if user_data[16:20] != bytes([0x00, 0x00, 0x80, 0x7F]):
    print(f"[ERROR] Frame tail corrupted!")

print("\n=== Possible Causes ===")
print("1. Double send (called twice in quick succession)")
print("2. Previous packet residual in buffer")
print("3. Interrupt/race condition during send")
print("4. USART TXE flag not cleared properly")
