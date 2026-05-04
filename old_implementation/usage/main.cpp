/*
    usage/main.cpp
    Q@khaa.pk
 */

#include <iostream>
#include "./../lib/Corpus/corpus.hh"

#define TEXT "The USA coded, China decoded, and the world watched as the two tech titans turned AGI development into the ultimate game of digital ping-pong.\n\
America, with its Silicon Valley wizards and venture capital potions, churned out algorithms faster than a caffeine-fueled hackathon. Meanwhile, China, with its army of engineers and state-backed supercomputers, reverse-engineered everything like it was a Black Friday sale on intellectual property.\n\
The U.S. shouted, \"Open AI for all!\" while China quietly whispered, \"Open AI for us.\"\n\
Fact: Both nations are pouring billions into AGI, but while the U.S. worries about ethics, China’s already training AI to do tai chi.\n\
Meanwhile, the rest of the world is just hoping the final AGI has a sense of humor—or at least a good off switch.\n"

int main(int argc, char *argv[]) 
{   

    cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char> text_parser(cc_tokenizer::String<char>(TEXT));

    class Corpus vocabulary(cc_tokenizer::String<char>(TEXT)); 

    std::cout<< "Number of lines = " << vocabulary.numberOfLines() << std::endl;
    std::cout<< "Number of tokens = " << vocabulary.numberOfTokens() << std::endl;
    std::cout<< "Number of unique tokens = " << vocabulary.numberOfUniqueTokens() << std::endl;

    // In csv_parser structure, the line and token numbers originate at 1.    
    while (text_parser.go_to_next_line() != cc_tokenizer::string_character_traits<char>::eof())
    {
        while (text_parser.go_to_next_token() != cc_tokenizer::string_character_traits<char>::eof())        
        {
            // In Corpus class, the line and token numbers originate at 1.
            
            /*
                The overloaded function call operator (`vocabulary()`) searches the entire vocabulary for the specified token,
                taking into account both the token itself and its position (line and token number, which are 1-indexed).
                Since the same token can appear at different positions in the corpus, this operator searches redundantly by default,
                considering all occurrences of the token in the text.
                It returns the index(indexed at INDEX_ORIGINATE_AT_VALUE) of the token if found, or `INDEX_NOT_FOUND_AT_VALUE` if the token is not present in the vocabulary.    
             */
            cc_tokenizer::string_character_traits<char>::size_type index = vocabulary(text_parser.get_current_token(), text_parser.get_current_line_number(), text_parser.get_current_token_number());

            if (index != INDEX_NOT_FOUND_AT_VALUE)
            {               
                COMPOSITE_PTR composite = NULL;
                
                try 
                {                   
                    /*
                        If the redundancy flag is not set, the `get_composite_ptr(index)` function will eventually throw an exception. 
                        This is because the Corpus class internally maintains two sets of indices:
                        1. The index of a token in the entire text without redundancy (unique tokens).
                        2. The index of a token in the actual text, which may include redundant occurrences of tokens.
                        (Both of these indices originate at INDEX_ORIGINATE_AT_VALUE.)

                        As we iterate through the entire text using the outer two loops, the `index` value will eventually exceed the range 
                        of the internally maintained non-redundant indices. When this happens, the call to `get_composite_ptr(index)` will 
                        attempt to access an invalid index, resulting in an exception being thrown.

                        This behavior is a safeguard to prevent accessing out-of-bounds data and ensures that the program does not proceed 
                        with invalid or undefined indices. The exception is caught in the `catch` block, where an error message is printed, 
                        and the program gracefully exits. This mechanism is crucial for maintaining the integrity of the Corpus structure 
                        and ensuring that only valid indices are processed.
                     */                            
                    composite = vocabulary.get_composite_ptr(index);
                }
                catch (ala_exception& e)
                {
                    std::cerr<< "main() -> " << e.what() << std::endl;

                    return 0;
                }
                
                if (composite->index != index)
                {
                    std::cout<< vocabulary[composite->index].c_str() << " [" << vocabulary[index].c_str() << "] "; 
                }
            }
        }

        std::cout<< std::endl;
    }



    #ifdef TEST_CORPUS_HH
        ARG arg_corpus, arg_help, arg_verbose, arg_version;

        cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char> argsv_parser(cc_tokenizer::String<char>(COMMAND));

        FIND_ARG(argv, argc, argsv_parser, "h", arg_help);
        if (arg_help.i)
        {
            HELP(argsv_parser, arg_help, ALL);
            HELP_DUMP(argsv_parser, arg_help);

            return 0;
        }

        if (argc < 2)
        {        
            HELP(argsv_parser, arg_help, "help");                
            HELP_DUMP(argsv_parser, arg_help); 

            return 0;                    
        }

        FIND_ARG(argv, argc, argsv_parser, "data", arg_corpus);
        FIND_ARG(argv, argc, argsv_parser, "verbose", arg_verbose);
        FIND_ARG(argv, argc, argsv_parser, "version", arg_version);

        if (!arg_corpus.i) 
        {
            HELP(argsv_parser, arg_corpus, "corpus");                
            HELP_DUMP(argsv_parser, arg_corpus); 

            return 0;       
        }

        cc_tokenizer::String<char> data;

        try 
        {
            data = cc_tokenizer::cooked_read<char>(argv[arg_corpus.i + 1]);
            if (arg_verbose.i)
            {
                std::cout<<"Corpus: "<<argv[arg_corpus.i + 1]<<std::endl;
            }
        }
        catch (ala_exception e)
        {
            std::cout<<e.what()<<std::endl;
            return -1;
        }

        class Corpus corpus(data); 
    
        std::cout<< "Number of lines = " << corpus.numberOfLines() << std::endl;
        std::cout<< "Number of tokens = " << corpus.numberOfTokens() << std::endl;
        std::cout<< "Number of unique tokens = " << corpus.numberOfUniqueTokens() << std::endl;

        for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < corpus.numberOfUniqueTokens(); i++)
        {
            std::cout<< corpus[i + INDEX_ORIGINATES_AT_VALUE].c_str() << " ";
        }
    
        std::cout<< std::endl;

        std::cout<< "Index = " << corpus[cc_tokenizer::String<char>("time")] << std::endl;
    #endif

    #ifdef TEST_SKIP_GRAM_PAIRS_HH

        ARG arg_corpus;

        cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char> argsv_parser(cc_tokenizer::String<char>(COMMAND));

        FIND_ARG(argv, argc, argsv_parser, "data", arg_corpus);

        cc_tokenizer::String<char> data;

        try 
        {
            data = cc_tokenizer::cooked_read<char>(argv[arg_corpus.i + 1]);            
        }
        catch (ala_exception e)
        {
            std::cout<<e.what()<<std::endl;
            
            return -1;
        }

        CORPUS vocab(data);
        SKIPGRAMPAIRS pairs(vocab, false);

        std::cout<< "------------------------------------------------------------------- " << std::endl;
        std::cout<< "Number of lines = " << vocab.numberOfLines() << std::endl;
        std::cout<< "Number of tokens = " << vocab.numberOfTokens() << std::endl;
        std::cout<< "Number of unique tokens = " << vocab.numberOfUniqueTokens() << std::endl;
        std::cout<< "------------------------------------------------------------------- " << std::endl;
                
        while (pairs.go_to_next_word_pair() != cc_tokenizer::string_character_traits<char>::eof())
        {
            WORDPAIRS_PTR pair = pairs.get_current_word_pair();
            
            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < SKIP_GRAM_WINDOW_SIZE; i++)
            {
                /*
                    Try these as well
                    //std::cout<< vocab[pair->Left->array[(SKIP_GRAM_WINDOW_SIZE - 1) - i]].c_str() << " "; 
                    //std::cout<< vocab[pair->getLeft((SKIP_GRAM_WINDOW_SIZE - 1) - i).c_str() << " "; 
                 */
                std::cout<< vocab[(*(pair->getLeft()))[(SKIP_GRAM_WINDOW_SIZE - 1) - i]].c_str() << " ";
            }

            std::cout<< " [ " << vocab[pair->getCenterWord()].c_str() << " ] ";

            for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < SKIP_GRAM_WINDOW_SIZE; i++)
            {
                /*
                    Try these as well
                    //std::cout<< vocab[pair->Right->array[i].c_str() << " "; 
                    //std::cout<< vocab[pair->getRight(i).c_str() << " ";
                 */
                std::cout<< vocab[(*(pair->getRight()))[i]].c_str() << " ";                
            }

            std::cout<< std::endl;
        }        
    #endif
    
    return 0;
}