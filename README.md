# CPP Module 09 - STL

## Index

- [Exercise 00: Bitcoin Exchange](#exercise-00-bitcoin-exchange)
- Exercise 01: Reverse Polish Notation (TBD)
- Exercise 02: PmergeMe (TBD)

---

## Exercise 00: Bitcoin Exchange

### What we're doing

The program reads a user input file of `date | value` pairs, looks up each date
in a bitcoin exchange rate database (`data.csv`), and prints `value * rate`. If
the exact date isn't in the database, we use the closest date **before** it.

Needs one container. We use `std::map<std::string, double>` for the database:
sorted by key automatically, and gives us fast "closest lower date" lookups.

---

### std::map

**Definition:** associative container. Stores key/value pairs. Keys are
unique. Always kept sorted by key.

**It's a class template**, so it works for any key/value type combo:
`map<string, double>`, `map<int, int>`, `map<string, vector<int>>`, whatever.
`template<class Key, class T> class map`.

**Sorted by key:** default comparator is `std::less<Key>`. For `std::string`
keys that means lexicographic order — basically dictionary order, which for
our `YYYY-MM-DD` dates happens to line up perfectly with chronological order
(character-by-character ASCII comparison).

**No duplicates:** `insert()` silently does nothing if the key already exists.
First value in wins, not last. One value per key, period.

**Iterator:** conceptually just a wrapper around a pointer to a tree node.
Dereferencing it gives you a `pair<const Key, T>`.

```cpp
// what map::iterator gives you when dereferenced (conceptually)
struct value_type {
    const Key first;   // the key, can't be modified through iterator
    T         second;  // the value, can be modified through iterator
};
```

Key/value table example (couple entries from data.csv):

```
key            | value
---------------|--------
2011-01-03     | 0.30
2011-01-09     | 0.32
2012-01-11     | 7.10
```

`it->first` = key, `it->second` = value. That's the whole interface.

---

### Memory representation: red-black tree

Confirmed: `std::map` (libstdc++, the one we use) is implemented as a
**self-balancing binary search tree**, specifically a red-black tree. Not
contiguous like a vector. Each node has its own allocation somewhere on the
heap, linked by pointers.

```
                (2011-01-09) B
                 /          \
       (2011-01-03) R    (2012-01-11) R
```
B/R = black/red, just internal bookkeeping the tree uses to stay balanced.
Doesn't matter for how we use the map, just explains why insert/find are
O(log n) instead of O(n).

Node shape (conceptual, not the real libstdc++ internals):
```cpp
struct Node {
    pair<Key, T> data;
    Node *left;
    Node *right;
    Node *parent;
    bool  color;
};
```

Why this matters: walking the tree in-order gives you the keys in sorted
order automatically. That's why iterating a `map` from `begin()` to `end()`
always comes out sorted — it's not sorting anything, it's just an in-order
traversal of a tree that's already ordered by construction.

---

### Implementation: loadDatabase()

**std::ifstream** — under the hood this wraps an OS file descriptor (the int
handle the OS gives you for an open file) plus an internal buffer that stages
chunks of bytes read from disk, so you're not making a syscall for every
single byte you read.

```
ifstream file
 ├── fd (int)        -> OS handle to the opened data.csv
 └── buffer          -> chunk of bytes pulled from disk in advance,
                         refilled transparently as you consume it
```

**getline(file, line)** pulls bytes out of that buffer one at a time,
building up `line` until it hits `'\n'`, then discards the `\n` and hands you
the finished string. If the buffer runs dry mid-line, it goes back to the OS
for more bytes without us having to care.

**strtod(const char *str, char **endptr)** — converts the leading numeric
part of `str` into a `double`. Skips leading whitespace, stops at the first
character that isn't part of a valid number. `endptr`, if given, gets pointed
at wherever conversion stopped. We pass `NULL` because we trust this file and
don't need to check leftover characters.

**npos** is `static const size_t std::string::npos = -1`. Since it's
`size_t` (unsigned), `-1` wraps around to the max possible value. Convention:
`find()` returns `npos` when it can't find what you're looking for.

**Our fix:** `getline` can hand us an empty line (trailing newline at EOF) or
a malformed line with no comma. In both cases `line.find(',')` returns
`npos`, so we check for that and skip the line instead of inserting garbage
into the map.

```cpp
size_t npos = line.find(',');
if (npos == std::string::npos)
    continue;
```

**continue** jumps straight back to the `while` condition (the next
`getline` call), skipping whatever's left in the loop body for that
iteration. That's how we throw away a bad line without stopping the whole
read.

---

### processInputFile() — TBD

*(next section, once we build out the input-file side)*