#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <configuration.hxx>
#include <UnitTest++/UnitTest++.h>
#include <store.hxx>

TEST(InsertionEventuallySucceeds) {
    riak::store s;
    auto t = s["apple"]["friend"].put(riak::object::content("banana"));
    
    {
        UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
        t.wait();
    }
    
    CHECK(t.has_value());
}


TEST(UnmappedKeysAreFalsy) {
    riak::store s;

    auto retrieved = s["apple"]["friend_anonymous"].fetch();
    {
        UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
        retrieved.wait();
    }
    
    CHECK(retrieved.has_value());
    CHECK(not retrieved.get());
}


// TEST(DeletingUnmappedKeysIsAllowed) {
//     riak::store s;
//     auto r = s["apple"].delete("friend");
//     {
//         UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
//         r.wait();
//     }
//     
//     CHECK(r.has_value());
//     CHECK(r.get() == false);
// }


// TEST(DeletingMappedKeysRemovesThem) {
//     riak::store s;
//     auto stored = s["apple"].set("banana");
//     {
//         UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
//         stored.wait();
//     }
//     
//     auto deletion = s["apple"].unmap();
//     {
//         UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
//         deletion.wait();
//     }
//     
//     auto retrieved = s["apple"].fetch();
//     {
//         UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
//         retrieved.wait();
//     }
//     
//     CHECK(retrieved.has_value());
//     CHECK(not retrieved.get());
// }


TEST(AsynchronousFetchOfMappedValueEventuallySucceeds)
{
    riak::store s;
    
    auto stored = s["apple"]["friend"].put(riak::object::content("banana"));
    {
        UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
        stored.wait();
    }
    
    auto r = s["apple"]["friend"].fetch();
    {
        UNITTEST_TIME_CONSTRAINT(configuration::request_timeout);
        r.wait();
    }
    
    CHECK(r.has_value());
    CHECK(!! r.get());
    CHECK(r.get()->siblings->front().value == "banana");
}


// void assign_thousand_times (std::shared_ptr<riak::store> s)
// {
//     using boost::lexical_cast;
//     
//     // It is unbelievable how much work we can do before my laptop starts choking.
//     for (size_t i = 0; i < 0xFFFFFFFFFFFFFFFF; ++i) {
//         (*s)[lexical_cast<std::string>(i)].set("abcdef");
//     }
// }
// 
// 
// TEST(ParallelQueriesShouldNotCrash)
// {
//     std::shared_ptr<riak::store> s(new riak::store);
//     
//     std::vector<boost::thread*> threads;
//     boost::thread_group tg;
//     
//     for (size_t i = 0; i < 500; ++i) {
//         threads.push_back(tg.create_thread(std::bind(assign_thousand_times, s)));
//     }
// }
