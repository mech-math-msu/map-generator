struct DisjointSet {
    std::vector<int> parent, rank;

    DisjointSet(int n) {
        rank.assign(n, 0);
        for (int i = 0; i < n; i++) {
            parent.push_back(i);
        }
    }

    int find_set(int v) {
        if (parent[v] != v) {
            parent[v] = find_set(parent[v]);
        }
        return parent[v];
    }

    void union_sets(int x, int y) {
        x = find_set(x);
        y = find_set(y);
        if (rank[x] > rank[y]) {
            parent[y] = x;
        }
        else {
            parent[x] = y;
            if (rank[x] == rank[y]) {
                ++rank[y];
            }
        }
    }
};