import subprocess
import sys

out_path = sys.argv[1]
try:
    h = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD'], text=True).strip()
except Exception:
    h = 'unknown'

with open(out_path, 'w') as f:
    f.write('VERSION_STR db "Solaro-alpha-26.0-intel8086-%s", 13, 10, 0\n' % h)
