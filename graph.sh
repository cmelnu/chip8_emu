(
    echo "digraph G {"
    echo "  node [shape=box];"

    # Procesar archivos .c
    find src -name "*.c" | while read c_file; do
        c_basename=$(basename "$c_file")
        echo "  \"$c_basename\" [color=blue];"

        # Encontrar includes
        grep -E '^#include\s*"' "$c_file" | sed 's/^#include\s*"\(.*\)".*/\1/' | while read include; do
            echo "  \"$c_basename\" -> \"$include\" [color=red];"
        done
    done

    # Añadir nodos para archivos .h
    find include -name "*.h" | while read h_file; do
        h_basename=$(basename "$h_file")
        echo "  \"$h_basename\" [color=green];"
    done

    echo "}"
) > dependency_graph.dot

# Generar SVG a partir del archivo dot
dot -Tsvg dependency_graph.dot -o dependency_graph.svg

# Ver el SVG (puede que necesites instalar eog o usar otro visor)
eog dependency_graph.svg
