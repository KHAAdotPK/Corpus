/*
    corpus/corpus.hh
    Q@khaa.pk
 */

#include <functional>
#include <vector>

#include "../ala_exception/ala_exception.hh"
#include "../allocator/allocator.hh"
#include "../argsv-cpp/lib/parser/parser.hh"
#include "../csv/parser.hh"
#include "../string/src/String.hh"
#include "../sundry/cooked_read.hh"

#include "composite.hh"

#ifndef CORPUS_FOR_LM_HH
#define CORPUS_FOR_LM_HH

#define INDEX_ORIGINATES_AT_VALUE  1
#define INDEX_NOT_FOUND_AT_VALUE   0 

typedef class Corpus 
{
    COMPOSITE_PTR head;

    // Number of lines
    cc_tokenizer::string_character_traits<char>::size_type nl;
    // Number of tokens
    cc_tokenizer::string_character_traits<char>::size_type nt;
    // Unique number of tokens
    cc_tokenizer::string_character_traits<char>::size_type unt;

    void build(cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char> &parser) 
    {
        parser.reset(LINES);
        parser.reset(TOKENS);

        cc_tokenizer::string_character_traits<char>::size_type index = 0;

        // Iterate through each line of the CSV data
        for (cc_tokenizer::string_character_traits<char>::size_type l = 1; l < numberOfLines(); l++)
        {
            /*
                Traverse each line   
             */
            parser.get_line_by_number(l);

            // Iterate through each token on the current line
            for (cc_tokenizer::string_character_traits<char>::size_type t = 1; t <= parser.get_total_number_of_tokens(); t++)
            {
                /*
                    Traverse each token
                 */
                cc_tokenizer::String<char> token = parser.get_token_by_number(t);

                if (head == NULL)
                {
                    head = reinterpret_cast<COMPOSITE_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(COMPOSITE)));
                    head->next = NULL;
                    head->prev = NULL;
                    head->probability = 0.0;
                    head->ptr = reinterpret_cast<LINETOKENNUMBER_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(LINETOKENNUMBER)));                    
                    head->ptr->next = NULL;
                    head->ptr->prev = NULL;

                    head->str = token;
                    head->index = INDEX_ORIGINATES_AT_VALUE;
                    head->n_ptr = 1; 
                    head->ptr->l = l;
                    head->ptr->t = t;

                    nt = nt + 1;
                    unt = unt + 1;

                    index = INDEX_ORIGINATES_AT_VALUE;
                    head->ptr->index = index;
                }
                else 
                {
                   COMPOSITE_PTR current_composite = head;

                   do
                   {                        
                        if (!current_composite->str.compare(token))
                        {                            
                            LINETOKENNUMBER_PTR current_linetoken = current_composite->ptr;

                            while (current_linetoken->next != NULL)
                            {
                                current_linetoken = current_linetoken->next;
                            }

                            // Fill next here, it is old word
                            current_linetoken->next = reinterpret_cast<LINETOKENNUMBER_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(LINETOKENNUMBER)));                    
                            current_linetoken->next->next = NULL;
                            current_linetoken->next->prev = current_linetoken;

                            current_linetoken = current_linetoken->next;
                            current_linetoken->l = l;
                            current_linetoken->t = t;

                            current_composite->n_ptr = current_composite->n_ptr + 1;

                            nt = nt + 1;

                            index = index + 1;
                            current_linetoken->index = index;

                            break;
                        }

                        current_composite = current_composite->next;

                   }  while (current_composite != NULL);

                   if (current_composite == NULL)
                   {
                        current_composite = head;

                        while (current_composite->next != NULL)
                        {
                            current_composite = current_composite->next;  
                        }

                        // Fill composite, it is new word.
                        current_composite->next = reinterpret_cast<COMPOSITE_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(COMPOSITE)));
                        current_composite->next->next = NULL;
                        current_composite->next->prev = current_composite;
                        current_composite->next->ptr = reinterpret_cast<LINETOKENNUMBER_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(LINETOKENNUMBER)));                    
                        current_composite->next->ptr->next = NULL;
                        current_composite->next->ptr->prev = NULL;

                        current_composite->next->str = token;
                        current_composite->next->index = current_composite->index + 1;
                        current_composite->next->probability = 0.0;
                        current_composite->next->n_ptr = 1;
                        current_composite->next->ptr->l = l;
                        current_composite->next->ptr->t = t;

                        nt = nt + 1;
                        unt = unt + 1;

                        index = index + 1; 
                        current_composite->next->ptr->index = index;   
                   }
                }                
            }            
        }

        //std::cout<< "Index = " << index << std::endl;                                
    }
        
    void buildProbablities(void) throw (ala_exception)
    {
        if (head == NULL || nt == 0) 
        {
            throw ala_exception("Corpus::buildProbablities() Error: Vocabulary has not yet been built.");
        }

        COMPOSITE_PTR current_composite = head;
        double sum_probabilities = 0.0; // Sum of probabilities. Used in the process of normalizing frequencies into probabilities.

        do {
             // Calculate token/word frequencies              
             current_composite->probability = static_cast<double>(current_composite->n_ptr) / static_cast<double>(nt);


             sum_probabilities = sum_probabilities + current_composite->probability;

             // Optional stuff.

             //Smoothing
             /*
                There's no single "best" value for the small constant used in smoothing for negative sampling in Skip-gram.
                The choice depends on factors like the size and characteristics of your vocabulary and your desired trade-off between efficiency and accuracy.
                Here's a breakdown of some considerations:

                - Impact of Smoothing:
                1. Raw word frequencies can be highly skewed, with a few very frequent words and many less frequent ones.
                   This can cause issues during negative sampling where very frequent words are almost never chosen.
                2. Smoothing by adding a small constant value to all frequencies helps alleviate this by ensuring a minimum probability for all words.
                   This increases the chance of selecting less frequent words as negative samples.

                - Choosing the Constant Value: There's no universally recommended value. Common practices include...
                1. Values in the range of 10^-5 to 10^-7: These are very small values 
                   that have a minimal impact on the overall probability distribution but help ensure all words have a non-zero chance of being selected.
                2. Heuristics based on vocabulary size: Some approaches scale the constant value based on the vocabulary size
                   (e.g., constant = k / total_words, where k is a small constant). This can be helpful for very large vocabularies.

                --------------------------
                | Experimentation is Key |
                -------------------------- 
                The best approach is often to experiment with different constant values and see how they affect the performance
                of your model. You can monitor metrics like negative sampling efficiency (how often you need to re-sample to find a valid negative word)
                or the overall training convergence of your model.   
              */
             #define CORPUS_PROBABILITY_SMOOTHING_VALUE 1e-5; // 10/(10*10*10*10*10)

             current_composite->probability = current_composite->probability + CORPUS_PROBABILITY_SMOOTHING_VALUE;   
                          
             current_composite = current_composite->next;   
                                 
        } while (current_composite != NULL); 

        // Normalize frequencies into probabilities
        current_composite = head;

        do {

            current_composite->probability = current_composite->probability / sum_probabilities;
            current_composite = current_composite->next;

        } while (current_composite != NULL);

        current_composite = head;

        /*do {

            std::cout<< current_composite->probability << " -- ";

            current_composite = current_composite->next;

        } while (current_composite != NULL); 

        std::cout<< std::endl; */
    }
        
    public:
        Corpus(cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char>& parser) : nt(0), unt(0), head(NULL)
        {            
            nl = parser.get_total_number_of_lines();

            build(parser);

            try 
            {    
                buildProbablities();
            }
            catch (ala_exception& e)
            {
                std::cout<< e.what() << std::endl;

                return;
            }
        }

        Corpus(cc_tokenizer::String<char> &text) : nt(0), unt(0), head(NULL)
        {
            cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char> parser(text);

            nl = parser.get_total_number_of_lines();

            build(parser);
            
            try 
            {    
                buildProbablities();
            }
            catch (ala_exception& e)
            {
                std::cout<< e.what() << std::endl;

                return;
            }
        }

        ~Corpus()
        {
            if (head != NULL)
            {
                COMPOSITE_PTR current_composite = head;

                // Get tail
                while (current_composite->next != NULL) 
                {
                    current_composite = current_composite->next;
                }

                current_composite = current_composite->prev;

                while (current_composite != NULL)                
                {
                    LINETOKENNUMBER_PTR current_linetokennumber = current_composite->ptr;

                    // Get tail
                    while (current_linetokennumber->next != NULL)
                    {
                        current_linetokennumber = current_linetokennumber->next;
                    }

                    current_linetokennumber = current_linetokennumber->prev;

                    while (current_linetokennumber != NULL)
                    {
                        cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current_linetokennumber->next));

                        current_linetokennumber = current_linetokennumber->prev;
                    }

                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current_linetokennumber));

                    cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current_composite->next));

                    current_composite = current_composite->prev;
                }

                cc_tokenizer::allocator<char>().deallocate(reinterpret_cast<char*>(current_composite));
            }
        }

        std::vector<double> getWordProbabilities(void) throw (ala_exception)
        {
            if (head == NULL)
            {
                throw ala_exception("Corpus::getWordProbabilities() Error: Vocabulary has not yet been built.");
            }

            std::vector<double> probabilities;

            COMPOSITE_PTR current_composite = head;

            do {

                probabilities.push_back(current_composite->probability);

                current_composite = current_composite->next;

            } while (current_composite != NULL);

            return probabilities;
        }

        /*
            Originating index is 1
         */
        cc_tokenizer::string_character_traits<char>::size_type numberOfLines(void) const
        {
            return nl;
        }

        cc_tokenizer::string_character_traits<char>::size_type numberOfTokens(void) const
        {
            return nt;
        }

        cc_tokenizer::string_character_traits<char>::size_type numberOfUniqueTokens(void) const
        {
            return unt;
        }

        /*
            Function call operator used to retrieve the token corresponding to a given index.
    
            Parameters:
            @index: The index (enumerated value) of the token in the corpus/vocabulary.
                    This index is used to retrieve the corresponding token.
        
            @redundency: Optional parameter indicating whether redundancy checks should be performed.
                         Defaults to false.
    
            Returns:
            The token corresponding to the provided index.
            If the index is out of range or invalid, an appropriate default sentinal value is returned which is a string literal of size equal to zero.
         */
        cc_tokenizer::String<char> operator()(cc_tokenizer::string_character_traits<char>::size_type index, bool redundency = false)
        {
            if (head == NULL)
            {
                return cc_tokenizer::String<char>();
            }

            if (redundency)            
            {
                if (index > numberOfTokens())
                {
                    return cc_tokenizer::String<char>();
                }

                std::function<cc_tokenizer::String<char>()> func = [&index, this]/* That is called capture list */() -> /*E**/ cc_tokenizer::String<char>
                {
                    COMPOSITE_PTR corpus_composite = head;

                    do {

                        if (corpus_composite->index == index)
                        {
                            return corpus_composite->str;
                        }

                        LINETOKENNUMBER_PTR linetoken_composite = corpus_composite->ptr;

                        while (linetoken_composite != NULL)
                        {
                            if (linetoken_composite->index == index)
                            {
                                return corpus_composite->str;
                            }

                            linetoken_composite = linetoken_composite->next;
                        }

                        corpus_composite = corpus_composite->next;

                    } while (corpus_composite != NULL);

                    return cc_tokenizer::String<char>();
                };

                return func();                                                          
            }
            else
            {
                if (index > numberOfUniqueTokens())
                {
                    return cc_tokenizer::String<char>();
                }

                std::function<cc_tokenizer::String<char>()> func = [&index, this]/* That is called capture list */() -> /*E**/ cc_tokenizer::String<char>
                {
                    COMPOSITE_PTR corpus_composite = head;

                    do {

                        if (corpus_composite->index == index)
                        {
                            return corpus_composite->str;
                        }

                        corpus_composite = corpus_composite->next;

                    } while (corpus_composite != NULL);

                    return cc_tokenizer::String<char>();
                };

                return func();
            }

            return cc_tokenizer::String<char>();
        }

        /* 
            Overloaded subscript operator for retrieving the enumerated token literal.

            Parameters:
            @index: The numerical index corresponding to each word in the corpus/vocabulary.
                    Each word is assigned a unique index value.
                    When a token is found, this index is used to retrieve its literal value.

            Returns: The literal value of the token whose enumerated index matches the value passed 
                     to the overloaded operator as the @index parameter.                     
         */
        cc_tokenizer::String<char> operator[](cc_tokenizer::string_character_traits<char>::size_type index) const
        {
            if (index > numberOfUniqueTokens() || head == NULL)
            {

                return cc_tokenizer::String<char>();
            }

            cc_tokenizer::String<char> ret;
            COMPOSITE_PTR current_composite = head;

            do {

                if (current_composite->index == index)
                {
                    ret = current_composite->str;

                    break;
                }

                current_composite = current_composite->next;

            } while (current_composite != NULL);

            return ret; 
        }

        /*
            Function call operator used to retrieve the index of a given token.
    
            Parameters:
            @str: A reference to a cc_tokenizer::String<char> object representing the token whose index is sought.
    
            Returns:
            The index (enumerated value) of the token if found in the corpus/vocabulary.
            If the token is not found, an appropriate default value is returned, such as sentinel string value with size of zero.
         */
        cc_tokenizer::string_character_traits<char>::size_type operator()(cc_tokenizer::String<char>& str) const 
        {        
            if (head == NULL)
            {
                return INDEX_NOT_FOUND_AT_VALUE;
            }

            COMPOSITE_PTR current_composite = head;

            do {

                if (!current_composite->str.compare(str))
                {
                    return current_composite->index;
                }

                current_composite = current_composite->next;
                
            } while (current_composite != NULL);
            
            return INDEX_NOT_FOUND_AT_VALUE;
        }

        /*
            Overloaded subscript operator for retrieving the index of a given token.
    
            Parameters:
            @str: A reference to a cc_tokenizer::String<char> object representing the token whose index is sought.
    
            Returns:
            The index (enumerated value) of the token if found in the corpus/vocabulary.
            If the token is not found, an appropriate default value(string with size zero) is returned as a sentinel value.
         */
        cc_tokenizer::string_character_traits<char>::size_type operator[](cc_tokenizer::String<char>& str) const 
        {
            if (head == NULL) 
            {
                return 0;
            }

            cc_tokenizer::string_character_traits<char>::size_type ret;
            COMPOSITE_PTR current_composite = head;

            do {

                if (!current_composite->str.compare(str))
                {
                    ret = current_composite->index;

                    break;
                }

                current_composite = current_composite->next;

            } while (current_composite != NULL);

            return ret;         
        }
}CORPUS;

typedef Corpus* CORPUS_PTR;
typedef CORPUS& CORPUS_REF;
typedef const CORPUS& CORPUS_CONST_REF;

#endif
