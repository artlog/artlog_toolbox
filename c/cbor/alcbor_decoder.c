#include "aljson_encoder.h"
#include "aldebug_output.h"

#include "alcbor_decoder.h"

#include <stddef.h>
#include <stdlib.h>
#include <strings.h>

/*
decode a cbor stream

encode it in json
*/


void alcbor_decode_not_yet_implemented(alcbor_pc * context, const char * text)
{
  aldebug_printf(DBGSTREAM,"[FATAL] Not yet implemented %s", text);
}

void alcbor_parsing_context_init(alcbor_pc * context, struct alinputstream * input, struct aloutputstream * output)
{
  context->input = input;
  context->block.data.ptr = NULL;
  context->header_byte = 0;
  context->value_length = 0;
  context->fet = ALCBOR_FET_NOT_SET;
  context->depth = 0;

  // HARDCODED maxdepth
  context->maxdepth = 1024;
  
  // build json output
  {
    struct json_parser_ctx * json_ctx = &context->output.json_ctx;
    alhash_context * hash_context =  &json_ctx->alparser;
    
    bzero(json_ctx,sizeof(*json_ctx));    
    // WARNING HARDCODED 100 words 1024 chars alhash_context_init(hash_context, words, chars, autogrow %/255);
    alhash_context_init(hash_context, 100, 1024, 200);
    // borrow allocator from hash table ( is it correct ? )
    context->output.allocator = &hash_context->allocator.ringbuffer;
    context->output.root = NULL;
  }
}

void alcbor_parsing_context_release(alcbor_pc * context)
{
  // TODO.
  alcbor_decode_not_yet_implemented(context, "NO RELEASE YET");
}


void alcbor_feed_block(aldatablock * block, void * data)
{
  // 
}

void alcbor_finalize_block(aldatablock * block, void * data)
{
  //
}

void alcbor_decode_invalid(alcbor_pc * context, const char * text)
{
  aldebug_printf(DBGSTREAM,"[ERROR] %s", text);
}

/* ALCBOR_MT0_UINT

   Major type 0:  an unsigned integer.  The 5-bit additional information
      is either the integer itself (for additional information values 0
      through 23) or the length of additional data.  Additional
      information 24 means the value is represented in an additional
      uint8_t, 25 means a uint16_t, 26 means a uint32_t, and 27 means a
      uint64_t.  For example, the integer 10 is denoted as the one byte
      0b000_01010 (major type 0, additional information 10).  The
      integer 500 would be 0b000_11001 (major type 0, additional
      information 25) followed by the two bytes 0x01f4, which is 500 in
      decimal.
*/

/* ALCBOR_MT1_NINT

   Major type 1:  a negative integer.  The encoding follows the rules
      for unsigned integers (major type 0), except that the value is
      then -1 minus the encoded unsigned integer.  For example, the
      integer -500 would be 0b001_11001 (major type 1, additional
      information 25) followed by the two bytes 0x01f3, which is 499 in
      decimal.
*/


enum al_global_error_code alcbor_read_bytes(alcbor_pc * context, int length)
{
  aldatablock * block = &context->block;
  if ( ( block != NULL ) && ( block->data.ptr != NULL ) )
    {
      enum al_global_error_code ec =  alinputstream_read_block_at(context->input, block, context->idxob, length);
      return ec;
    }
}

void alcbor_add_json_intern(struct alcbor_json_output * output,	struct json_object * object)
{
  aldebug_printf(DBGSTREAM,"some json\n");
  // TODO
  if ( output->root == NULL )
    {
      output->root = object;
    }
  output->last = object;
}



// used by both uint ALCBOR_MT0_UINT and nint ALCBOR_MT1_NINT
void alcbor_decode_uint(alcbor_pc * context)
{
  switch(context->fet)
    {
    case ALCBOR_FET_TINY:
      {
	struct alcbor_json_output * output = &context->output;
	// got value within context->value_length;
	struct json_object * object = aljson_encoder_int(context->value_length,&output->json_ctx,output->allocator);
	alcbor_add_json_intern(output, object);
      }
      break;
    case ALCBOR_FET_SHORT:
      {
	int length = context->value_length;
	if ( length > 8 )
	  {
	    alcbor_decode_invalid(context, "length > 8 bytes is not supported for uint.");
	  }
	alcbor_read_bytes(context,length);
      }
      // TODO aljson_encoder(context->value_length,context->json_ctx,context->allocator);
      break;
    default:
      alcbor_decode_invalid(context, "internal erro unexpected code path for uint decoding");
    }
}


void alcbor_decode_mt_generic(alcbor_pc * context)
{
  unsigned int inter_value = context->header_byte & 0x1f;
  if ( inter_value < 24 )
    {
      // got unsigned int value.
      context->fet=ALCBOR_FET_TINY ;
      context->value_length=inter_value;
    }
  else
    {
      if ( inter_value == 31 )
	{
	  // indefinite_form is not supported by uint.
	  alcbor_decode_invalid(context, "indefinite_form is not supported for uint.");
	}
      unsigned int power = inter_value - 24;      
      context->fet=ALCBOR_FET_SHORT;
      context->value_length=(1 << power);
    }
}

void alcbor_decode_mt_0_uint(alcbor_pc * context)
{
  alcbor_decode_mt_generic(context);
  alcbor_decode_uint(context);
}

size_t alcbor_decode_length(alcbor_pc * context)
{
  switch(context->fet)
    {
    case ALCBOR_FET_TINY:
      return (size_t) context->value_length;
      break;
    case ALCBOR_FET_SHORT:
      {
	int length = context->value_length;
	if ( length > 8 )
	  {
	    alcbor_decode_invalid(context, "length > 8 bytes is not supported for string.");
	  }
	if ( alcbor_read_bytes(context,length) == AL_EC_OK )
	  {
	    // should convert it to length. FIXME
	  }
      }
      // TODO aljson_encoder(context->value_length,context->json_ctx,context->allocator);
      break;
    default:
      alcbor_decode_invalid(context, "internal erro unexpected code path for uint decoding");
    }

}

/* ALCBOR_MT2_BSTR

   Major type 2:  a byte string.  The string's length in bytes is
      represented following the rules for positive integers (major type
      0).  For example, a byte string whose length is 5 would have an
      initial byte of 0b010_00101 (major type 2, additional information
      5 for the length), followed by 5 bytes of binary content.  A byte
      string whose length is 500 would have 3 initial bytes of
      0b010_11001 (major type 2, additional information 25 to indicate a
      two-byte length) followed by the two bytes 0x01f4 for a length of
      500, followed by 500 bytes of binary content.
*/

void alcbor_decode_mt_2_string(alcbor_pc * context)
{
  alcbor_decode_mt_generic(context);

  size_t length = alcbor_decode_length(context);

  // reset buffer.
  context->idxob = 0;
  aldatablock * block = &context->block;
  if ( length <= block->length )
    {
      // now should read string...
      if ( alcbor_read_bytes(context,length) == AL_EC_OK )
	{
	  struct alcbor_json_output * output = &context->output;
	  // string is now within block
	  // onvert block to non NULL terminated / pascal json string.
	  struct json_object * object = aljson_encoder_pascal_string(length,
								     block->data.constcharptr,
								     &output->json_ctx,
								     output->allocator);
	  alcbor_add_json_intern(output, object);
	}
    }
  else
    {
      // else should grow
      aldebug_printf(DBGSTREAM,"[FATAL] TODO Should grow buffer\n");
    }

}


// BSTRING NYI

alcbor_mt alcbor_decode_major_type(alcbor_pc * context)
{  
  if (  alcbor_read_bytes(context,1) == AL_EC_OK )
    {
      aldatablock * block = &context->block;
      unsigned int header_byte = block->data.ucharptr[context->idxob];
      context->idxob ++;

      alcbor_mt mt = header_byte >> 5;
      context->header_byte = header_byte;
  
      return mt;  

    }
else
{
      alcbor_decode_invalid(context, "input too short or invalid\n");
}
  return ALCBOR_FET_ERROR;
}

/*
   Major type 4:  an array of data items.  Arrays are also called lists,
      sequences, or tuples.  The array's length follows the rules for
      byte strings (major type 2), except that the length denotes the
      number of data items, not the length in bytes that the array takes
      up.  Items in an array do not need to all be of the same type.
      For example, an array that contains 10 items of any type would
      have an initial byte of 0b100_01010 (major type of 4, additional
      information of 10 for the length) followed by the 10 remaining
      items.
*/

// forward definition
void alcbor_decode_mt_4_array(alcbor_pc * context);


/*
   Major type 5:  a map of pairs of data items.  Maps are also called
      tables, dictionaries, hashes, or objects (in JSON).  A map is
      comprised of pairs of data items, each pair consisting of a key
      that is immediately followed by a value.  The map's length follows
      the rules for byte strings (major type 2), except that the length
      denotes the number of pairs, not the length in bytes that the map
      takes up.  For example, a map that contains 9 pairs would have an
      initial byte of 0b101_01001 (major type of 5, additional
      information of 9 for the number of pairs) followed by the 18
      remaining items.  The first item is the first key, the second item
      is the first value, the third item is the second key, and so on.
      A map that has duplicate keys may be well-formed, but it is not
      valid, and thus it causes indeterminate decoding; see also
      Section 3.7.
*/

// forward definition
void alcbor_decode_mt_5_map(alcbor_pc * context);

enum al_global_error_code alcbor_parse_from_header_byte(alcbor_pc * context)
{
  // should play with substream...
  alcbor_mt mt = alcbor_decode_major_type(context);

  aldebug_printf(DBGSTREAM,"[DEBUG] mt %i\n",mt);
  switch ( mt )
    {
    case ALCBOR_MT0_UINT:
      alcbor_decode_mt_0_uint(context);
      break;
    case ALCBOR_MT1_NINT:
      // FIXME 
      alcbor_decode_mt_0_uint(context);
      break;
    case ALCBOR_MT2_BSTR:
      alcbor_decode_mt_2_string(context);
      break;
    case ALCBOR_MT4_ARRAY:
      alcbor_decode_mt_4_array(context);
      break;
    case ALCBOR_MT5_MAP  :
      alcbor_decode_mt_5_map(context);
    default:
      alcbor_decode_not_yet_implemented(context, "early dev, might last longer than expected.");
      return AL_EC_NYI;
    }

  return AL_EC_OK;
}

// should not go deeper than maxdepth
enum al_global_error_code alcbor_inc_depth(alcbor_pc * context)
{
  if ( context->depth < context->maxdepth )
    {
      context->depth ++;
      return AL_EC_OK;
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[WARNING] max depth reached %i > %i %s %s L%i\n",context->depth, context->maxdepth,__FILE__,__func__,__LINE__);
      return AL_EC_FALSE;
    }
}

void  alcbor_dec_depth(alcbor_pc * context)
{
  context->depth --;
}

// STACK based recursive protected by alcbor_inc_depth
void alcbor_decode_mt_4_array(alcbor_pc * context)
{

  alcbor_decode_mt_generic(context);
  // alcbor_decode_uint(context);

  int length = context->value_length;

  aldebug_printf(DBGSTREAM,"[DEBUG] array length %i %s %s L%i\n",length,__FILE__,__func__,__LINE__);

  struct alcbor_json_output * output = &context->output;
  struct json_parser_ctx * json_ctx = &output->json_ctx;

  struct json_object * root = NULL;
  
  // create a growable for an array.
  struct json_object * parent =  aljson_new_growable(json_ctx,'[');
  root = output->root;

  if ( alcbor_inc_depth(context) == AL_EC_OK )
    {

      if ( parent != NULL )
	{
	  struct json_growable * growable = &parent->growable;

	  // length 0 is acceptable this is empty array
	  for (int index = 0 ; index < length ; index ++ )
	    {
	      alcbor_parse_from_header_byte(context);
      
	      struct json_object * last = output->last;
	      if (last != NULL )
		{
		  // add it into parent
		  aljson_add_to_growable(json_ctx,growable,last);
		}
	    }
	}

      alcbor_dec_depth(context);
    }
  
  struct json_object * object = aljson_concrete(json_ctx,parent);
  alcbor_add_json_intern(output,object);
  // due to root updated only by alcbor_add_json_intern and aljson_concrete creating a new element, it is mandatory to fix it.
  if ( root == NULL )
    {
      output->root = object;
    }

}

// STACK based recursive protected by alcbor_inc_depth
void alcbor_decode_mt_5_map(alcbor_pc * context)
{

  alcbor_decode_mt_generic(context);
  // alcbor_decode_uint(context);

  int length = context->value_length;

  aldebug_printf(DBGSTREAM,"[DEBUG] map length %i %s %s L%i\n",length,__FILE__,__func__,__LINE__);

  struct alcbor_json_output * output = &context->output;
  struct json_parser_ctx * json_ctx = &output->json_ctx;

  struct json_object * root = NULL;
  
  // create a growable for a map.
  struct json_object * parent =  aljson_new_growable(json_ctx,'{');
  root = output->root;

  if ( alcbor_inc_depth(context) == AL_EC_OK )
    {
      if ( parent != NULL )
	{
	  struct json_growable * growable = &parent->growable;
	  // length 0 is acceptable this is empty map
	  for (int index = 0 ; index < length ; index ++ )
	    {	      
	      // pair
	      struct json_object * key = NULL;
	      struct json_object * value = NULL;
	      {
		alcbor_parse_from_header_byte(context);
		key = output->last;
		if ( key  != NULL )
		  {
		    alcbor_parse_from_header_byte(context);
		    value = output->last;
		    
		  }             		    
	      }
	      if ( ( key != NULL ) && ( value != NULL ) )
		{
		  // create a pair and  add it into parent
		  struct json_object * pair = aljson_new_pair_key(json_ctx , key);
		  if (( pair != NULL ) && (key != NULL ))
		    {
		      pair->pair.value=value;
		    }
		  aljson_add_to_growable(json_ctx,growable,key);
		}
	    }
	}
      alcbor_dec_depth(context);
    }
  
  struct json_object * object = aljson_concrete(json_ctx,parent);
  alcbor_add_json_intern(output,object);
  // due to root updated only by alcbor_add_json_intern and aljson_concrete creating a new element, it is mandatory to fix it.
  if ( root == NULL )
    {
      output->root = object;
    }

}

void alcbor_parse(alcbor_pc * context)
{
  const char * text = "can only decode uint yet... currently useless.";
  // TODO
  alcbor_decode_not_yet_implemented(context, text);

  if ( context->block.data.ptr == NULL )
    {
      // HARDCODED
      int length = 4096;
      context->block.data.ptr = calloc(1,length);
      context->block.length = length;
      context->idxob = 0;      
    }  
 alcbor_parse_from_header_byte(context);
 struct alcbor_json_output * output = &context->output;      
 if ( output->root == NULL )
   {
     struct json_object * object = aljson_encoder_string(text, &output->json_ctx,output->allocator);
     output->root = object;
   }
}

struct json_object * alcbor_get_json_root(alcbor_pc * context)
{
  return context->output.root;
}

struct json_parser_ctx * alcbor_get_json_context(alcbor_pc * context)
{
  return &context->output.json_ctx;
}
