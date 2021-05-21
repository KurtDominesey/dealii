// ---------------------------------------------------------------------
//
// Copyright (C) 2020 by the deal.II authors
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

#include <deal.II/multigrid/mg_transfer_global_coarsening.templates.h>

DEAL_II_NAMESPACE_OPEN

namespace MGTransferGlobalCoarseningTools
{
  unsigned int
  create_next_polynomial_coarsening_degree(
    const unsigned int                      previous_fe_degree,
    const PolynomialCoarseningSequenceType &p_sequence)
  {
    switch (p_sequence)
      {
        case PolynomialCoarseningSequenceType::bisect:
          return std::max(previous_fe_degree / 2, 1u);
        case PolynomialCoarseningSequenceType::decrease_by_one:
          return std::max(previous_fe_degree - 1, 1u);
        case PolynomialCoarseningSequenceType::go_to_one:
          return 1u;
        default:
          Assert(false, StandardExceptions::ExcNotImplemented());
          return 1u;
      }
  }



  std::vector<unsigned int>
  create_polynomial_coarsening_sequence(
    const unsigned int                      max_degree,
    const PolynomialCoarseningSequenceType &p_sequence)
  {
    std::vector<unsigned int> degrees{max_degree};

    while (degrees.back() > 1)
      degrees.push_back(
        create_next_polynomial_coarsening_degree(degrees.back(), p_sequence));

    std::reverse(degrees.begin(), degrees.end());

    return degrees;
  }
} // namespace MGTransferGlobalCoarseningTools

#include "mg_transfer_global_coarsening.inst"

DEAL_II_NAMESPACE_CLOSE
