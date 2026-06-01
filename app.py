import streamlit as st
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
import os
import math

# --- SYNCHRONIZED GENERATOR FUNCTION ---
def generate_vertex_sizes(max_v=2000):
    """
    Generates vertex sizes exactly matching the C++ benchmark engine configuration.
    """
    vertices = []
    v = 16
    while v <= max_v:
        vertices.append(v)
        if v < 500:
            # Explicit float-to-int conversion matching C++ static_cast
            v = math.floor(int(v * 1.2))
        else:
            v = math.floor(int(v * 1.5))
    return vertices

# --- PAGE SETUP ---
st.set_page_config(page_title="Pathfinding Benchmark", layout="wide")
st.title("🗺️ Spatial Pathfinding Benchmark")
st.markdown("Comparing A*, Greedy Best-First, and Exhaustive Search on K-Nearest Neighbor Graphs.")

# --- SIDEBAR CONTROLS ---
st.sidebar.header("Map Viewer Controls")

# Dynamically populate the sidebar options up to 50,000 vertices
vertex_options = generate_vertex_sizes()
selected_v = st.sidebar.selectbox("Select Map Size to View (Vertices)", vertex_options)

map_file = f"data/map_{selected_v}.txt"
csv_file = f"result/stress_test_{selected_v}.csv"

if not os.path.exists(map_file) or not os.path.exists(csv_file):
    st.error(f"Missing data files for V={selected_v}. Ensure {map_file} and {csv_file} exist.")
    st.stop()

# --- DATA LOADING FUNCTIONS ---
@st.cache_data
def load_map_data(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
        
    v_count, e_count = map(int, lines[0].strip().split())
    
    nodes = []
    for i in range(1, v_count + 1):
        parts = lines[i].strip().split()
        nodes.append({'id': int(parts[0]), 'x': float(parts[1]), 'y': float(parts[2])})
    
    edges = []
    for i in range(v_count + 1, len(lines)):
        parts = lines[i].strip().split()
        edges.append({'u': int(parts[0]), 'v': int(parts[1]), 'weight': float(parts[2])})
        
    return pd.DataFrame(nodes).set_index('id'), pd.DataFrame(edges)

# Load current map data
with st.spinner("Loading Data..."):
    df_nodes, df_edges = load_map_data(map_file)
    df_bench_single = pd.read_csv(csv_file)
    
    # Keep a pure raw copy so we can still calculate the Failure Rate (-1)
    df_raw_single = df_bench_single.copy()

    # DATA IMPUTATION: Replace -1 distances with the perfect A* distance
    astar_lookup = df_bench_single[df_bench_single["Algorithm"] == "A_Star"][["StartNode", "TargetNode", "TotalDistance"]]
    astar_lookup = astar_lookup.rename(columns={"TotalDistance": "OptDist"})
    
    df_bench_single = df_bench_single.merge(astar_lookup, on=["StartNode", "TargetNode"], how="left")
    df_bench_single.loc[df_bench_single["TotalDistance"] < 0, "TotalDistance"] = df_bench_single["OptDist"]
    df_bench_single = df_bench_single.drop(columns=["OptDist"])

# ==========================================
# SECTION 1: MAP VISUALIZATION
# ==========================================
st.subheader(f"Graph Topology (V={selected_v})")

edge_x = []
edge_y = []
for _, row in df_edges.iterrows():
    x0, y0 = df_nodes.loc[row['u'], ['x', 'y']]
    x1, y1 = df_nodes.loc[row['v'], ['x', 'y']]
    edge_x.extend([x0, x1, None])
    edge_y.extend([y0, y1, None])

fig_map = go.Figure()
fig_map.add_trace(go.Scatter(
    x=edge_x, y=edge_y, line=dict(width=0.5, color='#888'), hoverinfo='none', mode='lines', name='Roads'
))
fig_map.add_trace(go.Scatter(
    x=df_nodes['x'], y=df_nodes['y'], mode='markers', marker=dict(size=4, color='#1f77b4'),
    text=[f"Node {id}" for id in df_nodes.index], hoverinfo='text', name='Cities'
))

fig_map.update_layout(
    height=400, margin=dict(l=0, r=0, t=0, b=0),
    xaxis=dict(showgrid=False, zeroline=False, showticklabels=False),
    yaxis=dict(showgrid=False, zeroline=False, showticklabels=False),
    plot_bgcolor='white'
)
st.plotly_chart(fig_map, use_container_width=True)

# ==========================================
# SECTION 2: SPECIFIC MAP STRESS TEST (BOX PLOTS)
# ==========================================
st.markdown("---")
st.subheader(f"Stress Test Variance (V={selected_v})")

col1, col2 = st.columns(2)
with col1:
    fig_time_box = px.box(df_bench_single, x="Algorithm", y="NodesVisited", color="Algorithm", points="all", log_y=True, template="plotly_white", title="Nodes Visited (Log Scale)")
    st.plotly_chart(fig_time_box, use_container_width=True)

with col2:
    fig_dist_box = px.box(df_bench_single, x="Algorithm", y="TotalDistance", color="Algorithm", points="all", template="plotly_white", title="Path Distance Accuracy (Imputed)")
    st.plotly_chart(fig_dist_box, use_container_width=True)

# ==========================================
# SECTION 2.5: HANDLING THE -1 VALUES (FAILURES/TIMEOUTS)
# ==========================================
st.markdown("---")
st.subheader("⚠️ Algorithm Reliability (Timeout & Failure Rate)")
st.markdown("*(Percentage of tests that returned `-1` because they hit the 5,000,000 node limit or couldn't find a path)*")

failure_rates = df_raw_single.groupby("Algorithm").apply(
    lambda x: (x["TotalDistance"] == -1).sum() / len(x) * 100
).reset_index(name="FailureRate")

fig_fail = px.bar(
    failure_rates, 
    x="Algorithm", 
    y="FailureRate", 
    color="Algorithm",
    text=failure_rates["FailureRate"].apply(lambda x: f"{x:.1f}%"),
    title=f"Failure / Timeout Rate for V={selected_v}",
    labels={"FailureRate": "Failure Rate (%)"},
    template="plotly_white"
)

fig_fail.update_traces(textposition='outside')
fig_fail.update_layout(yaxis_range=[0, 100]) 
st.plotly_chart(fig_fail, use_container_width=True)

# ==========================================
# SECTION 3: OVERALL GROWTH TRENDS (DYNAMIC DYNAMIC DYNAMIC)
# ==========================================
st.markdown("---")
st.subheader("📈 Overall Algorithmic Growth (Big-O Trends)")
st.markdown("*(Averaging the tests dynamically across all detected data points available in your results directory)*")

all_data = []
# REFACTORED: Loop uses the dynamic generator instead of a hardcoded array
for v in vertex_options:
    file = f"result/stress_test_{v}.csv"
    if os.path.exists(file):
        temp_df = pd.read_csv(file)
        
        # Apply the exact same Data Imputation to the master dataset
        astar_opt = temp_df[temp_df["Algorithm"] == "A_Star"][["StartNode", "TargetNode", "TotalDistance"]]
        astar_opt = astar_opt.rename(columns={"TotalDistance": "OptDist"})
        
        temp_df = temp_df.merge(astar_opt, on=["StartNode", "TargetNode"], how="left")
        temp_df.loc[temp_df["TotalDistance"] < 0, "TotalDistance"] = temp_df["OptDist"]
        temp_df = temp_df.drop(columns=["OptDist"])
        
        all_data.append(temp_df)
        
if all_data:
    full_df = pd.concat(all_data)
    
    # Calculate means grouped by Algorithm and Vertex Size
    agg_df = full_df.groupby(["Algorithm", "Vertices"])[["NodesVisited", "TotalDistance"]].mean().reset_index()

    # --- Graph 1: Time Complexity (Full Width) ---
    st.markdown("### ⏱️ Performance Metric: Nodes Visited")
    fig_line_time = px.line(
        agg_df, 
        x="Vertices", 
        y="NodesVisited", 
        color="Algorithm", 
        markers=True,
        log_y=True,  
        log_x=True, # Keeps geometric spacing equidistant
        template="plotly_white",
        title="Average Time Complexity Growth (Log-Log Scale)"
    )
    # Give it a bit more breathing room vertically since it's full-width
    fig_line_time.update_layout(height=500)
    st.plotly_chart(fig_line_time, use_container_width=True)

    st.markdown("---")

    # --- Graph 2: Path Accuracy (Full Width) ---
    st.markdown("### 📏 Accuracy Metric: Total Path Distance")
    fig_line_dist = px.line(
        agg_df, 
        x="Vertices", 
        y="TotalDistance", 
        color="Algorithm", 
        markers=True,
        log_x=True, 
        template="plotly_white",
        title="Average Path Distance Growth (Log-X Scale)"
    )
    fig_line_dist.update_layout(height=500, yaxis_rangemode="tozero")
    st.plotly_chart(fig_line_dist, use_container_width=True)

else:
    st.warning("Could not find multiple stress test CSV files to build the line graphs.")