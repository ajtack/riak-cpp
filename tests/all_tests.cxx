#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <UnitTest++/UnitTest++.h>
#include <storage.hxx>

TEST(InsertionEventuallySucceeds) {
    storage s;
    auto t = s["apple"].set("banana");
    
    {
        UNITTEST_TIME_CONSTRAINT(50);
        t.wait();
    }
    
    CHECK(t.has_value());
}


TEST(UnmappedKeysAreFalsy) {
    storage s;

    auto retrieved = s["apple"].fetch();
    {
        UNITTEST_TIME_CONSTRAINT(50);
        retrieved.wait();
    }
    
    CHECK(retrieved.has_value());
    CHECK(not retrieved.get());
}


TEST(DeletingUnmappedKeysIsAllowed) {
    storage s;
    auto r = s["apple"].unmap();
    {
        UNITTEST_TIME_CONSTRAINT(50);
        r.wait();
    }
    
    CHECK(r.has_value());
    CHECK(r.get() == false);
}


TEST(DeletingMappedKeysRemovesThem) {
    storage s;
    auto stored = s["apple"].set("banana");
    {
        UNITTEST_TIME_CONSTRAINT(50);
        stored.wait();
    }
    
    auto deletion = s["apple"].unmap();
    {
        UNITTEST_TIME_CONSTRAINT(50);
        deletion.wait();
    }
    
    auto retrieved = s["apple"].fetch();
    {
        UNITTEST_TIME_CONSTRAINT(50);
        retrieved.wait();
    }
    
    CHECK(retrieved.has_value());
    CHECK(not retrieved.get());
}


TEST(AsynchronousFetchOfUnmappedValueEventuallySucceeds)
{
    storage s;
    
    auto r = s["apple"].fetch();
    {
        UNITTEST_TIME_CONSTRAINT(50);
        r.wait();
    }
    
    CHECK(r.has_value());
    CHECK(not r.get());
}


TEST(AsynchronousFetchOfMappedValueEventuallySucceeds)
{
    storage s;
    
    auto stored = s["apple"].set("banana");
    {
        UNITTEST_TIME_CONSTRAINT(50);
        stored.wait();
    }
    
    auto r = s["apple"].fetch();
    {
        UNITTEST_TIME_CONSTRAINT(50);
        r.wait();
    }
    
    CHECK(r.has_value());
    CHECK(!! r.get());
    CHECK(*r.get() == "banana");
}


void assign_thousand_times (std::shared_ptr<storage> s)
{
    using boost::lexical_cast;
    
    // It is unbelievable how much work we can do before my laptop starts choking.
    for (size_t i = 0; i < 0xFFFFFFFFFFFFFFFF; ++i) {
        (*s)[lexical_cast<std::string>(i)].set("abcdef");
    }
}


TEST(ParallelQueriesShouldNotCrash)
{
    std::shared_ptr<storage> s(new storage);
    
    std::vector<boost::thread*> threads;
    boost::thread_group tg;
    
    for (size_t i = 0; i < 500; ++i) {
        threads.push_back(tg.create_thread(std::bind(assign_thousand_times, s)));
    }
}
