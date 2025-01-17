import numpy as np
import csv

class KDTreeNode:
    def __init__(self, split_dim=None, split_val=None, left=None, right=None, points=None):
        self.split_dim = split_dim
        self.split_val = split_val
        self.left = left
        self.right = right
        self.points = points

# Function to scale CSV values and return scaled numpy array
def scale_csv_values(input_csv, scale_factor=65000):
    """
    Scale float values in input_csv to uint16_t range and return scaled numpy array.
    """
    try:
        with open(input_csv, 'r') as csv_file:
            csv_reader = csv.reader(csv_file)
            rows = [[float(value) for value in row] for row in csv_reader]

        # Scale each value to uint16_t range
        scaled_rows = [
            [min(int(value * scale_factor), 65535) for value in row]
            for row in rows
        ]

        return np.array(scaled_rows, dtype=np.uint16)
    except Exception as e:
        print(f"Error during scaling: {e}")
        raise

# Function to build the k-d tree
def build_kdtree(points, depth=0):
    if len(points) == 0:
        return None

    k = len(points[0])  # Dimensionality of the data
    axis = depth % k  # Choose dimension based on depth

    # Sort points and choose median as pivot element
    points = sorted(points, key=lambda x: x[axis])
    median = len(points) //2

    if len(points) == 1:  # Leaf node case
        return KDTreeNode(
            split_dim=axis,
            split_val=points[0][axis],
            left=-1,
            right=-1,
            points=[points[0]]
        )

    # Internal node case
    return KDTreeNode(
        split_dim=axis,
        split_val=points[median][axis],
        left=build_kdtree(points[:median], depth + 1),
        right=build_kdtree(points[median + 1:], depth + 1),
        points=None  # Internal nodes do not store points
    )

# Serialize the tree into a structured array format
def serialize_kdtree(node):
    """
    Serialize the k-d tree into a structured array for C header export.
    """
    nodes = []

    def serialize_node(node):
        if node is None or node == -1:
            return -1

        current_index = len(nodes)
        left_index = serialize_node(node.left) if isinstance(node.left, KDTreeNode) else -1
        right_index = serialize_node(node.right) if isinstance(node.right, KDTreeNode) else -1

        # Serialize current node
        nodes.append({
            "split_dim": node.split_dim if node.split_dim is not None else 0,
            "split_val": node.split_val if node.split_val is not None else 0,
            "left": left_index,
            "right": right_index,
            "points": node.points[0] if node.points else [0] * 5,  # Leaf node vs. internal node
        })
        return current_index

    serialize_node(node)
    return nodes

# Export the structured tree to a C header file
def export_to_c_header(serialized_tree, filename):
    """
    Export the serialized k-d tree to a C header file using structs.
    """
    with open(filename, "w") as f:
        f.write("#ifndef KD_TREE_H\n")
        f.write("#define KD_TREE_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define KD_TREE_SIZE {len(serialized_tree)}\n\n")
        f.write("typedef struct {\n")
        f.write("    uint8_t split_dim;\n")
        f.write("    uint16_t split_val;\n")
        f.write("    int16_t left;\n")
        f.write("    int16_t right;\n")
        f.write("    uint16_t points[5];\n")
        f.write("} KDNode;\n\n")
        f.write("const KDNode kd_tree[KD_TREE_SIZE] = {\n")

        for node in serialized_tree:
            f.write("    {")
            f.write(f"{node['split_dim']}, ")
            f.write(f"{node['split_val']}, ")
            f.write(f"{node['left']}, ")
            f.write(f"{node['right']}, ")
            f.write( "{" + ", ".join(map(str, node['points'])) + "}")
            f.write("},\n")

        f.write("};\n\n")
        f.write("#endif // KD_TREE_H\n")

# Main function to scale CSV, build tree, and export
def main(input_csv, output_header):
    points = scale_csv_values(input_csv, 32768)
    kdtree = build_kdtree(points.tolist())
    serialized_tree = serialize_kdtree(kdtree)
    export_to_c_header(serialized_tree, output_header)

if __name__ == "__main__":
    input_csv = "measurements.csv"
    output_header = "kd_tree.h"
    main(input_csv, output_header)
