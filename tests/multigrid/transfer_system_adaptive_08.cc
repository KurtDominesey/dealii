// ---------------------------------------------------------------------
//
// Copyright (C) 2000 - 2020 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------


// test like _07 but with boundary conditions

#include <deal.II/base/function.h>
#include <deal.II/base/mg_level_object.h>

#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>

#include <deal.II/lac/block_vector.h>
#include <deal.II/lac/vector.h>

#include <deal.II/multigrid/mg_tools.h>
#include <deal.II/multigrid/mg_transfer.h>
#include <deal.II/multigrid/mg_transfer_component.h>

#include <algorithm>

#include "../tests.h"



template <int dim, typename number, int spacedim>
void
reinit_vector(const dealii::DoFHandler<dim, spacedim> &mg_dof,
              MGLevelObject<dealii::Vector<number>> &  v)
{
  for (unsigned int level = v.min_level(); level <= v.max_leve(); ++level)
    {
      unsigned int n = mg_dof.n_dofs(level);
      v[level].reinit(n);
    }
}


template <typename Transfer>
void
make_matrix(const Transfer &    transfer,
            const unsigned int  high_level,
            FullMatrix<double> &matrix)
{
  Vector<double> src(matrix.n());
  Vector<double> dst(matrix.m());
  for (unsigned int i = 0; i < src.size(); ++i)
    {
      src    = 0;
      src(i) = 1;
      transfer.prolongate(high_level, dst, src);
      for (unsigned int j = 0; j < dst.size(); ++j)
        matrix(j, i) = dst(j);
    }
}



void
print_matrix(const FullMatrix<double> &m)
{
  for (unsigned int i = 0; i < m.m(); ++i)
    {
      for (unsigned int j = 0; j < m.n(); ++j)
        deallog << m(i, j) << ' ';
      deallog << std::endl;
    }
}


template <int dim>
void
refine_mesh(Triangulation<dim> &triangulation)
{
  bool cell_refined = false;
  for (typename Triangulation<dim>::active_cell_iterator cell =
         triangulation.begin_active();
       cell != triangulation.end();
       ++cell)
    {
      const Point<dim> p        = cell->center();
      bool             positive = p(0) > 0;
      if (positive)
        {
          cell->set_refine_flag();
          cell_refined = true;
        }
    }
  if (!cell_refined) // if no cell was selected for refinement, refine global
    for (typename Triangulation<dim>::active_cell_iterator cell =
           triangulation.begin_active();
         cell != triangulation.end();
         ++cell)
      cell->set_refine_flag();
  triangulation.execute_coarsening_and_refinement();
}



template <int dim>
void
check(const FiniteElement<dim> &fe)
{
  deallog << fe.get_name() << std::endl;

  Triangulation<dim> tr(Triangulation<dim>::limit_level_difference_at_vertices);

  std::vector<unsigned int> subdivisions(dim, 1);
  subdivisions[0] = 2;

  const Point<dim> bottom_left =
    (dim == 2 ? Point<dim>(-1, -1) : Point<dim>(-1, -1, -1));
  const Point<dim> top_right =
    (dim == 2 ? Point<dim>(1, 1) : Point<dim>(1, 1, 1));
  GridGenerator::subdivided_hyper_rectangle(
    tr, subdivisions, bottom_left, top_right, true);
  refine_mesh(tr);

  DoFHandler<dim> mg_dof_handler(tr);
  mg_dof_handler.distribute_dofs(fe);
  mg_dof_handler.distribute_mg_dofs();

  std::vector<unsigned int> block_selected(3, 0);
  block_selected[2] = 1;

  deallog << "Global  dofs: " << mg_dof_handler.n_dofs() << std::endl;
  for (unsigned int l = 0; l < tr.n_levels(); ++l)
    {
      deallog << "Level " << l << " dofs:";
      deallog << ' ' << mg_dof_handler.n_dofs(l);
      deallog << std::endl;
    }

  DoFRenumbering::component_wise(mg_dof_handler, block_selected);
  for (unsigned int level = 0; level < tr.n_levels(); ++level)
    DoFRenumbering::component_wise(mg_dof_handler, level, block_selected);

  std::vector<std::set<types::global_dof_index>> boundary_indices(
    tr.n_levels());
  std::map<types::boundary_id, const Function<dim> *> dirichlet_boundary;
  Functions::ZeroFunction<dim> dirichlet_bc(fe.n_components());
  dirichlet_boundary[3] = &dirichlet_bc;

  MGTools::make_boundary_list(mg_dof_handler,
                              dirichlet_boundary,
                              boundary_indices);

  MGTransferSelect<double> transfer;
  transfer.build(
    mg_dof_handler, 0, 0, block_selected, block_selected, boundary_indices);

  std::vector<std::vector<types::global_dof_index>> dofs_per_block(
    tr.n_levels(), std::vector<types::global_dof_index>(2));
  MGTools::count_dofs_per_block(mg_dof_handler, dofs_per_block, block_selected);

  FullMatrix<double> prolong_0_1(dofs_per_block[1][0], dofs_per_block[0][0]);

  deallog << "Level 0->1" << std::endl;
  make_matrix(transfer, 1, prolong_0_1);
  print_matrix(prolong_0_1);
}


int
main()
{
  initlog();
  deallog << std::setprecision(4);

  // TODO: do in 1d
  check(FESystem<2>(FE_Q<2>(1), 3));
}
