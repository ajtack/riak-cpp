/*!
 * \file
 * Implements the generic "store" interface with a simple in-memory hash table.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <store.hxx>
#include <boost/optional.hpp>
#include <unordered_map>

//=============================================================================
namespace riak {
//=============================================================================

struct store::implementation
{    
    typedef std::unordered_map<key, ::riak::bucket> hash_table;
    hash_table buckets;
    boost::mutex mutex;
};


const object_access_parameters store::access_defaults = object_access_parameters()
    .with_r(2)
    .with_pr(2)
    .with_w(2)
    .with_pw(2)
    .with_dw(0)
    .with_rw(2)
    .with_basic_quorum()
    .with_notfound_ok();


store::store (const object_access_parameters& d)
  : pimpl_(new implementation()),
    access_defaults_(d)
{   }


store::~store ()
{   }


bucket store::bucket (const key& k)
{
    // Unless each hashed value is an active object, we can't do better than a plain mutex.
    boost::unique_lock<boost::mutex> protect(pimpl_->mutex);
    
    implementation::hash_table::iterator e = pimpl_->buckets.find(k);
    if (e != pimpl_->buckets.end()) {
        return e->second;
    } else {
        pimpl_->buckets.insert(std::make_pair(k, ::riak::bucket(*this, k)));
        return pimpl_->buckets.find(k)->second;
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
