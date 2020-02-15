
binary_string = "000001100010"
x = 0

decimal_representation = int(binary_string, 2)
hexadecimal_string = hex(decimal_representation)

print(hexadecimal_string)

num_of_bits = 12
binary_string = bin(int(hexadecimal_string, 16))[2:].zfill(num_of_bits)

print(binary_string)