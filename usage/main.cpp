/*
    corpus/example/main.cpp
    Q@khaa.pk
 */

#include "main.hh"

int main(int argc, char *argv[]) 
{
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