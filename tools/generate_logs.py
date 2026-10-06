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
    with open(path, "w") as f:
        for i in range(n_lines):
            t += timedelta(seconds=random.randint(1, 8))
            # Beech mein ek artificial spike: AUTH_FAILED ki bauchhar
            if 6000 <= i < 6300:
                level, code, msg = "ERROR", "AUTH_FAILED", "Invalid credentials"
            else:
                level, code, msg = random.choices(
                    LEVELS_CODES, weights=[40, 30, 10, 8, 8, 4])[0]
            f.write(f"{t:%Y-%m-%d %H:%M:%S} {level} {code} {msg}\n")

if __name__ == "__main__":
    generate("data/sample.log", 10000)
    print("Done: data/sample.log")