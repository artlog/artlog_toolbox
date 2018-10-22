#ifndef AL_CRYPTO_HASH_TOOL_H_
#define AL_CRYPTO_HASH_TOOL_H_

#include "alcryptohash.h"
#include "alinput.h"

/* dump to standard output */
void alcryptohash_tool_dump_result(struct alsha2_internal * shax, aldatablock * result);

/* call alsha2x_add_block on block */
void alcryptohash_tool_callback(aldatablock * block, void * data);

/* call alsha2x_final with last block */
void alcryptohash_tool_finalize (aldatablock * block, void * data);

/* walk full input by readblocksize to produce checksum */
aldatablock * alcryptohash_tool_from_input(  struct alsha2_internal *sha2x, struct alinputstream * input, int readblocksize);

#endif //  AL_CRYPTO_HASH_TOOL_H_
