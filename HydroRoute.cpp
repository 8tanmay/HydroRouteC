#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <climits>
#include <cstring>
#include <iomanip>
#include <string>

using namespace std;

#define INF INT_MAX
#define MAX_NAME_LEN 50
#define FILE_NAME "relief_camps.dat"

// ==========================================
// DATA STRUCTURES
// ==========================================

// Structure for binary file persistence
struct ReliefCamp {
    int campID;
    char name[MAX_NAME_LEN];
    int maxCapacity;
    int currentOccupancy;
    double foodStockKg;
    double medicalKits;
};
// Graph Edge Structure for Evacuation Routing
struct RoadEdge {
    int toNode;
    int distanceKm;
    double floodWaterDepthMeters;
    // Depth > 0.5m blocks the road
};
// ==========================================
// MODULE 1: REGISTER CAMP
// ==========================================

void registerCamp() {
    ReliefCamp camp;

    cout << "\n--- REGISTER NEW RELIEF CAMP ---\n";

    cout << "Enter Camp ID: ";
    cin >> camp.campID;

    cin.ignore();

    cout << "Enter Camp Name: ";
    cin.getline(camp.name, MAX_NAME_LEN);

    cout << "Enter Max Capacity: ";
    cin >> camp.maxCapacity;

    cout << "Enter Current Occupancy: ";
    cin >> camp.currentOccupancy;

    cout << "Enter Food Stock (in Kg): ";
    cin >> camp.foodStockKg;

    cout << "Enter Medical Kits Count: ";
    cin >> camp.medicalKits;

    // Append record to binary file
    ofstream outFile(FILE_NAME, ios::binary | ios::app);

    if (!outFile) {
        cerr << "[ERROR] Unable to open data file for writing!\n";
        return;
    }

    outFile.write(
        reinterpret_cast<char*>(&camp),
        sizeof(ReliefCamp)
    );

    outFile.close();

    cout << "[SUCCESS] Camp details saved to binary storage ('"
         << FILE_NAME << "').\n";
}

// ==========================================
// MODULE 2: DISPLAY ALL CAMPS
// ==========================================

void displayCamps() {
    ifstream inFile(FILE_NAME, ios::binary);

    if (!inFile) {
        cout << "\n[NOTICE] No records found. Please register a camp first.\n";
        return;
    }

    ReliefCamp camp;

    cout << "\n=========================================================================\n";

    cout << left
         << setw(8) << "ID"
         << setw(25) << "Camp Name"
         << setw(12) << "Occupancy"
         << setw(15) << "Food (Kg)"
         << setw(12) << "Med Kits"
         << "\n";

    cout << "=========================================================================\n";

    while (inFile.read(
        reinterpret_cast<char*>(&camp),
        sizeof(ReliefCamp)
    )) {
        string occStatus =
            to_string(camp.currentOccupancy) +
            "/" +
            to_string(camp.maxCapacity);

        cout << left
             << setw(8) << camp.campID
             << setw(25) << camp.name
             << setw(12) << occStatus
             << setw(15) << camp.foodStockKg
             << setw(12) << camp.medicalKits
             << "\n";
    }

    cout << "=========================================================================\n";

    inFile.close();
}

// ==========================================
// MODULE 3: DIJKSTRA'S EVACUATION ROUTER
// ==========================================

void calculateEvacuationRoute() {

    int numNodes = 5;

    string districts[] = {
        "Guwahati Hub",
        "Morigaon",
        "Nagaon",
        "Jorhat",
        "Majuli Shelter"
    };
    // Construct Adjacency List for Region Graph
    vector<vector<RoadEdge>> graph(numNodes);

    // Format:
    // addEdge(from, to, distance_km, water_depth_meters)

    graph[0].push_back({1, 50, 0.2});
    // Guwahati -> Morigaon (Passable)

    graph[1].push_back({2, 70, 0.8});
    // Morigaon -> Nagaon (BLOCKED: Water > 0.5m)

    graph[0].push_back({2, 110, 0.1});
    // Guwahati -> Nagaon (Passable Bypass)

    graph[2].push_back({3, 90, 0.3});
    // Nagaon -> Jorhat (Passable)

    graph[3].push_back({4, 20, 0.1});
    // Jorhat -> Majuli Shelter (Passable)

    int source = 0; // Guwahati
    int target = 4; // Majuli Safe Shelter

    vector<int> dist(numNodes, INF);
    vector<int> parent(numNodes, -1);
    vector<bool> visited(numNodes, false);

    priority_queue<
        pair<int, int>,
        vector<pair<int, int>>,
        greater<pair<int, int>>
    > pq;

    dist[source] = 0;
    pq.push({0, source});

    // Dijkstra's Algorithm
    while (!pq.empty()) {

        int u = pq.top().second;
        pq.pop();

        if (visited[u])
            continue;

        visited[u] = true;

        for (const auto& edge : graph[u]) {
            int v = edge.toNode;
            int weight = edge.distanceKm;

            // SAFETY FILTER:
            // Skip roads where flood water depth > 0.5m
            if (edge.floodWaterDepthMeters > 0.5) {
                continue;
            }

            if (!visited[v] &&
                dist[u] != INF &&
                dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;

                // Store the previous node
                parent[v] = u;

                // Add updated distance to priority queue
                pq.push({dist[v], v});
            }
        }
    }
    // Check whether destination is reachable
    if (dist[target] == INF) {

        cout << "\n[CRITICAL WARNING] No safe road path available to "
             << districts[target]
             << " due to flood severity!\n";

        return;
    }
    // Reconstruct Safe Path
    vector<int> path;

    for (int curr = target; curr != -1; curr = parent[curr]) {
        path.push_back(curr);
    }

    cout << "\n===================================================\n";
    cout << "   OPTIMAL SAFE EVACUATION ROUTE (FLOOD-AWARE)\n";
    cout << "===================================================\n";

    cout << "Start Location : "
         << districts[source] << "\n";

    cout << "Destination    : "
         << districts[target] << "\n";

    cout << "Total Distance : "
         << dist[target] << " KM\n";

    cout << "Path Taken     : ";

    for (int i = static_cast<int>(path.size()) - 1; i >= 0; i--) {
        cout << districts[path[i]];
        if (i > 0)
            cout << " -> ";
    }
    cout << "\n";
    cout << "===================================================\n";
}
// ==========================================
// MODULE 4: RESOURCE ALERTS & MONITORING
// ==========================================

void checkResourceAlerts() {

    ifstream inFile(FILE_NAME, ios::binary);
    if (!inFile) {
        cout << "\n[NOTICE] No records to scan. Add camps first.\n";
        return;
    }

    ReliefCamp camp;
    bool alertFound = false;

    cout << "\n===================================================\n";
    cout << "         CRITICAL RESOURCE SHORTAGE ALERTS\n";
    cout << "===================================================\n";

    while (inFile.read(
        reinterpret_cast<char*>(&camp),
        sizeof(ReliefCamp)
    )) {

        // Critical conditions:
        // Food < 100kg OR Occupancy >= Max Capacity

        if (camp.foodStockKg < 100.0 ||
            camp.currentOccupancy >= camp.maxCapacity) {
            alertFound = true;
            cout << "[ALERT] Camp ID: "
                 << camp.campID
                 << " (" << camp.name << ")\n";

            if (camp.foodStockKg < 100.0) {

                cout << "  -> CRITICAL FOOD SHORTAGE: "
                     << camp.foodStockKg
                     << " Kg remaining!\n";
            }
            if (camp.currentOccupancy >= camp.maxCapacity) {

                cout << "  -> CAMP OVERCROWDED: "
                     << camp.currentOccupancy
                     << "/"
                     << camp.maxCapacity
                     << " seats filled!\n";
            }

            cout << "---------------------------------------------------\n";
        }
    }

    if (!alertFound) {

        cout << "[STATUS] All camps operating with sufficient "
             << "stocks and safe capacities.\n";

        cout << "===================================================\n";
    }
    inFile.close();
}

// ==========================================
// MAIN DRIVER CONTROL LOOP
// ==========================================

int main() {
    int choice;
    while (true) {

        cout << "\n===================================================\n";
        cout << "   DISASTER MANAGEMENT ENGINE (ASSAM REGION)\n";
        cout << "===================================================\n";

        cout << "1. Register Relief Camp Data\n";
        cout << "2. View All Camps & Occupancy\n";
        cout << "3. Find Safe Evacuation Route (Dijkstra)\n";
        cout << "4. Scan Offline Resource Shortage Alerts\n";
        cout << "5. Exit System\n";

        cout << "---------------------------------------------------\n";
        cout << "Select Choice [1-5]: ";

        if (!(cin >> choice)) {

            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\nInvalid input. Please enter a number from 1 to 5.\n";
            continue;
        }

        switch (choice) {

            case 1:
                registerCamp();
                break;

            case 2:
                displayCamps();
                break;

            case 3:
                calculateEvacuationRoute();
                break;

            case 4:
                checkResourceAlerts();
                break;

            case 5:
                cout << "\nExiting System. Stay Safe!\n";
                return 0;

            default:
                cout << "\nInvalid choice. Select between 1 and 5.\n";
        }
    }
    return 0;
}
