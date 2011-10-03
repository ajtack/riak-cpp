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
    typedef std::unordered_map<store::key, store::optional_value> hash_table;
    hash_table records;
    boost::mutex mutex;
};


store::store ()
  : pimpl_(new implementation())
{   }


store::~store ()
{   }


store::optional_value& store::operator[] (const store::key& k)
{
    // Unless each hashed value is an active object, we can't do better than a plain mutex.
    boost::unique_lock<boost::mutex> protect(pimpl_->mutex);
    
    implementation::hash_table::iterator e = pimpl_->records.find(k);
    if (e != pimpl_->records.end()) {
        return e->second;
    } else {
        pimpl_->records.insert(std::make_pair(k, optional_value(k)));
        return pimpl_->records.find(k)->second;
    }
}


const store::optional_value& store::operator[] (const store::key& k) const
{
    return const_cast<store*>(this)->operator[](k);    
}


struct store::optional_value::implementation
{
    implementation (const store::key& k, boost::optional<store::value> v = boost::optional<store::value>())
      : key(k),
        value(v)
    {   }
    
    class linked_value_proxy;
    
    const std::string key;                    /*!< The key under which this value is assigned. */
    boost::optional<store::value> value;   /*!< The absolute most current value known. */
};


store::optional_value::optional_value (const store::key& k)
  : pimpl_(new implementation(k))
{   }


boost::shared_future<size_t>
store::optional_value::set (const std::string& new_value)
{
    boost::promise<size_t> promise;
    pimpl_->value = new_value;
    promise.set_value(0);
    return promise.get_future();
}


boost::shared_future<bool> store::optional_value::unmap ()
{
    bool value_existed = pimpl_->value;
    pimpl_->value.reset();
    boost::promise<bool> promise;
    promise.set_value(value_existed);
    return promise.get_future();
}


boost::shared_future<boost::optional<store::value>> store::optional_value::fetch () const
{
    boost::promise<boost::optional<store::value>> promise;
    promise.set_value(pimpl_->value);
    return promise.get_future();
}

//=============================================================================
}   // namespace riak
//=============================================================================
