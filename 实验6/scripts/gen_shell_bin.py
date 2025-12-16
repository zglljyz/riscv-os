#!/usr/bin/env python3
import sys

if len(sys.argv) != 3:
    print("Usage: gen_shell_bin.py <input> <output>")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file, 'r') as f:
    content = f.read()

content = content.replace('unsigned char shell_bin[]', 'unsigned char user_shell_bin[]')
content = content.replace('unsigned int shell_bin_len', 'unsigned int user_shell_bin_len')

with open(output_file, 'w') as f:
    f.write(content)

print(f'Generated {output_file}')
