/*!
 * \file
 * Implements asynchronous communication with a Riak server over objects.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#include <bucket.hxx>
#include <object.hxx>
#include <boost/thread/mutex.hpp>

//=============================================================================
namespace riak {
//=============================================================================

object::bucket_table object::buckets;
boost::mutex object::mutex;

boost::shared_future<size_t> object::put (
        const content& b,
        const boost::optional<std::string>& )
        // const object_access_parameters& )
{
    boost::promise<size_t> promise;
    object_table& objects = get_bucket(store_, bucket_.key());
    objects.insert(std::make_pair(key_, b));
    promise.set_value(0);
    return promise.get_future();   // Determine our own destiny!
}

        
boost::shared_future<std::tuple<size_t, object::content>> object::put_returning_body (
        const content& b,
        const boost::optional<std::string>& vector_clock )
        // const object_access_parameters& p )
{
    boost::promise<std::tuple<size_t, content>> result;
    boost::shared_future<size_t> insertion_time = put(b, vector_clock);
    assert(insertion_time.is_ready() and insertion_time.has_value());
    result.set_value(std::make_tuple(insertion_time.get(), buckets.find(bucket_.key())->second.find(key_)->second));
    return result.get_future();
}


boost::shared_future<boost::optional<object::fetched_value>> object::fetch (
        const boost::optional<std::string>& vector_clock ) const
        // const object_access_parameters& p = store_.object_access_defaults() ) const
{
    using namespace boost;
    using namespace std;
    
    promise<optional<fetched_value>> promise;
    object_table& objects = get_bucket(store_, bucket_.key());
    object_table::iterator object_location = objects.find(key_);
    if (object_location != objects.end()) {
        fetched_value f;
        f.siblings = list<sibling>();
        f.siblings->push_back(object_location->second);
        f.vclock = "";  // TODO
        f.unchanged = 0;
        promise.set_value(f);
        return promise.get_future();
    } else {
        promise.set_value(boost::none);
        return promise.get_future();
    }
}


object::object_table& object::get_bucket (store& s, const key& k) const
{
    boost::unique_lock<boost::mutex> protect(mutex);
    bucket_table::iterator bucket_location = buckets.find(k);
    if (bucket_location != buckets.end()) {
        return bucket_location->second;
    } else {
        buckets.insert(make_pair(k, object_table()));
        return buckets.find(k)->second;
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
