//-------------------------  filtered_iterator_03.cc  ------------------------
//    $Id$
//    Version: $Name$ 
//
//    Copyright (C) 2001, 2002, 2003, 2004, 2005, 2008, 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//-------------------------  filtered_iterator_03.cc  ------------------------

// check filtered iterators


#include "../tests.h"
#include <base/logstream.h>
#include <grid/tria.h>
#include <grid/grid_generator.h>
#include <grid/tria_iterator.h>
#include <grid/tria_accessor.h>
#include <grid/filtered_iterator.h>

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>


std::ofstream logfile("filtered_iterator_03/output");


DeclException2 (ExcNumberMismatch,
		int, int,
		<< "The numbers " << arg1 << " and " << arg2
		<< " should be equation, but are not.");



typedef Triangulation<2>::active_cell_iterator active_cell_iterator;

template <typename Iterator>
bool always_true (const Iterator)
{
  return true;
}





void test ()
{
  Triangulation<2> tria;
  GridGenerator::hyper_cube(tria, -1, 1);
  tria.refine_global (1);
  tria.begin_active()->set_refine_flag ();
  tria.execute_coarsening_and_refinement ();
  tria.refine_global (2);

				   // we now have a number of cells,
				   // flag them with some subdomain
				   // ids based on their position, in
				   // particular we take the quadrant
				   // (octant)
  active_cell_iterator cell = tria.begin_active (),
		       endc = tria.end ();
  for (; cell!=endc; ++cell)
    {
      unsigned int subdomain = 0;
      for (unsigned int d=0; d<2; ++d)
	if (cell->center()(d) > 0)
	  subdomain |= (1<<d);
      Assert (subdomain < (1<<2), ExcInternalError());

      cell->set_subdomain_id (subdomain);
    };


				   // check 1: count number of cells
				   // on some level
  if (true)
    {
      FilteredIterator<active_cell_iterator>
	begin = make_filtered_iterator(tria.begin_active(), &always_true<active_cell_iterator>),
	end = make_filtered_iterator (static_cast<active_cell_iterator>(tria.end()), &always_true<active_cell_iterator>);
      
      Assert (std::distance (begin, end) ==
	      static_cast<signed int>(tria.n_active_cells ()),
	      ExcInternalError());
      deallog << std::distance(begin,end) << ' '
	      << tria.n_active_cells() << std::endl;
      logfile << "Check 1: "
	      << (std::distance (begin, end) ==
		  static_cast<signed int>(tria.n_active_cells ())
		  ?
		  "OK" : "Failed")
	      << std::endl;
    };
}


int main ()
{
  deallog << std::setprecision(4);
  deallog.attach(logfile);
  deallog.depth_console(0);
  deallog.threshold_double(1.e-10);

  test ();
  
  return 0;
}

