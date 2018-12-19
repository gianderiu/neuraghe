#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xos.h"
#include "colors.h"

#ifdef _HALFFLOAT_
#include "hls_half.h"
#endif

#define _H0_ 224
#define _W0_ 224

#define _H1_ ((_H0_ + 1)/2)
#define _W1_ ((_W0_ + 1)/2)

#define _H2_ ((_H1_ + 1)/2)
#define _W2_ ((_W1_ + 1)/2)

#define _H3_ ((_H2_ + 1)/2)
#define _W3_ ((_W2_ + 1)/2)

#define _H4_ ((_H3_ + 1)/2)
#define _W4_ ((_W3_ + 1)/2)

#define _H5_ ((_H4_ + 1)/2)
#define _W5_ ((_W4_ + 1)/2)

#define _P0_ 3
#define _P1_ 64
#define _P2_ (2*_P1_)
#define _P3_ (2*_P2_)
#define _P4_ (2*_P3_)

#define _HK0_ 3
#define _WK0_ 3

#define _HS_ _HK0_

//weff == smallest width
#define _WEFF_ _W5_

#define _WS_ (_WEFF_ + (_WK0_ - 1))
#define _WPAD_ ((_WK0_ - 1)/2)
#define _HPAD_ ((_HK0_ - 1)/2)

//maximum channel sizes
#define _CH_N_ (_W2_ / _WEFF_)
#define _CH_H_ (_H2_)

//maximum memory size needed to store data
#define _MAXMEM_ (_P1_ * (_H0_+4) * (_W0_+4)) + (12 * (_H0_+4) * (_W0_+4))

#define _NIN_ 64
#define _NOUT_ 64
#define _NWORKER_ 64

#define _NOUT_PW_ ((_NOUT_)/(_NWORKER_))

#define STRLEN 200

#define QF 11

typedef short int DATA;
#define FIXED2FLOAT(a) (((float) (a)) / pow(2,QF))
#define FLOAT2FIXED(a) ((short int) round((a) * pow(2,QF)))
#define _MAX_ (1 << (sizeof(DATA)*8-1))-1
#define _MIN_ -(_MAX_+1)

// 		typedef float DATA;
// 		#define FIXED2FLOAT(a) a
//    		#define FLOAT2FIXED(a) a
// 		#define _MAX_ 0	
// 		#define _MIN_ 0
// 	#endif
// #endif

#define MAX_POOL 0
#define AVG_POOL 1
#define SUBSAMP  2

#define POOL_2x2 1
#define POOL_4x4 3

typedef const size_t SIZE;
typedef size_t VARSIZE;
typedef const char* NAME;
typedef char VARNAME[STRLEN];
typedef int ID;
typedef void* USER_DATA;

int equalSize(SIZE* a, SIZE* b, SIZE nelements);
RET assignSize(VARSIZE* target, SIZE* source, SIZE nelements);
RET loadData(const char* filename, size_t arraysize, DATA* array);

unsigned long get_wall_time(void); 

#ifdef __cplusplus
}
#endif

#endif
