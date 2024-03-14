#include <memory>
#include <stdlib.h>
#include <algorithm>
#include <fstream>

struct Point {
    int x;
    int y;

    Point(): x(0), y(0) {}
    Point(int x, int y): x(x), y(y) {}

    void to_json(std::ofstream& file) {
   file << "{\"x\": " << x << ", \"y\": " << y << "}";
    }
};


class QuarterEdge;
class QuadEdge;

class QuarterEdge {
private:
    int origin_;
    int index_;
    QuarterEdge* next_;

    friend QuadEdge;

public:
    QuarterEdge(int origin): origin_(origin) { };
    QuarterEdge() {};
    ~QuarterEdge() {};

    int index() { return index_; };

    QuarterEdge* Rot();
    QuarterEdge* InvRot();
    QuarterEdge* Sym();
    QuarterEdge* Onext() { return next_; };

    QuarterEdge* Oprev() { return Rot()->Onext()->Rot(); };
    QuarterEdge* Dnext() { return Sym()->Onext()->Sym(); };
    QuarterEdge* Dprev() { return InvRot()->Onext()->InvRot(); };
    QuarterEdge* Lnext() { return InvRot()->Onext()->Rot(); };
    QuarterEdge* Lprev() { return Onext()->Sym(); };
    QuarterEdge* Rnext() { return Rot()->Onext()->InvRot(); };
    QuarterEdge* Rprev() { return Sym()->Onext(); };

    int origin() { return origin_; };
    int destination() { return Sym()->origin(); };
   
    void setNext(QuarterEdge* next) { next_ = next; };
    void setIndex(int index) { index_ = index; };
    void setOrigin(int org) { origin_ = org; };
    void setDestination(int dest);

    static QuarterEdge* Make(std::vector<QuadEdge*>& list);
};

QuarterEdge* QuarterEdge::Rot() {
    return (index_ < 3) ? (this + 1) : (this - 3);
};

QuarterEdge* QuarterEdge::InvRot() {
    return (index_ > 0) ? (this - 1) : (this + 3);
}   

QuarterEdge* QuarterEdge::Sym() {
    return (index_ < 2) ? (this + 2) : (this - 2);
}

void QuarterEdge::setDestination(int dest) {
    QuarterEdge* sym = Sym();
    sym->setOrigin(dest);
}

void Splice(QuarterEdge* a, QuarterEdge* b) {

    QuarterEdge* alpha = a->Onext()->Rot();
    QuarterEdge* beta = b->Onext()->Rot();

    QuarterEdge* t1 = b->Onext();
    QuarterEdge* t2 = a->Onext();
    QuarterEdge* t3 = beta->Onext();
    QuarterEdge* t4 = alpha->Onext();

    a->setNext(t1);
    b->setNext(t2);
    alpha->setNext(t3);
    beta->setNext(t4);
}


class QuadEdge {
public:
    QuarterEdge edges[4];
    QuadEdge();

    bool to_json(std::ofstream& file) {
        int i = edges[0].origin(), j = edges[0].Onext()->destination(), k = edges[0].destination();
        //if (edges[0].Onext()->Rot()->destination() != edges[0].InvRot()->destination() && edges[0].InvRot()->destination() != edges[0].Onext()->Rot()->destination()) { return false; }
        file << "[" << i << "," << j << "," << k << "]";
        return true;
    }
};

QuadEdge::QuadEdge() {
    edges[0].setIndex(0);
    edges[1].setIndex(1);
    edges[2].setIndex(2);
    edges[3].setIndex(3);

    edges[0].setNext((edges + 0));
    edges[1].setNext((edges + 3));
    edges[2].setNext((edges + 2));
    edges[3].setNext((edges + 1));
}

QuarterEdge* QuarterEdge::Make(std::vector<QuadEdge*>& list) {
    list.push_back(new QuadEdge());
    return list.back()->edges;
}













typedef std::vector<QuarterEdge*> QuarterEdgeList;
typedef std::vector<QuadEdge*> QuadList;
typedef std::tuple<QuarterEdgeList, QuarterEdgeList> QuarterEdgePartition;

//  --------------------------------------------------------
// The class, creatively named, that will house our methods
//  --------------------------------------------------------

class Delaunay {
private:
    std::vector<Point>& pts;
    QuadList edges_;
    std::ofstream log;

    bool in_circle(int i, int j, int k, int l) {
        int Ax = pts[i].x, Ay = pts[i].y, Ax2Ay2 = Ax * Ax + Ay * Ay;
        int Bx = pts[j].x, By = pts[j].y, Bx2By2 = Bx * Bx + By * By;
        int Cx = pts[k].x, Cy = pts[k].y, Cx2Cy2 = Cx * Cx + Cy * Cy;
        int Px = pts[l].x, Py = pts[l].y, Px2Py2 = Px * Px + Py * Py;

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

        return det > 0;
    }

    bool triangle_orientation(int i, int j, int k) {
        int Ax = pts[i].x, Ay = pts[i].y;
        int Bx = pts[j].x, By = pts[j].y;
        int Cx = pts[k].x, Cy = pts[k].y;

        return Ax * (By - Cy) - Bx * (Ay - Cy) + Cx * (Ay - By) > 0;
    }

    bool LeftOf(QuarterEdge* e, int i) { 
        // Return true if the point is left of the oriented line defined by the edge
        return triangle_orientation(i, e->origin(), e->destination()); 
    };

    bool RightOf(QuarterEdge* e, int i) { 
        // Return true if the point is right of the oriented line defined by the edge
        return triangle_orientation(i, e->destination(), e->origin()); 
    };

    bool Valid(QuarterEdge* e, QuarterEdge* base_edge) {
        // Return false if e ends beneath the base edge, which disqualifies it for candidacy when we zip the hulls
        return RightOf(base_edge, e->destination()); 
    };

    QuarterEdge* MakeQuarterEdgeBetween(int a, int b);
    QuarterEdge* Connect(QuarterEdge* a, QuarterEdge* b);
    void Kill(QuarterEdge* edge);

    QuarterEdgePartition LinePrimitive(int idx);
    QuarterEdgePartition TrianglePrimitive(int idx);

    QuarterEdge* LowestCommonTangent(QuarterEdge*& left_inner, QuarterEdge*& right_inner);
    QuarterEdge* LeftCandidate(QuarterEdge* base_edge);
    QuarterEdge* RightCandidate(QuarterEdge* base_edge);
    void  MergeHulls(QuarterEdge*& base_edge);

    QuarterEdgePartition Triangulate(int s, int e);

public:
    Delaunay(std::vector<Point>& pts): pts(pts), edges_(QuadList()) { log.open("log.txt"); };
    QuadList GetTriangulation();
};

void Delaunay::Kill(QuarterEdge* edge) {
    Splice(edge, edge->Oprev());
    Splice(edge->Sym(), edge->Sym()->Oprev());

    QuadEdge* raw = (QuadEdge*)(edge - (edge->index()));
    edges_.erase(std::remove(edges_.begin(), edges_.end(), raw));
    delete raw;
}

QuarterEdge* Delaunay::MakeQuarterEdgeBetween(int a, int b) {
    QuarterEdge* e = QuarterEdge::Make(edges_);
    
    e->setOrigin(a);

    e->setDestination(b);

    return e;
}

// Connects the ends of two edges to form a coherently oriented triangle
QuarterEdge* Delaunay::Connect(QuarterEdge* a, QuarterEdge* b) {
    QuarterEdge* e = QuarterEdge::Make(edges_);

    e->setOrigin(a->destination());

    e->setDestination(b->origin());

    // Perform splice operations -- I'm still not quite sure why
    Splice(e, a->Lnext());
    Splice(e->Sym(), b);

    return e;
}

QuarterEdgePartition Delaunay::LinePrimitive(int idx) {
    log << "making line\n";
    QuarterEdge* e = MakeQuarterEdgeBetween(idx, idx + 1);
    QuarterEdge* e_sym = e->Sym();
    return QuarterEdgePartition({ e }, { e_sym });
}

// Connects three vertices into a coherently oriented triangle
QuarterEdgePartition Delaunay::TrianglePrimitive(int idx) {
    log << "making triangle\n";
    QuarterEdge* a = MakeQuarterEdgeBetween(idx, idx + 1);
    QuarterEdge* b = MakeQuarterEdgeBetween(idx + 1, idx + 2);

    // Do the splice thing; I'm not sure why
    Splice(a->Sym(), b);

    // We want a consistent face orientation, so determine which way we're going here
    if (triangle_orientation(idx, idx + 1, idx + 2)) {
        QuarterEdge* c = Connect(b, a);
        return QuarterEdgePartition({ a }, { b->Sym() });
    }
    else if (triangle_orientation(idx, idx + 2, idx + 1)) {
        QuarterEdge* c = Connect(b, a);
        return QuarterEdgePartition({ c->Sym() }, { c });
    }
    else {
        // The points are collinear
        return QuarterEdgePartition({ a }, { b->Sym() });
    }
}

QuarterEdge* Delaunay::LowestCommonTangent(QuarterEdge*& left_inner, QuarterEdge*& right_inner) {
    // Compute the lower common tangent of the two halves
    // Note the pointer references; we want to keep track of where the new inner edges end up

    // Until we can't do it anymore, take turns rotating along the hulls of the two shapes we're connecting
    while (true) {
        if (LeftOf(left_inner, right_inner->origin())) {
            left_inner = left_inner->Lnext();
        }
        else if (RightOf(right_inner, left_inner->origin())) {
            right_inner = right_inner->Rprev();
        }
        else {
            break;
        }
    }

    // Create the base edge once we hit the bottom
    QuarterEdge* base_edge = Connect(right_inner->Sym(), left_inner);
    return base_edge;
}

QuarterEdge* Delaunay::LeftCandidate(QuarterEdge* base_edge) {
    // Picks out a "candidate" edge from the left half of the domain
    QuarterEdge* left_candidate = base_edge->Sym()->Onext();

    if (Valid(left_candidate, base_edge)) {
        while (in_circle(base_edge->destination(), base_edge->origin(), left_candidate->destination(), left_candidate->Onext()->destination())) {
            QuarterEdge* t = left_candidate->Onext();
            Kill(left_candidate);
            left_candidate = t;
        }
    }

    return left_candidate;
}

QuarterEdge* Delaunay::RightCandidate(QuarterEdge* base_edge) {
    // Picks out a "candidate" edge from the right half of the domain
    QuarterEdge* right_candidate = base_edge->Oprev();

    if (Valid(right_candidate, base_edge)) {
        while (in_circle(base_edge->destination(), base_edge->origin(), right_candidate->destination(), right_candidate->Oprev()->destination())) {
            QuarterEdge* t = right_candidate->Oprev();
            Kill(right_candidate);
            right_candidate = t;
        }
    }

    return right_candidate;
}

void Delaunay::MergeHulls(QuarterEdge*& base_edge) {
    // Zip up the two halves of the hull once we've found the base edge
    while (true) {
        // Get our candidate edges (really becaue we care about their vertices)
        QuarterEdge* left_candidate = LeftCandidate(base_edge);
        QuarterEdge* right_candidate = RightCandidate(base_edge);

        if (!Valid(left_candidate, base_edge) && !Valid(right_candidate, base_edge)) {
            // If neither is valid, we have nothing more to do because we've reached the top
            break;
        } else if (
            !Valid(left_candidate, base_edge) ||
            in_circle(left_candidate->destination(), left_candidate->origin(), right_candidate->origin(), right_candidate->destination())
        ) {
            // Otherwise, if we can rule out the left guy, connect the right edge to the base and set the new base edge
            // This ruling out comes either from creating an invalid hypothetical triangle or from being beneath the base edge
            base_edge = Connect(right_candidate, base_edge->Sym());
        }
        else {
            // If we can't do that, then the left edge must be valid and we connect it to the base and set the new base edge
            base_edge = Connect(base_edge->Sym(), left_candidate->Sym());
        }
    }
}

//  --------------------------------------------------------
//  The main attraction
//  --------------------------------------------------------

QuarterEdgePartition Delaunay::Triangulate(int s, int e) {
    // Returns the left and right hulls created by triangulating
    // The ultimate value we care about is actually the edges_ member of the Delaunay class
    // This is recursive because divide-and-conquer is a good way to do this
    // See Guibas and Stolfi

    // (Also, we have to assume that the point set we're given is sorted lexicographically)

    /* Terminal cases */
    if (e - s == 1) {
        return LinePrimitive(s);
    }
    if (e - s == 2) {
        return TrianglePrimitive(s);
    }

    // Once we survive the terminal-case filter, split up the points
    int partition = ((e - s) / 2);

    log << "part: " << partition << "\n";

    QuarterEdgePartition left = Triangulate(s, s + partition);
    QuarterEdgePartition right = Triangulate(s + partition + 1, e);

    /* This part of the code is only reachable once we terminate, at which point the vectors are singleton sets */
    
    // Get the inner "inner" edges
    QuarterEdge* right_inner = std::get<0>(right)[0];
    log << "got right inner\n";
    QuarterEdge* left_inner = std::get<1>(left)[0];
    log << "got left inner\n";

    // Get the initial "outer" edges
    QuarterEdge* left_outer = std::get<0>(left)[0];
    QuarterEdge* right_outer = std::get<1>(right)[0];

    log << "got left and right outer\n";

    // Get the lowest common tangent from our initial inner edges
    QuarterEdge* base_edge = LowestCommonTangent(left_inner, right_inner);
    log << "got lowest common tangent\n";

    // Correct the base edge
    if (left_inner->origin() == left_outer->origin()) {
        left_outer = base_edge->Sym();
    }
    if (right_inner->origin() == right_outer->origin()) {
        right_outer = base_edge;
    }

    log << "after ifs\n";

    // Finish the merge operation by "zipping up the gap"
    // i.e. connect things if they don't violate the Delaunay criterion
    MergeHulls(base_edge);

    log << "merged hulls\n";

    // Return the outer edges, because they'll be the new inner edges when we do a higher-level merge
    return QuarterEdgePartition({ left_outer }, { right_outer });
}

QuadList Delaunay::GetTriangulation() {
    // Wrapper for the triangulation function
    // This should make it less confusing to call Triangulate with the right vertex list
    log << pts.size() - 1 << "\n";
    QuarterEdgePartition tuple = Triangulate(0, pts.size() - 1);
    log << "huh\n";
    return edges_;
}