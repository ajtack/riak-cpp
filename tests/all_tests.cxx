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
    assert(not t);
}
