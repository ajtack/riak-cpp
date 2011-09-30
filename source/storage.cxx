/*!
 * \file
 * Implements the generic "storage" interface with a simple in-memory hash table.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <storage.hxx>
#include <boost/optional.hpp>
#include <unordered_map>

struct storage::implementation
{
    typedef std::unordered_map<storage::key, storage::optional_value> hash_table;
    hash_table records;
};


storage::storage ()
  : pimpl_(new implementation())
{   }


storage::~storage ()
{   }


storage::optional_value& storage::operator[] (const storage::key& k)
{
    implementation::hash_table::iterator e = pimpl_->records.find(k);
    if (e != pimpl_->records.end()) {
        return e->second;
    } else {
        pimpl_->records.insert(std::make_pair(k, optional_value(k)));
        return pimpl_->records.find(k)->second;
    }
}


const storage::optional_value& storage::operator[] (const storage::key& k) const
{
    return const_cast<storage*>(this)->operator[](k);    
}


struct storage::optional_value::implementation
{
    implementation (const storage::key& k, boost::optional<storage::value> v = boost::optional<storage::value>())
      : key(k),
        value(v)
    {   }
    
    class linked_value_proxy;
    
    const std::string key;                    /*!< The key under which this value is assigned. */
    boost::optional<storage::value> value;   /*!< The absolute most current value known. */
};


storage::optional_value::optional_value (const storage::key& k)
  : pimpl_(new implementation(k))
{   }


boost::shared_future<std::unique_ptr<storage::optional_value>>
storage::optional_value::set (const std::string& new_value)
{
    typedef std::unique_ptr<optional_value> pointer;
    
    boost::promise<pointer> promise;
    pimpl_->value = new_value;
    pointer new_value_proxy(new optional_value(pimpl_));
    promise.set_value(std::move(new_value_proxy));
    return promise.get_future();
}


boost::shared_future<bool> storage::optional_value::unmap ()
{
    bool value_existed = pimpl_->value;
    pimpl_->value.reset();
    boost::promise<bool> promise;
    promise.set_value(value_existed);
    return promise.get_future();
}


boost::shared_future<boost::optional<storage::value>> storage::optional_value::fetch () const
{
    boost::promise<boost::optional<storage::value>> promise;
    promise.set_value(pimpl_->value);
    return promise.get_future();
}
