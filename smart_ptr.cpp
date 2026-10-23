// Raw pointers vs Smart pointers in C++
// Build & run:  ./run_cpp.sh smart_ptr.cpp
//
// One-line summary:
//   A RAW POINTER (T*) just stores an address. It never cleans up, so YOU must call delete.
//   A SMART POINTER is an object that OWNS the address and calls delete for you
//   when it goes out of scope (this idea is called RAII: cleanup is tied to scope).
//
//   std::unique_ptr<T>  exactly one owner. Can't copy, can move. Same cost as a raw pointer.
//   std::shared_ptr<T>  many owners, reference-counted. The last owner to leave deletes it.
//   std::weak_ptr<T>    watches a shared_ptr WITHOUT owning it. Can tell if the object died.
//
// Every claim below is checked with assert(), so if the program runs to the end, it's all true.

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

// Counts how many of itself are alive, so we can SEE leaks and deletes happen.
struct Tracker
{
    static inline int alive = 0;
    int id;

    explicit Tracker(int id) : id(id)
    {
        ++alive;
        std::cout << "  + Tracker " << id << " created   (alive=" << alive << ")\n";
    }
    ~Tracker()
    {
        --alive;
        std::cout << "  - Tracker " << id << " destroyed (alive=" << alive << ")\n";
    }
};

void mightThrow(bool fail)
{
    if (fail)
    {
        throw std::runtime_error("boom");
    }
}

// A function that only LOOKS at an object takes a raw pointer (or reference).
// It doesn't own it and doesn't care who does. This is the right use of T* today.
int readId(const Tracker *t)
{
    return t != nullptr ? t->id : -1;
}

// Taking unique_ptr BY VALUE means "give me ownership". Caller must std::move into it.
void consume(std::unique_ptr<Tracker> t)
{
    std::cout << "  consume() now owns Tracker " << t->id << "\n";
} // t goes out of scope here -> Tracker deleted

// Used in Lesson 8 to show reference cycles.
struct Node
{
    Tracker t;
    std::shared_ptr<Node> strong; // owning link
    std::weak_ptr<Node> weak;     // non-owning link
    explicit Node(int id) : t(id) {}
};

int main()
{
    // -----------------------------------------------------------------------
    // Lesson 1: Raw pointer, manual new/delete
    // -----------------------------------------------------------------------
    std::cout << "\nLesson 1: raw pointer, you must delete by hand\n";
    {
        Tracker *raw = new Tracker(1);
        assert(Tracker::alive == 1);
        delete raw; // forget this line and the memory is leaked forever
        assert(Tracker::alive == 0);
        // `raw` STILL holds the old address (a "dangling" pointer). Using it now is undefined behavior.
    }

    // -----------------------------------------------------------------------
    // Lesson 2: Raw pointer + exception = leak
    // -----------------------------------------------------------------------
    std::cout << "\nLesson 2: an exception skips your delete, so it leaks\n";
    Tracker *escaped = nullptr;
    try
    {
        Tracker *raw = new Tracker(2);
        escaped = raw;    // (only so this demo can clean up; real code would just lose the address)
        mightThrow(true); // throws, so the delete below never runs
        delete raw;
    }
    catch (const std::runtime_error &)
    {
    }
    assert(Tracker::alive == 1); // LEAKED
    delete escaped;
    assert(Tracker::alive == 0);

    // -----------------------------------------------------------------------
    // Lesson 3: unique_ptr, one owner, auto delete
    // -----------------------------------------------------------------------
    std::cout << "\nLesson 3: unique_ptr deletes itself at end of scope\n";
    {
        std::unique_ptr<Tracker> u = std::make_unique<Tracker>(3); // no `new` anywhere
        std::cout << "u: " << u << "\n";
        std::cout << "u->id: " << u->id << "\n";
        std::cout << "u->id == 3: " << (u->id == 3) << "\n";
        assert(u->id == 3); // use it like a pointer: -> and *
        assert((*u).id == 3);

        std::cout << "u.get(): " << u.get() << "\n";
        Tracker *peek = u.get(); // borrow a raw, NON-owning pointer
        std::cout << "peek: " << peek << "\n";
        std::cout << "&u: " << &u << "\n";

        int test_int = 10;
        std::cout << "&test_int: " << &test_int << "\n";
        int *p_test_int = &test_int; // raw pointer to a STACK variable: never delete this
        std::cout << "p_test_int: " << p_test_int << ", *p_test_int: " << *p_test_int << "\n";
        std::cout << "peek->id: " << peek->id << "\n";
        assert(readId(peek) == 3);

        // std::unique_ptr<Tracker> copy = u;          // COMPILE ERROR: two owners not allowed
        std::unique_ptr<Tracker> u2 = std::move(u); // transfer ownership instead
        assert(u == nullptr);                       // u is now empty
        assert(u2->id == 3);
        assert(Tracker::alive == 1); // still ONE object, just a new owner
    } // u2 leaves scope -> delete happens automatically
    assert(Tracker::alive == 0);

    // -----------------------------------------------------------------------
    // Lesson 4: unique_ptr + exception = no leak
    // -----------------------------------------------------------------------
    std::cout << "\nLesson 4: same exception as Lesson 2, but no leak\n";
    try
    {
        auto u = std::make_unique<Tracker>(4);
        mightThrow(true);
    }
    catch (const std::runtime_error &)
    {
    }
    assert(Tracker::alive == 0); // destructor ran while the exception unwound the stack

    // -----------------------------------------------------------------------
    // Lesson 5: Handing ownership to a function
    // -----------------------------------------------------------------------
    std::cout << "\nLesson 5: std::move a unique_ptr into a function\n";
    auto u5 = std::make_unique<Tracker>(5);
    consume(std::move(u5)); // the Tracker dies inside consume()
    assert(u5 == nullptr);
    assert(Tracker::alive == 0);

    // -----------------------------------------------------------------------
    // Lesson 6: shared_ptr, many owners, reference count
    // -----------------------------------------------------------------------
    std::cout << "\nLesson 6: shared_ptr deletes when the LAST owner is gone\n";
    {
        std::shared_ptr<Tracker> a = std::make_shared<Tracker>(6);
        assert(a.use_count() == 1);
        {
            std::shared_ptr<Tracker> b = a; // copying IS allowed: now 2 owners
            assert(a.use_count() == 2);
            assert(a.get() == b.get()); // same object, not a copy of it
        } // b gone: count drops to 1, object NOT deleted
        assert(a.use_count() == 1);
        assert(Tracker::alive == 1);
    } // a gone: count drops to 0, deleted now
    assert(Tracker::alive == 0);

    // // -----------------------------------------------------------------------
    // // Lesson 7: Dangling raw pointer vs weak_ptr
    // // -----------------------------------------------------------------------
    // std::cout << "\nLesson 7: weak_ptr knows when the object died, a raw pointer doesn't\n";
    // Tracker *dangling = nullptr;
    // std::weak_ptr<Tracker> watcher;
    // {
    //     auto s = std::make_shared<Tracker>(7);
    //     dangling = s.get();
    //     watcher = s;
    //     assert(s.use_count() == 1); // weak_ptr does NOT count as an owner
    //     assert(!watcher.expired());

    //     if (std::shared_ptr<Tracker> locked = watcher.lock()) // lock() = "give me a temporary owner, if still alive"
    //     {
    //         assert(locked->id == 7);
    //     }
    // }
    // assert(watcher.expired()); // weak_ptr can tell it's gone
    // assert(watcher.lock() == nullptr);
    // assert(dangling != nullptr); // raw pointer still looks valid. Dereferencing it = undefined behavior.

    // // -----------------------------------------------------------------------
    // // Lesson 8: shared_ptr cycles leak, weak_ptr breaks them
    // // -----------------------------------------------------------------------
    // std::cout << "\nLesson 8a: two shared_ptrs owning each other = leak\n";
    // std::weak_ptr<Node> spy;
    // {
    //     auto a = std::make_shared<Node>(8);
    //     auto b = std::make_shared<Node>(9);
    //     a->strong = b;
    //     b->strong = a; // a owns b AND b owns a
    //     spy = a;
    // }
    // assert(Tracker::alive == 2); // LEAKED: each keeps the other's count at 1 forever
    // spy.lock()->strong.reset();  // (demo cleanup: break the cycle by hand)
    // assert(Tracker::alive == 0);

    // std::cout << "\nLesson 8b: make the back-link weak and the cycle is gone\n";
    // {
    //     auto a = std::make_shared<Node>(10);
    //     auto b = std::make_shared<Node>(11);
    //     a->strong = b;
    //     b->weak = a; // back-link doesn't own, so it doesn't keep `a` alive
    // }
    // assert(Tracker::alive == 0);

    // // -----------------------------------------------------------------------
    // // Rules of thumb
    // // -----------------------------------------------------------------------
    // std::cout << "\nAll asserts passed.\n"
    //              "Rules of thumb:\n"
    //              "  1. Prefer no pointer at all: a plain local variable or std::vector.\n"
    //              "  2. Need heap + one owner?       std::unique_ptr  (std::make_unique)\n"
    //              "  3. Truly shared ownership?      std::shared_ptr  (std::make_shared)\n"
    //              "  4. Back-links / observers?      std::weak_ptr\n"
    //              "  5. Just looking, not owning?    T* or T& is fine.\n"
    //              "  6. Writing `new`/`delete` yourself is almost never needed.\n";
}
