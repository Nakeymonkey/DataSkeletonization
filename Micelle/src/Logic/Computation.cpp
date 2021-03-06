#include "Computation.h"
#include "Definitions.h"
#include "Graph.h"
#include "AbstractComplex.h"
#include<list>
#include <boost/math/special_functions/binomial.hpp>
#include "Write.h"

Computation::Computation()
{

}

double Computation::AverageEdgelength(MyGraphType & G)
{
    double sum = 0;
    double ct = 0;
    boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, G);

    for (auto eit = boost::edges(G).first; eit != boost::edges(G).second; eit++)
    {
        sum = sum + weightmap[*eit];
        ct = ct + 1;
    }

    return sum / ct;
}

double Computation::distance(MyGraphType & G, vertex_descriptor v1, vertex_descriptor v2)
{
    return sqrt(CGAL::squared_distance(G[v1].p,G[v2].p));
}

double Computation::unitRandom()
{
    return ((double) rand() / (RAND_MAX));
}

void Computation::treeGraph(MyGraphType &G,std::list<boost::graph_traits<MyGraphType>::edge_descriptor> & edges, MyGraphType & Tree)
{
    auto vpair = vertices(G);
    std::map<vertex_descriptor, vertex_descriptor> correspondance;
    for(auto iter = vpair.first; iter != vpair.second; iter++)
    {
        correspondance[*iter] = Graph::add_vertex(Tree,G[*iter].p);
    }
    for(auto iter=edges.begin();
            iter != edges.end();
            iter++)
    {

        vertex_descriptor first = source(*iter, G);
        vertex_descriptor second = target(*iter, G);
        Graph::add_edge(Tree, correspondance[first],correspondance[second]);
    }
}

void Computation::ComputeDeluanayTriangulation(MyGraphType & G, std::list<Point> & Vector)
{
    Triangulation T(Vector.begin(),Vector.end());
    std::map<Point, MyGraphType::vertex_descriptor> vertex_map;
    Triangulation::Finite_vertices_iterator viter;
    Triangulation::size_type n = T.number_of_vertices();

    for (viter =  T.finite_vertices_begin(); viter != T.finite_vertices_end(); viter++)
    {
        Triangulation::Triangulation_data_structure::Vertex v = *viter;
        Point p = v.point();
        vertex_descriptor pumpum = Graph::add_vertex(G, p);
        vertex_map[v.point()] = pumpum;
    }
    Triangulation::Finite_edges_iterator iter;
    for(iter =  T.finite_edges_begin();
            iter != T.finite_edges_end();
            iter++)
    {
        Triangulation::Triangulation_data_structure::Edge e = *iter;
        Triangulation::Triangulation_data_structure::Cell_handle c = e.first;
        int i = e.second;
        int j = e.third;
        Point pa = c->vertex(i)->point();
        Point pb = c->vertex(j)->point();
        Graph::add_edge(G,vertex_map[pa],vertex_map[pb]);
    }



}

MyGraphType Computation::computeMST( std::list<Point> & Vector)
{
    MyGraphType G;
    Computation::ComputeDeluanayTriangulation(G,Vector);
    std::list<boost::graph_traits<MyGraphType>::edge_descriptor> mst_kruskal;
    boost::kruskal_minimum_spanning_tree(G, std::back_inserter(mst_kruskal));
    MyGraphType savedtree;
    Computation::treeGraph(G, mst_kruskal, savedtree);
    return savedtree;
}

void Computation::MSTSpecialCompute(std::map<Point, MyGraphType::vertex_descriptor> & vertex_map, MyGraphType & savedtree, std::vector<Point> & Vector)
{
    MyGraphType G;
    Triangulation T(Vector.begin(),Vector.end());
    Triangulation::Finite_vertices_iterator viter;
    Triangulation::size_type n = T.number_of_vertices();

    for (viter =  T.finite_vertices_begin(); viter != T.finite_vertices_end(); viter++)
    {
        Triangulation::Triangulation_data_structure::Vertex v = *viter;
        Point p = v.point();
        vertex_descriptor pumpum = Graph::add_vertex(G, p);
        vertex_map[v.point()] = pumpum;
    }
    Triangulation::Finite_edges_iterator iter;
    for(iter =  T.finite_edges_begin();
            iter != T.finite_edges_end();
            iter++)
    {
        Triangulation::Triangulation_data_structure::Edge e = *iter;
        Triangulation::Triangulation_data_structure::Cell_handle c = e.first;
        int i = e.second;
        int j = e.third;
        Point pa = c->vertex(i)->point();
        Point pb = c->vertex(j)->point();
        Graph::add_edge(G,vertex_map[pa],vertex_map[pb]);

    }
    std::list<boost::graph_traits<MyGraphType>::edge_descriptor> mst_kruskal;
    boost::kruskal_minimum_spanning_tree(G, std::back_inserter(mst_kruskal));
    Computation::treeGraph(G, mst_kruskal, savedtree);
}


void Computation::AlphaShapeTest(std::list<Point> & pointcloud)
{
    Alpha_shape_3 as(pointcloud.begin(),pointcloud.end());
    Alpha_iterator opt = as.find_optimal_alpha(1);
    std::cout << "Optimal alpha value to get one connected component is "
              <<  *opt    << std::endl;

}


size_t CustomCoefficent(size_t n, size_t k)
{
    if (k > n)
    {
        return 0;
    }
    else
    {
        return round(boost::math::binomial_coefficient<double>(n, k));
    }
}

size_t Computation::BinomialHash(const Chandler & simplexPair )
{
    size_t sum = 0;
    int dim = simplexPair.size();
    for (int i = 0; i < dim; i++)
    {
        sum = sum + CustomCoefficent(simplexPair[i]-1,dim);
    }

    return sum;
}

size_t Computation::BinomialHash(const ST* st, const ST::Simplex_handle & u)
{
    size_t sum = 0;
// Because the gudhi library was built by non-programmers const_cast method
    ST* bm = const_cast<ST*>(st);
    int k = (*bm).dimension(u) + 1;
    for (ST::Vertex_handle v : (*bm).simplex_vertex_range(u))
    {
        sum = sum + CustomCoefficent(v-1,k);
        k--;
    }
    return sum;
}

void Computation::FiltrationOfAlphaShapes(std::list<Point> & pointcloud, std::string folder)
{
    int firsttime = true;
    double alpha;
    std::list<Point>* iteratingS;
    iteratingS = & pointcloud;
    int j = 0;
    while(((*iteratingS).size() != 0) && (j < 30))
    {
        std::list<Point>* newS = new std::list<Point>();
        Alpha_shape_3 as((*iteratingS).begin(),(*iteratingS).end());
        if (firsttime)
        {
            alpha = *(as.find_optimal_alpha(1));
            firsttime = false;
        }
        std::cout << "Optimal value:" << alpha << std::endl;

        as.set_alpha(alpha);
        std::string temp = folder + "Complex" + std::to_string(j) + ".vtk";
        Write::AlphaVTK(temp, as);
        for (auto v_it = as.finite_vertices_begin(); v_it != as.finite_vertices_end(); v_it++)
        {
            if (as.classify(v_it) != Alpha_shape_3::REGULAR)
            {
                Point p = v_it -> point();
                (*newS).push_back(p);
            }

        }
        iteratingS = newS;
        j++;
    }


}



double Computation::AABBDistance(std::list<std::list<Point>> & paths, std::list<Point> & cloud)
{

    std::list<Segment> segments;
    for (auto at = paths.begin(); at != paths.end(); at++)
    {
        Point* prev;
        prev = NULL;
        for (auto ut = (*at).begin(); ut != (*at).end(); ut++)
        {
            if (prev != NULL)
            {
                Segment l = Segment((*ut),(*prev));
                segments.push_back(l);
            }
            prev = &*ut;
        }
        prev = NULL;

    }
    SegmentTree AABB(segments.begin(), segments.end());
    AABB.accelerate_distance_queries();
    double maxx = 0;
    for (auto it = cloud.begin(); it != cloud.end(); it++)
    {

        Point closest = AABB.closest_point(*it);
        double distance = sqrt(CGAL::squared_distance(closest, *it));
        maxx = std::max(distance,maxx);


    }
    return maxx;
}

double Computation::AABBError(MyGraphType & G, std::list<Point> cloud)
{
    std::list<Segment> segments;
    for (auto eit = boost::edges(G).first; eit != boost::edges(G).second; eit++)
    {
    Point p1 = G[boost::source(*eit,G)].p;
    Point p2 = G[boost::target(*eit,G)].p;
    segments.push_back(Segment(p1,p2));
    }
    double maxdistance = 0;
    SegmentTree AABB(segments.begin(), segments.end());
    AABB.accelerate_distance_queries();
    for (auto it = cloud.begin(); it != cloud.end(); it++)
    {
    Point qq = AABB.closest_point(*it);
    maxdistance = std::max(maxdistance, sqrt(CGAL::squared_distance(*it,qq)));
    }
    return maxdistance;

}

void Computation::BruteNeighborhoodGraph(MyGraphType & G, std::list<Point> pointlist, double epsilon )
{
    for (auto it = pointlist.begin(); it != pointlist.end(); it++)
    {
        Graph::add_vertex(G,*it);
    }
    for (auto it = boost::vertices(G).first; it != boost::vertices(G).second; it++)
    {
        for (auto git = boost::vertices(G).first; git != boost::vertices(G).second; git++)
        {
            if (it != git)
            {
                Point p1 = G[*it].p;
                Point p2 = G[*git].p;
                double distanze = sqrt(CGAL::squared_distance(p1,p2));
                if (distanze < epsilon)
                {
                    Graph::add_edge(G, *it, *git);
                }
            }
        }
    }
}

void Computation::EpsilonSimplification(MyGraphType & G, double epsilon)
{
    boost::property_map<MyGraphType, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, G);
    std::list<edge_descriptor> container;

    for (auto eit = boost::edges(G).first ; eit != boost::edges(G).second ; eit++)
    {
        Point p2 = G[boost::source(*eit, G)].p;
        Point q2 = G[boost::target(*eit, G)].p;
        if (sqrt(CGAL::squared_distance(p2,q2)) > epsilon)
        {
            container.push_back(*eit);
        }
    }
    for (auto it = container.begin(); it != container.end(); it++)
    {
        boost::remove_edge(*it,G);
    }
    std::cout << "Succeful" << std::endl;
}











// branch detection:







