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


class storage::optional_value::implementation::linked_value_proxy
      : public storage::optional_value::async_value_proxy
{
  public:
    linked_value_proxy(const storage::key&, boost::optional<storage::value>& value);
    
    virtual const storage::value& cached_value () const { return *value_; }
    virtual boost::shared_future<std::unique_ptr<async_value_proxy>> operator= (std::string& new_value);
    
  private:
    const std::string key_;                   /*!< The key under which this value is assigned. */
    boost::optional<storage::value>& value_;  /*!< The current (cached) value, readable immediately. */
};


boost::shared_future<std::unique_ptr<storage::optional_value::async_value_proxy>>
storage::optional_value::implementation::linked_value_proxy::operator= (std::string& new_value)
{
    typedef std::unique_ptr<async_value_proxy> pointer;
    
    boost::promise<pointer> promise;
    value_ = new_value;
    pointer new_value_proxy(new linked_value_proxy(key_, value_));
    promise.set_value(std::move(new_value_proxy));
    return promise.get_future();
}


storage::optional_value::optional_value (const storage::key& k)
  : pimpl_(new implementation(k))
{   }


std::unique_ptr<storage::optional_value::async_value_proxy> storage::optional_value::operator* ()
{
    typedef std::unique_ptr<async_value_proxy> pointer;
    
    assert(pimpl_->value);
    return std::move(pointer(new implementation::linked_value_proxy(pimpl_->key, pimpl_->value)));
}
