// References vs Pointers in C++
// Build & run:  c++ -std=c++17 ref_vs_ptr.cpp -o ref_vs_ptr && ./ref_vs_ptr
//
// One-line summary:
//   A REFERENCE is another name for an existing object (an alias).
//   A POINTER is a separate variable that stores an address.
//
// Every claim below is checked with assert(), so if the program runs to the end, it's all true.

#include <cassert>
#include <iostream>

// ---------------------------------------------------------------------------
// Lesson 6 helpers: passing to functions
// ---------------------------------------------------------------------------

// By reference: caller writes `addOneRef(x)`. Can't be null, so no check needed.
void addOneRef(int &n)
{
    n += 1;
}

// By pointer: caller writes `addOnePtr(&x)`. The `&` at the call site makes the
// modification visible, and nullptr can mean "nothing given", so we must check.
void addOnePtr(int *n)
{
    if (n != nullptr)
    {
        *n += 1;
    }
}

// const reference: read-only, no copy. The usual way to pass big objects (strings, vectors).
int twice(const int &n)
{
    return n * 2;
}

int main()
{
    // -----------------------------------------------------------------------
    // Lesson 1: Syntax
    // -----------------------------------------------------------------------
    int x = 10;
    std::cout << "x: " << x;
    int &ref = x; // `&` in a DECLARATION means "reference to"
    std::cout << "ref: " << ref;
    ref = 20;
    std::cout << "x: " << x;
    int *ptr = &x; // `&` in an EXPRESSION means "address of"; `*` in a declaration means "pointer to"

    ref = 20; // use a reference like the original variable, no extra symbol
    assert(x == 20);

    *ptr = 30; // `*` in an expression means "dereference" (go to the thing it points at)
    assert(x == 30);

    std::cout << "\nLesson 1: x=" << x << " ref=" << ref << " *ptr=" << *ptr
              << " ptr=" << ptr << " (an address)\n";

    // -----------------------------------------------------------------------
    // Lesson 2: Initialization and null
    // -----------------------------------------------------------------------
    // int &badRef;          // ERROR: a reference MUST be bound when declared
    int *nothing = nullptr; // fine: a pointer can point at nothing
    int *later;             // also compiles, but it's garbage until assigned (don't use it!)
    later = &x;

    assert(nothing == nullptr);
    assert(*later == 30);
    // *nothing = 5;         // compiles, but crashes at runtime (undefined behavior)

    std::cout << "Lesson 2: references can't be null; pointers can (always check before *p)\n";

    // -----------------------------------------------------------------------
    // Lesson 3: Re-seating (the one that trips everyone up)
    // -----------------------------------------------------------------------
    int a = 1;
    int b = 2;

    int &r = a;
    r = b; // does NOT make r refer to b! It copies b's VALUE into a.
    assert(a == 2);
    assert(&r == &a); // r is still a, forever

    int *p = &a;
    p = &b; // a pointer CAN be re-pointed at something else
    *p = 99;
    assert(b == 99);
    assert(a == 2); // a untouched

    std::cout << "Lesson 3: a reference is bound for life; a pointer can be re-pointed\n";

    // -----------------------------------------------------------------------
    // Lesson 4: Identity -- a reference IS the object, a pointer is its own object
    // -----------------------------------------------------------------------
    assert(&ref == &x);                                           // same address: ref is just another name for x
    assert(static_cast<void *>(&ptr) != static_cast<void *>(&x)); // ptr lives at its own address
    assert(sizeof(ref) == sizeof(int));                           // sizeof a reference = sizeof the thing it refers to
    assert(sizeof(ptr) == sizeof(int *));                         // sizeof a pointer = size of an address (usually 8)

    std::cout << "Lesson 4: &x=" << &x << " &ref=" << &ref << " (same)  &ptr=" << &ptr
              << " (different)\n"
              << "          sizeof(ref)=" << sizeof(ref) << " sizeof(ptr)=" << sizeof(ptr) << "\n";

    // -----------------------------------------------------------------------
    // Lesson 5: Pointer arithmetic (pointers only)
    // -----------------------------------------------------------------------
    int arr[] = {100, 200, 300};
    int *walker = arr; // an array name turns into a pointer to its first element

    assert(*walker == 100);
    walker++; // moves by sizeof(int) bytes, i.e. to the next element
    assert(*walker == 200);
    assert(*(arr + 2) == 300); // arr[2] is literally defined as *(arr + 2)

    int &first = arr[0];
    first++; // on a reference, ++ just increments the VALUE
    assert(arr[0] == 101);

    std::cout << "Lesson 5: walker++ moves the pointer; ref++ changes the value\n";

    // -----------------------------------------------------------------------
    // Lesson 6: Passing to functions (see helpers at the top)
    // -----------------------------------------------------------------------
    int n = 5;
    addOneRef(n); // looks like pass-by-value at the call site -- read the signature!
    assert(n == 6);

    addOnePtr(&n); // explicit `&` shows n may be modified
    assert(n == 7);

    addOnePtr(nullptr); // legal, function handles it

    assert(twice(n) == 14);
    assert(twice(21) == 42); // const& can bind to a temporary like 21; plain int& can't

    std::cout << "Lesson 6: n=" << n << " after addOneRef + addOnePtr\n";

    // -----------------------------------------------------------------------
    // Lesson 7: const -- pointers have TWO kinds, references effectively one
    // -----------------------------------------------------------------------
    int c = 1;
    int d = 2;

    const int *ptrToConst = &c; // can't change *ptrToConst, CAN re-point
    // *ptrToConst = 5;          // ERROR
    ptrToConst = &d; // OK

    int *const constPtr = &c; // CAN change *constPtr, can't re-point
    *constPtr = 5;            // OK
    // constPtr = &d;          // ERROR
    assert(c == 5);

    const int &constRef = c; // can't change through it (a reference is already "const" in
    // constRef = 7;          // ERROR       the re-pointing sense, so `int &const` isn't a thing)
    assert(constRef == 5 && *ptrToConst == 2);

    std::cout << "Lesson 7: read `const` right-to-left: `int *const` = const pointer to int\n";

    // -----------------------------------------------------------------------
    // Lesson 8: Levels of indirection
    // -----------------------------------------------------------------------
    int v = 7;
    int *pv = &v;
    int **ppv = &pv; // pointer to a pointer: fine
    **ppv = 8;
    assert(v == 8);

    int *&refToPtr = pv; // reference to a pointer: fine (alias for pv)
    refToPtr = nullptr;  // this sets pv itself to null
    assert(pv == nullptr);
    // int &&& nope;      // no "reference to reference" (&& means something else: rvalue ref)

    std::cout << "Lesson 8: int** and int*& exist; reference-to-reference doesn't\n";

    // -----------------------------------------------------------------------
    // Lesson 9: Both can dangle (outlive what they refer to)
    // -----------------------------------------------------------------------
    // int &bad()  { int local = 1; return local; }   // reference to dead local -> UB
    // int *bad2() { int local = 1; return &local; }  // pointer to dead local   -> UB
    // References are SAFER (never null, never re-pointed), not SAFE.

    // -----------------------------------------------------------------------
    // Cheat sheet: which one should I use?
    // -----------------------------------------------------------------------
    std::cout << "\nRule of thumb:\n"
              << "  Use a REFERENCE when the thing always exists and you won't re-point it\n"
              << "    (function params, const& for big read-only args, range-for loops).\n"
              << "  Use a POINTER when it can be 'nothing' (nullptr), must be re-pointed,\n"
              << "    or you're walking through an array / working with C APIs.\n"
              << "\nAll asserts passed.\n";
    return 0;
}
