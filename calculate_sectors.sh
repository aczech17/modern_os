#!/bin/bash
FILE_SIZE=$(stat -c%s "$1")  # Pobierz rozmiar pliku w bajtach
SECTORS=$(( (FILE_SIZE + 511) / 512 ))  # Zaokrąglij w górę do pełnego sektora
echo "SECTORS_TO_READ equ $SECTORS" > out/sectors.inc
