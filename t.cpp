#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;

struct Task {
    string name;
    int priority;
    int deadline;
    vector<string> dependencies;

    // Convert Task to JSON
    json toJson() const {
        return json{{"name", name}, {"priority", priority}, {"deadline", deadline}, {"dependencies", dependencies}};
    }

    // Convert JSON to Task
    static Task fromJson(const json &j) {
        return Task{j.at("name"), j.at("priority"), j.at("deadline"), j.at("dependencies").get<vector<string>>()};
    }
};

// Comparator for priority queue
struct Compare {
    bool operator()(const Task &a, const Task &b) {
        if (a.deadline == b.deadline) {
            return a.priority < b.priority;
        }
        return a.deadline > b.deadline;
    }
};

// Display CLI Menu
void displayMenu() {
    cout << "\nTask Manager Menu:\n";
    cout << "1. Add Task\n";
    cout << "2. View Tasks\n";
    cout << "3. Save Tasks (JSON)\n";
    cout << "4. Load Tasks (JSON)\n";
    cout << "5. Execute Tasks\n";
    cout << "6. Exit\n";
}

// Get Valid Integer Input
int getValidInt(const string &prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a valid integer.\n";
        } else {
            return value;
        }
    }
}

// Add Task
void addTask(map<string, Task> &taskMap, map<string, int> &inDegree, map<string, vector<string>> &adjList) {
    Task task;
    cout << "Enter Task Name: ";
    cin >> task.name;
    
    if (taskMap.find(task.name) != taskMap.end()) {
        cout << "Error: Task name already exists!\n";
        return;
    }

    task.priority = getValidInt("Enter Priority: ");
    task.deadline = getValidInt("Enter Deadline: ");
    
    cout << "Enter Dependencies (comma-separated or 'none'): ";
    string depInput;
    cin.ignore();
    getline(cin, depInput);

    if (depInput != "none") {
        stringstream ss(depInput);
        string dep;
        while (getline(ss, dep, ',')) {
            dep.erase(remove_if(dep.begin(), dep.end(), ::isspace), dep.end());
            if (taskMap.find(dep) == taskMap.end()) {
                cout << "Warning: Dependency '" << dep << "' not found. Skipping.\n";
                continue;
            }
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

// View Tasks
void viewTasks(const map<string, Task> &taskMap) {
    if (taskMap.empty()) {
        cout << "No tasks available.\n";
        return;
    }

    cout << "\nTasks:\n";
    for (const auto &entry : taskMap) {
        cout << entry.first << " | Priority: " << entry.second.priority << " | Deadline: " << entry.second.deadline << endl;
    }
}

// Save Tasks to JSON
void saveTasksToJson(const map<string, Task> &taskMap) {
    if (taskMap.empty()) {
        cout << "No tasks to save.\n";
        return;
    }

    json jArray = json::array();
    for (const auto &entry : taskMap) {
        jArray.push_back(entry.second.toJson());
    }
    cout << "\nSaved JSON:\n" << jArray.dump(4) << endl;
}

// Load Tasks from JSON
void loadTasksFromJson(map<string, Task> &taskMap, map<string, int> &inDegree, map<string, vector<string>> &adjList) {
    cout << "Paste JSON Input: ";
    string jsonInput;
    cin.ignore();
    getline(cin, jsonInput);

    try {
        json jArray = json::parse(jsonInput);
        taskMap.clear();
        inDegree.clear();
        adjList.clear();

        for (const auto &j : jArray) {
            Task newTask = Task::fromJson(j);
            taskMap[newTask.name] = newTask;
            if (newTask.dependencies.empty()) {
                inDegree[newTask.name] = 0;
            }
            for (const string &dep : newTask.dependencies) {
                adjList[dep].push_back(newTask.name);
                inDegree[newTask.name]++;
            }
        }
        cout << "Tasks Loaded Successfully!\n";
    } catch (const json::parse_error &e) {
        cout << "JSON Parsing Error: " << e.what() << endl;
    }
}

// Detect Circular Dependencies using DFS
bool hasCycle(const string &task, map<string, vector<string>> &adjList, set<string> &visited, set<string> &recStack) {
    if (!visited.count(task)) {
        visited.insert(task);
        recStack.insert(task);

        for (const string &neighbor : adjList[task]) {
            if (!visited.count(neighbor) && hasCycle(neighbor, adjList, visited, recStack)) {
                return true;
            } else if (recStack.count(neighbor)) {
                return true;
            }
        }
    }
    recStack.erase(task);
    return false;
}

// Execute Tasks
void executeTasks(map<string, Task> &taskMap, map<string, int> &inDegree, map<string, vector<string>> &adjList) {
    priority_queue<Task, vector<Task>, Compare> minHeap;
    vector<string> executionOrder;

    set<string> visited, recStack;
    for (auto &entry : taskMap) {
        if (hasCycle(entry.first, adjList, visited, recStack)) {
            cout << "Error: Circular dependency detected! Execution not possible.\n";
            return;
        }
    }

    for (auto &entry : taskMap) {
        if (inDegree[entry.first] == 0) {
            minHeap.push(entry.second);
        }
    }

    if (minHeap.empty()) {
        cout << "Error: No executable tasks found.\n";
        return;
    }

    cout << "\nOptimized Task Execution Order:\n";

    while (!minHeap.empty()) {
        Task current = minHeap.top();
        minHeap.pop();

        cout << current.name << " | Priority: " << current.priority << " | Deadline: " << current.deadline << endl;
        executionOrder.push_back(current.name);

        for (const string &dependent : adjList[current.name]) {
            if (--inDegree[dependent] == 0) {
                minHeap.push(taskMap[dependent]);
            }
        }
    }
}

// Main Function
int main() {
    map<string, Task> taskMap;
    map<string, int> inDegree;
    map<string, vector<string>> adjList;

    while (true) {
        displayMenu();
        int choice = getValidInt("Enter your choice: ");

        switch (choice) {
            case 1:
                addTask(taskMap, inDegree, adjList);
                break;
            case 2:
                viewTasks(taskMap);
                break;
            case 3:
                saveTasksToJson(taskMap);
                break;
            case 4:
                loadTasksFromJson(taskMap, inDegree, adjList);
                break;
            case 5:
                executeTasks(taskMap, inDegree, adjList);
                break;
            case 6:
                cout << "Exiting Task Manager...\n";
                return 0;
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    }
}