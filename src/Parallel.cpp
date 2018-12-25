/******************************************************************************
    Parallel.cpp is source file of MPI related utility class
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

#include"Parallel.hpp"

#include"mpi.h"

#include<string>
#include<cstdio>
#include<cstdarg>

// idea: <http://blog.jholewinski.org/the-beauty-of-c-templates/index.html>
template<typename T>
struct MPIType
{
   MPI_Datatype mpiType;
   inline operator MPI_Datatype() { return mpiType; }
   inline MPIType();
};
#define CPP_MPI_TYPE(CPPT,MPIT)     \
   template<>                                   \
   MPIType<CPPT>::MPIType() : mpiType(MPIT) {}
CPP_MPI_TYPE(unsigned char,MPI_UNSIGNED_CHAR)
CPP_MPI_TYPE(char,MPI_CHAR)
CPP_MPI_TYPE(unsigned short int,MPI_UNSIGNED_SHORT)
CPP_MPI_TYPE(short int,MPI_SHORT)
CPP_MPI_TYPE(unsigned int,MPI_UNSIGNED)
CPP_MPI_TYPE(int,MPI_INT)
CPP_MPI_TYPE(unsigned long long int,MPI_UNSIGNED_LONG_LONG)
CPP_MPI_TYPE(long long int,MPI_LONG_LONG_INT)
CPP_MPI_TYPE(float,MPI_FLOAT)
CPP_MPI_TYPE(double,MPI_DOUBLE)
#undef CPP_MPI_TYPE

// returns zero for success, non-zero for failure
static int
MpiCall(const int val, bool verbose= true)
{
   bool ret= false;
   switch(val) {
   case MPI_SUCCESS:
      ret= true;
      break;
   case MPI_ERR_COMM:
      printf("Invalid communicator\n");
      break;
   case MPI_ERR_COUNT:
      printf("Invalid count argument\n");
      break;
   case MPI_ERR_TYPE:
      printf("Invalid datatype argument\n");
      break;
   case MPI_ERR_BUFFER:
      printf("Invalid  buffer  pointen\n");
      break;
   case MPI_ERR_ROOT:
      printf("Invalid root\n");
      break;
   default:
      printf("Unknown error\n");
      break;
   }
   return ret;
}

template<typename T>
void
Parallel::broadcast(T *const data, const int count) const
{
   if(MpiCall(MPI_Bcast(data, count, MPIType<T>(), master(), comm()))) {
      // do some destructive thing
   }
}

template<typename T>
void
Parallel::gather(const T *const send,
                 T *const recv,
                 const int count,
                 const int root) const
{
   if(MpiCall(MPI_Gather(send, count, MPIType<T>(),
                         recv, count, MPIType<T>(),
                         root, comm()))) {
      // do some destructive thing
   }
}

template<typename T>
void
Parallel::scatter(const T *const send,
                  T *const recv,
                  const int count,
                  const int root) const
{
   if(MpiCall(MPI_Scatter(send, count, MPIType<T>(),
                          recv, count, MPIType<T>(),
                          root, comm()))) {
      // do some destructive thing
   }
}

template<typename T>
void
Parallel::send(const T *const buf,
               const int dest,
               const int count) const
{
   if(MpiCall(MPI_Send(buf, count, MPIType<T>(),
                       dest, rank(), comm()))) {
      // do some destructive thing
   }
}

template<typename T>
void
Parallel::recv(T *const buf,
               const int source,
               const int count) const
{
   MPI_Status status;
   if(MpiCall(MPI_Recv(buf, count, MPIType<T>(),
                       source, source, comm(), &status))) {
      // do some destructive thing
   }
}

bool
Parallel::openFile(const std::string &fileName,
                   const std::string &mode,
                   FILE **const fp) const
{
   int goodFile= 0;
   if(isMaster()) {
      *fp= fopen(fileName.c_str(), mode.c_str());
      goodFile= 0!= (*fp)? 1: 0;
   }
   broadcast(&goodFile);
   return 0!= goodFile? true: false;
}

bool
Parallel::closeFile(FILE **const fp) const
{
   int goodClose= 0;
   if(isMaster()&& (*fp)) {
      goodClose= fclose(*fp);
      if(0== goodClose) { *fp= 0; }
   }
   broadcast(&goodClose);
   return 0== goodClose? true: false;
}

template<typename T>
int
Parallel::pfscanf(FILE *fp, const char *const format, T *const ptr) const
{
   int ret= 0;
   if(isMaster()) {
      if(NULL!= fp) {
         ret= fscanf(fp, format, ptr);
      } else {
         ret= 0;
      }
   }
   broadcast(&ret);
   if(1!= ret|| NULL== ptr) { goto bye; }
   broadcast(ptr);
bye:
   return ret;
}

template<typename T>
int
Parallel::pfscanfMaster(FILE *fp, const char *const format, T *const ptr) const
{
   int ret= 0;
   if(!isMaster()|| NULL== fp|| NULL== ptr) { goto bye; }
   ret= fscanf(fp, format, ptr);
bye:
   return ret;
}

int
Parallel::pprintfMaster(const char *const format, ...) const
{
   int ret= 0;
   if(!isMaster()) { goto bye; }
   va_list vl;
   va_start(vl,format);
   ret= vprintf(format,vl);
   va_end(vl);

bye:
   return ret;
}

template <typename T>
T
Parallel::getUniformLoad(const T count) const
{
   const register T tSize= static_cast<T>(_size);
   const register T tRank= static_cast<T>(_rank);
   const register T load= (count/ tSize)+ (tRank< count% tSize? 1: 0);
   return load;
}

static int
_getRank(const MPI_Comm comm)
{
   int rank= -1;
   if(MpiCall(MPI_Comm_rank(comm, &rank))) { rank= -1; }
   return rank;
}

static int
_getSize(const MPI_Comm comm)
{
   int size= -1;
   if(MpiCall(MPI_Comm_size(comm, &size))) { size= -1; }
   return size;
}

Parallel::Parallel(const MPI_Comm comm,
                   const int master)
   : _comm(comm), _rank(_getRank(comm)), _size(_getSize(comm)), _master(master)
{
}

Parallel::Parallel(const Parallel &right)
   : _comm(right._comm), _rank(right._rank), _size(right._size),
     _master(right._master)
{
}

Parallel::~Parallel()
{
}

Parallel &
Parallel::operator=(const Parallel &right)
{
   _comm= right._comm;
   _rank= right._rank;
   _size= right._size;
   _master= right._master;
   return *this;
}
