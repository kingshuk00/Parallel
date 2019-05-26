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
#include<new>
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
#define CPP_MPI_TYPE(CPPT,MPIT)                 \
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
_MpiCall(const int val, bool verbose= true)
{
   int ret= 1;
   switch(val) {
   case MPI_SUCCESS:
      ret= 0;
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

// idea: <https://computing.llnl.gov/tutorials/mpi/#Group_Management_Routines>
class MPIComm {
public:
   MPIComm(const int includeMe)
      : _comm(MPI_COMM_NULL)
   {
      MPI_Comm world= MPI_COMM_WORLD;
      if(-1== includeMe) {
         _comm= world;
         return;
      }
      int size;
      _MpiCall(MPI_Comm_size(world, &size));
      int *included= new int[size];
      _MpiCall(MPI_Allgather(&includeMe, 1, MPI_INT,
                             &included, 1, MPI_INT,
                             world));
      if(0!= includeMe) {
         int count= 0;
         for(int i= 0; i< size; ++i) {
            if(0!= included[i]) { included[count++]= i; }
         }
         MPI_Group  worldGroup, newGroup;
         _MpiCall(MPI_Comm_group(world, &worldGroup));
         _MpiCall(MPI_Group_incl(worldGroup, count, included, &newGroup));
         _MpiCall(MPI_Comm_create(world, newGroup, &_comm));
      }
      delete[] included; included= 0;
   }

   inline MPIComm()
      : _comm(MPI_COMM_WORLD)
   {
   }

   inline MPIComm(const MPIComm &right)
      : _comm(MPI_COMM_NULL)
   {
      if(MPI_COMM_WORLD== right._comm) { _comm= MPI_COMM_WORLD; }
      else { _MpiCall(MPI_Comm_dup(right._comm, &_comm)); }
   }

   inline MPIComm &operator=(const MPIComm &right)
   {
      if(MPI_COMM_WORLD== right._comm) { _comm= MPI_COMM_WORLD; }
      else { _MpiCall(MPI_Comm_dup(right._comm, &_comm)); }
      return *this;
   }

   inline operator MPI_Comm() const { return _comm; }

   ~MPIComm()
   {
      if(MPI_COMM_WORLD!= _comm) { MPI_Comm_free(&_comm); }
   }

private:
   MPI_Comm _comm;
};  // class MPIComm;

template<typename T>
void
Parallel::broadcast(T *const data, const int count) const
{
   if(_MpiCall(MPI_Bcast(data, count, MPIType<T>(), master(), *_comm))) {
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
   if(_MpiCall(MPI_Gather(send, count, MPIType<T>(),
                         recv, count, MPIType<T>(),
                         root, *_comm))) {
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
   if(_MpiCall(MPI_Scatter(send, count, MPIType<T>(),
                          recv, count, MPIType<T>(),
                          root, *_comm))) {
      // do some destructive thing
   }
}

template<typename T>
void
Parallel::send(const int dest,
               const T *const buf,
               const int count) const
{
   if(_MpiCall(MPI_Send(buf, count, MPIType<T>(),
                       dest, rank(), *_comm))) {
      // do some destructive thing
   }
}

template<typename T>
void
Parallel::recv(const int source,
               T *const buf,
               const int count) const
{
   MPI_Status status;
   if(_MpiCall(MPI_Recv(buf, count, MPIType<T>(),
                       source, source, *_comm, &status))) {
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
Parallel::pscanf(const char *const format, T *const ptr) const
{
   int ret= 0;
   if(isMaster()) {
      ret= scanf(format, ptr);
   }
   broadcast(&ret);
   if(1!= ret|| NULL== ptr) { goto bye; }
   broadcast(ptr);
bye:
   return ret;
}

template<typename T>
int
Parallel::pfscanf(FILE *fp, const char *const format, T *const ptr) const
{
   int ret= 0;
   if(isMaster()) {
      if(NULL!= fp) { ret= fscanf(fp, format, ptr); }
      else { ret= 0; }
   }
   broadcast(&ret);
   if(1!= ret|| NULL== ptr) { goto bye; }
   broadcast(ptr);
bye:
   return ret;
}

int
Parallel::pfscanf(FILE *fp, const char *const format) const
{
   return pfscanf<int>(fp, format, 0);
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
   ret= vprintf(format, vl);
   va_end(vl);

bye:
   return ret;
}

int
Parallel::pprintf(const char *const format, ...) const
{
   char buf[2048]= { '\0' };
   va_list vl;
   va_start(vl,format);
   int ret= vsnprintf(buf, 2048, format, vl);
   va_end(vl);
   ret= std::min(ret, 2047);
   if(isMaster()) {
      for(int ip= 0; ip< size(); ++ip) {
         if(!isMaster(ip)) {
            int count= 0;
            recv(ip, &count);
            recv(ip, &buf[0], count);
            buf[count]= '\0';
         }
         printf("(%d): %s\n", ip, buf);
      }
   } else {
      send(master(), &ret);
      send(master(), &buf[0], ret);
   }
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
   if(_MpiCall(MPI_Comm_rank(comm, &rank))) { rank= -1; }
   return rank;
}

static int
_getSize(const MPI_Comm comm)
{
   int size= -1;
   if(_MpiCall(MPI_Comm_size(comm, &size))) { size= -1; }
   return size;
}

Parallel::Parallel(const int master,
                   const int includeMe)
   : _comm(new MPIComm(includeMe)), _rank(_getRank(*_comm)),
     _size(_getSize(*_comm)), _master(master)
{
}

Parallel::Parallel(const Parallel &right)
   : _comm(new MPIComm(*(right._comm))), _rank(right._rank), _size(right._size),
     _master(right._master)
{
}

Parallel::~Parallel()
{
}

Parallel &
Parallel::operator=(const Parallel &right)
{
   _comm.reset(new MPIComm(*(right._comm)));
   _rank= right._rank;
   _size= right._size;
   _master= right._master;
   return *this;
}


#define CPP_BROADCAST(T)                                        \
   template void Parallel::broadcast<T>(T *const, const int) const
CPP_BROADCAST(unsigned char);
CPP_BROADCAST(char);
CPP_BROADCAST(unsigned short int);
CPP_BROADCAST(short int);
CPP_BROADCAST(unsigned int);
CPP_BROADCAST(int);
CPP_BROADCAST(unsigned long long int);
CPP_BROADCAST(long long int);
CPP_BROADCAST(float);
CPP_BROADCAST(double);

#define CPP_GATHER(T)                                                   \
   template void Parallel::gather<T>(const T*const, T *const, const int, const int) const
CPP_GATHER(unsigned char);
CPP_GATHER(char);
CPP_GATHER(unsigned short int);
CPP_GATHER(short int);
CPP_GATHER(unsigned int);
CPP_GATHER(int);
CPP_GATHER(unsigned long long int);
CPP_GATHER(long long int);
CPP_GATHER(float);
CPP_GATHER(double);

#define CPP_SCATTER(T)                          \
   template void Parallel::scatter<T>(const T *const, T *const, const int, const int) const
CPP_SCATTER(unsigned char);
CPP_SCATTER(char);
CPP_SCATTER(unsigned short int);
CPP_SCATTER(short int);
CPP_SCATTER(unsigned int);
CPP_SCATTER(int);
CPP_SCATTER(unsigned long long int);
CPP_SCATTER(long long int);
CPP_SCATTER(float);
CPP_SCATTER(double);

#define CPP_SEND(T)                                                     \
   template void Parallel::send(const int, const T *const, const int) const
CPP_SEND(unsigned char);
CPP_SEND(char);
CPP_SEND(unsigned short int);
CPP_SEND(short int);
CPP_SEND(unsigned int);
CPP_SEND(int);
CPP_SEND(unsigned long long int);
CPP_SEND(long long int);
CPP_SEND(float);
CPP_SEND(double);

#define CPP_RECV(T)                                                 \
   template void Parallel::recv(const int, T *const, const int) const
CPP_RECV(unsigned char);
CPP_RECV(char);
CPP_RECV(unsigned short int);
CPP_RECV(short int);
CPP_RECV(unsigned int);
CPP_RECV(int);
CPP_RECV(unsigned long long int);
CPP_RECV(long long int);
CPP_RECV(float);
CPP_RECV(double);

#define CPP_PSCANF(T)                                           \
   template int Parallel::pscanf(const char *const, T *const) const
CPP_PSCANF(unsigned char);
CPP_PSCANF(char);
CPP_PSCANF(unsigned short int);
CPP_PSCANF(short int);
CPP_PSCANF(unsigned int);
CPP_PSCANF(int);
CPP_PSCANF(unsigned long long int);
CPP_PSCANF(long long int);
CPP_PSCANF(float);
CPP_PSCANF(double);

#define CPP_PFSCANF(T)                          \
   template int Parallel::pfscanf(FILE *, const char *const, T *const) const;
CPP_PFSCANF(unsigned char);
CPP_PFSCANF(char);
CPP_PFSCANF(unsigned short int);
CPP_PFSCANF(short int);
CPP_PFSCANF(unsigned int);
CPP_PFSCANF(int);
CPP_PFSCANF(unsigned long long int);
CPP_PFSCANF(long long int);
CPP_PFSCANF(float);
CPP_PFSCANF(double);

#define CPP_PFSCANF_MASTER(T)                   \
   template int Parallel::pfscanfMaster(FILE *, const char *const, T *const) const;
CPP_PFSCANF_MASTER(unsigned char);
CPP_PFSCANF_MASTER(char);
CPP_PFSCANF_MASTER(unsigned short int);
CPP_PFSCANF_MASTER(short int);
CPP_PFSCANF_MASTER(unsigned int);
CPP_PFSCANF_MASTER(int);
CPP_PFSCANF_MASTER(unsigned long long int);
CPP_PFSCANF_MASTER(long long int);
CPP_PFSCANF_MASTER(float);
CPP_PFSCANF_MASTER(double);

