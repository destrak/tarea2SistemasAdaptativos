#!/usr/bin/env bash
set -euo pipefail

# ---- Config ----
BIN=./solverRandom                             # ejecutable
DIR=new_3000_dataset                    # carpeta con .graph
CRIT=0.7                                # parámetro 1 de tu programa
K=0.2                                   # parámetro 2 de tu programa

OUT_CSV="resultados_globalesRandom3000.csv"
OUT_RESUMEN="resumen_por_densidadRandom3000.csv"

# ---- Salidas ----
echo "densidad,idx,solucion,tiempo,archivo" > "$OUT_CSV"

# Lista explícita de densidades (evita problemas de flotantes en bash)
for dens in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
  for idx in $(seq 1 30); do
    # Busca archivos comunes: p0c0.1_1.graph, p0c0.1_01.graph, p0c0.1_001.graph, etc.
    candidate=""
    for pat in \
      "*p0c${dens}_$idx".graph \
      "*p0c${dens}_0$idx".graph \
      "*p0c${dens}_00$idx".graph \
      "*p0c${dens}_*${idx}".graph; do
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
    # Ejecuta tu programa (igual que antes)
    out="$("$BIN" -i "$candidate" "$CRIT" "$K" || true)"

    # Parsear la salida: líneas tipo "Solucion: X" y "Tiempo: Y"
    sol=$(echo "$out" | awk '/^Solucion:/ {print $2}' | tail -n1)
    tiempo=$(echo "$out" | awk '/^Tiempo:/ {print $2}' | tail -n1)

    sol=${sol:-NA}
    tiempo=${tiempo:-NA}

    echo "$dens,$idx,$sol,$tiempo,$candidate" >> "$OUT_CSV"
  done
done

# ---- Resumen por densidad ----
# Calcula promedio y desviación estándar por densidad
echo "densidad,count,mean_best,std_best,mean_time,std_time" > "$OUT_RESUMEN"

awk -F, '
NR>1 && $3!="NA" && $4!="NA" {
  dens=$1; sol=$3; time=$4
  n[dens]++
  sumS[dens]+=sol; sumT[dens]+=time
  sumSqS[dens]+=(sol)^2; sumSqT[dens]+=(time)^2
}
END {
  for (d in n) {
    meanS=sumS[d]/n[d]
    meanT=sumT[d]/n[d]
    stdS=sqrt(sumSqS[d]/n[d] - meanS^2)
    stdT=sqrt(sumSqT[d]/n[d] - meanT^2)
    printf("%s,%d,%.6f,%.6f,%.6f,%.6f\n", d, n[d], meanS, stdS, meanT, stdT)
  }
}' "$OUT_CSV" | sort -t, -k1,1 >> "$OUT_RESUMEN"

# ---- Promedio general ----
echo
echo "===== PROMEDIO GENERAL ====="
awk -F, 'NR>1 && $3!="NA" && $4!="NA" {
  sb+=$3; st+=$4; c++
}
END {
  if (c>0)
    printf("AVG_SOL=%.6f  AVG_TIME=%.6f  (count=%d)\n", sb/c, st/c, c);
  else
    print "Sin datos válidos."
}' "$OUT_CSV"

echo
echo "Archivos generados:"
echo " - $OUT_CSV"
echo " - $OUT_RESUMEN"
