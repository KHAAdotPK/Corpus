/*
    lib/Corpus/header.hh

    This header file serves as a central inclusion point for all necessary tables created by Parser and provides a unfied interface to access elements to those tables
    
    Maintainer: Sohail.
 */

#ifndef CSV_CORPUS_LIB_CORPUS_HEADER_HH
#define CSV_CORPUS_LIB_CORPUS_HEADER_HH

#include <fstream>

#ifdef MAX_VOCAB_SIZE
#undef MAX_VOCAB_SIZE
#endif
#define MAX_VOCAB_SIZE 20000

#include "./../Parser/header.hh"

#include "./lib/src/Serialisation.hh"
#include "./lib/src/Corpus.hh"

#endif // CSV_CORPUS_LIB_CORPUS_HEADER_HH