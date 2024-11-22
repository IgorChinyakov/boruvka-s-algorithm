#include <iostream>
#include <fstream>
#include <vector>
#include <limits>

struct Edge
{
    int from;
    int to;
    int weight;
};

// Функция для чтения графа из бинарного файла
std::vector<Edge> readGraph(const std::string &filename, int &numVertices)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Ошибка при открытии файла.");
    }

    file.read(reinterpret_cast<char *>(&numVertices), sizeof(int16_t));

    std::vector<Edge> edges;
    int16_t from, to, weight;
    while (file.read(reinterpret_cast<char *>(&from), sizeof(int16_t)) &&
           file.read(reinterpret_cast<char *>(&to), sizeof(int16_t)) &&
           file.read(reinterpret_cast<char *>(&weight), sizeof(int16_t)))
    {
        edges.push_back({from, to, weight});
    }

    return edges;
}

// Поиск компоненты для вершины
int findComponent(int vertex, int *component)
{
    while (vertex != component[vertex])
    {
        vertex = component[vertex];
    }
    return vertex;
}

// Объединение компонент
void mergeComponents(int u, int v, int *component)
{
    int rootU = findComponent(u, component);
    int rootV = findComponent(v, component);
    if (rootU != rootV)
    {
        component[rootV] = rootU;
    }
}

// Поиск минимального остовного дерева алгоритмом Борувки
std::pair<int, std::vector<Edge>> boruvkaMST(int numVertices, const std::vector<Edge> &edges)
{
    int *component = new int[numVertices];
    for (int i = 0; i < numVertices; ++i)
    {
        component[i] = i;
    }

    int mstWeight = 0;
    std::vector<Edge> mstEdges;

    while (true)
    {
        std::vector<Edge> minEdges(numVertices, {-1, -1, std::numeric_limits<int>::max()});

// Поиск минимальных рёбер для каждой компоненты
#pragma omp parallel for
        // Поиск минимальных рёбер для каждой компоненты
        for (const auto &edge : edges)
        {
            int compU = findComponent(edge.from, component);
            int compV = findComponent(edge.to, component);
            if (compU != compV)
            {
#pragma omp critical
                if (edge.weight < minEdges[compU].weight)
                {
                    minEdges[compU] = edge;
                }
                if (edge.weight < minEdges[compV].weight)
                {
                    minEdges[compV] = edge;
                }
            }
        }

        bool merged = false;

// Объединение компонент на основе найденных минимальных рёбер
// #pragma omp parallel for reduction(|| : merged)
        for (int i = 0; i < numVertices; ++i)
        {
            Edge edge = minEdges[i];
            if (edge.weight != std::numeric_limits<int>::max())
            {
                int compU = findComponent(edge.from, component);
                int compV = findComponent(edge.to, component);
                if (compU != compV)
                {
// #pragma omp critical
                    {
                        mstWeight += edge.weight;
                        mstEdges.push_back(edge);
                    }
                    mergeComponents(edge.from, edge.to, component);
                    merged = true;
                }
            }
        }

        if (!merged)
        {
            break;
        }
    }

    delete[] component;
    return {mstWeight, mstEdges};
}

// Запись результата в файл
void writeResult(const std::string &filename, int mstWeight, const std::vector<Edge> &mstEdges)
{
    std::ofstream file(filename);
    if (!file)
    {
        throw std::runtime_error("Ошибка при записи в файл.");
    }

    file << "Суммарный вес минимального остовного дерева: " << mstWeight << "\n";
    file << "Рёбра минимального остовного дерева:\n";
    for (const auto &edge : mstEdges)
    {
        file << edge.from << " - " << edge.to << " : " << edge.weight << "\n";
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Использование: " << argv[0] << " <input_file> [-o output_file]\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = "mst_output.txt";
    if (argc > 3 && std::string(argv[2]) == "-o")
    {
        outputFile = argv[3];
    }

    try
    {
        int numVertices;
        auto edges = readGraph(inputFile, numVertices);

        auto [mstWeight, mstEdges] = boruvkaMST(numVertices, edges);

        writeResult(outputFile, mstWeight, mstEdges);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
