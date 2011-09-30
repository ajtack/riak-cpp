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
    CHECK(t.get()->value() == "banana");
}


TEST(UnmappedKeysAreFalsy) {
    storage s;
    auto t = s["apple"];
    CHECK(not t);
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
    
    auto retrieved = s["apple"];
    CHECK(not retrieved);
}
