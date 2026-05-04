/*
    lib/src/Corpus.hh
    Q@khaa.pk
 */

#ifndef CORPUS_FOR_LM_HH
#define CORPUS_FOR_LM_HH

class Corpus
{
    private:
        TABLES* tables;

    public:

        /*
            Constructor
            -----------
            Accepts a raw pointer to a pre-built TABLES object.
            Does not take ownership — caller retains responsibility
            for the lifetime of TABLES.
            Accepts nullptr safely — all methods guard against it.
         */
        Corpus(TABLES* t) : tables(t) {}

        /*
            frequency()
            -----------
            Returns the number of times word_id appears in the corpus.
            This is WordRecord::n, accumulated during build_hash_table()
            in O(1) — no list traversal.

            Returns 0 if:
                - tables is nullptr
                - word_id is out of range  (>= bucket_used)
                - the bucket resolves to nullptr (should never happen
                  under normal TABLES invariants, but guarded anyway)

            IMPORTANT: word_id must be in [0, bucket_used).
            Passing a raw hash key instead of a word_id is a logic
            error — word_id_to_hash translates word_id to bucket key.
         */
        size_t frequency(size_t word_id) const
        {
            if (tables == nullptr)
            {
                return 0;
            }

            if (word_id >= tables->get_bucket_used())
            {
                return 0;
            }

            size_t h = tables->word_id_to_hash[word_id];

            if (h >= tables->get_bucket_count())
            {
                /*
                    index_table[word_id] should always be a valid bucket
                    index. If it is not, TABLES is in a corrupt state.
                    Return 0 rather than indexing out of bounds.
                 */
                return 0;
            }

            WordRecord* wr = tables->hash_to_word_record[h];

            if (wr == nullptr)
            {
                /*
                    word_id_to_hash[word_id] pointed to an empty bucket.
                    This should never happen under correct TABLES invariants
                    but is guarded here defensively.
                 */
                return 0;
            }

            return wr->n;
        }

        /*
            probability()
            -------------
            Returns the probability of word_id occurring in the corpus:

                P(word) = WordRecord::n / TABLES::total_tokens

            This is an O(1) operation — both values are pre-computed
            during build_hash_table().

            Returns 0.0 if:
                - tables is nullptr
                - word_id is out of range  (>= bucket_used)
                - total_tokens is 0  (empty corpus guard, prevents div/0)
                - the bucket resolves to nullptr

            The return value is in [0.0, 1.0] when TABLES invariants hold.
         */
        double probability(size_t word_id) const
        {
            if (tables == nullptr)
            {
                return 0.0;
            }

            if (word_id >= tables->get_bucket_used())
            {
                return 0.0;
            }

            if (tables->get_total_tokens() == 0)
            {
                /*
                    Empty corpus guard. Prevents division by zero.
                    Should never happen if build_hash_table() ran on a
                    non-empty file, but guarded defensively.
                 */
                return 0.0;
            }

            size_t h = tables->word_id_to_hash[word_id];

            if (h >= tables->get_bucket_count())
            {
                return 0.0;
            }

            WordRecord* wr = tables->hash_to_word_record[h];

            if (wr == nullptr)
            {
                return 0.0;
            }

            return (double)wr->n / (double)tables->get_total_tokens();
        }

        /*
            is_valid()
            ----------
            Returns true if the Corpus object is in a usable state —
            tables is non-null, non-empty, and has at least one token.
            Useful as a pre-flight check before calling frequency()
            or probability() in a loop.
         */
        bool is_valid() const
        {
            return tables != nullptr
                && tables->get_bucket_used() > 0
                && tables->get_total_tokens() > 0;
        }
};

#endif // CORPUS_FOR_LM_HH