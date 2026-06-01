import matplotlib.pyplot as plt

def load_cpp_graph(filename):
    nodes = {}  
    edges = []  
    
    with open(filename, 'r') as file:
        header = file.readline().strip().split()
        V = int(header[0])
        E = int(header[1])
        
        # 2. Read Coordinates (Notice we do NOT convert line[0] to int anymore)
        for _ in range(V):
            line = file.readline().strip().split()
            node_id = line[0]      # Keeps it as a string like 'A1'
            x = float(line[1])
            y = float(line[2])
            nodes[node_id] = (x, y)
            
        # 3. Read Edges
        for _ in range(E):
            line = file.readline().strip().split()
            u = line[0]            # Keeps it as a string
            v = line[1]            # Keeps it as a string
            weight = float(line[2])
            edges.append((u, v, weight))
            
    return V, E, nodes, edges

def plot_map(nodes, edges):
    """Draws the nodes and edges to verify the network topology."""
    plt.figure(figsize=(10, 10))
    
    # 1. Plot all the roads (edges)
    for u, v, weight in edges:
        x_coords = [nodes[u][0], nodes[v][0]]
        y_coords = [nodes[u][1], nodes[v][1]]
        
        # Draw a line between the two coordinates
        plt.plot(x_coords, y_coords, color='#888888', alpha=0.5, linewidth=0.7)
        
    # 2. Plot all the intersections (nodes)
    x_vals = [pos[0] for pos in nodes.values()]
    y_vals = [pos[1] for pos in nodes.values()]
    plt.scatter(x_vals, y_vals, c='#1f77b4', s=15, zorder=5) # zorder puts nodes on top of lines
    
    # 3. Format the chart
    plt.title("C++ Generated KNN + Skeleton Graph", fontsize=16, fontweight='bold')
    plt.xlabel("X Coordinate")
    plt.ylabel("Y Coordinate")
    plt.grid(True, linestyle='--', alpha=0.3)
    
    # Show the window
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Point this to whatever text file your C++ program just made
    filename = "data/map_15.txt" 
    
    try:
        V, E, nodes, edges = load_cpp_graph(filename)
        print("Data loaded successfully! Rendering map...")
        plot_map(nodes, edges)
    except FileNotFoundError:
        print(f"Error: Could not find {filename}. Did you run the C++ generator first?")