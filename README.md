# Corpus — Vocabulary Query Interface

[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/status-rewrite%20in%20progress-orange)]()

---

## What Is This Package

`Corpus` is a lightweight query interface that sits in front of a pre-built `TABLES`
object and provides clean, safe, O(1) access to vocabulary statistics needed by
downstream NLP components — primarily `Pairs` and the Skip-gram training loop.

Its current interface is intentionally minimal. Two methods exist because two things
are needed right now:

```
frequency(word_id)    → size_t    how many times this word appears in the corpus
probability(word_id)  → double    frequency / total_tokens
```

Both are O(1). No list traversal. No string comparison. No corpus re-reading.
As the ML model implementation grows, this package will grow with it — driven by
what the model actually needs, not by speculation.

---

## Role in the Pipeline

```
corpus file
    │
    ▼
Parser::build_hash_table()     ← one pass, builds TABLES
    │
    ▼
TABLES*                        ← vocabulary + corpus layout, fully in memory
    │
    ├──▶ Corpus(TABLES*)       ← this package — vocabulary query interface
    │        frequency()
    │        probability()
    │        is_valid()
    │
    └──▶ Pairs(TABLES*)        ← training pair generator (see Pairs package)
```

`Corpus` and `Pairs` are developed together. Both take `TABLES*` as their input.
Neither owns `TABLES` — the caller retains ownership and is responsible for its
lifetime.

---

## Why the Old Implementation Is Being Replaced

The old implementation lives in `old_implementation/`. It is preserved for reference.

### What the Old `Corpus` Was

The old `Corpus` class (`corpus.hh` + `composite.hh`) was a self-contained vocabulary
builder. It owned its own parser (`cc_tokenizer::csv_parser`), built its own linked
list of `COMPOSITE` nodes from scratch, and managed its own occurrence tracking via
`LINETOKENNUMBER` nodes. It was, in effect, doing the job that
`Parser::build_hash_table()` now does — but more slowly, with string-based lookups,
and without producing a reusable in-memory index.

### Old Data Structures

```
COMPOSITE  (one node per unique word)
├── str        → cc_tokenizer::String<char>   the word string
├── index      → size_type                    unique word index (1-based)
├── n_ptr      → size_type                    frequency count
├── probability → double                      stored as a field (could go stale)
├── ptr        → LINETOKENNUMBER*             head of occurrence list
├── next / prev

LINETOKENNUMBER  (one node per occurrence)
├── index  → size_type   global corpus-wide token index (with redundancy, 1-based)
├── l      → size_type   line number (1-based)
├── t      → size_type   token number within line (1-based)
├── next / prev
```

### What the Old Implementation Had to Do

To answer "what is the vocabulary index of this word?", the old `operator()` walked
the entire `COMPOSITE` linked list doing string comparisons until it found a match —
O(vocabulary_size) per lookup. The `Pairs` constructor called this for every token
in the corpus, including repetitions. For a corpus of T total tokens and vocabulary
size V, pair generation cost O(T × V) string comparisons.

`probability` was stored as a field inside `COMPOSITE`. This meant it could go stale
if the corpus changed, and it had to be computed and stored explicitly rather than
derived on demand.

The `build()` method had to do a full copy of the entire linked list — `COMPOSITE`
nodes and `LINETOKENNUMBER` nodes — to produce a second `Corpus` from an existing one,
because there was no shared underlying index to reference.

`get_composite_ptr(index, redundancy)` — the core lookup used by `Pairs` — walked the
list linearly. With `redundancy=true` it walked the `LINETOKENNUMBER` sub-list inside
each `COMPOSITE` node as well, to find which word occupied a given global token
position. This was the bottleneck that made `Pairs` generation expensive enough to
require serialisation to disk.

### What Changed

`Parser::build_hash_table()` now produces `TABLES*`, which pre-computes everything
the old `Corpus` was computing at query time:

| Old `Corpus` query | Old cost | New equivalent | New cost |
|---|---|---|---|
| Word string → vocabulary index | O(V) string scan | `generate_key()` → `hash_to_word_record[key]->word_id` | O(1) average |
| Vocabulary index → word string | O(V) scan | `hash_to_word_record[word_id_to_hash[id]]->word` | O(1) |
| Frequency of word | O(1) via `n_ptr` field | `WordRecord::n` | O(1) |
| Probability of word | O(1) via stored field | `WordRecord::n / total_tokens` | O(1), never stale |
| Global token position → word | O(V × k) scan | `LINE → TOKEN → token_id` | O(T) traversal |
| Occurrence list | `LINETOKENNUMBER` linked list | `OccurrenceNode` linked list | equivalent |

The `Corpus` class in the new implementation is not a builder. It is a query facade.
All the data it needs was built during the single corpus pass in `build_hash_table()`.

---

## Old vs New — Side by Side

### Old `Corpus` (builder + query, string-based)

```cpp
// Build from file — expensive, re-parses corpus
Corpus vocab("corpus.txt");

// Lookup by string — O(V) scan
size_type idx = vocab[cc_tokenizer::String<char>("word")];

// Lookup by index with redundancy — O(V × k) scan
COMPOSITE_PTR ptr = vocab.get_composite_ptr(i, true);

// Probability — stored field, could be stale
double p = ptr->probability;

// Frequency
size_type f = ptr->get_frequency();   // returns n_ptr
```

### New `Corpus` (query facade, integer-based)

```cpp
// No build step — TABLES already built by Parser
TABLES* tables = parser.build_hash_table();
Corpus corpus(tables);

// Frequency — O(1), reads WordRecord::n directly
size_t f = corpus.frequency(word_id);

// Probability — O(1), never stale
double p = corpus.probability(word_id);

// Pre-flight check
if (!corpus.is_valid()) { /* handle */ }
```

---

## Current Interface

```cpp
class Corpus
{
    public:
        Corpus(TABLES* t);

        size_t frequency(size_t word_id) const;
        double probability(size_t word_id) const;
        bool   is_valid() const;
};
```

### `frequency(word_id)`

Returns the number of times `word_id` appears in the corpus. This is
`WordRecord::n`, accumulated during `build_hash_table()` at zero additional cost —
`n` is incremented in exactly the same cases where a new `OccurrenceNode` is appended
to the occurrence list, so `n` always equals the length of that list.

Returns `0` if `tables` is null, `word_id` is out of range, the bucket index is
invalid, or the `WordRecord` pointer is null.

### `probability(word_id)`

Returns `(double)WordRecord::n / (double)TABLES::total_tokens`.

Both values are pre-computed during `build_hash_table()`. The result is always
current — there is no stored probability field that can go stale. Returns `0.0`
on any null or out-of-range condition, and on empty corpus (zero total tokens,
prevents division by zero).

### `is_valid()`

Returns `true` if the `Corpus` object is in a usable state — `tables` is non-null,
`bucket_used > 0`, and `total_tokens > 0`. Use as a pre-flight check before entering
a training loop.

---

## Null and Range Safety

Every method guards against:

| Condition | Guard | Return |
|---|---|---|
| `tables == nullptr` | First check in every method | `0` / `0.0` / `false` |
| `word_id >= bucket_used` | Range check before array access | `0` / `0.0` |
| `h >= bucket_count` | Bucket index bounds check | `0` / `0.0` |
| `hash_to_word_record[h] == nullptr` | Null check after array access | `0` / `0.0` |
| `total_tokens == 0` | Division by zero guard in `probability()` | `0.0` |

The last two guards should never fire under correct `TABLES` invariants, but are
present defensively so that a corrupt or partially-built `TABLES` does not produce
undefined behaviour.

---

## What Will Be Added Here

As the ML model implementation progresses, this package will grow to expose whatever
`TABLES` data the model needs. Likely additions, in rough order of when they will be
needed:

- `word_string(word_id)` — decode a word_id back to its string, for logging and evaluation
- `word_id(string)` — encode a string to its word_id, for inference-time tokenisation
- `subsampling_probability(word_id, threshold)` — for Skip-gram subsampling of frequent words
- `negative_sampling_weight(word_id)` — `freq^(3/4)` weighted table for negative sampling
- `vocab_size()` — convenience wrapper for `tables->get_bucket_used()`
- `total_tokens()` — convenience wrapper for `tables->get_total_tokens()`

None of these are added speculatively. They will be added when `Pairs` or the training
loop actually needs them.

---

## Relationship to `Pairs`

`Corpus` and `Pairs` are developed together and both depend on `TABLES`. Their
responsibilities are distinct:

| Package | Responsibility |
|---|---|
| `Corpus` | Vocabulary statistics — frequency, probability, word↔id mapping |
| `Pairs` | Training pair generation — sliding window over `LINE → TOKEN` list |

`Pairs` may call `Corpus::probability()` during negative sampling weight computation.
`Corpus` never calls into `Pairs`.

---

## Repository Layout

```
Corpus/
├── README.md                  ← this file
├── lib/
│   └── src/
│       └── Corpus.hh          ← new implementation (query facade over TABLES)
└── old_implementation/
    ├── corpus.hh              ← old builder + query class (COMPOSITE linked list)
    └── composite.hh           ← old COMPOSITE and LINETOKENNUMBER struct definitions
```

---

## Dependencies

`Corpus` depends on `TABLES`, which is produced by `Parser`. The full dependency
chain for a package using `Corpus` is:

```
your package
└── Corpus          ← this package
└── Parser          ← https://github.com/KHAAdotPK/Parser.git
    └── Hash        ← https://github.com/KHAAdotPK/Hash.git
```

Optionally, a text cleaning package plugs into `Parser`'s `Iterator`:

```
Naqsh             ← https://github.com/KHAAdotPK/Naqsh.git     (Urdu)
Imprint           ← https://github.com/KHAAdotPK/imprint.git   (English)
```

---

## Related Packages

| Package | Role |
|---|---|
| [Parser](https://github.com/KHAAdotPK/Parser.git) | Builds `TABLES` from corpus in one pass |
| [Pairs](https://github.com/KHAAdotPK/Pairs.git) | Skip-gram training pair generator, developed alongside `Corpus` |
| [Naqsh](https://github.com/KHAAdotPK/Naqsh.git) | Urdu text cleaner, plugs into Parser |
| [Imprint](https://github.com/KHAAdotPK/imprint.git) | English text cleaner, plugs into Parser |
| [Hash](https://github.com/KHAAdotPK/Hash.git) | Hash function and prime growth strategy used by Parser |

---

## License

This project is governed by a license, the details of which can be located in the
accompanying file named `LICENSE`.
