#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <memory>

using namespace std;

// Struktura dla węzłów na siatce
struct Node {
    int x, y;
    float gCost, hCost, fCost;
    shared_ptr<Node> parent;

    Node(int x, int y, float gCost = 0, float hCost = 0, shared_ptr<Node> parent = nullptr)
        : x(x), y(y), gCost(gCost), hCost(hCost), fCost(gCost + hCost), parent(parent) {
    }
};

// Funkcja heurystyczna (odległość euklidesowa)
float heuristic(int x1, int y1, int x2, int y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

// Funkcja do wczytania gridu z pliku
vector<vector<int>> loadGrid(const string& filename) {
    vector<vector<int>> grid;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Nie można otworzyć pliku: " << filename << endl;
        exit(1);
    }
    string line;
    while (getline(file, line)) {
        vector<int> row;
        stringstream ss(line);
        int value;
        while (ss >> value) {
            row.push_back(value);
        }
        grid.push_back(row);
    }
    file.close();
    return grid;
}

// Funkcja do zapisywania gridu do pliku
void saveGridToFile(const vector<vector<int>>& grid, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Nie można otworzyć pliku do zapisu: " << filename << endl;
        exit(1);
    }
    for (const auto& row : grid) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) file << " ";
        }
        file << endl;
    }
    file.close();
    cout << "Grid zapisany do pliku: " << filename << endl;
}

// Algorytm A* z wizualizacją w SFML
void aStarWithVisualization(vector<vector<int>>& grid, int startX, int startY, int endX, int endY) {
    int rows = grid.size();
    int cols = grid[0].size();

    auto compare = [](shared_ptr<Node> a, shared_ptr<Node> b) { return a->fCost > b->fCost; };
    priority_queue<shared_ptr<Node>, vector<shared_ptr<Node>>, decltype(compare)> openList(compare);
    unordered_map<int, shared_ptr<Node>> openMap; // Mapa pozycji -> wskaźnik na węzeł
    vector<vector<bool>> closedList(rows, vector<bool>(cols, false));

    auto startNode = make_shared<Node>(startX, startY, 0, heuristic(startX, startY, endX, endY));
    openList.push(startNode);
    openMap[startX * cols + startY] = startNode;

    vector<pair<int, int>> directions = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} }; // Kierunki (góra, dół, lewo, prawo)

    const int tileSize = 20;
    sf::RenderWindow window(sf::VideoMode(cols * tileSize, rows * tileSize), "A* Visualization");

    while (!openList.empty()) {
        if (!window.isOpen())
            return;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
        }

        auto current = openList.top();
        openList.pop();
        openMap.erase(current->x * cols + current->y);

        if (current->x == endX && current->y == endY) {
            auto node = current;
            while (node != nullptr) {
                grid[node->x][node->y] = 3; // Znak trasy
                node = node->parent;
            }
            break;
        }

        closedList[current->x][current->y] = true;

        for (const auto& direction : directions) {
            int newX = current->x + direction.first;
            int newY = current->y + direction.second;

            if (newX < 0 || newX >= rows || newY < 0 || newY >= cols || grid[newX][newY] == 5 || closedList[newX][newY]) {
                continue;
            }

            float gCost = current->gCost + 1;
            float hCost = heuristic(newX, newY, endX, endY);

            int newKey = newX * cols + newY;
            if (openMap.find(newKey) != openMap.end()) {
                auto existingNode = openMap[newKey];
                if (existingNode->fCost > gCost + hCost) {
                    existingNode->gCost = gCost;
                    existingNode->hCost = hCost;
                    existingNode->fCost = gCost + hCost;
                    existingNode->parent = current;
                }
            }
            else {
                auto newNode = make_shared<Node>(newX, newY, gCost, hCost, current);
                openList.push(newNode);
                openMap[newKey] = newNode;
            }
        }

        window.clear();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                sf::RectangleShape cell(sf::Vector2f(tileSize - 1, tileSize - 1));
                cell.setPosition(j * tileSize, i * tileSize);

                if (grid[i][j] == 5)
                    cell.setFillColor(sf::Color::Black);
                else if (grid[i][j] == 3)
                    cell.setFillColor(sf::Color::Blue);
                else if (closedList[i][j])
                    cell.setFillColor(sf::Color::Red);
                else
                    cell.setFillColor(sf::Color::White);

                window.draw(cell);
            }
        }

        for (const auto& node : openMap) {
            sf::RectangleShape openCell(sf::Vector2f(tileSize - 1, tileSize - 1));
            openCell.setPosition((node.second->y) * tileSize, (node.second->x) * tileSize);
            openCell.setFillColor(sf::Color::Green);
            window.draw(openCell);
        }

        window.display();
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                sf::RectangleShape cell(sf::Vector2f(tileSize - 1, tileSize - 1));
                cell.setPosition(j * tileSize, i * tileSize);

                if (grid[i][j] == 5)
                    cell.setFillColor(sf::Color::Black);
                else if (grid[i][j] == 3)
                    cell.setFillColor(sf::Color::Blue);
                else if (closedList[i][j])
                    cell.setFillColor(sf::Color::Red);
                else
                    cell.setFillColor(sf::Color::White);

                window.draw(cell);
            }
        }

        window.display();
    }
}

int main() {
    string inputFilename = "grid.txt";
    string outputFilename = "result.txt";

    vector<vector<int>> grid = loadGrid(inputFilename);

    int startX = 0, startY = 0;
    int endX = grid.size() - 1, endY = grid[0].size() - 1;

    aStarWithVisualization(grid, startX, startY, endX, endY);

    saveGridToFile(grid, outputFilename);

    return 0;
}
