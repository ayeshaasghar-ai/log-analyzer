import sys
import random
from datetime import datetime, timedelta

LEVELS_CODES = [
    ("INFO",  "LOGIN_OK",     "User logged in"),
    ("INFO",  "PAGE_LOAD",    "Page served successfully"),
    ("WARN",  "HIGH_MEMORY",  "Memory usage above 80%"),
    ("ERROR", "DB_TIMEOUT",   "Database connection timed out"),
    ("ERROR", "AUTH_FAILED",  "Invalid credentials"),
    ("ERROR", "DISK_FULL",    "No space left on device"),
]

def generate(path, n_lines=10000):
    t = datetime(2026, 10, 5, 0, 0, 0)
    spike_start = int(n_lines * 0.6)
    spike_end = spike_start + 300
    with open(path, "w") as f:
        for i in range(n_lines):
            t += timedelta(seconds=random.randint(1, 8))
            # Beech mein ek artificial spike: AUTH_FAILED ki bauchhar
            if spike_start <= i < spike_end:
                level, code, msg = "ERROR", "AUTH_FAILED", "Invalid credentials"
            else:
                level, code, msg = random.choices(
                    LEVELS_CODES, weights=[40, 30, 10, 8, 8, 4])[0]
            f.write(f"{t:%Y-%m-%d %H:%M:%S} {level} {code} {msg}\n")

if __name__ == "__main__":
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
    out = sys.argv[2] if len(sys.argv) > 2 else "data/sample.log"
    generate(out, n)
    print(f"Done: {out} ({n} lines)")