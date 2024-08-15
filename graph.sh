(
    echo "digraph G {"
    echo "  node [shape=box];"

    # Process .c files
    find src -name "*.c" | while read c_file; do
        c_basename=$(basename "$c_file")
        echo "  \"$c_basename\" [color=blue];"

        # Find includes in .c files
        grep -E '^#include\s*"' "$c_file" | sed 's/^#include\s*"\(.*\)".*/\1/' | while read include; do
            echo "  \"$c_basename\" -> \"$include\" [color=red];"
        done
    done

    # Process .h files
    find include -name "*.h" | while read h_file; do
        h_basename=$(basename "$h_file")
        echo "  \"$h_basename\" [color=green];"

        # Find includes in .h files
        grep -E '^#include\s*"' "$h_file" | sed 's/^#include\s*"\(.*\)".*/\1/' | while read include; do
            echo "  \"$h_basename\" -> \"$include\" [color=blue];"
        done
    done

    echo "}"
) > dependency_graph.dot

# Generate SVG from the dot file
dot -Tsvg dependency_graph.dot -o dependency_graph.svg

# View the SVG (you might need to install eog or use another viewer)
eog dependency_graph.svg
