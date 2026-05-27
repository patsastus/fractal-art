import os
import sys
import csv
import statistics

def read_results(filepath):
    checksum = None
    times = []
    with open(filepath, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if not row:
                continue
            if row[0] == 'checksum':
                checksum = int(row[1])
            elif row[0] == 'frame':
                continue
            else:
                times.append(float(row[1]))
    return checksum, times

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analyze.py <results_dir>")
        sys.exit(1)
        
    results_dir = sys.argv[1]
    
    scenarios = [
        ("S1: 500x500 x 100", "S1_original"),
        ("S2: 1080p x 100", "S2_hd"),
        ("S3: 500x500 x 1000", "S3_deep"),
        ("S4: 1080p x 1000", "S4_hd_deep"),
        ("S5: Julia 1080p", "S5_julia"),
        ("S6: Newton 1080p", "S6_newton")
    ]
    
    # dimensions for throughput calculation
    dims = {
        "S1_original": 500*500,
        "S2_hd": 1920*1080,
        "S3_deep": 500*500,
        "S4_hd_deep": 1920*1080,
        "S5_julia": 1920*1080,
        "S6_newton": 1920*1080
    }
    
    print(f"{'Scenario':<20} | {'C median (ms)':<15} | {'C++ median (ms)':<15} | {'Speedup':<10} | {'C++ Mpx/s':<10}")
    print("-" * 80)
    
    for label, base_name in scenarios:
        c_file = os.path.join(results_dir, f"{base_name}_c.csv")
        cpp_file = os.path.join(results_dir, f"{base_name}_cpp.csv")
        
        if not os.path.exists(c_file) or not os.path.exists(cpp_file):
            print(f"{label:<20} | Missing data")
            continue
            
        c_check, c_times = read_results(c_file)
        cpp_check, cpp_times = read_results(cpp_file)
        
        if c_check != cpp_check:
            print(f"{label:<20} | CHECKSUM MISMATCH (C: {c_check}, C++: {cpp_check})")
            continue
            
        c_med = statistics.median(c_times)
        cpp_med = statistics.median(cpp_times)
        
        speedup = c_med / cpp_med if cpp_med > 0 else 0
        
        pixels = dims[base_name]
        mpx_s = (pixels / 1_000_000) / (cpp_med / 1000.0)
        
        print(f"{label:<20} | {c_med:<15.2f} | {cpp_med:<15.2f} | {speedup:<9.2f}x | {mpx_s:<10.2f}")

if __name__ == '__main__':
    main()
