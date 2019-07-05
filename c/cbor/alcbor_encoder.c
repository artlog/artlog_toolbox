#include "alcbor_encoder.h"
#include "alcbor.h"
#include "aljson_output.h"
#include <stddef.h>

struct alcbor_encoder * alcbor_encoder_json_get_encoder(struct aljson_output_context * output_context)
{
  return (struct alcbor_encoder *) output_context->data;
}

// where we rely on generic
void alcbor_encoder_json_object(struct json_object * object, struct aljson_output_context * output_context)
{
  aljson_output_with_callback(object,output_context);
}

void alcbor_encoder_json_growable_object(struct json_object * object, struct aljson_output_context * output_context)
{
  struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);
}

void alcbor_encoder_write_big_endian(struct aloutputstream * output, int byte_length, size_t length)
{
  if ( byte_length < 4096 )
    {
      unsigned char buffer[byte_length];
      for (int i = 1 ;  i <= byte_length; i++ )
	{
	  buffer[byte_length-i]=( length & 0xff );
	  length >>= 8;
	}
      if ( aloutputstream_memcpy(byte_length,output,buffer) == byte_length )
	{
	  // cool
	}
      else
	{
	  // less cool :-(
	}
    }
}

alcbor_fet alcbor_encoder_encode_mt_and_length(alcbor_mt mt, size_t length, struct aloutputstream * output)
{

  int byte_length = 0;
  // should prepare length
  if ( length < 24 )
    {
      // ALCBOR_FET_TINY ;
      byte_length = 0;
      aloutputstream_write_byte( output, ( ((unsigned char) mt) << 5 ) | ((unsigned char) length));
      return ALCBOR_FET_TINY ;
    }
  else
    {
      // ALCBOR_FET_SHORT
      // find best encoding for length...
      int power = 1;
      size_t maxlength = 256;
      while (( length < maxlength ) && ( power + 24 < 31 ))
	{
	  power ++;
	  maxlength*=maxlength;
	}
      if ( power + 24 < 31 )
	{
	  aloutputstream_write_byte( output, ( ((unsigned char) ALCBOR_MT2_BSTR) << 5 ) | ((unsigned char) ( power + 24 )));
	  // number of bytes to encode length
	  byte_length=(1 << power);
	  alcbor_encoder_write_big_endian(output,byte_length,length);
	  return ALCBOR_FET_SHORT;
	}
      // else too big ... don't see how it can happen unless a corrupted data or buggy code.
    }
  return ALCBOR_FET_ERROR;
}

void alcbor_encoder_json_list_object(struct json_object * object, struct aljson_output_context * output_context)
{
  struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);
  struct aloutputstream * output=encoder->output;
  int length = object->list.nitems;
  if ( alcbor_encoder_encode_mt_and_length(ALCBOR_MT4_ARRAY,length,output) != ALCBOR_FET_ERROR )
    {
      for(int i=0;i< length;i++)
	{
	  alcbor_encoder_json_object(object->list.value[i],output_context);
	}
    }
}

void alcbor_encoder_json_string_object(struct json_object * object, struct aljson_output_context * output_context)
{
  struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);
  struct json_string * string = &object->string;
  struct aloutputstream * output = encoder->output;
  int length = string->internal.length;
  if ( alcbor_encoder_encode_mt_and_length(ALCBOR_MT2_BSTR,length,output) != ALCBOR_FET_ERROR )
    {
      // then print string
      if ( aloutputstream_memcpy(length,output,string->internal.data.charptr) == length )
	{
	  // ok
	}
      else
	{
	  // uhm might require mutliple calls ?
	}
    }
}


void alcbor_encoder_json_pair(struct json_pair * pair, struct aljson_output_context * output_context)
{
  struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);
  struct aloutputstream * output=encoder->output;

  alcbor_encoder_json_object(pair->key,output_context);
  alcbor_encoder_json_object(pair->value,output_context);
}

void alcbor_encoder_json_pair_object(struct json_object * object, struct aljson_output_context * output_context)
{
  alcbor_encoder_json_pair(&object->pair, output_context);
}

void alcbor_encoder_json_dict_object(struct json_object * object, struct aljson_output_context * output_context)
{
  struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);

  struct aloutputstream * output=encoder->output;
  size_t length = object->dict.nitems;
  if ( alcbor_encoder_encode_mt_and_length(ALCBOR_MT5_MAP,length,output) != ALCBOR_FET_ERROR )
    {
      for(int i=0;i< length;i++)
	{
	  alcbor_encoder_json_pair(object->dict.items[i], output_context);
	}
    }
}

void alcbor_encoder_json_string_number_object(struct json_object * object, struct aljson_output_context * output_context)
{
    struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);
    size_t number = json_get_int(object);
    struct aloutputstream * output = encoder->output;
    enum alcbor_major_type mt = ( number >= 0 ) ? ALCBOR_MT0_UINT : ALCBOR_MT1_NINT;
    if ( mt == ALCBOR_MT1_NINT )
      {
	number = - number - 1;
      }
    if ( alcbor_encoder_encode_mt_and_length(mt,number,output) != ALCBOR_FET_ERROR )
      {
	// ok
      }
    else
      {
	// uhm too bad
      }
}


void alcbor_encoder_json_variable_object(struct json_object * object, struct aljson_output_context * output_context)
{
    struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);

    // TODO
}

void alcbor_encoder_json_constant_object(struct json_object * object, struct aljson_output_context * output_context)
{
    struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);

    // TODO
}


void alcbor_encoder_json_error_object(struct json_object * object, struct aljson_output_context * output_context)
{
    struct alcbor_encoder * encoder=alcbor_encoder_json_get_encoder(output_context);

    // TODO
}


void alcbor_encoder_init(struct alcbor_encoder * encoder, struct aloutputstream * output)
{
  struct aljson_output_context * output_context = &encoder->output_context;
  struct aljson_output_callback_context * callback = & output_context->callback;

  callback->object=alcbor_encoder_json_object;
  callback->growable_object=alcbor_encoder_json_growable_object; 
  callback->dict_object=alcbor_encoder_json_dict_object;
  callback->list_object=alcbor_encoder_json_list_object;
  callback->string=alcbor_encoder_json_string_object;
  callback->pair_object=alcbor_encoder_json_pair_object;
  callback->string_number=alcbor_encoder_json_string_number_object;
  callback->variable_object=alcbor_encoder_json_variable_object;
  callback->constant_object=alcbor_encoder_json_error_object;
  callback->error_object=alcbor_encoder_json_error_object;

  encoder->output = output;
  aljson_output_init(output_context,callback,(void *)encoder);

}
