/*!
 * \file
 * Defines a key/value storage class with a hash table backend.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <boost/thread/future.hpp>
#include <memory>
#include <stdexcept>

/*!
 * This key value storage may be thought of as a simple hash table, only accessible by key, with one primary
 * exception: Assignments to this hash table return immediately, but resolve asynchronously to a future value.
 * You may, for instance, block on a write to this hash table, and thus be sure the value was delivered:
 *
 *    storage s;
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
class storage
{
  public:
    typedef std::string key;
    typedef std::string value;
  
    /*! \post the table is empty. */
    storage ();
    ~storage ();
    
    /*! A return type for accessors which may be able to respond, "no item here." */
    class optional_value;
    
    /*!
     * Returns the value mapped by the given key. May return a value evaluating to "false", in which case
     * there was no value at this key. The value returned is directly assignable, so storage["foo"] = "bar"
     * constitutes setting the value at "foo" to "bar", irrespective of any previous values present.
     */
    optional_value& operator[] (const key&);
    const optional_value& operator[] (const key&) const;
    
  private:
    class implementation;
    std::unique_ptr<implementation> pimpl_;
};


class storage::optional_value
{
  private:
    class implementation;
      
  protected:
    friend class storage;
    optional_value (const storage::key& k);
    optional_value (std::shared_ptr<implementation>& p)
      : pimpl_(p)
    {   }
    
  public:
    /*!
     * Begins an asynchronous assignment to the value keyed here.
     * \return The time (in milliseconds) required to complete the storage.
     */
    boost::shared_future<size_t> set (const storage::value& v);
    
    /*! Removes the value mapped here from the store, asynchronously. */
    boost::shared_future<bool> unmap ();
    
    /*!
     * Begins an asynchronous read of this value from the store. The retrieved value will not be
     * cached; the only memory of it will be given with the returned future.
     */
    boost::shared_future<boost::optional<storage::value>> fetch () const;
    
  private:
    std::shared_ptr<implementation> pimpl_;  /*!< The single concrete value referred to by all copies by a particular key. */
};
