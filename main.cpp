#include <iostream>
#include <cmath>
#include <queue>
#include <set>
#include <limits>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <optional>

using namespace std;

const int MAX_ENERGY = 1000;
const int TRAVEL_TIME = 2;

class Edge
{
public:
    string from;
    string to;
    int weight;
    int obstacle;

    Edge(string from, string to, int weight, int obstacle)
        : from(from),
          to(to),
          weight(weight),
          obstacle(obstacle) {}
};

class Node
{
public:
    string name;
    unordered_map<string, Edge> edges;
    bool is_rest_point;
    bool is_charging_station;

    Node(string name) : name(name) {}
};

class Graph
{
public:
    struct Solution
    {
        int minimum_energy;
        vector<pair<string, int>> path;

        Solution(int minimum_energy, vector<pair<string, int>> path)
            : minimum_energy(minimum_energy), path(path) {};
    };

    unordered_map<string, Node> nodes;

    bool IsExist(string name)
    {
        return nodes.find(name) != nodes.end();
    }

    void AddNode(string name)
    {
        nodes.try_emplace(name, name);
    }

    void AddEdge(string u, string v, int w, int o)
    {
        nodes.at(u).edges.try_emplace(v, u, v, w, o);
    }

    void SetRestPoint(string name)
    {
        AddNode(name);
        nodes.at(name).is_rest_point = true;
    }

    void SetChargingStation(string name)
    {
        AddNode(name);
        nodes.at(name).is_charging_station = true;
    }

    optional<Solution> SolveShortestPath(string start, string target, int start_hour)
    {
        auto CompareFunc = [](NodeState a, NodeState b)
        { return a.energy_consumed > b.energy_consumed; };

        unordered_map<string, NodeState> states;
        set<string> visited_nodes;
        priority_queue<NodeState, vector<NodeState>, decltype(CompareFunc)> pq(CompareFunc);

        for (auto [name, _] : nodes)
        {
            states.emplace(name, NodeState(name, numeric_limits<int>::max(), 0, 0, vector<string>()));
        }
        states.at(start).energy_consumed = 0;
        states.at(start).energy_left = MAX_ENERGY;
        states.at(start).path = {start};
        pq.emplace(states.at(start));

        while (!pq.empty())
        {
            NodeState current = pq.top();
            pq.pop();

            // Cek apakah node sudah pernah didatangi
            if (visited_nodes.find(current.name) != visited_nodes.end())
            {
                continue;
            }
            visited_nodes.emplace(current.name);

            if (nodes.at(current.name).is_rest_point && (start_hour + current.time / 60) % 2 != 0)
            {
                current.time = (current.time / 60 + 1) * 60;
            }

            if (nodes.at(current.name).is_charging_station)
            {
                current.energy_left = MAX_ENERGY;
            }

            for (auto [n, e] : nodes.at(current.name).edges)
            {
                NodeState &s = states.at(n);

                double multiplier = ((start_hour + current.time / 60) % 2) ? 1.3 : 0.8;
                int energy_consumption = (e.weight + e.obstacle) * multiplier;

                if (current.energy_left >= energy_consumption &&
                    current.energy_consumed + energy_consumption < s.energy_consumed)
                {
                    s.energy_consumed = current.energy_consumed + energy_consumption;
                    s.energy_left = current.energy_left - energy_consumption;
                    s.time = current.time + TRAVEL_TIME;
                    s.path = current.path;
                    s.path.emplace_back(s.name);
                    pq.emplace(s);
                }
            }
        }

        if (states.at(target).energy_consumed == numeric_limits<int>::max())
        {
            return nullopt;
        }

        vector<pair<string, int>> path;
        for (auto n : states.at(target).path)
        {
            path.emplace_back(n, states.at(n).time);
        }
        return Solution(states.at(target).energy_consumed, path);
    }

    static vector<string> ExtractNodeNames(string line)
    {
        stringstream ss(line);
        string name;
        vector<string> names;
        while (ss >> name)
        {
            names.emplace_back(name);
        }
        return names;
    }

    static void PrintShortestPath(Solution solution)
    {
        vector<pair<string, int>> path = solution.path;
        cout << "Total energi minimum: " << solution.minimum_energy << "\n";
        cout << "Jalur: " << path.at(0).first;
        for (int i = 1; i < path.size(); i++)
        {
            cout << " -> " << path.at(i).first;
        }
        cout << "\n";
        cout << "Waktu tiba:\n";
        for (auto [name, time] : path)
        {
            cout << name << " " << "(menit " << time << ")\n";
        }
    }

private:
    struct NodeState
    {
        string name;
        int energy_consumed;
        int energy_left;
        int time;
        vector<string> path;

        NodeState(string name, int energy_consumed, int energy_left, int time, vector<string> path)
            : name(name),
              energy_consumed(energy_consumed),
              energy_left(energy_left),
              time(time),
              path(path)
        {
        }
    };
};

int main()
{
    int N, M;
    std::cin >> N >> M;

    Graph graph;

    for (int i = 0; i < M; i++)
    {
        string u, v;
        int w, o;
        cin >> u >> v >> w >> o;
        graph.AddNode(u);
        graph.AddNode(v);
        graph.AddEdge(u, v, w, o);
    }

    string start, target;
    cin >> start >> target;
    cin.ignore();

    string line;
    getline(cin, line);
    vector<string> rest_points = Graph::ExtractNodeNames(line);
    for (auto name : rest_points)
    {
        if (name == "-")
            continue;
        graph.SetRestPoint(name);
    }

    getline(cin, line);
    vector<string> charging_stations = Graph::ExtractNodeNames(line);
    for (auto name : charging_stations)
    {
        if (name == "-")
            continue;
        graph.SetChargingStation(name);
    }

    // Mechanical dan Electrical tidak berpengaruh pada input
    getline(cin, line);
    getline(cin, line);

    int start_hour;
    cin >> start_hour;

    optional<Graph::Solution> result = graph.SolveShortestPath(start, target, start_hour);
    if (!result.has_value())
    {
        cout << "Robot gagal dalam mencapai tujuan :(\n";
        return 0;
    }
    Graph::PrintShortestPath(result.value());

    return 0;
}