#include <iostream>
#include <vector>
#include <string>
#include <climits>
#include <chrono>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <iomanip>

using namespace std;
using namespace std::chrono;

#define INF INT_MAX
#define N 10
#define REPEAT 100000  // jalankan 100.000 kali untuk hasil waktu yang terukur

vector<string> nameNode = {
    "Titik_Awal", "Pasar_Kotagede", "Pasar_Demangan", "Pasar_Gentan",
    "Pasar_Pingit", "Pasar_Sentul", "Pasar_Giwangan", "Pasar_Beringharjo",
    "Pasar_Kranggan", "Pasar_Serangan"
};

double coordX[N] = { 0.0, 8.0, 4.0, 7.0, 10.0, 6.0, 5.0, 7.0, 5.0, 8.0 };
double coordY[N] = { 0.0, 3.0, 2.0, 6.0,  9.0, 5.0, 2.0, 4.0, 3.0, 6.0 };

struct Edge { int to, weight; };
vector<Edge> adj[N];

void add_edge(int u, int v, int w) { adj[u].push_back({v, w}); }

double heuristic(int u, int goal) {
    double dx = coordX[u] - coordX[goal];
    double dy = coordY[u] - coordY[goal];
    return sqrt(dx * dx + dy * dy);
}

void print_path(vector<int>& parent, int destination) {
    vector<int> path;
    int cur = destination;
    while (cur != -1) { path.push_back(cur); cur = parent[cur]; }
    for (int i = path.size() - 1; i >= 0; i--) {
        cout << nameNode[path[i]];
        if (i != 0) cout << " -> ";
    }
}

int path_cost(vector<int>& parent, int destination) {
    vector<int> path;
    int cur = destination;
    while (cur != -1) { path.push_back(cur); cur = parent[cur]; }
    int total = 0;
    for (int i = path.size() - 1; i > 0; i--) {
        int u = path[i], v = path[i-1];
        for (auto e : adj[u])
            if (e.to == v) { total += e.weight; break; }
    }
    return total;
}

// ======================
// BFS
// ======================
void bfs(int source, int goal, vector<int>& parent, int& nodesExplored) {
    vector<bool> visited(N, false);
    parent.assign(N, -1);
    nodesExplored = 0;
    queue<int> q;
    visited[source] = true;
    q.push(source);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        nodesExplored++;
        if (u == goal) break;
        for (auto e : adj[u])
            if (!visited[e.to]) {
                visited[e.to] = true;
                parent[e.to] = u;
                q.push(e.to);
            }
    }
}

// ======================
// DFS
// ======================
void dfs(int source, int goal, vector<int>& parent, int& nodesExplored) {
    vector<bool> visited(N, false);
    parent.assign(N, -1);
    nodesExplored = 0;
    stack<int> s;
    s.push(source);
    while (!s.empty()) {
        int u = s.top(); s.pop();
        if (visited[u]) continue;
        visited[u] = true;
        nodesExplored++;
        if (u == goal) break;
        for (int i = adj[u].size() - 1; i >= 0; i--) {
            int v = adj[u][i].to;
            if (!visited[v]) { parent[v] = u; s.push(v); }
        }
    }
}

// ======================
// GREEDY Best-First Search
// ======================
void greedy(int source, int goal, vector<int>& parent, int& nodesExplored) {
    vector<bool> visited(N, false);
    parent.assign(N, -1);
    nodesExplored = 0;
    priority_queue<pair<double,int>, vector<pair<double,int>>, greater<pair<double,int>>> pq;
    pq.push({heuristic(source, goal), source});
    while (!pq.empty()) {
        int u = pq.top().second; pq.pop();
        if (visited[u]) continue;
        visited[u] = true;
        nodesExplored++;
        if (u == goal) break;
        for (auto e : adj[u])
            if (!visited[e.to]) {
                parent[e.to] = u;
                pq.push({heuristic(e.to, goal), e.to});
            }
    }
}

// ======================
// Benchmark: jalankan REPEAT kali, ukur total waktu
// ======================
long long benchmark_bfs(int goal, vector<int>& parent, int& explored) {
    auto start = high_resolution_clock::now();
    for (int i = 0; i < REPEAT; i++) bfs(0, goal, parent, explored);
    auto stop = high_resolution_clock::now();
    return duration_cast<nanoseconds>(stop - start).count();
}

long long benchmark_dfs(int goal, vector<int>& parent, int& explored) {
    auto start = high_resolution_clock::now();
    for (int i = 0; i < REPEAT; i++) dfs(0, goal, parent, explored);
    auto stop = high_resolution_clock::now();
    return duration_cast<nanoseconds>(stop - start).count();
}

long long benchmark_greedy(int goal, vector<int>& parent, int& explored) {
    auto start = high_resolution_clock::now();
    for (int i = 0; i < REPEAT; i++) greedy(0, goal, parent, explored);
    auto stop = high_resolution_clock::now();
    return duration_cast<nanoseconds>(stop - start).count();
}

void print_result(const string& name, vector<int>& parent, int dest,
                  int explored, long long total_ns) {
    bool valid = false;
    int cur = dest;
    while (cur != -1) { if (cur == 0) { valid = true; break; } cur = parent[cur]; }

    cout << "\n+-----------------------------------------+\n";
    cout << "| Algoritma : " << left << setw(27) << name << "|\n";
    cout << "+-----------------------------------------+\n";
    if (!valid) {
        cout << "  Tujuan tidak dapat dijangkau.\n";
        return;
    }
    cout << "  Rute   : "; print_path(parent, dest); cout << "\n";
    cout << "  Jarak  : " << path_cost(parent, dest) << " km\n";
    cout << "  Node dieksplorasi : " << explored << " node\n";
    cout << fixed << setprecision(4);
    cout << "  Waktu rata-rata   : " << (double)total_ns / REPEAT << " ns/run\n";
    cout << "  Waktu rata-rata   : " << (double)total_ns / REPEAT / 1000000.0 << " ms/run\n";
    cout << "  Total (" << REPEAT << "x)  : " << total_ns / 1000000.0 << " ms\n";
}

int main() {
    add_edge(0, 1, 8); add_edge(0, 2, 4); add_edge(0, 3, 7);
    add_edge(3, 4, 9); add_edge(1, 5, 4); add_edge(1, 6, 2);
    add_edge(6, 7, 5); add_edge(5, 7, 1); add_edge(2, 4, 3);
    add_edge(2, 8, 3); add_edge(8, 7, 3); add_edge(8, 9, 3);
    add_edge(7, 9, 2); add_edge(9, 4, 3);

    cout << "===== PENCARIAN RUTE PASAR YOGYAKARTA =====\n";
    cout << "Daftar Node:\n";
    for (int i = 0; i < N; i++)
        cout << "  " << i << " : " << nameNode[i] << "\n";

    int destination;
    cout << "\nMasukkan index tujuan (1-9): ";
    cin >> destination;

    if (destination <= 0 || destination >= N) {
        cout << "Index tidak valid.\n";
        return 1;
    }

    cout << "\nTujuan     : " << nameNode[destination] << "\n";
    cout << "Pengulangan: " << REPEAT << "x per algoritma\n";
    cout << "Menjalankan benchmark...\n";

    vector<int> parent;
    int explored;

    long long t_bfs    = benchmark_bfs   (destination, parent, explored);
    int exp_bfs = explored; vector<int> par_bfs = parent;

    long long t_dfs    = benchmark_dfs   (destination, parent, explored);
    int exp_dfs = explored; vector<int> par_dfs = parent;

    long long t_greedy = benchmark_greedy(destination, parent, explored);
    int exp_greedy = explored; vector<int> par_greedy = parent;

    print_result("BFS",                  par_bfs,    destination, exp_bfs,    t_bfs);
    print_result("DFS",                  par_dfs,    destination, exp_dfs,    t_dfs);
    print_result("GREEDY Best-First",    par_greedy, destination, exp_greedy, t_greedy);

    // Ringkasan ranking waktu
    cout << "\n===== RANKING KECEPATAN =====\n";
    vector<pair<long long, string>> ranking = {
        {t_bfs,    "BFS"},
        {t_dfs,    "DFS"},
        {t_greedy, "GREEDY"}
    };
    sort(ranking.begin(), ranking.end());
    for (int i = 0; i < 3; i++) {
        cout << "  " << i+1 << ". " << left << setw(10) << ranking[i].second
             << " : " << fixed << setprecision(4)
             << (double)ranking[i].first / REPEAT << " ns/run\n";
    }

    cout << "\n===== CATATAN =====\n";
    cout << "  BFS    : optimal hop, BUKAN optimal jarak km\n";
    cout << "  DFS    : tidak optimal, tapi cepat\n";
    cout << "  GREEDY : tidak selalu optimal, tergantung heuristic\n";
    cout << "  Untuk optimal jarak (km) -> gunakan A* atau DAG Shortest Path\n";

    return 0;
}
