/*
    corpus/composite.hh
    Q@khaa.pk
 */

#include "../ala_exception/ala_exception.hh"
#include "../allocator/allocator.hh"
#include "../argsv-cpp/lib/parser/parser.hh"
#include "../csv/parser.hh"
#include "../string/src/String.hh"
#include "../sundry/cooked_read.hh"

#ifndef CORPUS_COMPOSITE_FOR_LM_HH
#define CORPUS_COMPOSITE_FOR_LM_HH

typedef struct line_token_number 
{
    cc_tokenizer::string_character_traits<char>::size_type index;
    cc_tokenizer::string_character_traits<char>::size_type l;
    cc_tokenizer::string_character_traits<char>::size_type t;

    struct line_token_number* next;
    struct line_token_number* prev;
} LINETOKENNUMBER;

typedef LINETOKENNUMBER* LINETOKENNUMBER_PTR;

typedef struct corpus_composite
{   
    cc_tokenizer::String<char> str;     
    cc_tokenizer::string_character_traits<char>::size_type index;

    cc_tokenizer::string_character_traits<char>::size_type n_ptr; // Number of instances of this word, it is one more than the number of links pointed by "ptr"
    LINETOKENNUMBER_PTR ptr;

    double probability; // Token/word probability   

    struct corpus_composite* next;
    struct corpus_composite* prev;    
} COMPOSITE;

typedef COMPOSITE* COMPOSITE_PTR;

#endif