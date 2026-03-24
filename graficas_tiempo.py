import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("summary_results.csv")

for dims in sorted(df["dimension"].unique()):
    df_dim = df[df["dimension"] == dims]

    plt.figure(figsize=(10, 6))

    serial_curve = (
        df_dim[["points", "serial_avg_ms"]]
        .drop_duplicates()
        .sort_values("points")
    )
    plt.plot(serial_curve["points"], serial_curve["serial_avg_ms"], marker="o", label="Serial")

    for threads in sorted(df_dim["threads"].unique()):
        subset = df_dim[df_dim["threads"] == threads].sort_values("points")
        plt.plot(subset["points"], subset["parallel_avg_ms"], marker="o", label=f"Paralelo {threads} hilos")

    plt.title(f"Tiempo promedio vs Número de puntos - {dims}D")
    plt.xlabel("Número de puntos")
    plt.ylabel("Tiempo promedio (ms)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"times_{dims}d.png")
    plt.show()