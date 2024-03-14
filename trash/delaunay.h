struct Point {
    int x;
    int y;

    Point(): x(0), y(0) {}
    Point(int x, int y): x(x), y(y) {}

    void to_json(std::ofstream& file) {
   file << "{\"x\": " << x << ", \"y\": " << y << "}";
    }
};


class Edge;
class QuadEdge;

//  --------------------------------------------------------
//  The Vert class
//  --------------------------------------------------------

// This extends the SFML class, which, yes, costs some memory for computation
// This is a nominal thing so I don't get a headache; I could rewrite the code to use just int tuples
// (And probably should)
class Vert {
private:
    Edge* edge_;
    int x_;
    int y_;
public:
    Vert(int x, int y): x_(x), y_(y);

    Edge* edge() { return edge_; };
    void  AddEdge(Edge* edge) { edge_ = edge; };
    int x() { return x_; };
    int y() { return y_; };
    int lengthsquared() { return x_ * x_ + y_ * y_; };
};

class Edge {
private:
    int origin_;
    int index_;
    Edge* next_;

    friend QuadEdge;

public:
    Edge(int origin): origin_(origin);
    Edge() {};
    ~Edge() {};

    int index() { return index_; };

    Edge* Rot();
    Edge* InvRot();
    Edge* Sym();
    Edge* Onext() { return next_; };

    Edge* Oprev() { return Rot()->Onext()->Rot(); };
    Edge* Dnext() { return Sym()->Onext()->Sym(); };
    Edge* Dprev() { return InvRot()->Onext()->InvRot(); };
    Edge* Lnext() { return InvRot()->Onext()->Rot(); };
    Edge* Lprev() { return Onext()->Sym(); };
    Edge* Rnext() { return Rot()->Onext()->InvRot(); };
    Edge* Rprev() { return Sym()->Onext(); };

    Vert* origin() { return origin_; };
    Vert* destination() { return Sym()->origin(); };
   
    void setNext(Edge* next) { next_ = next; };
    void setIndex(int index) { index_ = index; };
    void setOrigin(int org) { origin_ = org; };
    void setOrigin(sf::Vector2f org);
    void setDestination(Vert* dest);

    static Edge* Make(std::vector<QuadEdge*>& list);
};

Edge* Edge::Rot()
{
    return (index_ < 3) ? (this + 1) : (this - 3);
};

Edge* Edge::InvRot()
{
    return (index_ > 0) ? (this - 1) : (this + 3);
}   

Edge* Edge::Sym()
{
    return (index_ < 2) ? (this + 2) : (this - 2);
}

void Edge::setOrigin(Vert* origin)
{
    origin->AddEdge(this);
    origin_ = origin;
    draw = true;
}

void Edge::setOrigin(sf::Vector2f origin)
{
    origin_ = new Vert(origin.x, origin.y);
    origin_->AddEdge(this);
    draw = true;
}

void Edge::setDestination(Vert* dest)
{
    Edge* sym = Sym();
    dest->AddEdge(sym);
    sym->setOrigin(dest);
}

void Splice(Edge* a, Edge* b)
{

    Edge* alpha = a->Onext()->Rot();
    Edge* beta = b->Onext()->Rot();

    Edge* t1 = b->Onext();
    Edge* t2 = a->Onext();
    Edge* t3 = beta->Onext();
    Edge* t4 = alpha->Onext();

    a->setNext(t1);
    b->setNext(t2);
    alpha->setNext(t3);
    beta->setNext(t4);
}


class QuadEdge
{
public:
    Edge edges[4];
    QuadEdge();
};

QuadEdge::QuadEdge()
{
    // Make sure the edges know their own indices for memory magic
    edges[0].setIndex(0);
    edges[1].setIndex(1);
    edges[2].setIndex(2);
    edges[3].setIndex(3);

    edges[0].setNext((edges + 0));
    edges[1].setNext((edges + 3));
    edges[2].setNext((edges + 2));
    edges[3].setNext((edges + 1));
}

Edge* Edge::Make(std::vector<QuadEdge*>& list)
{
    list.push_back(new QuadEdge());
    return list.back()->edges;
}






















int Det3x3(int* col_0, int* col_1, int* col_2)
{
    int a = col_0[0];
    int b = col_1[0];
    int c = col_2[0];
    int d = col_0[1];
    int e = col_1[1];
    int f = col_2[1];
    int g = col_0[2];
    int h = col_1[2];
    int i = col_2[2];

    int det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    return det;
}

int Det4x4(int* col_0, int* col_1, int* col_2, int* col_3)
{
    int a = col_0[0];
    int b = col_1[0];
    int c = col_2[0];
    int d = col_3[0];
    int e = col_0[1];
    int f = col_1[1];
    int g = col_2[1];
    int h = col_3[1];
    int i = col_0[2];
    int j = col_1[2];
    int k = col_2[2];
    int l = col_3[2];
    int m = col_0[3];
    int n = col_1[3];
    int o = col_2[3];
    int p = col_3[3];

    int adet = a * ((f * k * p) - (f * l * o) - (g * j * p) + (g * l * n) + (h * j * o) - (h * k * n));
    int bdet = b * ((e * k * p) - (e * l * o) - (g * i * p) + (g * l * m) + (h * i * o) - (h * k * m));
    int cdet = c * ((e * j * p) - (e * l * n) - (f * i * p) + (f * l * m) + (h * i * n) - (h * j * m));
    int ddet = d * ((e * j * o) - (e * k * n) - (f * i * o) + (f * k * m) + (g * i * n) - (g * j * m));

    int det = adet - bdet + cdet - ddet;
    return det;
}

bool InCircle(Vert* a, Vert* b, Vert* c, Vert* d)
{
    int m[4][4] = {  { a->x(), b->x(), c->x(), d->x() },
                        { a->y(), b->y(), c->y(), d->y() },
                        { a->lengthsquared(), b->lengthsquared(), c->lengthsquared(), d->lengthsquared() },
                        { 1, 1, 1, 1 } };

    return Det4x4(m[0], m[1], m[2], m[3]) > 0;
}

bool CCW(Vert* a, Vert* b, Vert* c)
{
    // Returns true if c lies above the line through a and b
    // Bear in mind that this is mirrored when rendering because of SFML conventions
    // This reduces to a linear algebraic question; see Guibas and Stolfi

    float a_x = a->x();
    float a_y = a->y();
    float b_x = b->x();
    float b_y = b->y();
    float c_x = c->x();
    float c_y = c->y();

    int m[3][3] = { { a_x, b_x, c_x }, { a_y, b_y, c_y }, { 1, 1, 1 } };

    return Det3x3(m[0], m[1], m[2]) > 0;
}

bool LeftOf(Edge* e, Vert* z)
{ 
    // Return true if the point is left of the oriented line defined by the edge
    return CCW(z, e->origin(), e->destination()); 
};

bool RightOf(Edge* e, Vert* z)
{ 
    // Return true if the point is right of the oriented line defined by the edge
    return CCW(z, e->destination(), e->origin()); 
};

bool Valid(Edge* e, Edge* base_edge)
{
    // Return false if e ends beneath the base edge, which disqualifies it for candidacy when we zip the hulls
    return RightOf(base_edge, e->destination()); 
};






















typedef std::vector<Edge*>                  EdgeList;
typedef std::vector<Vert*>                  PointsList;
typedef std::vector<QuadEdge*>              QuadList;
typedef std::tuple<EdgeList, EdgeList>      EdgePartition;
typedef std::tuple<PointsList, PointsList>  PointsPartition;

//  --------------------------------------------------------
// The class, creatively named, that will house our methods
//  --------------------------------------------------------

class Delaunay
{
private:
    // Components of the graph
    PointsList                              vertices_;
    QuadList                                edges_;

    // Helper to create a bunch of random vertices
    void                                    GenerateRandomVerts(int n);

    // Helper to cut the array of points in half
    PointsPartition                         SplitPoints(const PointsList& points);

    // Functions that create or remove edges
    Edge*                                   MakeEdgeBetween(int a, int b, const PointsList& points);
    Edge*                                   Connect(Edge* a, Edge* b);
    void                                    Kill(Edge* edge);

    // Functions for generating primitive shapes that we'll merge together
    EdgePartition                           LinePrimitive(const PointsList& points);
    EdgePartition                           TrianglePrimitive(const PointsList& points);

    // Refactored subroutines to make the big algorithm more readable
    Edge*                                   LowestCommonTangent(Edge*& left_inner, Edge*& right_inner);
    Edge*                                   LeftCandidate(Edge* base_edge);
    Edge*                                   RightCandidate(Edge* base_edge);
    void                                    MergeHulls(Edge*& base_edge);

    // The main attraction
    EdgePartition                           Triangulate(const PointsList& points);

public:
    // Constructor
    Delaunay(std::vector<Point>& pts);

    // Triangulate the vertices
    QuadList                                GetTriangulation();
};

//  --------------------------------------------------------
//  Constructor
//  --------------------------------------------------------

Delaunay::Delaunay(int n)
{
    // For the moment, we generate the vertices
    edges_ = QuadList();
    GenerateRandomVerts(n);
}

void Delaunay::GenerateRandomVerts(int n)
{
    // Generate a field of random vertices for debug/demonstration

    srand(time(NULL));

    std::vector<std::vector<int>> buffer;

    // Build a buffer list
    for (int i = 0; i < n; i++)
    {
        std::vector<int> xy = { rand() % 512, rand() % 512 };
        buffer.push_back(xy);
    }

    // Sort it lexicographically; we need this step
    std::sort(buffer.begin(), buffer.end());
    buffer.erase(std::unique(buffer.begin(), buffer.end()), buffer.end());

    // Turn it into Verts for the convenience of our algorithm
    for (int i = 0; i < buffer.size(); i++)
    {
        vertices_.push_back(new Vert(buffer[i][0], buffer[i][1]));
    }
}

//  --------------------------------------------------------
//  Functions for managing the QuadEdges
//  --------------------------------------------------------

void Delaunay::Kill(Edge* edge)
{
    // Fix the local mesh
    Splice(edge, edge->Oprev());
    Splice(edge->Sym(), edge->Sym()->Oprev());

    // Free the quad edge that the edge belongs to
    QuadEdge* raw = (QuadEdge*)(edge - (edge->index()));
    edges_.erase(std::remove(edges_.begin(), edges_.end(), raw));
    delete raw;
}

//  --------------------------------------------------------
//  Helper functions
//  --------------------------------------------------------

// Split the list of vertices in the center
// Thsi relies on the assumption that they're ordered lexicographically
PointsPartition Delaunay::SplitPoints(const PointsList& points)
{
    int halfway = (points.size() / 2);

    PointsList left(points.begin(), points.begin() + halfway);
    PointsList right(points.begin() + halfway, points.end());

    return PointsPartition(left, right);
}

// Creates an edge between the vertices at the given indices
// This is accomplished by creating a new QuadEdge, setting its 0th edge to originate at points[a] and setting its 2nd edge to originate at points[b]
Edge* Delaunay::MakeEdgeBetween(int a, int b, const PointsList& points)
{
    // Create the QuadEdge and return the memory address of its 0th edge
    Edge* e = Edge::Make(edges_);
    
    // Set it to originate from the Vert at index a
    e->setOrigin(points[a]);

    // Set its twin to originate from the Vert at index b
    Vert* points_b = points[b];
    e->setDestination(points_b);

    // Return a pointer to our new edge
    return e;
}

// Connects the ends of two edges to form a coherently oriented triangle
Edge* Delaunay::Connect(Edge* a, Edge* b)
{
    // See Guibas and Stolfi for more

    // Create a new QuadEdge and return the memory address of its 0th edge
    Edge* e = Edge::Make(edges_);

    // Set it to originate at the end point of b
    e->setOrigin(a->destination());

    // Set it to end at the beginning of a, thus giving it a coherent orientation
    e->setDestination(b->origin());

    // Perform splice operations -- I'm still not quite sure why
    Splice(e, a->Lnext());
    Splice(e->Sym(), b);

    // Return a pointer to our new edge
    return e;
}

// Connects two vertices into an edge
EdgePartition Delaunay::LinePrimitive(const PointsList& points)
{
    // Build a line primitive
    // And return it twice?
    Edge* e = MakeEdgeBetween(0, 1, points);
    Edge* e_sym = e->Sym();
    return EdgePartition({ e }, { e_sym });
}

// Connects three vertices into a coherently oriented triangle
EdgePartition Delaunay::TrianglePrimitive(const PointsList& points)
{
    // Build our first two edges here
    Edge* a = MakeEdgeBetween(0, 1, points);
    Edge* b = MakeEdgeBetween(1, 2, points);

    // Do the splice thing; I'm not sure why
    Splice(a->Sym(), b);

    // We want a consistent face orientation, so determine which way we're going here
    if (CCW(points[0], points[1], points[2]))
    {
        Edge* c = Connect(b, a);
        return EdgePartition({ a }, { b->Sym() });
    }
    else if (CCW(points[0], points[2], points[1]))
    {
        Edge* c = Connect(b, a);
        return EdgePartition({ c->Sym() }, { c });
    }
    else
    {
        // The points are collinear
        return EdgePartition({ a }, { b->Sym() });
    }
}

Edge* Delaunay::LowestCommonTangent(Edge*& left_inner, Edge*& right_inner)
{
    // Compute the lower common tangent of the two halves
    // Note the pointer references; we want to keep track of where the new inner edges end up

    // Until we can't do it anymore, take turns rotating along the hulls of the two shapes we're connecting
    while (true)
    {
        if (LeftOf(left_inner, right_inner->origin()))
        {
            left_inner = left_inner->Lnext();
        }
        else if (RightOf(right_inner, left_inner->origin()))
        {
            right_inner = right_inner->Rprev();
        }
        else
        {
            break;
        }
    }

    // Create the base edge once we hit the bottom
    Edge* base_edge = Connect(right_inner->Sym(), left_inner);
    return base_edge;
}

Edge* Delaunay::LeftCandidate(Edge* base_edge)
{
    // Picks out a "candidate" edge from the left half of the domain
    Edge* left_candidate = base_edge->Sym()->Onext();

    if (Valid(left_candidate, base_edge))
    {
        while (InCircle(base_edge->destination(), base_edge->origin(), left_candidate->destination(), left_candidate->Onext()->destination()))
        {
            Edge* t = left_candidate->Onext();
            Kill(left_candidate);
            left_candidate = t;
        }
    }

    return left_candidate;
}

Edge* Delaunay::RightCandidate(Edge* base_edge)
{
    // Picks out a "candidate" edge from the right half of the domain
    Edge* right_candidate = base_edge->Oprev();

    if (Valid(right_candidate, base_edge))
    {
        while (InCircle(base_edge->destination(), base_edge->origin(), right_candidate->destination(), right_candidate->Oprev()->destination()))
        {
            Edge* t = right_candidate->Oprev();
            Kill(right_candidate);
            right_candidate = t;
        }
    }

    return right_candidate;
}

void Delaunay::MergeHulls(Edge*& base_edge)
{
    // Zip up the two halves of the hull once we've found the base edge
    while (true)
    {
        // Get our candidate edges (really becaue we care about their vertices)
        Edge* left_candidate = LeftCandidate(base_edge);
        Edge* right_candidate = RightCandidate(base_edge);

        if (!Valid(left_candidate, base_edge) && !Valid(right_candidate, base_edge))
        {
            // If neither is valid, we have nothing more to do because we've reached the top
            break;
        }
        else if (   !Valid(left_candidate, base_edge) ||
                    InCircle(left_candidate->destination(), left_candidate->origin(), right_candidate->origin(), right_candidate->destination()))
        {
            // Otherwise, if we can rule out the left guy, connect the right edge to the base and set the new base edge
            // This ruling out comes either from creating an invalid hypothetical triangle or from being beneath the base edge
            base_edge = Connect(right_candidate, base_edge->Sym());
        }
        else
        {
            // If we can't do that, then the left edge must be valid and we connect it to the base and set the new base edge
            base_edge = Connect(base_edge->Sym(), left_candidate->Sym());
        }
    }
}

//  --------------------------------------------------------
//  The main attraction
//  --------------------------------------------------------

EdgePartition Delaunay::Triangulate(const PointsList& points)
{
    // Returns the left and right hulls created by triangulating
    // The ultimate value we care about is actually the edges_ member of the Delaunay class
    // This is recursive because divide-and-conquer is a good way to do this
    // See Guibas and Stolfi

    // (Also, we have to assume that the point set we're given is sorted lexicographically)

    /* Terminal cases */
    
    if (points.size() == 2)
    {
        return LinePrimitive(points);
    }
    if (points.size() == 3)
    {
        return TrianglePrimitive(points);
    }

    // Once we survive the terminal-case filter, split up the points
    PointsPartition partition = SplitPoints(points);

    EdgePartition left = Triangulate(std::get<0>(partition));
    EdgePartition right = Triangulate(std::get<1>(partition));

    /* This part of the code is only reachable once we terminate, at which point the vectors are singleton sets */
    
    // Get the inner "inner" edges
    Edge* right_inner = std::get<0>(right)[0];
    Edge* left_inner = std::get<1>(left)[0];

    // Get the initial "outer" edges
    Edge* left_outer = std::get<0>(left)[0];
    Edge* right_outer = std::get<1>(right)[0];

    // Get the lowest common tangent from our initial inner edges
    Edge* base_edge = LowestCommonTangent(left_inner, right_inner);

    // Correct the base edge
    if (left_inner->origin() == left_outer->origin())
    {
        left_outer = base_edge->Sym();
    }
    if (right_inner->origin() == right_outer->origin())
    {
        right_outer = base_edge;
    }

    // Finish the merge operation by "zipping up the gap"
    // i.e. connect things if they don't violate the Delaunay criterion
    MergeHulls(base_edge);

    // Return the outer edges, because they'll be the new inner edges when we do a higher-level merge
    return EdgePartition({ left_outer }, { right_outer });
}

QuadList Delaunay::GetTriangulation()
{
    // Wrapper for the triangulation function
    // This should make it less confusing to call Triangulate with the right vertex list
    EdgePartition tuple = Triangulate(vertices_);
    return edges_;
}