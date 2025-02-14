/*
    corpus/corpus.hh
    Q@khaa.pk
 */

#include <functional>
#include <vector>

#include "../ala_exception/ala_exception.hh"
#include "../allocator/allocator.hh"
#include "../csv/parser.hh"
#include "../string/src/String.hh"

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

    void build(Corpus& ref) throw (ala_exception)
    {
        if (!(head == NULL))
        {
            return;
        }    

        COMPOSITE_PTR current_composite = ref.head, local_current_composite = NULL;

        while (current_composite != NULL)
        {             
            if (head == NULL)
            {
                try 
                {
                    head = reinterpret_cast<COMPOSITE_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(COMPOSITE)));
                }
                catch (std::bad_alloc& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Corpus::build() Error: ") + cc_tokenizer::String<char>(e.what()));
                }
                catch (std::length_error& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Corpus::build() Error: ") + cc_tokenizer::String<char>(e.what()));    
                }

                head->prev = NULL;
                local_current_composite = head;                                
            }
            else 
            {               
                 try 
                {
                    local_current_composite->next = reinterpret_cast<COMPOSITE_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(COMPOSITE)));
                }
                catch (std::bad_alloc& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Corpus::build() Error: ") + cc_tokenizer::String<char>(e.what()));
                }
                catch (std::length_error& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Corpus::build() Error: ") + cc_tokenizer::String<char>(e.what()));    
                }

                local_current_composite->next->prev = local_current_composite;
                local_current_composite = local_current_composite->next;                
            }

            // Fill local_current_composite
            local_current_composite->index = current_composite->index;
            local_current_composite->next = NULL;
            local_current_composite->probability = current_composite->probability;
            local_current_composite->n_ptr = current_composite->n_ptr;
            local_current_composite->ptr = NULL;
            local_current_composite->str = current_composite->str;

            LINETOKENNUMBER_PTR current_linetoken = current_composite->ptr, local_current_linetoken;
            while (current_linetoken != NULL) 
            {   
                try
                {   if (local_current_composite->ptr == NULL)
                    {
                        local_current_linetoken = reinterpret_cast<LINETOKENNUMBER_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(LINETOKENNUMBER)));
                        local_current_composite->ptr = local_current_linetoken;

                        local_current_linetoken->prev = NULL;
                    }
                    else
                    {
                        local_current_linetoken->next = reinterpret_cast<LINETOKENNUMBER_PTR>(cc_tokenizer::allocator<char>().allocate(sizeof(LINETOKENNUMBER)));
                        local_current_linetoken->next->prev = local_current_linetoken;

                        local_current_linetoken = local_current_linetoken->next;
                    }
                }
                catch (std::bad_alloc& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Corpus::build() Error: ") + cc_tokenizer::String<char>(e.what()));
                }
                catch (std::length_error& e)
                {
                    throw ala_exception(cc_tokenizer::String<char>("Corpus::build() Error: ") + cc_tokenizer::String<char>(e.what()));    
                }

                local_current_linetoken->next = NULL;
                local_current_linetoken->index = current_linetoken->index;
                local_current_linetoken->l = current_linetoken->l;
                local_current_linetoken->t = current_linetoken->t;
                                                                                         
                current_linetoken = current_linetoken->next;
            }
            
            current_composite = current_composite->next;
        }
    }

    void build(cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char> &parser) 
    {
        parser.reset(LINES);
        parser.reset(TOKENS);
        
        cc_tokenizer::string_character_traits<char>::size_type index = 0;

        // Iterate through each line of the CSV data
        /*
            Changed followinf for loop, from l < numberOfLines() to l <= numberOfLines(). The prvious version of this expression left out the last line of the corpus
         */
        for (cc_tokenizer::string_character_traits<char>::size_type l = 1; l <= numberOfLines(); l++)
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

                //std::cout<< token.c_str() << std::endl;

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
                    // Incremented and assigned to each unique token
                    head->index = INDEX_ORIGINATES_AT_VALUE; 
                    head->n_ptr = 1; 
                    head->ptr->l = l;
                    head->ptr->t = t;

                    nt = nt + 1;
                    unt = unt + 1;

                    // Incremented for each token and then...
                    index = INDEX_ORIGINATES_AT_VALUE; 
                    // assigned to that token
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

                            // Reach last link
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

            //std::cout<< std::endl;
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

        Corpus() : nl(0), nt(0), unt(0), head(NULL)
        {

        }

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

        Corpus& operator= (Corpus& ref)
        {
            if (&ref == this)
            {
                return *this;
            }

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

            head = NULL;
            nl = ref.nl;
            nt = ref.nt;
            unt = ref.unt;
            
            build(ref);

            /*head = ref.head;
            nl = ref.nl;
            nt = ref.nt;
            unt = ref.unt;*/

            return *this;
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
            Keep in mind that internally, l and t both originate at INDEX_ORIGINATE_AT_VALUE, and index (returned by this function) originates at (vocabulary index(which originates at 0) + INDEX_ORIGINATE_AT_VALUE).

            Overloads the function call operator to search for a token at a specific line and token position.
            It iterates through the linked list of corpus_composite nodes to find the token.
            Once the token is located, it searches through the linked list of line_token_number nodes to find the exact occurrence at the specified line and token position.
            If no matching token is found, it returns a predefined constant indicating that the token was not found.
            
            @str: The token (word) to search for.
            @l: Line number where the token is expected. Originates at 1.
            @t: Token number within the line. Originates at 1.

            Returns: The index of the token if found.
                     INDEX_NOT_FOUND_AT_VALUE if the token is not found.
         */
        cc_tokenizer::string_character_traits<char>::size_type operator() (cc_tokenizer::String<char>& str, cc_tokenizer::string_character_traits<char>::size_type l, cc_tokenizer::string_character_traits<char>::size_type t)
        {
            if (head == NULL)
            {
                return INDEX_NOT_FOUND_AT_VALUE;
            }

            COMPOSITE_PTR current_composite = head;
            LINETOKENNUMBER_PTR linetokennumber = NULL;

            do 
            {                
                if (!current_composite->str.compare(str))
                {
                    linetokennumber = current_composite->ptr;
                    break;
                }

                current_composite = current_composite->next;                
            } 
            while (current_composite != NULL);

            while (linetokennumber) 
            {
                if (linetokennumber->l == l && linetokennumber->t == t)
                {
                    return linetokennumber->index;
                }

                linetokennumber = linetokennumber->next;
            }
            
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
                return INDEX_NOT_FOUND_AT_VALUE;
            }

            cc_tokenizer::string_character_traits<char>::size_type ret = INDEX_NOT_FOUND_AT_VALUE;
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

        cc_tokenizer::string_character_traits<char>::size_type get_number_of_lines(void)
        {
            return nl;
        }

        /*
            Retrieves a pointer to a line_token_number node based on the specified index from a given corpus_composite structure.
            It starts with the first node in the linked list (ptr->ptr).
            If the node's index does not match, it traverses the linked list until a match is found or the end of the list is reached.

            @ptr: Pointer to a corpus_composite structure.
            @i: Index of the desired token occurrence. The index originates at INDEX_ORIGINATE_AT_VALUE

            Throws:
            ala_exception if the provided corpus_composite pointer or the resulting line_token_number pointer is NULL.
         */
        LINETOKENNUMBER_PTR get_line_token_number(COMPOSITE_PTR ptr, cc_tokenizer::string_character_traits<char>::size_type i) throw (ala_exception)
        {
            if (ptr == NULL)
            {
                throw ala_exception("Corpus::get_line_token_number() Error: Composite pointer is NULL.");
            }
            
            LINETOKENNUMBER_PTR linetokennumber_ptr = ptr->ptr;

            if (linetokennumber_ptr->index != i)
            {
                linetokennumber_ptr = linetokennumber_ptr->next;

                while (linetokennumber_ptr != NULL)
                {
                    if (linetokennumber_ptr->index == i)
                    {                        
                        break;
                    }

                    linetokennumber_ptr = linetokennumber_ptr->next;
                }
            }

            if (linetokennumber_ptr == NULL)
            {
                throw ala_exception("Corpus::get_line_token_number() Error: Line token number pointer is NULL.");
            }
            
            return linetokennumber_ptr;
        }

        /*
            Retrieves a pointer to a corpus_composite structure based on the provided index. 
            It handles both unique tokens and redundant occurrences

            @index: Index of the token to search for. Index originate at INDEX_ORIGINATE_AT_VALUE
            @redundency: Boolean flag indicating whether to consider redundant occurrences (true) or only unique tokens (false),
                        If redundency is false, it searches for the unique token with the specified index.
                        If redundency is true, it iterates through occurrences of the token in linked lists until the specified index is found

            Throws:
            ala_exception if no matching corpus_composite is found or if the list is empty.                    
         */
        COMPOSITE_PTR get_composite_ptr (cc_tokenizer::string_character_traits<char>::size_type index, bool redundency = false)  throw (ala_exception)
        {
            COMPOSITE_PTR ret = NULL;

            cc_tokenizer::string_character_traits<char>::size_type total_number_of_tokens = 0;

            if (!redundency)
            {
                total_number_of_tokens = numberOfUniqueTokens();
            }
            else
            {
                total_number_of_tokens = numberOfTokens();
            }

            if (!(index > total_number_of_tokens || head == NULL))
            {                                        
                COMPOSITE_PTR current_composite = head;

                do
                {
                    if (!redundency)
                    {
                        if (current_composite->index == index)
                        {
                            ret = current_composite;

                            break;
                        } 
                    }
                    else
                    {   
                        cc_tokenizer::string_character_traits<char>::size_type i = current_composite->n_ptr;

                        LINETOKENNUMBER_PTR linetokennumber_ptr = current_composite->ptr;

                        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < current_composite->n_ptr; i++)
                        {
                            if (linetokennumber_ptr->index == index)
                            {
                                ret = current_composite;

                                break;
                            }

                            linetokennumber_ptr = linetokennumber_ptr->next;
                        }

                        if (ret != NULL)
                        {
                            break;
                        }                    
                    } 

                    current_composite = current_composite->next;
                    
                } while (current_composite != NULL);
            }

            if (ret == NULL)
            {
                throw ala_exception("Corpus::get_composite_ptr() Error: Composite pointer is NULL.");
            }

            return ret; 
        }
}CORPUS;

typedef Corpus* CORPUS_PTR;
typedef CORPUS& CORPUS_REF;
typedef const CORPUS& CORPUS_CONST_REF;

#endif
