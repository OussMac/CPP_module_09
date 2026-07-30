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

### processInputFile()

This is the side that reads the **untrusted** file — the one the user hands
in. Unlike `loadDatabase()` (our own file, we control its shape), everything
here has to be validated before it's trusted.

Flow:
```
open file, bail with "Error: could not open file." if it fails
skip header line
for each line:
    split on '|' -> date, value        (bad input if no '|')
    validate date format                (bad input if malformed)
    validate value + parse it           (bad input / range errors)
    look up closest rate for that date  (error if none exists)
    print date => value = result
```
Every failure path does `continue`, never `return` or `exit` — one bad line
never stops the rest of the file from being processed. That's a hard
requirement from the eval sheet.

---

### splitInputLine() + trim()

Same idea as splitting `date,value` in `loadDatabase`, just on `'|'` instead
of `','`, plus a trim step this time — because unlike our own DB file, the
input file has spaces around the `|` (`"2011-01-03 | 3"`), and comparing
`"2011-01-03 "` (with a trailing space) against a map key `"2011-01-03"`
will never match.

```cpp
static std::string trim(const std::string &str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
        start++;
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        end--;
    return str.substr(start, end - start);
}
```
Two independent scans: walk in from the left until a non-space char, walk in
from the right until a non-space char, take the slice between.

Note: `std::isspace` takes an `int`. `char` is signed on most platforms
(g++/x86_64), so a `char` with the high bit set becomes a negative `int`
when implicitly converted, and `isspace` on a negative value that isn't
`EOF` is undefined behaviour. `static_cast<unsigned char>` first avoids
that entirely. Won't ever bite on plain ASCII digits/dates, but it's the
correct habit either way.

---

### isValidDate()

Only checks what the subject actually tests — no leap-year handling, no
per-month day limits. The subject's only bad-date example is `2001-42-42`,
which fails because the **month** is out of range, not because of any
calendar edge case.

```
1. length must be 10        ("YYYY-MM-DD")
2. date[4] and date[7] must be '-'
3. every other character must be a digit
4. month must be 1-12
5. day must be 1-31 (flat range, no per-month/leap-year logic)
```

Deliberately dropped: `year` extraction, `isLeapYear`, days-per-month table.
None of it is asked for, so none of it is written — simplest version that
still catches everything the subject requires.

---

### isValidValue()

Needs to report **which** rule failed, not just pass/fail, because the
subject wants two different messages for two different failures:
```
Error: not a positive number.
Error: too large a number.
```
So the signature carries an out-parameter for the parsed value and one for
the specific error string:
```cpp
static bool isValidValue(const std::string &value, double &outValue, std::string &errorMsg);
```

```
1. empty string -> bad input
2. strtod parses it; if anything is left over after the number -> bad input
   (e.g. "12x" parses "12" but leaves "x" unread)
3. negative -> "not a positive number."
4. > 1000    -> "too large a number."
5. otherwise -> valid, outValue = parsed number
```
Using `strtod` + `endPtr` here (not `istringstream`) — `endPtr` points to
wherever the numeric parse stopped, so checking `*endPtr != '\0'` tells you
directly if there's leftover garbage after the number.

---

### getExchangeRate() and lower_bound()

This is the "find the closest date at or before this one" logic, and it's
worth actually understanding rather than just trusting it works.

```cpp
double BitcoinExchange::getExchangeRate(const std::string &date) const {
    std::map<std::string, double>::const_iterator it = this->database.lower_bound(date);
    if (it != this->database.end() && it->first == date)
        return it->second;
    if (it == this->database.begin())
        return -1;
    --it;
    return it->second;
}
```

**What `lower_bound(key)` actually does:** returns an iterator to the first
element whose key is **not less than** `key`. In plain terms: the first
entry that is `>= key`. It does *not* mean "the closest one before" — it
leans forward, toward equal-or-greater, never backward on its own.

Walk through the tree with a few DB dates: `2009-01-02`, `2011-01-03`,
`2011-01-09`, `2012-01-11`.

**Case A — exact match.** `lower_bound("2011-01-03")`:
```
tree search walks down comparing keys...
finds a node whose key == "2011-01-03" exactly
that IS "not less than" the target, and it's the first such node
-> iterator points AT that node
```
So `it->first == date` is true, first `if` fires, return that rate directly.
No decrement needed — we already landed exactly where we wanted.

**Case B — date not in DB, e.g. "2011-01-05".** `lower_bound("2011-01-05")`:
```
"2011-01-03" < "2011-01-05"   -> not a candidate, keep going
"2011-01-09" >= "2011-01-05"  -> first one that qualifies
-> iterator points at "2011-01-09"
```
`it->first == date`? No (`"2011-01-09" != "2011-01-05"`). So we fall through
to the `begin()` check (not begin, there's an earlier entry), then
`--it` steps backward one position in sorted order — from `"2011-01-09"`
back to `"2011-01-03"` — which is exactly the closest date **before** our
input. That's the whole trick: `lower_bound` finds the first thing at or
past your target, and stepping back one gives you the last thing strictly
before it.

**Case C — date before every entry, e.g. "2005-01-01".**
```
lower_bound returns the very first element in the map (2009-01-02),
since everything in the DB is >= 2005-01-01
```
`it->first == date`? No. `it == begin()`? Yes — there's nothing earlier to
step back to. Return `-1` as a sentinel meaning "no rate available," which
`processInputFile` checks for and reports as an error.

Simple visual of the decision:
```
lower_bound(date)
        |
        v
  exact match? ---- yes ---> return it->second
        |
        no
        |
        v
  at begin()? ------ yes ---> return -1 (nothing earlier exists)
        |
        no
        |
        v
  --it  (step back to the last key < date)
        |
        v
  return it->second
```

This only needs **one** tree traversal (`lower_bound`), unlike an earlier
version of this function that called `find()` first and then
`lower_bound()` separately — two traversals for one answer. Collapsing it
to just `lower_bound` plus the exact-match check does the same job in half
the tree walks.