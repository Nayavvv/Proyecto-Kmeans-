# Este código genera la gráfica que compara el speedup según el número de hilos.
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("summary_results.csv")

# Generamos una gráfica por cada dimensión
for dims in sorted(df["dimension"].unique()):
    df_dim = df[df["dimension"] == dims]

    plt.figure(figsize=(10, 6))

    # Graficamos una línea por cada tamaño de dataset
    for points in sorted(df_dim["points"].unique()):
        subset = df_dim[df_dim["points"] == points].sort_values("threads")
        plt.plot(subset["threads"], subset["speedup"], marker="o", label=f"{points} puntos")

    plt.title(f"Speedup vs Hilos - {dims}D")
    plt.xlabel("Número de hilos")
    plt.ylabel("Speedup")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"speedup_{dims}d.png")
    plt.show()