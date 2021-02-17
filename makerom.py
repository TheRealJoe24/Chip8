code = bytearray([
    0x40, 0x45
])
rom = code + bytearray([0xea] * (0x0E00 - len(code)))

with open("test.bin", "wb") as out_file:
    out_file.write(rom)