import bisect
import os
import re
import subprocess
import sys
from datetime import datetime, timezone

# Project root (is file ke ek folder upar), wahin se sab chalega
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
os.makedirs("data", exist_ok=True)   # fresh clone mein data/ folder nahi hota

EXE = ".exe" if os.name == "nt" else ""
MAIN = os.path.abspath("loganalyzer" + EXE)
TESTMAP = os.path.abspath("test_hashmap" + EXE)
TESTHEAP = os.path.abspath("test_minheap" + EXE)
TESTTIME = os.path.abspath("test_timeutil" + EXE)
TESTRING = os.path.abspath("test_ringdeque" + EXE)
BENCH = os.path.abspath("bench_maps" + EXE)
BENCHTOPK = os.path.abspath("bench_topk" + EXE)
BENCHWIN = os.path.abspath("bench_window" + EXE)
RANGEPREFIX = os.path.abspath("range_prefix" + EXE)

CHECK_LOG = os.path.join("data", "_check.log")
EDGE_LOG = os.path.join("data", "_edge.log")
SPIKE_LOG = os.path.join("data", "_spike.log")
OOO_LOG = os.path.join("data", "_ooo.log")

results = []


def check(name, ok, detail=""):
    results.append(ok)
    status = "PASS" if ok else "FAIL"
    line = f"[{status}] {name}"
    if not ok and detail:
        line += f"   -> {detail}"
    print(line)


def run(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode, r.stdout + r.stderr


def compile_all():
    jobs = [
        ("src/main.cpp", "loganalyzer"),
        ("src/test_hashmap.cpp", "test_hashmap"),
        ("src/test_minheap.cpp", "test_minheap"),
        ("src/test_timeutil.cpp", "test_timeutil"),
        ("src/test_ringdeque.cpp", "test_ringdeque"),
        ("src/bench_maps.cpp", "bench_maps"),
        ("src/bench_topk.cpp", "bench_topk"),
        ("src/bench_window.cpp", "bench_window"),
        ("src/range_prefix.cpp", "range_prefix"),
    ]
    for src, out in jobs:
        code, text = run(["g++", "-std=c++17", "-O2", src, "-o", out + EXE])
        check(f"compile {src}", code == 0, text.strip()[:300])
        if code != 0:
            return False
    return True


def parse_summary(text):
    m = re.search(r"Parsed: (\d+)\s+Bad lines: (\d+)\s+Blank: (\d+)", text)
    return tuple(int(x) for x in m.groups()) if m else None


def section_total(text, title):
    total = 0
    inside = False
    for line in text.splitlines():
        if line.startswith(title):
            inside = True
            continue
        if inside:
            m = re.match(r"\s+\S+: (\d+)$", line)
            if m:
                total += int(m.group(1))
            else:
                break
    return total


def section_items(text, title):
    items = []
    inside = False
    for line in text.splitlines():
        if line.startswith(title):
            inside = True
            continue
        if inside:
            m = re.match(r"\s+(.+): (\d+)$", line)
            if m:
                items.append((m.group(1), int(m.group(2))))
            else:
                break
    return items


def python_truth(path):
    codes, levels, hours = {}, {}, {}
    with open(path) as f:
        for line in f:
            parts = line.split(None, 4)
            date, time, level, code = parts[0], parts[1], parts[2], parts[3]
            codes[code] = codes.get(code, 0) + 1
            levels[level] = levels.get(level, 0) + 1
            if level == "ERROR":
                key = date + " " + time[:2]
                hours[key] = hours.get(key, 0) + 1
    return codes, levels, hours


def spike_truth(path, window, threshold):
    """Python ka apna hisaab (deque ke bina): bisect se window ginti."""
    errs = []
    alerts = []
    cur = None
    last_ts = None
    last_stamp = ""
    out_of_order = 0
    with open(path) as f:
        for line in f:
            parts = line.split(None, 4)
            if len(parts) < 4:
                continue
            date, time, level = parts[0], parts[1], parts[2]
            dt = datetime.strptime(date + " " + time, "%Y-%m-%d %H:%M:%S")
            ts = int(dt.replace(tzinfo=timezone.utc).timestamp())
            stamp = date + " " + time
            if last_ts is not None and ts < last_ts:
                out_of_order += 1
                continue
            last_ts = ts
            last_stamp = stamp
            if level == "ERROR":
                errs.append(ts)
            count = len(errs) - bisect.bisect_right(errs, ts - window)
            if count >= threshold:
                if cur is None:
                    cur = {"start": stamp, "start_ts": ts, "peak": count,
                           "peak_at": stamp, "merged": 1}
                elif count > cur["peak"]:
                    cur["peak"] = count
                    cur["peak_at"] = stamp
            elif cur is not None:
                cur["end"] = stamp
                cur["end_ts"] = ts
                cur["ongoing"] = False
                alerts.append(cur)
                cur = None
    if cur is not None:
        cur["end"] = last_stamp
        cur["end_ts"] = last_ts
        cur["ongoing"] = True
        alerts.append(cur)
    return alerts, out_of_order


def merge_truth(alerts, gap):
    """Paas paas ke alerts ko jodo (Python ka apna hisaab)."""
    out = []
    for a in alerts:
        if out and a["start_ts"] - out[-1]["end_ts"] <= gap:
            last = out[-1]
            last["end"] = a["end"]
            last["end_ts"] = a["end_ts"]
            last["ongoing"] = a["ongoing"]
            last["merged"] += a["merged"]
            if a["peak"] > last["peak"]:
                last["peak"] = a["peak"]
                last["peak_at"] = a["peak_at"]
        else:
            out.append(dict(a))
    return out


def visible(alerts):
    return [{"start": a["start"], "end": a["end"], "peak": a["peak"],
             "peak_at": a["peak_at"], "ongoing": a["ongoing"], "merged": a["merged"]}
            for a in alerts]


def parse_alerts(text):
    alerts = []
    pattern = (r"ALERT \d+: (\S+ \S+) to (\S+ \S+)  peak (\d+) errors at (\S+ \S+)"
               r"(?:  \[(\d+) alerts merged\])?(  \(still active at end of file\))?")
    for m in re.finditer(pattern, text):
        alerts.append({"start": m.group(1), "end": m.group(2),
                       "peak": int(m.group(3)), "peak_at": m.group(4),
                       "ongoing": m.group(6) is not None,
                       "merged": int(m.group(5)) if m.group(5) else 1})
    return alerts


def spike_checks():
    configs = [(300, 40, 0), (300, 14, 0), (60, 6, 0),
               (300, 14, 600), (60, 6, 120), (300, 40, 600)]
    for window, threshold, gap in configs:
        args = ["--spike", str(window), str(threshold)]
        if gap:
            args += ["--merge", str(gap)]
        _, t = run([MAIN, CHECK_LOG] + args)
        got = parse_alerts(t)
        raw, _ = spike_truth(CHECK_LOG, window, threshold)
        want = visible(merge_truth(raw, gap) if gap else raw)
        label = f"--spike {window} {threshold}"
        if gap:
            label += f" --merge {gap}"
        check(f"{label}: {len(want)} alert(s) match Python",
              got == want, f"got {got[:2]} want {want[:2]}")

    small = [
        "2026-10-05 10:00:00 ERROR A x",
        "2026-10-05 10:00:02 ERROR A x",
        "2026-10-05 10:00:04 ERROR A x",
        "2026-10-05 10:00:06 ERROR A x",
        "2026-10-05 10:00:30 INFO OK x",
        "2026-10-05 10:01:00 ERROR A x",
        "2026-10-05 10:02:00 ERROR A x",
        "2026-10-05 10:02:01 ERROR A x",
        "2026-10-05 10:02:02 ERROR A x",
    ]
    with open(SPIKE_LOG, "w") as f:
        f.write("\n".join(small) + "\n")

    two_alerts = [
        {"start": "2026-10-05 10:00:04", "end": "2026-10-05 10:00:30",
         "peak": 4, "peak_at": "2026-10-05 10:00:06", "ongoing": False, "merged": 1},
        {"start": "2026-10-05 10:02:02", "end": "2026-10-05 10:02:02",
         "peak": 3, "peak_at": "2026-10-05 10:02:02", "ongoing": True, "merged": 1},
    ]
    one_incident = [
        {"start": "2026-10-05 10:00:04", "end": "2026-10-05 10:02:02",
         "peak": 4, "peak_at": "2026-10-05 10:00:06", "ongoing": True, "merged": 2},
    ]

    _, t = run([MAIN, SPIKE_LOG, "--spike", "10", "3"])
    got = parse_alerts(t)
    check("hand-made file: 2 alerts, second still active", got == two_alerts, str(got))

    _, t = run([MAIN, SPIKE_LOG, "--spike", "10", "3", "--merge", "100"])
    got = parse_alerts(t)
    check("hand-made file: --merge 100 joins them into 1 incident",
          got == one_incident, str(got))

    _, t = run([MAIN, SPIKE_LOG, "--spike", "10", "3", "--merge", "50"])
    got = parse_alerts(t)
    check("hand-made file: --merge 50 keeps them separate", got == two_alerts, str(got))

    backwards = [
        "2026-10-05 10:00:10 ERROR A x",
        "2026-10-05 10:00:05 ERROR A x",
        "2026-10-05 10:00:12 ERROR A x",
    ]
    with open(OOO_LOG, "w") as f:
        f.write("\n".join(backwards) + "\n")
    _, t = run([MAIN, OOO_LOG, "--spike", "10", "2"])
    check("out-of-order line skipped and reported",
          "Out-of-order lines skipped for spike detection: 1" in t, t[-200:])

    for args in (["--spike", "0", "5"], ["--spike", "abc", "5"], ["--spike", "300"]):
        code, t = run([MAIN, CHECK_LOG] + args)
        check(f"{' '.join(args)} rejected",
              code != 0 and "--spike needs two positive numbers" in t)

    for args in (["--merge", "0"], ["--merge", "abc"], ["--merge"]):
        code, t = run([MAIN, CHECK_LOG, "--spike", "300", "40"] + args)
        check(f"{' '.join(args)} rejected",
              code != 0 and "--merge needs a positive number" in t)


def main_checks():
    code, _ = run([sys.executable, "tools/generate_logs.py", "10000", CHECK_LOG])
    check("generate 10000-line test file", code == 0)

    code, text = run([MAIN, CHECK_LOG])
    check("main runs on test file", code == 0, text[:200])

    summary = parse_summary(text)
    check("Parsed=10000, Bad=0, Blank=0", summary == (10000, 0, 0), str(summary))

    codes, levels, hours = python_truth(CHECK_LOG)

    def grab(label):
        m = re.search(label + r":\s+(\d+)", text)
        return int(m.group(1)) if m else -1

    check("INFO count matches file", grab("INFO") == levels.get("INFO", 0))
    check("WARN count matches file", grab("WARN") == levels.get("WARN", 0))
    check("ERROR count matches file", grab("ERROR") == levels.get("ERROR", 0))

    check("all-codes total equals Parsed",
          section_total(text, "All codes") == 10000)
    check("ERROR-codes total equals ERROR count",
          section_total(text, "ERROR codes") == levels.get("ERROR", 0))

    for c in sorted(codes):
        _, t = run([MAIN, CHECK_LOG, "--code", c])
        m = re.search(r"Query: code \S+ appeared (\d+) times", t)
        got = int(m.group(1)) if m else -1
        check(f"--code {c} = {codes[c]}", got == codes[c], f"got {got}")

    _, t = run([MAIN, CHECK_LOG, "--code", "NOT_A_CODE"])
    check("--code NOT_A_CODE = 0", "appeared 0 times" in t)

    top_hour = sorted(hours.items(), key=lambda kv: (-kv[1], kv[0]))[0]
    expected = f"{top_hour[0]}: {top_hour[1]}"
    check(f"busiest hour = {expected}", expected in text)

    ranked = sorted(hours.items(), key=lambda kv: (-kv[1], kv[0]))
    for k in (1, 3, 5, 100):
        _, t = run([MAIN, CHECK_LOG, "--top", str(k), "--check-top"])
        got = section_items(t, "Busiest hours")
        want = ranked[:k]
        ok = got == want and "Top-K heap matches full sort: YES" in t
        check(f"--top {k}: heap result equals Python's top {k}", ok,
              f"got {got[:3]} want {want[:3]}")

    code, t = run([MAIN, CHECK_LOG, "--top", "0"])
    check("--top 0 rejected", code != 0 and "positive number" in t)
    code, t = run([MAIN, CHECK_LOG, "--top", "abc"])
    check("--top abc rejected", code != 0 and "positive number" in t)

    code, t = run([MAIN, CHECK_LOG, "--abc"])
    check("unknown option rejected", code != 0 and "Unknown option: --abc" in t)

    code, t = run([MAIN, os.path.join("data", "no_such_file.log")])
    check("missing file handled", code != 0 and "File nahi khuli" in t)


def edge_checks():
    lines = [
        "2026-10-05 10:00:00 ERROR DB_TIMEOUT Database down",
        "2026-10-05 10:00:01 INFO LOGIN_OK",
        "2026-10-05   10:00:02    WARN    HIGH_MEMORY    Memory   high   ",
        "2026-10-05\t10:00:03\tINFO\tPAGE_LOAD\tTab separated",
        "",
        "   ",
        "hello world",
        "2026-10-05 10:00:04 DEBUG TEST_CODE wrong level",
        "2026-10-05 10:00:05 error DB_TIMEOUT lowercase level",
        "05-10-2026 10:00:06 INFO LOGIN_OK bad date format",
        "2026-10-05 1000 INFO LOGIN_OK bad time format",
        "2026-10-05 10:00:07 ERROR AUTH_FAILED Invalid credentials",
    ]
    with open(EDGE_LOG, "w") as f:
        f.write("\n".join(lines) + "\n")

    code, text = run([MAIN, EDGE_LOG, "--show"])
    check("edge file: Parsed=5, Bad=5, Blank=2",
          parse_summary(text) == (5, 5, 2), str(parse_summary(text)))
    check("edge file: empty message kept as []", "INFO LOGIN_OK []" in text)
    check("edge file: extra spaces trimmed",
          "WARN HIGH_MEMORY [Memory   high]" in text)

def minute_of(text):
    """'2026-10-05 07:00' ya '2026-10-05 07:00:12' ko 1970 se minutes mein badlo."""
    fmt = "%Y-%m-%d %H:%M" if len(text) == 16 else "%Y-%m-%d %H:%M:%S"
    dt = datetime.strptime(text, fmt).replace(tzinfo=timezone.utc)
    return int(dt.timestamp()) // 60


def python_range_count(path, from_text, to_text):
    """Python ka apna hisaab: [from, to) mein kitne ERROR lines (minute ke hisaab se)."""
    lo = minute_of(from_text)
    hi = minute_of(to_text)
    count = 0
    with open(path) as f:
        for line in f:
            parts = line.split(None, 4)
            if len(parts) < 3 or parts[2] != "ERROR":
                continue
            m = minute_of(parts[0] + " " + parts[1])
            if lo <= m < hi:
                count += 1
    return count


def range_checks():
    ranges = [
        ("2026-10-05 07:00", "2026-10-05 08:00"),
        ("2026-10-05 03:15", "2026-10-05 04:45"),
        ("2026-10-05 00:00", "2026-10-06 00:00"),
        ("2026-10-05 05:10", "2026-10-05 05:11"),
        ("2025-01-01 00:00", "2025-01-02 00:00"),
    ]
    for a, b in ranges:
        _, t = run([RANGEPREFIX, CHECK_LOG, "--from", a, "--to", b])
        m1 = re.search(r"Brute force \(saari errors scan\):\s+(\d+)", t)
        m2 = re.search(r"Prefix sums \(2 lookups\):\s+(\d+)", t)
        brute = int(m1.group(1)) if m1 else -1
        prefix = int(m2.group(1)) if m2 else -1
        want = python_range_count(CHECK_LOG, a, b)
        check(f"range [{a}, {b}) = {want}: both methods match Python",
              brute == want and prefix == want and "Both agree: YES" in t,
              f"brute {brute} prefix {prefix} python {want}")

    code, t = run([RANGEPREFIX, CHECK_LOG, "--from", "2026-10-05 08:00",
                   "--to", "2026-10-05 07:00"])
    check("range with from after to rejected",
          code != 0 and "--from must be earlier than --to" in t)
    code, t = run([RANGEPREFIX, CHECK_LOG, "--from", "2026-13-05 07:00",
                   "--to", "2026-10-05 08:00"])
    check("range with bad time format rejected",
          code != 0 and "Bad time format" in t)

def other_programs():
    code, text = run([TESTMAP])
    check("test_hashmap: TEST PASSED", code == 0 and "TEST PASSED" in text, text[:200])

    code, text = run([TESTHEAP])
    check("test_minheap: TEST PASSED", code == 0 and "TEST PASSED" in text, text[:300])

    code, text = run([TESTTIME])
    check("test_timeutil: TEST PASSED", code == 0 and "TEST PASSED" in text, text[:300])

    code, text = run([TESTRING])
    check("test_ringdeque: TEST PASSED", code == 0 and "TEST PASSED" in text, text[:300])

    code, text = run([BENCH, CHECK_LOG])
    check("bench_maps: all three agree", code == 0 and "All three agree: YES" in text,
          text[-200:])

    code, text = run([BENCHTOPK, CHECK_LOG, "--level", "hour", "--k", "3"])
    check("bench_topk: all methods agree",
          code == 0 and "All methods agree: YES" in text, text[-200:])

    code, text = run([BENCHWIN, CHECK_LOG])
    check("bench_window: all methods agree",
          code == 0 and "All methods agree: YES" in text, text[-200:])


def cleanup():
    for p in (CHECK_LOG, EDGE_LOG, SPIKE_LOG, OOO_LOG):
        if os.path.exists(p):
            os.remove(p)


if __name__ == "__main__":
    print("== Compiling ==")
    if compile_all():
        print("\n== Checks ==")
        main_checks()
        edge_checks()
        spike_checks()
        range_checks()
        other_programs()
    cleanup()
    passed = sum(results)
    print(f"\n{passed}/{len(results)} checks passed")
    sys.exit(0 if results and all(results) else 1)