/*!
 * \file
 * Defines a key/value storage class with a hash table backend.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <object_access_parameters.hxx>
#include <boost/thread/future.hpp>
#include <memory>
#include <stdexcept>

//=============================================================================
namespace riak {
//=============================================================================

/*!
 * This key value store may be thought of as a simple hash table, only accessible by key, with one primary
 * exception: Assignments to this hash table return immediately, but resolve asynchronously to a future value.
 * You may, for instance, block on a write to this hash table, and thus be sure the value was delivered:
 *
 *    store s;
 *    auto result = s["breakfast"].set("ham and eggs");
 *    result.wait();
 *    if (result.has_value())
 *        assert(*s["breakfast"] == "ham and eggs")
 *    else
 *        cerr << "Write failed!" << endl;
 *
 * You may also "fire and forget" the write, by eliding everything after set(). A failure will be logged by
 * the implementation, but it will not invalidate future accesses of any keys in s.
 */
class store
{
  public:
    typedef std::string key;
    typedef std::string value;
    
    /*! A sane set of defaults that should work fine for buckets with N=3. */
    static const object_access_parameters access_defaults;
  
    /*! \post the table is empty. */
    store (const object_access_parameters& access_defaults = access_defaults);
    ~store ();
    
    /*! A return type for accessors which may be able to respond, "no item here." */
    class optional_value;
    
    /*!
     * Returns the value mapped by the given key. May return a value evaluating to "false", in which case
     * there was no value at this key. The value returned is directly assignable, so store["foo"] = "bar"
     * constitutes setting the value at "foo" to "bar", irrespective of any previous values present.
     */
    optional_value& operator[] (const key&);
    const optional_value& operator[] (const key&) const;
    
  private:
    class implementation;
    std::unique_ptr<implementation> pimpl_;
};


class store::optional_value
{
  private:
    class implementation;
      
  protected:
    friend class store;
    optional_value (const store::key& k);
    optional_value (std::shared_ptr<implementation>& p)
      : pimpl_(p)
    {   }
    
  public:
    /*!
     * Begins an asynchronous assignment to the value keyed here.
     * \return The time (in milliseconds) required to complete the store.
     */
    boost::shared_future<size_t> set (const store::value& v);
    
    /*! Removes the value mapped here from the store, asynchronously. */
    boost::shared_future<bool> unmap ();
    
    /*!
     * Begins an asynchronous read of this value from the store. The retrieved value will not be
     * cached; the only memory of it will be given with the returned future.
     */
    boost::shared_future<boost::optional<store::value>> fetch () const;
    
  private:
    std::shared_ptr<implementation> pimpl_;  /*!< The single concrete value referred to by all copies by a particular key. */
};

//=============================================================================
}   // namespace riak
//=============================================================================
