# Corpus Repository

This repository contains C++ code for managing corpora, including parsing CSV files and organizing text data.

__Description__

This repository implements a C/C++ class by the name of **corpus**. This class processes the textual data. The class includes methods for building the corpus from CSV data, accessing various statistics about the corpus (such as the number of lines, tokens, and unique tokens), and retrieving tokens by index or by string.

The class uses a linked list data structure (COMPOSITE_PTR) to store information about each token in the corpus, including the token itself (str), its index, and the line and token numbers where it appears. Tokens are added to the corpus while parsing CSV data, and the class provides methods for retrieving tokens by their index or string value.

Additionally, the class includes methods for memory management, such as a destructor to deallocate memory used by the linked list when the object is destroyed.

Overall, this class provides a basic framework for working with a corpus of textual data, allowing for efficient storage and retrieval of token information.

### Files
- `composite.hh`: Header file defining structures for composite data.
- `corpus.hh`: Header file implementing a class for corpus management.

### Dependencies
- [ala_exception](https://github.com/KHAAdotPK/ala_exception)
- [allocator](https://github.com/KHAAdotPK/allocator)
- [csv](https://github.com/KHAAdotPK/csv)
- [String](https://github.com/KHAAdotPK/String)


### Usage
To use this code, include the necessary header files in your C++ project and instantiate the `corpus` class with a CSV parser or text data.

```C++
#include <iostream>
#include "lib/corpus/corpus.hh"

#define TEXT "The USA coded, China decoded, and the world watched as the two tech titans turned AGI development into the ultimate game of digital ping-pong.\n\
America, with its Silicon Valley wizards and venture capital potions, churned out algorithms faster than a caffeine-fueled hackathon. Meanwhile, China, with its army of engineers and state-backed supercomputers, reverse-engineered everything like it was a Black Friday sale on intellectual property.\n\
The U.S. shouted, \"Open AI for all!\" while China quietly whispered, \"Open AI for us.\"\n\
Fact: Both nations are pouring billions into AGI, but while the U.S. worries about ethics, China’s already training AI to do tai chi.\n\
Meanwhile, the rest of the world is just hoping the final AGI has a sense of humor—or at least a good off switch.\n"

int main(int argc, char *argv[]) 
{
    corpus vocabulary(cc_tokenizer::String<char>(TEXT));

    std::cout<< "Number of lines = " << vocabulary.numberOfLines() << std::endl;

    /* Redundency is allowed */
    std::cout<< "Number of tokens = " << vocabulary.numberOfTokens() << std::endl;
    /* No redundency is allowed */    
    std::cout<< "Number of unique tokens = " << vocabulary.numberOfUniqueTokens() << std::endl;

    return 0;
}    
```

```C++
#include <iostream>
#include "lib/corpus/corpus.hh"

#define TEXT "The USA coded, China decoded, and the world watched as the two tech titans turned AGI development into the ultimate game of digital ping-pong.\n\
America, with its Silicon Valley wizards and venture capital potions, churned out algorithms faster than a caffeine-fueled hackathon. Meanwhile, China, with its army of engineers and state-backed supercomputers, reverse-engineered everything like it was a Black Friday sale on intellectual property.\n\
The U.S. shouted, \"Open AI for all!\" while China quietly whispered, \"Open AI for us.\"\n\
Fact: Both nations are pouring billions into AGI, but while the U.S. worries about ethics, China’s already training AI to do tai chi.\n\
Meanwhile, the rest of the world is just hoping the final AGI has a sense of humor—or at least a good off switch.\n"

int main(int argc, char *argv[]) 
{   
    cc_tokenizer::csv_parser<cc_tokenizer::String<char>, char> text_parser(cc_tokenizer::String<char>(TEXT));

    class Corpus vocabulary(cc_tokenizer::String<char>(TEXT)); 
    
    // In csv_parser structure, the line and token numbers originate at 1.    
    while (text_parser.go_to_next_line() != cc_tokenizer::string_character_traits<char>::eof())
    {
        while (text_parser.go_to_next_token() != cc_tokenizer::string_character_traits<char>::eof())        
        {
            // In Corpus class, the line and token numbers originate at 1.
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
            }
        }

        std::cout<< std::endl;
    }
}

```

### License
This project is governed by a license, the details of which can be located in the accompanying file named 'LICENSE.' Please refer to this file for comprehensive information.
