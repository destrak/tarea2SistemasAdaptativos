#!/usr/bin/env bash
set -euo pipefail

# ========================
# CONFIGURACIÓN GENERAL
# ========================
BIN=./solver                     # ejecutable
DIR=new_1000_dataset             # carpeta con .graph
OUT_CSV="resultados_globales_determinista_1000.csv"
OUT_RESUMEN="resumen_por_densidad_determinista_1000.csv"
LOGDIR="logs_determinista_1000"

mkdir -p "$LOGDIR"

# Encabezado CSV
echo "densidad,idx,solucion,tiempo,archivo" > "$OUT_CSV"

# ========================
# BUCLE PRINCIPAL
# ========================
for dens in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9; do
  for idx in $(seq 1 30); do
    candidate=""
    for pat in \
      "*p0c${dens}_$idx".graph \
      "*p0c${dens}_0$idx".graph \
      "*p0c${dens}_00$idx".graph \
      "*p0c${dens}_*${idx}".graph; do
      match=( "$DIR"/$pat )
      if [ -e "${match[0]:-}" ]; then
        candidate="${match[0]}"
        break
      fi
    done

    if [ -z "$candidate" ] || [ ! -f "$candidate" ]; then
      echo "No se encontró archivo para densidad=$dens idx=$idx"
      echo "$dens,$idx,NA,NA,$DIR/erdos_n3000_p0c${dens}_${idx}.graph" >> "$OUT_CSV"
      continue
    fi

    echo "▶ Ejecutando $candidate (densidad=$dens, idx=$idx)"
    logfile="$LOGDIR/d${dens}_i${idx}.log"

    # Ejecuta el solver y guarda salida
    set +e
    out="$("$BIN" -i "$candidate" 2>&1)"
    rc=$?
    set -e
    printf "%s\n" "$out" | tr -d '\r' > "$logfile"

    # ========================
    # PARSEO DE SALIDA
    # ========================
    sol=$(sed -n 's/.*Soluci[^0-9]*\([0-9][0-9]*\).*/\1/p' "$logfile" | tail -n1)
    tiempo=$(sed -n 's/.*Tiempo[^0-9]*\([0-9]\+\(\.[0-9]\+\)\?\).*/\1/p' "$logfile" | head -n1)

    if [ $rc -ne 0 ]; then
      echo "Error al ejecutar ($rc) -> $candidate"
      sol=""; tiempo=""
    fi

    if [ -z "${sol:-}" ] || [ -z "${tiempo:-}" ]; then
      echo "No pude parsear Solucion/Tiempo (ver $logfile)"
      sol="NA"; tiempo="NA"
    fi

    echo "$dens,$idx,$sol,$tiempo,$candidate" >> "$OUT_CSV"
  done
done

# ========================
# RESUMEN POR DENSIDAD
# ========================
echo "densidad,count,mean_best,std_best,mean_time,std_time" > "$OUT_RESUMEN"
awk -F, '
NR>1 && $3!="NA" && $4!="NA" {
  d=$1; s=$3; t=$4
  n[d]++; sumS[d]+=s; sumT[d]+=t; sumSqS[d]+=s*s; sumSqT[d]+=t*t
}
END {
  for (d in n) {
    meanS=sumS[d]/n[d]; meanT=sumT[d]/n[d]
    stdS=sqrt(sumSqS[d]/n[d]-meanS*meanS)
    stdT=sqrt(sumSqT[d]/n[d]-meanT*meanT)
    printf("%s,%d,%.6f,%.6f,%.6f,%.6f\n", d, n[d], meanS, stdS, meanT, stdT)
  }
}' "$OUT_CSV" | sort -t, -k1,1 >> "$OUT_RESUMEN"

# ========================
# PROMEDIO GENERAL
# ========================
echo
echo "===== PROMEDIO GENERAL ====="
awk -F, 'NR>1 && $3!="NA" && $4!="NA" { sb+=$3; st+=$4; c++ }
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
echo "Logs por instancia en: $LOGDIR/"
