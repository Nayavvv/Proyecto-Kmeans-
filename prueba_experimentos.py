# Este código ejecuta varias veces las versiones serial y paralela de K-means, guarda los tiempos de ejecución y calcula speedup y eficiencia.
import subprocess
import re
import csv
import os
import statistics

sizes = [100000, 200000, 300000, 400000, 600000, 800000, 1000000]
dimensions = [2, 3]
repetitions = 10
thread_counts = [1, 2, 4, 8]
k = 8


vcores = int(subprocess.check_output(["sysctl", "-n", "hw.logicalcpu"]).decode().strip()) #Cantidad de nucleos logicos de la maquina 


serial_exe = "./kmeans_serial"
parallel_exe = "./kmeans_paralelo"

time_pattern = re.compile(r"Tiempo_ms:\s*([0-9]+(?:\.[0-9]+)?)")

raw_results_file = "raw_results.csv"
summary_results_file = "summary_results.csv"

raw_rows = []
summary_rows = []

def extract_time(output): 
    match = time_pattern.search(output)
    if not match:
        raise ValueError("No se encontró Tiempo_ms en la salida:\n" + output)
    return float(match.group(1)) #Devuelve el tiempo como num. decimal 

for dims in dimensions:
    for n in sizes:
        input_file = f"Datasets/{n}_data_{dims}d.csv"

        if not os.path.exists(input_file):
            print(f"Archivo faltante: {input_file}")
            continue

        # -------- SERIAL --------
        serial_times = []

        for rep in range(1, repetitions + 1):
            output_file = f"tmp_serial_{n}_{dims}d_rep{rep}.csv"

            result = subprocess.run(
                [serial_exe, input_file, str(k), output_file, "0"], # "0" indica que no se guarde el archivo final de salida
                capture_output=True,
                text=True
            )

            if result.returncode != 0:
                print("Error serial:")
                print(result.stderr)
                raise RuntimeError("Falló ejecución serial")

            t = extract_time(result.stdout) # Extrae el tiempo de ejecución 
            serial_times.append(t)

            raw_rows.append([dims, n, 1, "serial", rep, t])

            if os.path.exists(output_file):
                os.remove(output_file)

        serial_avg = statistics.mean(serial_times) # Calcula el tiempo promedio de la versión serial

        # -------- PARALELO --------
        for threads in thread_counts:
            parallel_times = []

            for rep in range(1, repetitions + 1):
                output_file = f"tmp_parallel_{n}_{dims}d_t{threads}_rep{rep}.csv"

                result = subprocess.run(
                    [parallel_exe, input_file, str(k), output_file, str(threads), "0"],
                    capture_output=True,
                    text=True
                )

                if result.returncode != 0:
                    print("Error paralelo:")
                    print(result.stderr)
                    raise RuntimeError("Falló ejecución paralela")

                t = extract_time(result.stdout)
                parallel_times.append(t)

                raw_rows.append([dims, n, threads, "parallel", rep, t])

                if os.path.exists(output_file):
                    os.remove(output_file)

            parallel_avg = statistics.mean(parallel_times) # Calcula el tiempo promedio de la versión paralela
            speedup = serial_avg / parallel_avg  # Calcula el speedup comparando el tiempo serial con el paralelo
            efficiency = speedup / threads # Calcula la eficiencia dividiendo el speedup entre el número de hilos

            summary_rows.append([
                dims,
                n,
                threads,
                serial_avg,
                parallel_avg,
                speedup,
                efficiency
            ]) #Resumen del experiemnto actual 

# Guardar resultados crudos
with open(raw_results_file, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["dimension", "points", "threads", "type", "rep", "time_ms"])
    writer.writerows(raw_rows)

# Guardar resumen
with open(summary_results_file, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow([
        "dimension",
        "points",
        "threads",
        "serial_avg_ms",
        "parallel_avg_ms",
        "speedup",
        "efficiency"
    ])
    writer.writerows(summary_rows)

print(f"Resultados guardados en {raw_results_file} y {summary_results_file}")
print(f"vcores detectados: {vcores}")
print(f"Hilos probados: {thread_counts}")
