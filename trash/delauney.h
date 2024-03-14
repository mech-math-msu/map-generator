#include <iostream>

// Define a Point struct
struct Point {
    int x;
    int y;
};

// Function to compute the determinant of the given matrix
int det4x4(const Point& A, const Point& B, const Point& C, const Point& P) {
    int Ax = A.x, Ay = A.y, Ax2Ay2 = Ax * Ax + Ay * Ay;
    int Bx = B.x, By = B.y, Bx2By2 = Bx * Bx + By * By;
    int Cx = C.x, Cy = C.y, Cx2Cy2 = Cx * Cx + Cy * Cy;
    int Px = P.x, Py = P.y, Px2Py2 = Px * Px + Py * Py;

    int det = 
        Ax * (
            By * (Cx2Cy2 - Px2Py2) -
            Cy * (Bx2By2 - Px2Py2) +
            Py * (Bx2By2 - Cx2Cy2)
        ) -
        Bx * (
            Ay * (Cx2Cy2 - Px2Py2) -
            Cy * (Ax2Ay2 - Px2Py2) +
            Py * (Ax2Ay2 - Cx2Cy2)
        ) +
        Cx * (
            Ay * (Bx2By2 - Px2Py2) -
            By * (Ax2Ay2 - Px2Py2) +
            Py * (Ax2Ay2 - Bx2By2)
        ) -
        Px * (
            Ay * (Bx2By2 - Cx2Cy2) -
            By * (Ax2Ay2 - Cx2Cy2) +
            Cy * (Ax2Ay2 - Bx2By2)
        );

    return det;
}

bool check_pt_inside_circle(const Point& A, const Point& B, const Point& C, const Point& P) {
    return det4x4(A, B, C, P) > 0;
}

"""

Ax  Ay  1
Bx  By  1
Cx  Cy  1
"""

bool triangle_sign(const Point& A, const Point& B, const Point& C) {
    int Ax = A.x, Ay = A.y;
    int Bx = B.x, By = B.y;
    int Cx = C.x, Cy = C.y;

    return Ax * (By - Cy) - Bx * (Ay - Cy) + Cx * (Ay - By) > 0;
}

// struct QuadEdge {
//     QuarterEdge ref[4];
// };

// struct QuarterEdge {
//     QuadEdge* cur;
//     int cur_idx;
//     QuadEdge* next;
//     int next_idx;

//     QuarterEdge* rot() {
//         return cur->ref[(cur_idx + 1) % 4];
//     }

//     QuarterEdge* sym() {
//         return cur->ref[(cur_idx + 2) % 4];
//     }

//     QuarterEdge* tor() {
//         return cur->ref[(cur_idx + 3) % 4];
//     }

//     QuarterEdge* next() {
//         return next->refs[next_idx];
//     }
// };

struct QuarterEdge {
    Point* data;
    QuarterEdge* next;
    QuarterEdge* rot;

    QuarterEdge(const Point& a, const Point& b) {
        QuarterEdge* ab;
        QuarterEdge* ab_rot90;
        QuarterEdge* ba;
        QuarterEdge* ba_rot90;

        ab.data = a;
        ba.data = b;

        ab.rot = ab_rot90;
        ab_rot90.rot = ba;
        ba.rot = ba_rot90;
        ba_rot90.rot = ab;

        // normal edges are on different vertices,
        // and initially they are the only edges out
        // of each vertex
        ab.next = ab;
        ba.next = ba;

        // but dual edges share the same face,
        // so they point to one another
        ab_rot90.next = ba_rot90;
        ba_rot90.next = ab_rot90;
    }

    QuarterEdge& rot() {
        return *rot;
    }

    QuarterEdge& sym() {
        return *(rot->rot);
    }

    QuarterEdge& tor() {
        return *(rot->rot->rot);
    }

    QuarterEdge& prev() {
        return *(rot->next->rot);
    }
    
    QuarterEdge& lnext() {
        return *(tor->next->rot);
    }

    Point* dest() {
        return sym().data;
    }
};

void swap_nexts(const QuarterEdge& a, const QuarterEdge& b) {
    QuarterEdge* anext = a.next;
    a.next = b.next;
    b.next = anext;
}

void splice(const QuarterEdge& a, const QuarterEdge& b) {
    swap_nexts(a.next.rot, b.next.rot);
    swap_nexts(a, b);
}

QuarterEdge makeTriangle(const Point& a, const Point& b, const Point& c) {
    QuarterEdge ab(a, b);
    QuarterEdge bc(b, c);
    QuarterEdge ca(c, a);

    splice(ab.sym(), bc);
    splice(bc.sym(), ca);
    splice(ca.sym(), ab);

    return ab;
}

QuarterEdge connect(const QuarterEdge& a, const QuarterEdge& b) {
    QuarterEdge new_edge(a.dest(), b.data());
    splice(new_edge, a.lnext());
    splice(new_edge.sym(), b);
    return new_edge;
}

void sever(const QuarterEdge& edge) {
    splice(edge, edge.prev());
    splice(edge.sym(), edge.sym().prev());
}

void flip(const QuarterEdge& edge) {
    QuarterEdge a = edge.prev();
    QuarterEdge b = edge.sym().prev();
    splice(edge, a);
    splice(edge.sym(), b);
    splice(edge, a.lnext());
    splice(edge.sym(), b.lnext());
    edge.data = a.dest();
    edge.dest() = b.dest();
}

int main() {
    // Sample Point instances
    Point A = {1, 2};
    Point B = {3, 4};
    Point C = {5, 6};
    Point P = {7, 8};

    // Compute the determinant
    double determinant = computeDeterminant(A, B, C, P);

    // Display the result
    std::cout << "Determinant: " << determinant << std::endl;

    return 0;
}

