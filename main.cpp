#include <random>
#include <iostream>
#include "jsonify.h"
#include "db.h"

struct MapOptions {
    int width;
    int height;
    int n_points;
    int n_trees;
    int n_sheeps;
    int n_bushes;
    std::string fn;

    MapOptions(char* argv[], int argc): width(800), height(600), n_points(70), n_trees(10), n_sheeps(20), n_bushes(30), fn("db.json") {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if ((arg == "-w" || arg == "--width") && i + 1 < argc) {
                width = std::stoi(argv[i + 1]);
                i++;
            } else if ((arg == "-h" || arg == "--height") && i + 1 < argc) {
                height = std::stoi(argv[i + 1]);
                i++;
            } else if ((arg == "-s" || arg == "--n-sheeps") && i + 1 < argc) {
                n_sheeps = std::stoi(argv[i + 1]);
                i++;
            } else if ((arg == "-p" || arg == "--n-points") && i + 1 < argc) {
                n_points = std::stoi(argv[i + 1]);
                i++;
            } else if ((arg == "-t" || arg == "--n-trees") && i + 1 < argc) {
                n_trees = std::stoi(argv[i + 1]);
                i++;
            } else if ((arg == "-b" || arg == "--n-bushes") && i + 1 < argc) {
                n_bushes = std::stoi(argv[i + 1]);
                i++;
            } else if ((arg == "-f" || arg == "--filename") && i + 1 < argc) {
                fn = argv[i + 1];
                i++;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    MapOptions opts_(argv, argc);

    std::ofstream file;
    file.open(opts_.fn);

    Map map(opts_.width, opts_.height, opts_.n_points, opts_.n_bushes, opts_.n_trees, opts_.n_sheeps);
    json(map, file);

    file.close();

    RoughObjectOptions opts;

    DB db;
    std::string fn = "db.db";
    db.connect(fn);

    std::vector<uint16_t> pts;

    for (size_t i = 0; i < map.roads.size(); ++i) {
        pts.clear();
        pts.push_back(map.points[map.roads[i].i].x);
        pts.push_back(map.points[map.roads[i].i].y);
        pts.push_back(map.points[map.roads[i].j].x);
        pts.push_back(map.points[map.roads[i].j].y);

        db._insert(RoughObjectType::LINE, opts, pts);
    }

    for (size_t i = 0; i < map.fields.size(); ++i) {
        pts.clear();
        pts.push_back(map.fields[i].a.x);
        pts.push_back(map.fields[i].a.y);
        pts.push_back(map.fields[i].b.x);
        pts.push_back(map.fields[i].b.y);
        pts.push_back(map.fields[i].c.x);
        pts.push_back(map.fields[i].c.y);

        db._insert(RoughObjectType::POLYGON, opts, pts);
    }

    for (size_t i = 0; i < map.trees.size(); ++i) {
        pts.clear();
        pts.push_back(map.trees[i].x);
        pts.push_back(map.trees[i].y);

        db._insert(RoughObjectType::CIRCLE, opts, pts);
    }

    for (size_t i = 0; i < map.bushes.size(); ++i) {
        pts.clear();
        pts.push_back(map.bushes[i].x);
        pts.push_back(map.bushes[i].y);

        db._insert(RoughObjectType::CIRCLE, opts, pts);
    }

    for (size_t i = 0; i < map.sheeps.size(); ++i) {
        pts.clear();
        pts.push_back(map.sheeps[i].x);
        pts.push_back(map.sheeps[i].y);

        db._insert(RoughObjectType::CIRCLE, opts, pts);
    }

    // std::mt19937 eng;
    // std::random_device r;
    // std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
    // eng.seed(seed);

    // int N = 1'00;

    // std::uniform_int_distribution<> w_dist(200, 800);
    // std::uniform_int_distribution<> h_dist(200, 500);
    // std::uniform_int_distribution<> p_dist(10, 100);
    // std::uniform_int_distribution<> s_dist(10, 100);
    // std::uniform_int_distribution<> f_dist(10, 100);
    // std::uniform_int_distribution<> b_dist(10, 100);

    // file.open("deaths.json");

    // file << "{";
    // int w, h, p, f, s, b;
    // for (int i = 0; i < N; ++i) {
    //     w = w_dist(eng);
    //     h = h_dist(eng);
    //     p = p_dist(eng);
    //     s = s_dist(eng);
    //     f = f_dist(eng);
    //     b = b_dist(eng);
    //     file << "\"map" << i << "\":";
    //     Map(w, h, p, b, f, s).json(file);
    //     file << ",\n";
    // }
    // file << "\"map" << N << "\":";
    // Map(w, h, p, b, f, s).json(file);
    // file << "}";

}

// good maps: 44
