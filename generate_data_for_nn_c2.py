import subprocess
import os
import re
import csv
import numpy as np
import time

# --- Configuration ---
NUM_RUNS_PER_LINE = 3
# MIN_TARGET_LINE = 3448
MIN_TARGET_LINE = 1
MAX_TARGET_LINE = 15854


BASE_DIR = os.path.dirname(os.path.abspath(__file__))
BENCHMARK_DIR = os.path.join(BASE_DIR, "build", "benchmarks", "llm_based_benchmark")

HE_DIR = os.path.join(BENCHMARK_DIR, "he") 

HE_MAIN_EXECUTABLE = os.path.join(HE_DIR, "build") 

OUTPUT_CSV_FILE = "results_for_nn_on_c2.csv"

def run_command(command, cwd=None, check=True):
    try:
        process = subprocess.run(
            command,
            cwd=cwd,
            shell=True,
            check=check,
            stdout=subprocess.PIPE,     
            stderr=subprocess.STDOUT,  
            text=True,
            encoding='utf-8'
        )
        return process.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {command}")
        print(f"Output: {e.stdout}")
        raise
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        raise


def parse_benchmark_output(output_text):
    """Parses the output of llm_based_benchmark and extracts metrics."""
    metrics = {
        "depth_max": None,
        "xdepth_max": None,
        "mul_total": 0,
        "relin_total": 0,
        "mod_switch_total": 0,
        "he_add_total": 0,
        "rotate_total": 0,
        "rotation_keys": 0,
        "relin_keys": 0,
        "llm_benchmark_compile_time_ms": None 
    }

    # Compile time
    compile_time_match = re.search(r"Compile time :\s*(\d+\.?\d*)\s*ms", output_text)
    if compile_time_match:
        metrics["llm_benchmark_compile_time_ms"] = float(compile_time_match.group(1))

    # Depth metrics
    depth_match = re.search(r"max:\s*\((\d+),\s*(\d+)\)", output_text)
    if depth_match:
        metrics["depth_max"] = int(depth_match.group(1))
        metrics["xdepth_max"] = int(depth_match.group(2))

    # Operation counts
    mul_match = re.search(r"\|mul\| .*total:\s*(\d+)", output_text, re.DOTALL)
    if mul_match:
        metrics["mul_total"] = int(mul_match.group(1))

    relin_match = re.search(r"\|relin\| .*total:\s*(\d+)", output_text, re.DOTALL)
    if relin_match:
        metrics["relin_total"] = int(relin_match.group(1))

    mod_switch_match = re.search(r"\|mod_switch\| .*total:\s*(\d+)", output_text, re.DOTALL)
    if mod_switch_match:
        metrics["mod_switch_total"] = int(mod_switch_match.group(1))

    he_add_match = re.search(r"\|he_add\| .*total:\s*(\d+)", output_text, re.DOTALL)
    if he_add_match:
        metrics["he_add_total"] = int(he_add_match.group(1))

    rotate_match = re.search(r"\|rotate\| .*total:\s*(\d+)", output_text, re.DOTALL)
    if rotate_match:
        metrics["rotate_total"] = int(rotate_match.group(1))

    # Key counts
    rotation_keys_match = re.search(r"\|rotation_keys\|:\s*(\d+)", output_text)
    if rotation_keys_match:
        metrics["rotation_keys"] = int(rotation_keys_match.group(1))

    relin_keys_match = re.search(r"\|relin_keys\|:\s*(\d+)", output_text)
    if relin_keys_match:
        metrics["relin_keys"] = int(relin_keys_match.group(1))

    return metrics

def parse_he_output(output_text):
    """Parses the output of ./main in the he directory for noise and execution time."""
    noise = None
    exec_time_ms = None

   
    noise_regex = r"c\d+:\s*\d+,\s*\d+,\s*(\d+)" 
    noise_match = re.search(noise_regex, output_text)
    
    print(f"    Parsing HE output with regex: '{noise_regex}'")
    if noise_match:
        print(f"    Noise regex matched: {noise_match.group(0)}")
        print(f"    Captured noise value: {noise_match.group(1)}")
    else:
        print(f"    Noise regex did NOT match in output.")

    if noise_match:
        noise = int(noise_match.group(1))

    exec_time_match = re.search(r"(\d+\.?\d*)\s*ms", output_text)
    if exec_time_match:
        exec_time_ms = float(exec_time_match.group(1))

    return {"noise": noise, "execution_time_ms": exec_time_ms}


def main():
    print("--- Building main project ---")
    try:
        run_command("cmake -S . -B build", cwd=BASE_DIR)
        run_command("make", cwd=os.path.join(BASE_DIR, "build"))
        print("Main project built successfully.")
    except Exception as e:
        print(f"Failed to build main project: {e}")
        return

    # Prepare CSV file
    header = [
        "run_id",
        "target_line",
        "run_number",
        "depth_max", "xdepth_max", "mul_total", "relin_total",
        "mod_switch_total", "he_add_total", "rotate_total",
        "rotation_keys", "relin_keys", "execution_time_ms", "noise",
        "llm_benchmark_compile_time_ms"
    ]
    with open(OUTPUT_CSV_FILE, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(header)

    global_run_counter = 0

    for target_line in range(MIN_TARGET_LINE, MAX_TARGET_LINE + 1):
        print(f"\n--- Processing Target Line: {target_line} ---")

        for run_num in range(1, NUM_RUNS_PER_LINE + 1):
            global_run_counter += 1
            print(f"  Run ID: {global_run_counter}, Run {run_num}/{NUM_RUNS_PER_LINE} for target_line {target_line}...")

            bm_metrics = {}
            he_metrics = {}

            try:
                # --- Run llm_based_benchmark ---
                # This generates fhe_io_example_adapted.txt in BENCHMARK_DIR
                benchmark_command = f"./llm_based_benchmark 1 1 1 {target_line}"
                benchmark_output = run_command(benchmark_command, cwd=BENCHMARK_DIR)
                bm_metrics = parse_benchmark_output(benchmark_output)
                print(f"    Benchmark metrics extracted: {bm_metrics}")

                # --- Re-build 'he' sub-project AFTER benchmark run ---
                print("--- Rebuilding 'he' sub-project for current target line ---")
                run_command("cmake -S . -B build", cwd=HE_DIR) 
                # run_command("make", cwd=os.path.join(HE_DIR, "build")) 
                print("'he' sub-project rebuilt successfully.")

                he_command = "./main" 
                he_output = run_command(he_command, cwd=os.path.join(HE_DIR, "build"))
                he_metrics = parse_he_output(he_output)
                print(f"    HE metrics extracted: {he_metrics}")

            except Exception as e:
                print(f"  Skipping run {run_num} for target_line {target_line} due to error: {e}")
                pass

            row_data = {
                "run_id": global_run_counter,
                "target_line": target_line,
                "run_number": run_num,
                "depth_max": bm_metrics.get("depth_max"),
                "xdepth_max": bm_metrics.get("xdepth_max"),
                "mul_total": bm_metrics.get("mul_total"),
                "relin_total": bm_metrics.get("relin_total"),
                "mod_switch_total": bm_metrics.get("mod_switch_total"),
                "he_add_total": bm_metrics.get("he_add_total"),
                "rotate_total": bm_metrics.get("rotate_total"),
                "rotation_keys": bm_metrics.get("rotation_keys"),
                "relin_keys": bm_metrics.get("relin_keys"),
                "execution_time_ms": he_metrics.get("execution_time_ms"),
                "noise": he_metrics.get("noise"),
                "llm_benchmark_compile_time_ms": bm_metrics.get("llm_benchmark_compile_time_ms")
            }

            with open(OUTPUT_CSV_FILE, 'a', newline='') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow([row_data[key] for key in header])

            time.sleep(0.1)

    print(f"\n--- Script Finished ---")
    print(f"Results saved to {OUTPUT_CSV_FILE}")

if __name__ == "__main__":
    main()