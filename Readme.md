## Compile first
### Linux
```
g++ -std=c++17 -O3 -I./lib StressTest.cpp lib/Graph.cpp lib/FileIO.cpp lib/Algorithms.cpp -o StressTest
g++ -std=c++17 -O3 KNNGraph.cpp -o KNNGraph
```
### Windows
```
g++ -std=c++17 -O3 -I./lib StressTest.cpp lib/Graph.cpp lib/FileIO.cpp lib/Algorithms.cpp -o StressTest.exe
g++ -std=c++17 -O3 KNNGraph.cpp -o KNNGraph.exe
```

## Running the Program
### Linux
```
pip install streamlit pandas plotly
./KNNGraph
./StressTest
streamlit run app.py
```
### Windows
```
pip install streamlit pandas plotly
./KNNGraph.exe
./StressTest.exe
streamlit run app.py
```

