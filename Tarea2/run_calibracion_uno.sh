#!/usr/bin/env bash
set -euo pipefail

# ---- Config ----
BIN=./IterativeLocalSearch             # ejecutable
DIR=new_3000_dataset                   # carpeta con .graph
TMAX=10                                # tiempo -t (segundos)
ALPHA=0.5                             # puedes calibrar después
PERTURB=3
LS=4000
SEED=1

OUT_CSV="resultados3000.csv"
OUT_RESUMEN="resumen_por_densidad3000.csv"

# ---- Salidas ----
echo "densidad,idx,final_best,found_at,archivo" > "$OUT_CSV"

# Lista explícita de densidades (evita problemas de flotantes en bash)
for dens in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
  for idx in $(seq 1 30); do
    # Intenta varios patrones comunes de nombre:
    #   ...p0c0.1_1.graph    | ...p0c0.1_graph1.graph | ...p0c0.1_001.graph (etc)
    # Ajusta/añade patrones si lo necesitas.
    candidate=""
    for pat in \
      "*p0c${dens}_$idx".graph \
      "*p0c${dens}_0$idx".graph \
      "*p0c${dens}_00$idx".graph \
      "*p0c${dens}_*${idx}".graph \
      "*p0c${dens}*graph${idx}.graph" \
      "*p0c${dens}*_${idx}.graph"; do
      # shellcheck disable=SC2086
      match=( "$DIR"/$pat )
      if [ -e "${match[0]}" ]; then
        candidate="${match[0]}"
        break
      fi
    done

    if [ -z "$candidate" ] || [ ! -f "$candidate" ]; then
      echo "⚠️  No se encontró archivo para densidad=$dens idx=$idx (saltando)"
      continue
    fi

    echo "▶ Ejecutando $candidate (densidad=$dens, idx=$idx)"
    # Ejecuta el solver
    out="$("$BIN" ILS -i "$candidate" -t "$TMAX" --alpha "$ALPHA" --perturb "$PERTURB" --ls "$LS" --seed "$SEED" || true)"

    # Parsear la línea FINAL_BEST
    best=$(echo "$out" | awk '/^FINAL_BEST/{print $2}' | tail -n1)
    at=$(  echo "$out" | awk '/^FINAL_BEST/{print $4}' | tail -n1)

    # Si no logró parsear (algo raro), marca NA
    best=${best:-NA}
    at=${at:-NA}

    echo "$dens,$idx,$best,$at,$candidate" >> "$OUT_CSV"
  done
done

# ---- Resumen por densidad ----
# Promedia best y found_at por densidad (omite filas NA)
echo "densidad,avg_best,avg_found_at,count" > "$OUT_RESUMEN"
awk -F, 'NR>1 && $3!="NA" && $4!="NA" {sumBest[$1]+=$3; sumT[$1]+=$4; cnt[$1]++}
END{ for (d in cnt) printf("%s,%.6f,%.6f,%d\n", d, sumBest[d]/cnt[d], sumT[d]/cnt[d], cnt[d]) }' "$OUT_CSV" \
| sort -t, -k1,1 >> "$OUT_RESUMEN"

# ---- Promedio general (todas las densidades) ----
echo
echo "===== PROMEDIO GENERAL ====="
awk -F, 'NR>1 && $3!="NA" && $4!="NA" {sb+=$3; st+=$4; c++}
END{
  if (c>0) printf("AVG_FINAL_BEST=%.6f  AVG_FOUND_AT=%.6f  (count=%d)\n", sb/c, st/c, c);
  else print "Sin datos válidos."
}' "$OUT_CSV"
echo
echo "Archivos generados:"
echo " - $OUT_CSV"
echo " - $OUT_RESUMEN"
