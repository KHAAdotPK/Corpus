/*
    lib/src/Serialisation.hh
    Q@khaa.pk
 */

#ifndef CORPUS_SERIALISATION_HH
#define CORPUS_SERIALISATION_HH

/*
    Include parser-level structures.
    Explicitly include headers where a file specifically relies on a type
    from another package. 
 */
#include "./../../../Parser/lib/src/WordRecord.hh"

/*
    CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL
    -----------------------------------------
    Controls how often build_hash_table() automatically saves the current
    state of TABLES to disk during corpus processing.

    When the current line number is a non-zero multiple of this value,
    a checkpoint is written automatically. This allows the build to be
    interrupted at any time (power off, crash, Ctrl+C) without losing
    more than CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL lines of work.

    Example: with a value of 10000, checkpoints are written after lines
    10000, 20000, 30000, ... and so on.

    The default value of 10000 is a reasonable balance between:
        - Checkpoint frequency (how much work can be lost)
        - I/O overhead     (writing TABLES to disk takes time)

    Override before including this header if a different interval is needed:

        #define CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL 5000
        #include "lib/Corpus/lib/src/Serialisation.hh"

    Setting this to 0 disables automatic checkpointing entirely.
    Setting this to 1 writes a checkpoint after every single line —
    useful for debugging serialisation but extremely slow in practice.

    CONNECTION TO build_hash_table()
    ---------------------------------
    Inside the per-line loop in build_hash_table(), after all tokens in
    the current line have been processed and line_number has been
    incremented, the following check fires the checkpoint:

        if (CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL > 0 &&
            line_number % CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL == 0)
        {
            save_checkpoint(tables, line_number);
        }

    Note: line_number is 0-based internally. The first checkpoint fires
    after CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL lines have been fully
    processed, i.e. when line_number == PARSER_SERIALIZATION_CHECKPOINT_INTERVAL.
 */
#ifndef CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL
#define CORPUS_SERIALIZATION_CHECKPOINT_INTERVAL 1000
#endif

/*
    CORPUS_SERIALIZATION_CHECKPOINT_FILENAME
    -----------------------------------------
    Base filename for checkpoint files. The current line number is
    appended automatically so each checkpoint is a distinct file:

        corpus_10000.bin
        corpus_20000.bin
        corpus_30000.bin

    Keeping versioned files means a write failure mid-checkpoint never
    destroys the previous good checkpoint — the previous file is untouched
    until the new one is fully written and closed.

    Override before including this header if a different base name is needed:

        #define CORPUS_SERIALIZATION_CHECKPOINT_FILENAME "my_corpus_checkpoint"
        #include "lib/Corpus/lib/src/Serialisation.hh"
 */
#ifndef CORPUS_SERIALIZATION_CHECKPOINT_FILENAME
#define CORPUS_SERIALIZATION_CHECKPOINT_FILENAME "corpus"
#endif

/*
    PARSER_SERIALIZATION_CHECKPOINT_EXTENSION
    ------------------------------------------
    File extension for checkpoint files. Combined with
    CORPUS_SERIALIZATION_CHECKPOINT_FILENAME and the line number to
    produce the full filename:

        corpus_10000.bin
 */
#ifndef CORPUS_SERIALIZATION_CHECKPOINT_EXTENSION
#define CORPUS_SERIALIZATION_CHECKPOINT_EXTENSION ".bin"
#endif

/*
    HEADER
    ------
    magic number        4 bytes  integrity check e.g. 0x54424C53 ("TBLS")
    version             4 bytes  format version for forward compatibility    
    bucket_count        8 bytes  size_t Number of buckets in the hash table used to index tokens encountered during iteration.
    start_bucket        8 bytes  size_t Start bucket_id saved, originate at 1
    bucket_used         8 bytes  size_t Vocabulary size (number of unique tokens). Together with start_bucket, it defines the range of saved buckets (end_bucket = start_bucket + bucket_used - 1).
    total_tokens        8 bytes  size_t Total Number of Tokens (includes redundancy)
    start_line          8 bytes  First line number, originate at 1
    line_count          8 bytes  Number of LINE nodes saved. Together with start_line, it defines the range of saved lines (end_line = start_line + line_count - 1).
    max_tokens_per_line 8 bytes  Maximum number of tokens in any single line
    min_tokens_per_line 8 bytes  Minimum number of tokens in any single line
 */
struct Header
{
    uint32_t magic_number; // Magic number for integrity check e.g. 0x54424C53 ("TBLS")
    uint32_t version; // Format version for forward compatibility
    size_t bucket_count; // Number of buckets in the hash table used to index tokens encountered during iteration.
    size_t start_bucket; // First bucket_id saved
    size_t bucket_used; // Vocabulary size (number of unique tokens). Together with start_bucket, it defines the range of saved buckets (end_bucket = start_bucket + bucket_used - 1).
    size_t total_tokens; // Total Number of Tokens (includes redundancy)
    size_t start_line; // First line number of the corpus
    size_t line_count; // Number of LINE nodes saved. Together with start_line, it defines the range of saved lines (end_line = start_line + line_count - 1).
    size_t max_tokens_per_line; // Maximum number of tokens in any single line
    size_t min_tokens_per_line; // Minimum number of tokens in any single line
};
typedef Header HEADER;

class Serialisation 
{
  public:
    bool write_header(std::ofstream& ofs, const Header& h)
    {
        ofs.write(reinterpret_cast<const char*>(&h.magic_number), sizeof(h.magic_number));
        ofs.write(reinterpret_cast<const char*>(&h.version), sizeof(h.version));
        ofs.write(reinterpret_cast<const char*>(&h.bucket_count), sizeof(h.bucket_count));
        ofs.write(reinterpret_cast<const char*>(&h.start_bucket), sizeof(h.start_bucket));
        ofs.write(reinterpret_cast<const char*>(&h.bucket_used), sizeof(h.bucket_used));
        ofs.write(reinterpret_cast<const char*>(&h.total_tokens), sizeof(h.total_tokens));
        ofs.write(reinterpret_cast<const char*>(&h.start_line), sizeof(h.start_line));
        ofs.write(reinterpret_cast<const char*>(&h.line_count), sizeof(h.line_count));
        ofs.write(reinterpret_cast<const char*>(&h.max_tokens_per_line), sizeof(h.max_tokens_per_line));
        ofs.write(reinterpret_cast<const char*>(&h.min_tokens_per_line), sizeof(h.min_tokens_per_line));

        return ofs.good();
    }

    bool read_header(std::ifstream& ifs, Header& h)
    {
        ifs.read(reinterpret_cast<char*>(&h.magic_number), sizeof(h.magic_number));
        ifs.read(reinterpret_cast<char*>(&h.version), sizeof(h.version));
        ifs.read(reinterpret_cast<char*>(&h.bucket_count), sizeof(h.bucket_count));
        ifs.read(reinterpret_cast<char*>(&h.start_bucket), sizeof(h.start_bucket));
        ifs.read(reinterpret_cast<char*>(&h.bucket_used), sizeof(h.bucket_used));
        ifs.read(reinterpret_cast<char*>(&h.total_tokens), sizeof(h.total_tokens));
        ifs.read(reinterpret_cast<char*>(&h.start_line), sizeof(h.start_line));
        ifs.read(reinterpret_cast<char*>(&h.line_count), sizeof(h.line_count));
        ifs.read(reinterpret_cast<char*>(&h.max_tokens_per_line), sizeof(h.max_tokens_per_line));
        ifs.read(reinterpret_cast<char*>(&h.min_tokens_per_line), sizeof(h.min_tokens_per_line));

        return ifs.good();
    }

    WordRecord** read_hash_to_record_table(std::ifstream& ifs, Header& h)
    {
        WordRecord** hash_to_record_table = new WordRecord*[h.bucket_count];

        for (size_t i = 0; i < h.bucket_count; i++)
        {
            WordRecord* word_record = new WordRecord;

            ifs.read(reinterpret_cast<char*>(&word_record->word_id), sizeof(word_record->word_id));

            if (word_record->word_id != 0)
            {
                size_t word_length;
                ifs.read(reinterpret_cast<char*>(&word_length), sizeof(word_length));
                word_record->word.resize(word_length);

                //std::cout<< "String length = " << word_length << ", ";
                
                ifs.read(&word_record->word[0], word_length); 

                //std::cout<< "String = " << word_record->word << ", ";

                ifs.read(reinterpret_cast<char*>(&word_record->n), sizeof(word_record->n));
                word_record->head = nullptr;

                hash_to_record_table[i] = word_record;

                if (word_record->n > 0)
                {
                    size_t line = 0;
                    size_t token = 0;

                    OccurrenceNode* occurrence_tail = nullptr;

                    for (size_t j = 0; j < word_record->n; j++)
                    {
                        ifs.read(reinterpret_cast<char*>(&line), sizeof(line));
                        ifs.read(reinterpret_cast<char*>(&token), sizeof(token));

                        OccurrenceNode* occurrence = new OccurrenceNode(line, token, nullptr, occurrence_tail);

                        if (word_record->head == nullptr)
                        {
                            word_record->head = occurrence;
                            occurrence_tail = occurrence;                            
                        }
                        else
                        {
                            occurrence_tail->next = occurrence;
                            occurrence_tail = occurrence;
                        }
                    }
                }
            }
            else
            {
                hash_to_record_table[i] = nullptr;
            }
        }

        /*for (size_t i = 0; i < h.bucket_count; i++)
        {
            WordRecord* word_record = hash_to_record_table[i];
            
            if (word_record != nullptr)
            {
                std::cout<< "\t word_id: " << word_record->get_word_id() << " word: " << word_record->get_word() << " word size: " << word_record->get_word().size() << " n: " << word_record->n << std::endl;
            }
            else
            {
                std::cout<< "nullptr" << std::endl;
            }            
        }*/

        return hash_to_record_table;
    }
    
    /*
        Index table is a dynamic array of size bucket_used, each element of which contains a pointer to the first token in the vocabulary whose hash value falls into that bucket.
     */
    size_t* read_index_to_hash_table(std::ifstream& ifs, Header& h)
    {
        size_t* index_table = nullptr;

        try
        {
            index_table = new size_t[h.bucket_count];
            if (!index_table)
            {
                throw std::runtime_error("Serialisation::read_index_to_hash_table(std::ifstream&, Header&) Error: failed to allocate memory for index table");
            }

            ifs.read(reinterpret_cast<char*>(index_table), sizeof(size_t) * h.bucket_used);            
        }
        catch (const std::exception& e)
        {
            delete[] index_table;

            throw std::runtime_error("Serialisation::read_index_to_hash_table(std::ifstream&, Header&) Error: " + std::string(e.what()));
        }

        return index_table;
    }

    /*
        Linked list of LINE nodes and linked list of TOKEN nodes for each LINE node
     */
    LINE* read_lines(std::ifstream& ifs, Header& h)
    {        
        //size_t n = h.start_line + h.line_count;

        size_t n = h.line_count - h.start_line;
                
        LINE* line_head = nullptr;
        LINE* line_tail = nullptr;
        
        for (size_t i = 0; i < n; i++)
        {                        
            if (line_head == nullptr)
            {
                line_head = new LINE;
                line_tail = line_head;                
            }
            else
            {
                line_tail->next = new LINE;
                line_tail = line_tail->next;
            }

            line_tail->next = nullptr;

            line_tail->tokens = nullptr;
            TOKEN* token_tail = nullptr;
            
            ifs.read(reinterpret_cast<char*>(&line_tail->n), sizeof(line_tail->n));
            
            //std::cout<< "i: " << i << " n: " << line_tail->n << std::endl;
            
            for (size_t j = 0; j < line_tail->n; j++)
            {
                if (line_tail->tokens == nullptr)
                {
                    line_tail->tokens = new TOKEN;
                    token_tail = line_tail->tokens;                    
                }
                else
                {
                    token_tail->next = new TOKEN;
                    token_tail = token_tail->next;
                }

                token_tail->next = nullptr;

                ifs.read(reinterpret_cast<char*>(&token_tail->token_id), sizeof(token_tail->token_id));

                token_tail->next = nullptr;

                token_tail->occurrence = nullptr;

                /*token_tail->occurrence = new OccurrenceNode;
                ifs.read(reinterpret_cast<char*>(&token_tail->occurrence->line), sizeof(token_tail->occurrence->line));
                ifs.read(reinterpret_cast<char*>(&token_tail->occurrence->token), sizeof(token_tail->occurrence->token));*/

                //std::cout<< "\t j: " << j << " token_id: " << token_tail->token_id << " line_number: " << token_tail->occurrence->line << " token_number: " << token_tail->occurrence->token << std::endl;
            }
        }
        
        return line_head;
    }

    TABLES* read_tables(const std::string& filepath) 
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);
        if (!ifs)
        {
            throw std::runtime_error("Corpus::read_tables(const std::string&) Error: failed to open file for reading"); 
        } 
        
        Header h = Header();
        if (!read_header(ifs, h))
        {
            throw std::runtime_error("Corpus::read_tables(const std::string&) Error: failed to read header");
        }
        
        // Check magic number
        if (h.magic_number != 0x54424C53)
        {
            throw std::runtime_error("Corpus::read_tables(const std::string&) Error: invalid magic number");
        }

        // Print header info
        std::cout << "Magic number: " << std::hex << std::showbase << std::uppercase << h.magic_number << std::endl;

        // Reset to Decimal and turn off prefix for everything else
        std::cout << std::dec << std::noshowbase;

        std::cout<< "Version: " << h.version << std::endl;
        std::cout<< "Bucket count: " << h.bucket_count << std::endl;
        std::cout<< "Start bucket: " << h.start_bucket << std::endl;
        std::cout<< "Bucket used: " << h.bucket_used << std::endl;
        std::cout<< "Total tokens: " << h.total_tokens << std::endl;
        std::cout<< "Start line: " << h.start_line << std::endl;
        std::cout<< "Line count: " << h.line_count << std::endl;
        std::cout<< "Max tokens per line: " << h.max_tokens_per_line << std::endl;
        std::cout<< "Min tokens per line: " << h.min_tokens_per_line << std::endl;

        size_t* index_table = read_index_to_hash_table(ifs, h);

        std::cout<< "Finished reading index table" << std::endl;

        /*for (size_t i = 0; i < h.bucket_used; i++)
        {
            std::cout<< "Bucket " << i << ": " << index_table[i] << std::endl;
        }*/

        LINE* line_head = read_lines(ifs, h);

        std::cout<< "Finished reading lines" << std::endl;

        /*LINE* line_tail = line_head;
        while (line_tail != nullptr)
        {
            std::cout<< line_tail->n << ": ";
            TOKEN* token_tail = line_tail->tokens;
            while (token_tail != nullptr)
            {
                std::cout<< token_tail->token_id << ", ";
                token_tail = token_tail->next;
            }

            std::cout<< std::endl;

            line_tail = line_tail->next;
        }*/

        WordRecord** hash_to_record_table = read_hash_to_record_table(ifs, h);

        std::cout<< "Finished reading tables" << std::endl;

        //delete[] index_table;

        TABLES* tables = new TABLES(); /*nullptr*/

        tables->hash_to_word_record = hash_to_record_table;
        tables->word_id_to_hash = index_table;
        tables->lines = line_head;
        tables->bucket_count = h.bucket_count;
        tables->bucket_used = h.bucket_used;
        tables->maximum_tokens_per_line = h.max_tokens_per_line;
        tables->minimum_tokens_per_line = h.min_tokens_per_line;
        tables->total_tokens = h.total_tokens;

        std::cout<< "DONE" << std::endl;

        return tables;;
    }

/*
      save_corpus()
      -------------
      Serialises the given TABLES object to the specified file path.
    
      On success:
        - Returns true
        - Writes a binary header + word records + line/token list
        - Follows the format documented in SERIALISATION.md
        - Sets magic_number = 0x54424C53 ('TBLS')
        - Updates bucket_count, bucket_used, total_tokens, etc.
      
      On failure (any reason):
        - Returns false
        - Leaves the file in whatever state it reached (or empty)
        - Performs no partial writes (atomic-like on success)
      
      Caller responsibility:
        - The TABLES pointer must be valid (non-null) for the duration
          of the call.
        - Tables is NOT copied; the caller keeps ownership.
        - The caller must NOT delete TABLES while the file exists,
          as the file is a snapshot of that TABLES instance.
  */
  void save_tables(TABLES* tables, size_t starting_buket, size_t starting_line, size_t ending_line, const std::string& filepath)
  {
      if (tables == nullptr)
      {
        throw std::runtime_error("Corpus::save_tables(TABLES*, const std::string&) Error: tables is nullptr"); 
      }

      std::ofstream ofs(filepath, std::ios::binary | std::ios::trunc);
      if (!ofs)
      {
          throw std::runtime_error("Corpus::save_tables(TABLES*, const std::string&) Error: failed to open file for writing"); 
      }

      //std::cout << "Starting Line = " << starting_line << std::endl; 

      Header h = {
          .magic_number = 0x54424C53,
          .version = 1,
          .bucket_count = tables->get_bucket_count(),
          .start_bucket = starting_buket,
          .bucket_used = tables->get_bucket_used(),
          .total_tokens = tables->get_total_tokens(),
          .start_line = starting_line,
          .line_count = ending_line,
          .max_tokens_per_line = tables->get_maximum_tokens_per_line(),
          .min_tokens_per_line = tables->get_minimum_tokens_per_line()
      };

      if (!write_header(ofs, h))
      {
          throw std::runtime_error("Corpus::save_tables(TABLES*, const std::string&) Error: failed to write header"); 
      }

      // Save index table
      size_t* index_table = tables->word_id_to_hash;
      for (size_t i = 0; i < tables->get_bucket_used(); i++)
      {
          ofs.write(reinterpret_cast<const char*>(&index_table[i]), sizeof(index_table[i]));
      }

      // Save lines
      LINE* line = tables->lines;
      while (line != nullptr)
      {
          ofs.write(reinterpret_cast<const char*>(&line->n), sizeof(line->n));
          Token* token = line->tokens;
          while (token != nullptr)
          {
              ofs.write(reinterpret_cast<const char*>(&token->token_id), sizeof(token->token_id));
              /*ofs.write(reinterpret_cast<const char*>(&token->occurrence->line), sizeof(token->occurrence->line));
              ofs.write(reinterpret_cast<const char*>(&token->occurrence->token), sizeof(token->occurrence->token));*/
              token = token->next;
          }
                
          line = line->next;
      }

      // Save hash table
      WordRecord** hash_to_word_record = tables->hash_to_word_record;
      for (size_t i = 0; i < tables->get_bucket_count(); i++)
      {
          //std::cout<< hash_to_word_record[i]<< std::endl; 

          WordRecord* word_record = hash_to_word_record[i];
          if (word_record != nullptr)
          {
              //std::cout<< "\t word_id: " << word_record->get_word_id() << " word: " << word_record->get_word() << " word size: " << word_record->get_word().size() << " n: " << word_record->n << std::endl;
              size_t word_id = word_record->get_word_id();
              ofs.write(reinterpret_cast<const char*>(&word_id), sizeof(word_id));
              size_t word_length = word_record->get_word().size();
              ofs.write(reinterpret_cast<const char*>(&word_length), sizeof(word_length));              
              ofs.write(word_record->get_word().c_str(), word_length);
              ofs.write(reinterpret_cast<const char*>(&word_record->n), sizeof(word_record->n));
              // Save OccurrenceNodes for this WordRecord
              OccurrenceNode* occurrence = word_record->head;
              while (occurrence != nullptr)
              {
                  ofs.write(reinterpret_cast<const char*>(&occurrence->line), sizeof(occurrence->line));
                  ofs.write(reinterpret_cast<const char*>(&occurrence->token), sizeof(occurrence->token));
                  occurrence = occurrence->next;
              }
          }
          else
          {
              size_t word_id = 0;
              ofs.write(reinterpret_cast<const char*>(&word_id), sizeof(word_id));                
          }
      }

      ofs.close();
  }

/*
    load_corpus()
    -------------
    Attempts to load a corpus from the specified file path into a
    newly-allocated TABLES structure.
    
    On success:
      - Returns a non-null pointer to a new TABLES object
      - Validates the magic number and version
      - Reconstructs:
            - word_id_to_hash
            - hash_to_word_record
            - word_id, n, word_bytes, word_length for each WordRecord
            - OccurrenceNodes (line, token) in preserved order
            - LINES list (line_count nodes)
            - Metadata fields (bucket_count, bucket_used, total_tokens,
              max_tokens_per_line, min_tokens_per_line)
      
      - Sets:
            - ref_count = 1
            - word_record->ref_count = 1 per record
            
      - The caller becomes the owner of the returned TABLES pointer
        and is responsible for calling delete_tables(tables) when done.
    
    On failure (any reason):
      - Returns nullptr
      - File is NOT modified
      - No memory is leaked; resources are cleaned up before returning
      
    Failure conditions:
      - File does not exist
      - Magic number does not match 0x54424C53
      - Version number is unsupported (currently exactly 1)
      - File is too small to contain header
      - Any read operation fails
      - Data corruption detected (e.g., out-of-bounds index, negative count)
      - Allocation failure (returns nullptr without leaking)
*/
/*TABLES* load_corpus(const std::string& filepath)
{

  return nullptr;
}*/

/*
    delete_tables()
    ---------------
    Deallocates a TABLES object and all associated memory.
    
    This is the **only safe way** to release a TABLES pointer returned
    by load_corpus().
    
    Operations:
      - Traverses the LINES list and deletes all LINE nodes
      - Traverses each WordRecord list and deletes all OccurrenceNodes
      - Deletes each WordRecord object
      - Deletes word_id_to_hash array
      - Deletes hash_to_word_record array
      - Sets the passed pointer to nullptr to prevent dangling pointer use
*/
/*void delete_tables(TABLES*& tables)
{

}*/

/*
    TESTS
    -----
    All tests should be placed in lib/Corpus/test.cc
    - Save a populated TABLES object to a binary file
    - Load it back into a new TABLES object
    - Verify that:
      * The header matches
      * The word_id_to_hash mapping is correct
      * Each WordRecord has the same n, word_bytes, word_length
      * The occurrence_count matches
      * OccurrenceNodes are present in the same line/token order
      * The LINES list is reconstructible
      * All memory is cleaned up without leaks
    - Test edge cases:
      * Empty corpus (zero tokens, zero lines)
      * Single-word corpus
      * File with invalid magic number
      * File with incorrect version
      * File that is too small
      * File that is corrupted mid-stream
*/

};

#endif // CORPUS_SERIALISATION_HH  