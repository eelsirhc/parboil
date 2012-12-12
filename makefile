INC_MPI=/nasa/sgi/mpt/1.23.nas/include
LIB_MPI=/nasa/sgi/mpt/1.23.nas/lib64

PARALLEL_LIB=-L${LIB_MPI} -lmpi
PARALLEL    =#-I${INC_MPI}

MAKE=make
CC=icc
FC=ifort
DEBUG=-g
OPTIM=-O2



CFLAGS=${PARALLEL} -fPIC ${DEBUG} ${OPTIM} -i_static
FFLAGS=-fPIC ${DEBUG} ${OPTIM} -D__G77__ -w95 -Kpic -l32

INCLUDE_OPTS=
INCLUDE=-I${INC_MPI} ${INCLUDE_OPTS}

LIB_OPTS=-lifcore ${PARALLEL_LIB} -lpthread 

LIB=${LIB_OPTS}
BIN=./

OBJS=parboil.o

EXEC=parboil

all: ${EXEC}

${EXEC} : ${OBJS}
	${CC} ${CFLAGS} $@.cc ${LIB} -o ${BIN}$@.exe 

.cc.o : 
	${CC} -c ${CFLAGS} ${INCLUDE} $*.cc

.f.o :
	${FC} -c ${FFLAGS} ${INCLUDE} $*.f

.f90.o :
	${FC} -c ${FFLAGS} ${INCLUDE} $*.f

