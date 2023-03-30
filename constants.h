
namespace tspg {
enum datatype { NUMERICAL = 1, STRING=2, CUSTOMDF=3, SET=4 };
}

#define D_L2 0 
#define D_L1 1 
#define D_COS 2 

#define DIST_L2 0
#define DIST_MINKW 1
#define DIST_COS 2
#define DIST_DICE 10
#define DIST_LEV 11


#define T_NUMERICAL 1
#define T_STRING 2
#define T_CUSTOMDF 3
#define T_SET 4

// gtype
#define RPDIV 0
#define COMPL 1

// costf
#define TSE 5

// Run assert() functions only in debug mode
#ifdef DEBUG
// #define debug_assert(x) assert(x)
#define debug_assert(x)
#else
#define debug_assert(x)
#endif


