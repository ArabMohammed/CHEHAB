import os
import shutil
import subprocess
import csv
import re

# Specify the parent folder containing the benchmarks and build subfolders
benchmarks_folder = "benchmarks"
build_folder = os.path.join("build", "benchmarks")
output_csv = "results.csv"
#vectorization_csv = "vectorization.csv"
operations=["multiply_plain","multiply","sub","add","rotate_rows","square"]
infos = ["benchmark","optimization_time( ms )","execution_time (ms)"]
infos.extend(operations)
with open(output_csv, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(infos)
    
""" with open(vectorization_csv, mode = 'w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Benchmark","Vecorization Time"])     """
    
try:
    result = subprocess.run(['cmake','-S','.','-B','build'], check=True, capture_output=False, text=True)
    result = subprocess.run(['cmake','--build','build'], check=True, capture_output=False, text=True)
    # result = subprocess.run(['sudo','cmake','--install','build'], check=True, capture_output=False, text=True)
except subprocess.CalledProcessError as e:
    print(f"Command failed with error:\n{e.stderr}")    

# Iterate through each item in the benchmarks folder
for subfolder_name in os.listdir(benchmarks_folder):
    benchmark_path = os.path.join(benchmarks_folder, subfolder_name)
    build_path = os.path.join(build_folder, subfolder_name)
    # build_path = build/benchmarks/dot_product 
    ## informations to collect 
    optimization_time=""
    execution_time=""
    if os.path.isdir(build_path):
        # Paths for the .cpp files
        #benchmark_cpp_path = os.path.join(benchmark_path, f"{subfolder_name}.cpp")
        #backup_cpp_path = os.path.join(benchmark_path, "backup.cpp")
        #fhe_generated_cpp_path = os.path.join(build_path, "fhe_vectorized.cpp")

        # Step 1: Construct the command to run `.\\benchmark` in the subfolder
        command = f"./{subfolder_name} "
        print(f"Benchmark '{subfolder_name}' will be runned...")
        # # Set the current working directory to the subfolder
        try:
            result = subprocess.run(command, shell=True, check=True, capture_output=True, text=True, cwd=build_path)
            lines = result.stdout.splitlines()
            for line in lines:
                if 'ms' in line:
                    optimization_time = line.split()[0]
                    break
            
            # print(f"Output for {subfolder_name}:\n{result.stdout}")
        except subprocess.CalledProcessError as e:
            print(f"Command for {subfolder_name} failed with error:\n{e.stderr}")
        #########################################################################
        print("running fhe code \n")
        ## building and running fhe code 
        build_path_he = os.path.join(build_path, "he")
        try:
            result = subprocess.run(['cmake','-S','.','-B','build'], check=True, capture_output=False, text=True,cwd=build_path_he)
            result = subprocess.run(['cmake','--build','build'], check=True, capture_output=False, text=True,cwd=build_path_he)
            # result = subprocess.run(['sudo','cmake','--install','build'], check=True, capture_output=False, text=True)
        except :
            print(f"Failed in building fhe_code for benchmark:{subfolder_name} ,with error \n")   
        build_path_he_build = os.path.join(build_path_he, "build")
        ########################################################################## 
        try:
            command = f"./main"
            result = subprocess.run(command, shell=True, check=True, capture_output=True, text=True, cwd=build_path_he_build)
            lines = result.stdout.splitlines()
            for line in lines :
                if 'ms' in line:
                    execution_time = line.split()[0]
                    break
            ## analyzing generated code for statistics ###
            file_name = build_path_he +"/_gen_he_"+subfolder_name+".cpp"
            row = [subfolder_name,optimization_time,execution_time]
            with open(file_name,"r") as file : 
                file_content = file.read()
                for op in operations :
                    nb_occurences= len(re.findall(rf'\b{op}', file_content))
                    row.append(nb_occurences)

            ###### building and running fhe code 
            with open(output_csv, mode = 'a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow(row)
            
            # print(f"Output for {subfolder_name}:\n{result.stdout}")
        except subprocess.CalledProcessError as e:
            print(f"Command for {subfolder_name} failed with error:\n{e.stderr}")
        
     