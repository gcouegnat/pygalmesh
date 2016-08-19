#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>

#include <CGAL/Implicit_to_labeling_function_wrapper.h>
#include <CGAL/Labeled_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>

#include <sstream>

using namespace CGAL::parameters;

// Domain
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::FT FT;

template <int Sq_radius>
double sphere_function (double x, double y, double z) // (c=(0,0,0), r=Sq_radius)
{
  double x2=x*x, y2=y*y, z2=z*z;
  return (x2+y2+z2)/Sq_radius - 1;
}

template <typename FT, typename P>
class FT_to_point_function_wrapper : public std::unary_function<P, FT>
{
  typedef FT (*Implicit_function)(FT, FT, FT);
  Implicit_function function;
public:
  typedef P Point;
  explicit FT_to_point_function_wrapper(Implicit_function f) : function(f) {}
  FT operator()(Point p) const { return function(p.x(), p.y(), p.z()); }
};

double torus_function (double x, double y, double z) {
  double x2=x*x, y2=y*y, z2=z*z;
  double x4=x2*x2, y4=y2*y2, z4=z2*z2;

  return x4  + y4  + z4  + 2 *x2*  y2  + 2*
    x2*z2  + 2*y2*  z2  - 5 *x2  + 4* y2  - 5*z2+4;
}

typedef FT_to_point_function_wrapper<K::FT, K::Point_3> Function;
typedef CGAL::Implicit_multi_domain_to_labeling_function_wrapper<Function>
                                                        Function_wrapper;
typedef Function_wrapper::Function_vector Function_vector;
typedef CGAL::Labeled_mesh_domain_3<Function_wrapper, K> Mesh_domain;

// Triangulation
typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;

// Mesh Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;
typedef Mesh_criteria::Facet_criteria    Facet_criteria;
typedef Mesh_criteria::Cell_criteria     Cell_criteria;


void
generate_mesh(const double alpha, const bool lloyd)
{
  std::cout << alpha << std::endl;

  // Define functions
  Function f1(&torus_function);
  Function f2(&sphere_function<3>);

  Function_vector v;
  v.push_back(f1);
  v.push_back(f2);

  std::vector<std::string> vps;
  vps.push_back("+-");

  // Domain (Warning: Sphere_3 constructor uses square radius !)
  Mesh_domain domain(
      Function_wrapper(v, vps),
      K::Sphere_3(CGAL::ORIGIN, 5.*5.),
      1.0e-4
      );

  // Set mesh criteria
  Facet_criteria facet_criteria(30, 0.2, 0.02); // angle, size, approximation
  Cell_criteria cell_criteria(2., 0.4); // radius-edge ratio, size
  Mesh_criteria criteria(facet_criteria, cell_criteria);

  const auto lloyd_param =
    lloyd ? CGAL::parameters::lloyd() : CGAL::parameters::no_lloyd();

  std::cout << "lloyd? " << lloyd << std::endl;

  // Mesh generation
  C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(
      domain,
      criteria,
      lloyd_param
      );
      // CGAL::parameters::odt(),
      // CGAL::parameters::perturb(),
      // CGAL::parameters::exude()

  // // Output
  // std::ofstream medit_file("out.mesh");
  // c3t3.output_to_medit(medit_file);

  return;
}
