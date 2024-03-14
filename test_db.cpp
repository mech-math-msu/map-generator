#include <iostream>
#include <vector>
#include "db.h"

int main() {
    std::vector<uint16_t> pts = {8, 4, 1024, 128};
    for (auto v : pts) {
        std::cout << v << "\n";
    }
    std::cout << "pts size: " << pts.size() << "\n";
    RoughObjectOptions opts;
    RoughObjectType t = RoughObjectType::LINE;

    DB db;
    std::string fn = "db.db";
    db.connect(fn);
    //db._insert(RoughObjectType::LINE, opts, pts);
    //db._insert(RoughObjectType::LINEAR_PATH, opts, pts);
    db._insert(RoughObjectType::LINE, opts, pts);
    db._insert(RoughObjectType::POLYGON, opts, pts);
    //db._insert(RoughObjectType::POLYGON, opts, pts);

    for (int i = 0; i < 2; ++i) {
        RoughObject obj = db._read(i);
        std::cout << (obj.type == RoughObjectType::LINE) << "\n";
        std::cout << (obj.type == RoughObjectType::POLYGON) << "\n";
    }

    db.disconnect();
}