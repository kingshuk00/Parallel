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

#include<string>
#include<memory>
#include<cstdio>

class MPIComm;

class Parallel {
public:
   // Information
   inline int rank() const { return _rank; }

   inline int size() const { return _size; }

   inline int master() const { return _master; }

   inline bool isMaster() const { return _master== _rank; }
   inline bool isMaster(const int rank) const { return _master== rank; }

   // Communication

   // broadcast
   template<typename T>
   void broadcast(T *const, const int= 1) const;

   // gather & scatter
   template<typename T>
   void gather(const T *const, T *const, const int= 1, const int= 0) const;
   template<typename T>
   void scatter(const T *const, T *const, const int= 1, const int= 0) const;

   // send & recv
   template<typename T>
   void send(const int dest, const T *const, const int count= 1) const;
   template<typename T>
   void recv(const int source, T *const, const int count= 1) const;

   // file, I/O
   bool openFile(const std::string &,
                 const std::string &,
                 FILE **const) const;
   bool closeFile(FILE **const) const;
   template<typename T>
   int pfscanf(FILE *, const char *const, T *const) const;
   template<typename T>
   int pfscanfMaster(FILE *, const char *const, T *const) const;
   int pprintf(const char *consr, ...) const;
   int pprintfMaster(const char *const, ...) const;

   // calculation
   template <typename T>
   T getUniformLoad(const T) const;

   // Constructors, destructors, assignment operator
   inline Parallel(const int master= 0,
                   const int includeMe= -1);
   inline Parallel(const Parallel &);
   inline ~Parallel();
   inline Parallel &operator=(const Parallel &);

private:
   std::unique_ptr<const MPIComm> _comm;
   int _rank;
   int _size;
   int _master;
};  // Parallel

#endif  // __PARALLEL_PARALLEL HPP_
