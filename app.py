import streamlit as st
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
import os

# --- PAGE SETUP ---
st.set_page_config(page_title="Pathfinding Benchmark", layout="wide")
st.title("Spatial Pathfinding Benchmark")
st.markdown("Comparing A*, Greedy Best-First, and Exhaustive Search on K-Nearest Neighbor Graphs.")

# --- SIDEBAR CONTROLS ---
st.sidebar.header("Map Viewer Controls")
vertex_options = [15, 100, 500, 1000]
selected_v = st.sidebar.selectbox("Select Map Size to View (Vertices)", vertex_options)

map_file = f"data/map_{selected_v}.txt"
csv_file = f"result/stress_test_{selected_v}.csv"

if not os.path.exists(map_file) or not os.path.exists(csv_file):
    st.error(f"Missing data files for V={selected_v}. Ensure {map_file} and {csv_file} are in the folder.")
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
    df_valid_single = df_bench_single[df_bench_single["TotalDistance"] >= 0]

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
st.subheader(f"Stress Test Variance (V={selected_v}, 100 Tests)")

col1, col2 = st.columns(2)
with col1:
    fig_time_box = px.box(df_valid_single, x="Algorithm", y="NodesVisited", color="Algorithm", points="all", log_y=True, template="plotly_white", title="Nodes Visited (Log Scale)")
    st.plotly_chart(fig_time_box, use_container_width=True)

with col2:
    fig_dist_box = px.box(df_valid_single, x="Algorithm", y="TotalDistance", color="Algorithm", points="all", template="plotly_white", title="Path Distance Accuracy")
    st.plotly_chart(fig_dist_box, use_container_width=True)


# ==========================================
# SECTION 3: OVERALL GROWTH TRENDS (LINE GRAPHS)
# ==========================================
st.markdown("---")
st.subheader("📈 Overall Algorithmic Growth (Big-O Trends)")
st.markdown("*(Averaging the 100 tests across all available map sizes to show mathematical scaling)*")

# 1. Automatically load all available CSV files in the folder
all_data = []
for v in [15, 100, 500, 1000]:
    file = f"stress_test_{v}.csv"
    if os.path.exists(file):
        temp_df = pd.read_csv(file)
        temp_df = temp_df[temp_df["TotalDistance"] >= 0] # Remove failed paths
        all_data.append(temp_df)

if all_data:
    # 2. Combine them all into one massive dataframe
    full_df = pd.concat(all_data)
    
    # 3. Calculate the AVERAGE (mean) for each algorithm at each Vertex size
    # This turns 100 scattered points into 1 clean data point for the line graph
    agg_df = full_df.groupby(["Algorithm", "Vertices"]).mean().reset_index()

    col3, col4 = st.columns(2)

    with col3:
        fig_line_time = px.line(
            agg_df, 
            x="Vertices", 
            y="NodesVisited", 
            color="Algorithm", 
            markers=True,
            log_y=True,  # CRITICAL: Keep log scale so Exhaustive doesn't flatten the others
            template="plotly_white",
            title="Average Time Complexity Growth"
        )
        st.plotly_chart(fig_line_time, use_container_width=True)

    with col4:
        fig_line_dist = px.line(
            agg_df, 
            x="Vertices", 
            y="TotalDistance", 
            color="Algorithm", 
            markers=True,
            template="plotly_white",
            title="Average Path Distance Growth"
        )
        # Force the Y-axis to start at 0 so the visual difference is accurate
        fig_line_dist.update_layout(yaxis_rangemode="tozero")
        st.plotly_chart(fig_line_dist, use_container_width=True)
else:
    st.warning("Could not find multiple stress test CSV files to build the line graphs.")