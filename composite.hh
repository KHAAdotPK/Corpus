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

/*
     These structs represent a data structure for storing information about tokens (words) in a text corpus.
     The LINETOKENNUMBER struct keeps track of the position (line and token number) of each word occurrence,
     while the COMPOSITE struct stores the unique word itself, its overall frequency, probability, 
     and a linked list of LINETOKENNUMBER structs that provide details about each instance of the word.
 */

#ifndef CORPUS_COMPOSITE_FOR_LM_HH
#define CORPUS_COMPOSITE_FOR_LM_HH

typedef struct line_token_number 
{
    /*
        Index number which also takes into consideration that tokens in a text can be redundent, so each instance of a same word has a unique index
        This index is incremented at each token. 
     */
    /*
        This variable stores an index number that uniquely identifies each token occurrence within a text.
        It considers that tokens can be redundant, meaning the same word can appear multiple times.
        Each instance of the same word receives a unique index, and the index is incremented for each new token encountered
     */
    cc_tokenizer::string_character_traits<char>::size_type index;
    /*
        Line number where the token appears in the text
        Token number within the specific line where it appears
     */
    cc_tokenizer::string_character_traits<char>::size_type l; 
    cc_tokenizer::string_character_traits<char>::size_type t;

    struct line_token_number* next;
    struct line_token_number* prev;
} LINETOKENNUMBER;

typedef LINETOKENNUMBER* LINETOKENNUMBER_PTR;

typedef struct corpus_composite
{   
    /*
        The actual string value of the token/word
     */
    cc_tokenizer::String<char> str;
    /*        
        This variable stores a unique index number assigned to the token/word itself.
        This index is only incremented when a new unique word is encountered, not for each occurrence of the same word
     */     
    cc_tokenizer::string_character_traits<char>::size_type index; 

    /*
        This stores the total number of times the word pointed to by str appears in the text       
     */
    cc_tokenizer::string_character_traits<char>::size_type n_ptr; 
    /*
        This pointer points to the first LINETOKENNUMBER struct in a linked list that stores information about all occurrences 
        of the word represented by str
     */
    LINETOKENNUMBER_PTR ptr;

    /*
        This variable stores the probability of the token/word appearing in the text
     */
    double probability; // Token/word probability   

    struct corpus_composite* next;
    struct corpus_composite* prev;    
} COMPOSITE;

typedef COMPOSITE* COMPOSITE_PTR;

#endif