#include <bits/stdc++.h>
#include <nlohmann/json.hpp> // JSON Library
using namespace std;
using json = nlohmann::json;

struct Task {
    string name;
    int priority;
    int deadline;
    vector<string> dependencies;
};

// Comparator for the priority queue
struct Compare {
    bool operator()(const Task &a, const Task &b) {
        if (a.deadline == b.deadline) {
            if (a.priority == b.priority)
                return a.name > b.name;
            return a.priority < b.priority;
        }
        return a.deadline > b.deadline;
    }
};

// Function to log task execution
void logTaskExecution(const string &taskName) {
    ofstream logFile("tasks_log.txt", ios_base::app);
    time_t now = time(0);
    string timeStr = ctime(&now);
    timeStr.pop_back();  // Remove newline character
    logFile << "[" << timeStr << "] Executed Task: " << taskName << endl;
    logFile.close();
}

// Save tasks to JSON file
void saveTasksToJson(const map<string, Task> &taskMap) {
    json tasks = json::array();
    for (const auto &entry : taskMap) {
        json task = {
            {"name", entry.first},
            {"priority", entry.second.priority},
            {"deadline", entry.second.deadline},
            {"dependencies", entry.second.dependencies}
        };
        tasks.push_back(task);
    }
    ofstream file("tasks.json");
    file << tasks.dump(4);
    file.close();
}

// Load tasks from JSON file
void loadTasksFromJson(map<string, Task> &taskMap, set<string> &taskNames,
                       map<string, int> &inDegree, map<string, vector<string>> &adjList) {
    ifstream file("tasks.json");
    if (!file) {
        cout << "No saved tasks found.\n";
        return;
    }
    json tasks = json::parse(file);
    file.close();
    for (const auto &task : tasks) {
        Task newTask;
        newTask.name = task["name"];
        newTask.priority = task["priority"];
        newTask.deadline = task["deadline"];
        newTask.dependencies = task["dependencies"];
        taskMap[newTask.name] = newTask;
        taskNames.insert(newTask.name);
        for (const auto &dep : newTask.dependencies) {
            adjList[dep].push_back(newTask.name);
            inDegree[newTask.name]++;
        }
        if (inDegree.find(newTask.name) == inDegree.end()) {
            inDegree[newTask.name] = 0;
        }
    }
}

// Function to display tasks from JSON
void displaySavedTasks() {
    ifstream file("tasks.json");
    if (!file) {
        cout << "No saved tasks available.\n";
        return;
    }
    json tasks = json::parse(file);
    file.close();

    cout << "\nSaved Tasks:\n";
    for (const auto &task : tasks) {
        cout << "Task: " << task["name"] << ", Priority: " << task["priority"]
             << ", Deadline: " << task["deadline"] << "\nDependencies: ";
        for (const auto &dep : task["dependencies"]) {
            cout << dep << " ";
        }
        cout << "\n-------------------\n";
    }
}

// Main Execution
int main() {
    int n;
    cout << "Enter number of tasks: ";
    cin >> n;
    cin.ignore();

    map<string, Task> taskMap;
    map<string, int> inDegree;
    map<string, vector<string>> adjList;
    set<string> taskNames;

    // Load previous tasks from JSON if available
    loadTasksFromJson(taskMap, taskNames, inDegree, adjList);

    for (int i = 0; i < n; i++) {
        Task task;
        cout << "Enter Task Name, Priority, Deadline, Dependencies (comma-separated or 'none'): ";
        string depInput;
        cin >> task.name >> task.priority >> task.deadline;
        cin.ignore();
        getline(cin, depInput);

        taskNames.insert(task.name);

        if (depInput != "none") {
            stringstream ss(depInput);
            string dep;
            while (getline(ss, dep, ',')) {
                dep.erase(remove_if(dep.begin(), dep.end(), ::isspace), dep.end());
                task.dependencies.push_back(dep);
                adjList[dep].push_back(task.name);
                inDegree[task.name]++;
            }
        }

        taskMap[task.name] = task;
        if (inDegree.find(task.name) == inDegree.end()) {
            inDegree[task.name] = 0;
        }
    }

    // Validate dependencies
    for (auto &entry : taskMap) {
        for (const string &dep : entry.second.dependencies) {
            if (taskNames.find(dep) == taskNames.end()) {
                cout << "Error: Task " << entry.first << " depends on non-existent task " << dep << "!\n";
                return 1;
            }
        }
    }

    priority_queue<Task, vector<Task>, Compare> minHeap;
    vector<string> executionOrder;

    for (auto &entry : taskMap) {
        if (inDegree[entry.first] == 0) {
            minHeap.push(entry.second);
        }
    }

    cout << "\nOptimized Task Execution Order:\n";

    while (!minHeap.empty()) {
        Task current = minHeap.top();
        minHeap.pop();
        cout << current.name << " | Priority: " << current.priority << " | Deadline: " << current.deadline << endl;
        executionOrder.push_back(current.name);
        logTaskExecution(current.name);

        for (const string &dependent : adjList[current.name]) {
            inDegree[dependent]--;
            if (inDegree[dependent] == 0) {
                minHeap.push(taskMap[dependent]);
            }
        }
    }

    if (executionOrder.size() != taskMap.size()) {
        cout << "Error: Circular dependency detected! Execution not possible.\n";
        return 1;
    }

    // Save tasks to JSON file
    saveTasksToJson(taskMap);
   
    // Display saved tasks
    displaySavedTasks();

    return 0;
}

