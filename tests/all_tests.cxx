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
