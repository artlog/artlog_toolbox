#ifndef ALCBOR_HEADER_
#define ALCBOR_HEADER_


// field encoding type
typedef enum alcbor_field_encoding_type {
					 ALCBOR_FET_NOT_SET = -2,
					 ALCBOR_FET_NYI = -2,
					 ALCBOR_FET_ERROR = -1,
					 ALCBOR_FET_TINY = 0,
					 ALCBOR_FET_SHORT = 1,
					 ALCBOR_FER_LONG = 2
} alcbor_fet;

typedef enum alcbor_major_type {
				ALCBOR_MT0_UINT = 0,
				ALCBOR_MT1_NINT = 1,
				ALCBOR_MT2_BSTR = 2,
				ALCBOR_MT4_ARRAY = 4,
				ALCBOR_MT5_MAP = 5,
} alcbor_mt;



#endif // ALCBOR_HEADER_
 
