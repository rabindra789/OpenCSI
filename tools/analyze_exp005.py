"""Analyze EXP-005 results across all environmental scenarios."""
import os

data_dir = r'D:\workspace\development\OpenCSI\experiments\EXP-005\data'

scenarios = [
    ('empty-baseline', 'exp-005-empty-baseline-raw.txt'),
    ('door-closed', 'exp-005-door-closed-raw.txt'),
    ('door-open', 'exp-005-door-open-raw.txt'),
    ('metal-object', 'exp-005-metal-object-raw.txt'),
    ('baseline-repeat', 'exp-005-baseline-repeat-raw.txt'),
]

header = f"{'Scenario':20s} {'Calls':>6s} {'Rate/s':>8s} {'RSSImin':>7s} {'RSSImax':>7s} {'Range':>6s} {'HT%':>6s}"
print(header)
print('-' * len(header))

for name, fname in scenarios:
    path = os.path.join(data_dir, fname)
    if not os.path.exists(path):
        print(f"{name:20s} FILE NOT FOUND")
        continue
    with open(path, 'rb') as f:
        text = f.read().decode('utf-8', errors='replace')
    lines = text.split('\n')
    csv = [l for l in lines if l.startswith('CSV,')]
    
    in_final = False
    total_csi = 0
    elapsed_s = 0
    for l in lines:
        if 'FINAL STATS' in l:
            in_final = True
        if in_final and 'total_csi:' in l:
            total_csi = int(l.split(':')[1].strip())
        if in_final and 'elapsed_s:' in l:
            elapsed_s = float(l.split(':')[1].strip())
        if in_final and 'EXP-004 COMPLETE' in l:
            break
    
    rate = total_csi / elapsed_s if elapsed_s else 0
    
    if csv:
        rssis = [int(l.split(',')[4]) for l in csv]
        rssi_min, rssi_max = min(rssis), max(rssis)
        sig_modes = [l.split(',')[6] for l in csv]
        ht = sum(1 for s in sig_modes if s == '1')
        ht_pct = ht / len(sig_modes) * 100 if sig_modes else 0
    else:
        rssi_min, rssi_max = 0, 0
        ht_pct = 0.0
    
    print(f"{name:20s} {total_csi:6d} {rate:8.3f} {rssi_min:7d} {rssi_max:7d} {rssi_max - rssi_min:6d} {ht_pct:6.1f}")

print()
print("=== RSSI PER-SECOND VARIANCE ===")
for name, fname in scenarios:
    path = os.path.join(data_dir, fname)
    with open(path, 'rb') as f:
        text = f.read().decode('utf-8', errors='replace')
    lines = text.split('\n')
    csv = [l for l in lines if l.startswith('CSV,')]
    if len(csv) < 2:
        print(f"{name:20s}: insufficient data")
        continue
    
    # Compute per-second RSSI stats
    t0 = int(csv[0].split(',')[2])
    rssi_by_second = {}
    for l in csv:
        parts = l.split(',')
        ts = int(parts[2])
        rssi = int(parts[4])
        sec = (ts - t0) // 1000000  # seconds since first callback
        if sec not in rssi_by_second:
            rssi_by_second[sec] = []
        rssi_by_second[sec].append(rssi)
    
    # Variance across seconds
    if len(rssi_by_second) >= 2:
        sec_means = [sum(v)/len(v) for v in rssi_by_second.values()]
        overall_mean = sum(sec_means) / len(sec_means)
        variance = sum((m - overall_mean)**2 for m in sec_means) / len(sec_means)
        print(f"{name:20s}: {len(rssi_by_second)} seconds, mean={overall_mean:.1f}, var={variance:.2f}")
    else:
        print(f"{name:20s}: only {len(rssi_by_second)} second(s) with data")
