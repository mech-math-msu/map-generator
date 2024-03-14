#include <random>
#include <vector>
#include <map>
#include "map_style.h"
#include "disjointset.h"
#include "delaunator.h"

struct Point {
    int x;
    int y;

    Point(): x(0), y(0) { }
    Point(int x, int y): x(x), y(y) { }
};

int dst(Point& a, Point& b) {
    return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}

struct Edge {
    int i;
    int j;

    Edge(): i(0), j(0) { }
    Edge(int i, int j): i(i), j(j) { }
};

struct Triangle {
    Point a, b, c;

    Triangle(): a(0, 0), b(0, 0), c(0, 0) { }
    Triangle(const Point& a, const Point& b, const Point& c): a(a), b(b), c(c) { }
};

struct Map {
    int width;
    int height;
    std::vector<Point> points;
    std::vector<Point> bushes;
    std::vector<Edge> roads;
    std::vector<Point> trees;
    std::vector<Point> sheeps;
    std::vector<Triangle> fields;

    void fill(std::vector<Point>& pts, int n, std::uniform_int_distribution<>& x_dist, std::uniform_int_distribution<>& y_dist, std::mt19937& eng) {
        for(int i = 0; i < n; i++) {
            int x = x_dist(eng);
            int y = y_dist(eng);
            pts.push_back(Point(x, y));
        }
    }

    Map(int width, int height, int n_points, int n_bushes, int n_trees, int n_sheeps):
        width(width),
        height(height) {
        std::mt19937 eng;

        std::random_device r;
        std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
        eng.seed(seed);

        std::uniform_int_distribution<> x_dist(0, width);
        std::uniform_int_distribution<> y_dist(0, height);

        fill(points, n_points, x_dist, y_dist, eng);
        fill(bushes, n_bushes, x_dist, y_dist, eng);
        fill(trees, n_trees, x_dist, y_dist, eng);
        fill(sheeps, n_sheeps, x_dist, y_dist, eng);

        kruskal();
        delaunay();
    }

    void kruskal() {
        DisjointSet s(points.size());

        std::map<int, Edge> E;
        int d;
        for (size_t i = 0; i < points.size(); ++i) {
            for (size_t j = i + 1; j < points.size(); ++j) {
                d = dst(points[i], points[j]);

                E[d] = Edge(i, j);
            }
        }

        for (auto p : E) {
            Edge edge = p.second;
            if (s.find_set(edge.i) != s.find_set(edge.j)) {
                roads.push_back(edge);
                s.union_sets(edge.i, edge.j);
            }
        }
    }

    void delaunay() {
        std::vector<double> coords;

        for (size_t i = 0; i < points.size(); ++i) {
            coords.push_back(points[i].x);
            coords.push_back(points[i].y);
        }


        delaunator::Delaunator d(coords);

        Point a, b, c;
        for (size_t i = 0; i < d.triangles.size(); i += 3) {
            a = Point(d.coords[2 * d.triangles[i]], d.coords[2 * d.triangles[i] + 1]);
            b = Point(d.coords[2 * d.triangles[i + 1]], d.coords[2 * d.triangles[i + 1] + 1]);
            c = Point(d.coords[2 * d.triangles[i + 2]], d.coords[2 * d.triangles[i + 2] + 1]);
            fields.push_back(Triangle(a, b, c));
        }
    }
};