#include <fstream>
#include <string>
#include "map.h"

void json(const Point& pt, std::ofstream& file) {
    file << "[" << pt.x << ", " << pt.y << "]";
}

void json(const Edge& e, std::ofstream& file) {
    file << "[" << e.i << ", " << e.j << "]";
}

void json(const Triangle& t, std::ofstream& file) {
    file << "[";
    json(t.a, file);
    file << ",";
    json(t.b, file);
    file << ",";
    json(t.c, file);
    file << "]";
}

template <typename T>
void json(const std::vector<T>& arr, std::ofstream& file) {
    file << "[";
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        json(arr[i], file);
        file << ",";
    }
    json(arr[arr.size() - 1], file);
    file << "]";
}

void json(const Map& map, std::ofstream& file) {
    file << "{";

    file << "\"width\": " << map.width << ",";
    file << "\"height\": " << map.height << ",";

    file << "\"points\":";
    json<Point>(map.points, file);

    file << ",\"roads\":";
    json<Edge>(map.roads, file);

    file << ",\"bushes\":";
    json<Point>(map.bushes, file);

    file << ",\"trees\":";
    json<Point>(map.trees, file);

    file << ",\"sheeps\":";
    json<Point>(map.sheeps, file);

    file << ",\"fields\":";
    json<Triangle>(map.fields, file);

    file << "}";
}