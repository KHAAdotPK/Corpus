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

#define TEXT "giggles fill the air friends skipping hand in hand endless smiles blooming"

int main(int argc, char *argv[]) 
{
    corpus vocab(cc_tokenizer::String<char>(TEXT));

    std::cout<< "Number of lines = " << vocab.numberOfLines() << std::endl;

    /* Redundency is allowed */
    std::cout<< "Number of tokens = " << vocab.numberOfTokens() << std::endl;
    /* No redundency is allowed */    
    std::cout<< "Number of unique tokens = " << vocab.numberOfUniqueTokens() << std::endl;

    return 0;
}    
```

### License
This project is governed by a license, the details of which can be located in the accompanying file named 'LICENSE.' Please refer to this file for comprehensive information.
