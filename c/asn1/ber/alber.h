#ifndef __ALBER_H__
#define __ALBER_H__

#include "alstrings.h"

/**

End-of-Content (EOC) 	Primitive 	0 	0
BOOLEAN 	Primitive 	1 	1
INTEGER 	Primitive 	2 	2
BIT STRING 	Both 	3 	3
OCTET STRING 	Both 	4 	4
NULL 	Primitive 	5 	5
OBJECT IDENTIFIER 	Primitive 	6 	6
Object Descriptor 	Both 	7 	7
EXTERNAL 	Constructed 	8 	8
REAL (float) 	Primitive 	9 	9
ENUMERATED 	Primitive 	10 	A
EMBEDDED PDV 	Constructed 	11 	B
UTF8String 	Both 	12 	C
RELATIVE-OID 	Primitive 	13 	D
Reserved 		14 	E
Reserved 		15 	F
SEQUENCE and SEQUENCE OF 	Constructed 	16 	10
SET and SET OF 	Constructed 	17 	11
NumericString 	Both 	18 	12
PrintableString 	Both 	19 	13
T61String 	Both 	20 	14
VideotexString 	Both 	21 	15
IA5String 	Both 	22 	16
UTCTime 	Both 	23 	17
GeneralizedTime 	Both 	24 	18
GraphicString 	Both 	25 	19
VisibleString 	Both 	26 	1A
GeneralString 	Both 	27 	1B
UniversalString 	Both 	28 	1C
CHARACTER STRING 	Both 	29 	1D
BMPString 	Both 	30 	1E

 */

enum alasn1type {
  ALASN1_EOC=0,
  ALASN1_BOOLEAN=1,
  ALASN1_INTEGER=2,
  ALASN1_BIT_STRING=3,
  ALASN1_OCTET_STRING=4,
  ALASN1_NULL=5,
  ALASN1_OBJECT_IDENTIFIER=6,
  ALASN1_Object_Descriptor=7,
  ALASN1_EXTERNAL=8,
  ALASN1_REAL=9,
  ALASN1_ENUMERATED=10,
  ALASN1_EMBEDDED_PDV=11,
  ALASN1_UTF8String=12,
  ALASN1_RELATIVE_OID=13,
  //Reserved 		14 
  //Reserved 		15
  ALASN1_SEQUENCE_OF=16,
  ALASN1_SET_OF=17,
  ALASN1_NumericString=18,
  ALASN1_PrintableString=19,
  ALASN1_T61String=20,
  ALASN1_VideotexString=21,
  ALASN1_IA5String=22,
  ALASN1_UTCTime=23,
  ALASN1_GeneralizedTime=24,
  ALASN1_GraphicString=25,
  ALASN1_VisibleString=26,
  ALASN1_GeneralString=27,
  ALASN1_UniversalString=28,
  ALASN1_CHARACTER_STRING=29,
  ALASN1_BMPString=30
};


/*
Class 	Value 	Description
Universal 	0 	The type is native to ASN.1
Application 	1 	The type is only valid for one specific application
Context-specific 	2 	Meaning of this type depends on the context (such as within a sequence, set or choice)
Private 	3 	Defined in private specifications
*/
/*
P/C 	Value 	Description
Primitive (P) 	0 	The contents octets directly encode the element value.
Constructed (C) 	1 	The contents octets contain 0, 1, or more element encodings.
*/

enum alasn1class {
  ALASN1_Universal=0,
  ALASN1_Application=1,
  ALASN1_Context_specific=2,
  ALASN1_Private=3
};

// constructed : 1 constructed, 0 primitive, other values are errors.
int al_berencode_type(struct alhash_datablock * datablock, int offset, enum alasn1type asn1type, enum alasn1class asn1class, int constructed, int length);

#endif // #ifndef __ALBER_H__
