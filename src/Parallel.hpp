/******************************************************************************
    Parallel.hpp is header of MPI related utility class
    Copyright (C) 2018  Kingshuk
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef __PARALLEL_PARALLEL_HPP_
#define __PARALLEL_PARALLEL_HPP_

#include"mpi.h"

#include<string>

class Parallel {
public:
   // Information
   inline MPI_Comm getComm() const { return _comm; }

   inline int getRank() const { return _rank; }

   inline int getSize() const { return _size; }

   inline int getRoot() const { return _root; }

   inline bool isRoot() const { return _root== _rank; }

   // Utility
   // communication
   template<typename T>
   void broadcast(T *const, const int= 1) const;

   // file
   bool openFileAtRoot(const std::string &,
                       const std::string &,
                       FILE **const) const;
   bool closeFileAtRoot(FILE **const) const;

   // calculation
   template <typename T>
   T getUniformLoad(const T) const;

   // Constructors, destructors, assignment operator
   inline Parallel(const MPI_Comm comm= MPI_COMM_WORLD,
                   const int root= 0);
   inline Parallel(const Parallel &);
   inline ~Parallel();
   inline Parallel &operator=(const Parallel &);

private:
   MPI_Comm _comm;
   int _rank;
   int _size;
   int _root;
};  // Parallel

#endif  // __PARALLEL_PARALLEL HPP_
