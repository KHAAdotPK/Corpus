#### Example code, showing how to use skip gram pairs class.
```C++
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
SKIPGRAMPAIRS pairs(vocab);

while (pairs.go_to_next_word_pair() != cc_tokenizer::string_character_traits<char>::eof())
{
    WORDPAIRS_PTR pair = pairs.get_current_word_pair();
            
    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < SKIP_GRAM_WINDOW_SIZE; i++)
    {
        std::cout<< vocab[pair->left->array[(SKIP_GRAM_WINDOW_SIZE - 1) - i]].c_str() << " "; 
    }

    std::cout<< " [ " << vocab[pair->centerWord].c_str() << " ] ";

    for (cc_tokenizer::string_character_traits<char>::size_type i = 0; i < SKIP_GRAM_WINDOW_SIZE; i++)
    {
        std::cout<< vocab[pair->right->array[i]].c_str() << " ";                
    }

    std::cout<< std::endl;
}        
```