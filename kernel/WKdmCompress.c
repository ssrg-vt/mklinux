#include "WKdm.h"
#include <linux/slab.h>
#include <linux/kernel.h>
/***********************************************************************
 *                   THE PACKING ROUTINES
 */

/* WK_pack_2bits()
 * Pack some multiple of four words holding two-bit tags (in the low
 * two bits of each byte) into an integral number of words, i.e.,
 * one fourth as many.
 * NOTE: Pad the input out with zeroes to a multiple of four words!
 */
static WK_word*
WK_pack_2bits(WK_word* source_buf,
              WK_word* source_end,
	      WK_word* dest_buf) {

   register WK_word* src_next = source_buf;
   WK_word* dest_next = dest_buf;

   while (src_next < source_end) {
      register WK_word temp = src_next[0];
      temp |= (src_next[1] << 2);
      temp |= (src_next[2] << 4);
      temp |= (src_next[3] << 6);

      dest_next[0] = temp;
      dest_next++;
      src_next += 4;
   }

   return dest_next;

}

/* WK_pack_4bits()
 * Pack an even number of words holding 4-bit patterns in the low bits
 * of each byte into half as many words.
 * note: pad out the input with zeroes to an even number of words!
 */

static WK_word*
WK_pack_4bits(WK_word* source_buf,
	      WK_word* source_end,
	      WK_word* dest_buf) {
   register WK_word* src_next = source_buf;
   WK_word* dest_next = dest_buf;

   /* this loop should probably be unrolled */
   while (src_next < source_end) {
     register WK_word temp = src_next[0];
     temp |= (src_next[1] << 4);

     dest_next[0] = temp;
     dest_next++;
     src_next += 2;
   }

   return dest_next;

}



static WK_word*
WK_pack_3_twentybits(WK_word* source_buf,
		  WK_word* source_end,
		  WK_word* dest_buf) {

   register WK_word* src_next = source_buf;
   WK_word* dest_next = dest_buf;

   /* this loop should probably be unrolled */
   while (src_next < source_end) {
      register WK_word temp = src_next[0];
      temp |= (src_next[1] << 20);
      temp |= (src_next[2] << 40);

      dest_next[0] = temp;
      dest_next++;
      src_next += 3;
   }


   return dest_next;

}

/***************************************************************************
 *  WKdm_compress()---THE COMPRESSOR
 */

unsigned int
WKdm_compress (WK_word* src_buf,
               WK_word* dest_buf)
{
  DictionaryElement dictionary[DICTIONARY_SIZE];

  /* arrays that hold output data in intermediate form during modeling */
  /* and whose contents are packed into the actual output after modeling */

  /* sizes of these arrays should be increased if you want to compress
   * pages larger than 4KB
   */
  //WK_word tempTagsArray[300];         /* tags for everything          */
 // WK_word tempQPosArray[300];         /* queue positions for matches  */
 // WK_word tempLowBitsArray[1200];     /* low bits for partial matches */

  WK_word* tempTagsArray= ( WK_word*) kmalloc(sizeof(WK_word)*150, GFP_ATOMIC);
    if(tempTagsArray==NULL){
  	  printk("Impossible to kmalloc in compress \n");
  	  return 0;
    }
    WK_word* tempQPosArray= ( WK_word*) kmalloc(sizeof(WK_word)*150, GFP_ATOMIC);
    if(tempQPosArray==NULL){
  	  printk("Impossible to kmalloc in compress \n");
  	  kfree(tempTagsArray);
  	  return 0;
    }
    WK_word* tempLowBitsArray= ( WK_word*) kmalloc(sizeof(WK_word)*600, GFP_ATOMIC);
    if(tempLowBitsArray==NULL){
  	  printk("Impossible to kmalloc in compress \n");
  	  kfree(tempTagsArray);
  	  kfree(tempQPosArray);
  	  return 0;
    }
  /* boundary_tmp will be used for keeping track of what's where in
   * the compressed page during packing
   */
  WK_word* boundary_tmp;

  /* Fill pointers for filling intermediate arrays (of queue positions
   * and low bits) during encoding.
   * Full words go straight to the destination buffer area reserved
   * for them.  (Right after where the tags go.)
   */
  WK_word* next_full_patt;
  char* next_tag = (char *) tempTagsArray;
  char* next_qp = (char *) tempQPosArray;
  WK_word* next_low_bits = tempLowBitsArray;

  WK_word* next_input_word = src_buf;
  WK_word* end_of_input = src_buf + PAGE_SIZE_IN_WORDS;

  PRELOAD_DICTIONARY;

  //next_full_patt = dest_buf + TAGS_AREA_OFFSET + (num_input_words / 16);
  next_full_patt = dest_buf + TAGS_AREA_OFFSET + TAGS_AREA_SIZE;

#ifdef WK_DEBUG
  printk("\nIn WKdm_compress\n");
  printk("About to actually compress, src_buf is %u\n", src_buf);
  printk("dictionary is at %u\n", dictionary);
  printk("dest_buf is %u next_full_patt is %u\n", dest_buf, next_full_patt);
#endif

  while (next_input_word < end_of_input)
  {
     WK_word *dict_location;
     WK_word dict_word;
     WK_word input_word = *next_input_word;

     /* compute hash value, which is a byte offset into the dictionary,
      * and add it to the base address of the dictionary. Cast back and
      * forth to/from char * so no shifts are needed
      */
     dict_location =
       (WK_word *)
       (((char*) dictionary) + HASH_TO_DICT_BYTE_OFFSET(input_word));

     dict_word = *dict_location;

//printk("hash %i input %lu dictionary %lu\n",HASH_TO_DICT_BYTE_OFFSET(input_word),input_word,dict_word);
     if (input_word == 0)
     {
    	 RECORD_ZERO ;

     }
     else if (input_word == dict_word) {
    	 RECORD_EXACT(dict_location - dictionary);
    	 //printk("exact\n");
     }
     else
     {
        WK_word input_high_bits = HIGH_BITS(input_word);
        if (input_high_bits == HIGH_BITS(dict_word)) {
	  RECORD_PARTIAL(dict_location - dictionary, LOW_BITS(input_word));
          *dict_location = input_word;
          //printk("partial\n");
        }
        else {
	  RECORD_MISS(input_word);
            *dict_location = input_word;
            // printk("miss\n");
        }
     }
     next_input_word++;
  }

#ifdef WK_DEBUG
  printk("AFTER MODELING in WKdm_compress()\n");
  printk("tempTagsArray holds %u bytes\n",
         next_tag - (char *) tempTagsArray);
  printk("tempQPosArray holds %u bytes\n",
         next_qp - (char *) tempQPosArray);
  printk("tempLowBitsArray holds %u bytes\n",
         (char *) next_low_bits - (char *) tempLowBitsArray);

  printk("next_full_patt is %u\n",
         (unsigned long) next_full_patt);

  printk(" i.e., there are %u full patterns\n",
     next_full_patt - (dest_buf + TAGS_AREA_OFFSET + TAGS_AREA_SIZE));


  { int i;
    WK_word *arr =(dest_buf + TAGS_AREA_OFFSET + TAGS_AREA_SIZE);

    printk("  first 20 full patterns are: \n");
    for (i = 0; i < 20; i++) {
      printk(" %d", arr[i]);
    }
    printk("\n");
  }
#endif

  /* Record (into the header) where we stopped writing full words,
   * which is where we will pack the queue positions.  (Recall
   * that we wrote the full words directly into the dest buffer
   * during modeling.
   */

  SET_QPOS_AREA_START(dest_buf,next_full_patt);

  /* Pack the tags into the tags area, between the page header
   * and the full words area.  We don't pad for the packer
   * because we assume that the page size is a multiple of 16.
   */

#ifdef WK_DEBUG
  printk("about to pack %u bytes holding tags\n",
         next_tag - (char *) tempTagsArray);

  { int i;
    char* arr = (char *) tempTagsArray;

    printk("  first 200 tags are: \n");
    for (i = 0; i < 200; i++) {
      printk(" %d", arr[i]);
    }
    printk("\n");
  }
#endif

  boundary_tmp = WK_pack_2bits(tempTagsArray,
		               (WK_word *) next_tag,
			       dest_buf + HEADER_SIZE_IN_WORDS);


#ifdef WK_DEBUG
    printk("packing tags stopped at %u\n", boundary_tmp);
#endif

  /* Pack the queue positions into the area just after
   * the full words.  We have to round up the source
   * region to a multiple of two words.
   */

  {
    unsigned int num_bytes_to_pack = next_qp - (char *) tempQPosArray;
    unsigned int num_packed_words = (num_bytes_to_pack + 7) >> 3; // ceil((double) num_bytes_to_pack / 8);
    // unsigned int num_source_words = num_packed_words * 2;
    unsigned int num_source_words = num_packed_words ;
    WK_word* endQPosArray = tempQPosArray + num_source_words;

    /* Pad out the array with zeros to avoid corrupting real packed
       values. */
    for (; /* next_qp is already set as desired */
	 next_qp < (char*)endQPosArray;
	 next_qp++) {
      *next_qp = 0;
    }

#ifdef WK_DEBUG
    printk("about to pack %u (bytes holding) queue posns.\n",
           num_bytes_to_pack);
    printk("packing them from %u words into %u words\n",
           num_source_words, num_packed_words);
    printk("dest is range %u to %u\n",
           next_full_patt, next_full_patt + num_packed_words);
    { int i;
      char *arr = (char *) tempQPosArray;
      printk("  first 200 queue positions are: \n");
      for (i = 0; i < 200; i++) {
        printk(" %d", arr[i]);
      }
      printk("\n");
    }
#endif

    boundary_tmp = WK_pack_4bits(tempQPosArray,
			         endQPosArray,
				 next_full_patt);

#ifdef WK_DEBUG
     printk("Packing of queue positions stopped at %u\n", boundary_tmp);
#endif WK_DEBUG

    /* Record (into the header) where we stopped packing queue positions,
     * which is where we will start packing low bits.
     */
    SET_LOW_BITS_AREA_START(dest_buf,boundary_tmp);

  }

  /* Pack the low bit patterns into the area just after
   * the queue positions.  We have to round up the source
   * region to a multiple of three words.
   */

  {
   // unsigned int num_tenbits_to_pack = next_low_bits - tempLowBitsArray;
    //unsigned int num_packed_words = (num_tenbits_to_pack + 2) / 3; //ceil((double) num_tenbits_to_pack / 3);
    //unsigned int num_source_words = num_packed_words * 3;


    unsigned int num_tewntybits_to_pack= next_low_bits - tempLowBitsArray;
    unsigned int num_packed_words = (num_tewntybits_to_pack + 2) / 3;
    unsigned int num_source_words = num_packed_words * 3;

    WK_word* endLowBitsArray = tempLowBitsArray + num_source_words;

    /* Pad out the array with zeros to avoid corrupting real packed
       values. */

    for (; /* next_low_bits is already set as desired */
	 next_low_bits < endLowBitsArray;
	 next_low_bits++) {
      *next_low_bits = 0;
    }

#ifdef WK_DEBUG
	  printk("about to pack low bits\n");
         // printk("num_tenbits_to_pack is %u\n", num_tenbits_to_pack);
          printk("endLowBitsArray is %u\n", endLowBitsArray);
#endif

   // boundary_tmp = WK_pack_3_tenbits (tempLowBitsArray,
	//	                      endLowBitsArray,
	//			      boundary_tmp);

	boundary_tmp = WK_pack_3_twentybits (tempLowBitsArray,
		                      endLowBitsArray,
				      boundary_tmp);

    SET_LOW_BITS_AREA_END(dest_buf,boundary_tmp);

  }
  kfree(tempTagsArray);
  kfree(tempQPosArray);
  kfree(tempLowBitsArray);
  return ((char *) boundary_tmp - (char *) dest_buf);
}

unsigned int
WKdm_diff_and_compress (WK_word* src_buf1, WK_word* src_buf2,
		WK_word* dest_buf)
{

  DictionaryElement dictionary[DICTIONARY_SIZE];

  /* arrays that hold output data in intermediate form during modeling */
  /* and whose contents are packed into the actual output after modeling */

  /* sizes of these arrays should be increased if you want to compress
   * pages larger than 4KB
   */
  //WK_word tempTagsArray[300];         /* tags for everything          */
 // WK_word tempQPosArray[300];         /* queue positions for matches  */
 // WK_word tempLowBitsArray[1200];     /* low bits for partial matches */

  WK_word* tempTagsArray= ( WK_word*) kmalloc(sizeof(WK_word)*150, GFP_ATOMIC);
  if(tempTagsArray==NULL){
	  printk("Impossible to kmalloc in compress \n");
	  return 0;
  }
  WK_word* tempQPosArray= ( WK_word*) kmalloc(sizeof(WK_word)*150, GFP_ATOMIC);
  if(tempQPosArray==NULL){
	  printk("Impossible to kmalloc in compress \n");
	  kfree(tempTagsArray);
	  return 0;
  }
  WK_word* tempLowBitsArray= ( WK_word*) kmalloc(sizeof(WK_word)*600, GFP_ATOMIC);
  if(tempLowBitsArray==NULL){
	  printk("Impossible to kmalloc in compress \n");
	  kfree(tempTagsArray);
	  kfree(tempQPosArray);
	  return 0;
  }
  /* boundary_tmp will be used for keeping track of what's where in
   * the compressed page during packing
   */
  WK_word* boundary_tmp;

  /* Fill pointers for filling intermediate arrays (of queue positions
   * and low bits) during encoding.
   * Full words go straight to the destination buffer area reserved
   * for them.  (Right after where the tags go.)
   */
  WK_word* next_full_patt;
  char* next_tag = (char *) tempTagsArray;
  char* next_qp = (char *) tempQPosArray;
  WK_word* next_low_bits = tempLowBitsArray;

  WK_word* next_input_word1 = src_buf1;
  WK_word* next_input_word2 = src_buf2;
  WK_word* end_of_input = src_buf1 + PAGE_SIZE_IN_WORDS;

  PRELOAD_DICTIONARY;

  //next_full_patt = dest_buf + TAGS_AREA_OFFSET + (num_input_words / 16);
  next_full_patt = dest_buf + TAGS_AREA_OFFSET + TAGS_AREA_SIZE;

#ifdef WK_DEBUG
  printk("\nIn WKdm_compress\n");
  printk("About to actually compress, src_buf is %u\n", src_buf1);
  printk("dictionary is at %u\n", dictionary);
  printk("dest_buf is %u next_full_patt is %u\n", dest_buf, next_full_patt);
#endif

  while (next_input_word1 < end_of_input)
  {
     WK_word *dict_location;
     WK_word dict_word;

     WK_word input_word = (*next_input_word1)^(*next_input_word2);
     /* compute hash value, which is a byte offset into the dictionary,
      * and add it to the base address of the dictionary. Cast back and
      * forth to/from char * so no shifts are needed
      */
     dict_location =
       (WK_word *)
       (((char*) dictionary) + HASH_TO_DICT_BYTE_OFFSET(input_word));
		//printk("hash %i input %lu\n",HASH_TO_DICT_BYTE_OFFSET(input_word),input_word);
     dict_word = *dict_location;

     if (input_word == 0)
     {
        RECORD_ZERO;
     }
     else if (input_word == dict_word) {

    	 RECORD_EXACT(dict_location - dictionary);
     }
     else
     {
        WK_word input_high_bits = HIGH_BITS(input_word);
        if (input_high_bits == HIGH_BITS(dict_word)) {
	  RECORD_PARTIAL(dict_location - dictionary, LOW_BITS(input_word));
          *dict_location = input_word;

        }
        else {
	  RECORD_MISS(input_word);
            *dict_location = input_word;

        }
     }
     next_input_word1++;
     next_input_word2++;
  }

#ifdef WK_DEBUG
  printk("AFTER MODELING in WKdm_compress()\n");
  printk("tempTagsArray holds %u bytes\n",
         next_tag - (char *) tempTagsArray);
  printk("tempQPosArray holds %u bytes\n",
         next_qp - (char *) tempQPosArray);
  printk("tempLowBitsArray holds %u bytes\n",
         (char *) next_low_bits - (char *) tempLowBitsArray);

  printk("next_full_patt is %u\n",
         (unsigned long) next_full_patt);

  printk(" i.e., there are %u full patterns\n",
     next_full_patt - (dest_buf + TAGS_AREA_OFFSET + TAGS_AREA_SIZE));


  { int i;
    WK_word *arr =(dest_buf + TAGS_AREA_OFFSET + TAGS_AREA_SIZE);

    printk("  first 20 full patterns are: \n");
    for (i = 0; i < 20; i++) {
      printk(" %d", arr[i]);
    }
    printk("\n");
  }
#endif


  /* Record (into the header) where we stopped writing full words,
   * which is where we will pack the queue positions.  (Recall
   * that we wrote the full words directly into the dest buffer
   * during modeling.
   */

  SET_QPOS_AREA_START(dest_buf,next_full_patt);

  /* Pack the tags into the tags area, between the page header
   * and the full words area.  We don't pad for the packer
   * because we assume that the page size is a multiple of 16.
   */

#ifdef WK_DEBUG
  printk("about to pack %u bytes holding tags\n",
         next_tag - (char *) tempTagsArray);

  { int i;
    char* arr = (char *) tempTagsArray;

    printk("  first 200 tags are: \n");
    for (i = 0; i < 200; i++) {
      printk(" %d", arr[i]);
    }
    printk("\n");
  }
#endif

  boundary_tmp = WK_pack_2bits(tempTagsArray,
		               (WK_word *) next_tag,
			       dest_buf + HEADER_SIZE_IN_WORDS);


#ifdef WK_DEBUG
    printk("packing tags stopped at %u\n", boundary_tmp);
#endif

  /* Pack the queue positions into the area just after
   * the full words.  We have to round up the source
   * region to a multiple of two words.
   */

  {
    unsigned int num_bytes_to_pack = next_qp - (char *) tempQPosArray;
    unsigned int num_packed_words = (num_bytes_to_pack + 7) >> 3; // ceil((double) num_bytes_to_pack / 8);
    // unsigned int num_source_words = num_packed_words * 2;
    unsigned int num_source_words = num_packed_words ;
    WK_word* endQPosArray = tempQPosArray + num_source_words;

    /* Pad out the array with zeros to avoid corrupting real packed
       values. */
    for (; /* next_qp is already set as desired */
	 next_qp < (char*)endQPosArray;
	 next_qp++) {
      *next_qp = 0;
    }

#ifdef WK_DEBUG
    printk("about to pack %u (bytes holding) queue posns.\n",
           num_bytes_to_pack);
    printk("packing them from %u words into %u words\n",
           num_source_words, num_packed_words);
    printk("dest is range %u to %u\n",
           next_full_patt, next_full_patt + num_packed_words);
    { int i;
      char *arr = (char *) tempQPosArray;
      printk("  first 200 queue positions are: \n");
      for (i = 0; i < 200; i++) {
        printk(" %d", arr[i]);
      }
      printk("\n");
    }
#endif

    boundary_tmp = WK_pack_4bits(tempQPosArray,
			         endQPosArray,
				 next_full_patt);


#ifdef WK_DEBUG
     printk("Packing of queue positions stopped at %u\n", boundary_tmp);
#endif WK_DEBUG

    /* Record (into the header) where we stopped packing queue positions,
     * which is where we will start packing low bits.
     */
    SET_LOW_BITS_AREA_START(dest_buf,boundary_tmp);

  }

  /* Pack the low bit patterns into the area just after
   * the queue positions.  We have to round up the source
   * region to a multiple of three words.
   */

  {
   // unsigned int num_tenbits_to_pack = next_low_bits - tempLowBitsArray;
    //unsigned int num_packed_words = (num_tenbits_to_pack + 2) / 3; //ceil((double) num_tenbits_to_pack / 3);
    //unsigned int num_source_words = num_packed_words * 3;


    unsigned int num_tewntybits_to_pack= next_low_bits - tempLowBitsArray;
    unsigned int num_packed_words = (num_tewntybits_to_pack + 2) / 3;
    unsigned int num_source_words = num_packed_words * 3;

    WK_word* endLowBitsArray = tempLowBitsArray + num_source_words;

    /* Pad out the array with zeros to avoid corrupting real packed
       values. */

    for (; /* next_low_bits is already set as desired */
	 next_low_bits < endLowBitsArray;
	 next_low_bits++) {
      *next_low_bits = 0;
    }

#ifdef WK_DEBUG
    printk("about to pack low bits\n");
    printk("num_tewntybits_to_pack is %u\n", num_tewntybits_to_pack);
    printk("endLowBitsArray is %u\n", endLowBitsArray);
#endif

   // boundary_tmp = WK_pack_3_tenbits (tempLowBitsArray,
	//	                      endLowBitsArray,
	//			      boundary_tmp);

	boundary_tmp = WK_pack_3_twentybits (tempLowBitsArray,
		                      endLowBitsArray,
				      boundary_tmp);

    SET_LOW_BITS_AREA_END(dest_buf,boundary_tmp);

  }

  kfree(tempTagsArray);
  kfree(tempQPosArray);
  kfree(tempLowBitsArray);

  return ((char *) boundary_tmp - (char *) dest_buf);
}
