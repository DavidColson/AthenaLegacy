
#pragma once


#include <EASTL/string.h>


namespace cppfs
{


/**
*  @brief
*    Url wrapper
*
*    This class is a wrapper that enables limited parsing of URLs.
*/
class Url
{
public:
    /**
    *  @brief
    *    Constructor
    */
    Url();

    /**
    *  @brief
    *    Copy constructor
    *
    *  @param[in] url
    *    Url to copy
    */
    Url(const Url & url);

    /**
    *  @brief
    *    Move constructor
    *
    *  @param[in] url
    *    Url to move
    */
    Url(Url && url);

    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] url
    *    Url
    */
    Url(const eastl::string & url);

    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] url
    *    Url
    */
    Url(eastl::string && url);

    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] url
    *    Url
    */
    Url(const char * url);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~Url();

    /**
    *  @brief
    *    Copy assignment operator
    *
    *  @param[in] url
    *    Right-hand value to copy
    *
    *  @return
    *    Reference to this value
    */
    Url & operator=(const Url & url);

    /**
    *  @brief
    *    Move assignment operator
    *
    *  @param[in] url
    *    Right-hand value to move
    *
    *  @return
    *    Reference to this value
    */
    Url & operator=(Url && url);

    /**
    *  @brief
    *    Get url as string
    *
    *  @return
    *    Url
    */
    const eastl::string & toString() const;

    /**
    *  @brief
    *    Get protocol
    *
    *  @return
    *    Protocol part (including "://")
    */
    const eastl::string & protocol() const;

    /**
    *  @brief
    *    Get location
    *
    *  @return
    *    Location (everything after the protocol part)
    */
    const eastl::string & location() const;

    /**
    *  @brief
    *    Get address
    *
    *  @return
    *    Address (including username, password, and hostname)
    */
    const eastl::string & address() const;

    /**
    *  @brief
    *    Get path
    *
    *  @return
    *    Path (everything after the address)
    */
    const eastl::string & path() const;

    /**
    *  @brief
    *    Get login
    *
    *  @return
    *    Login part (username and password)
    */
    const eastl::string & login() const;

    /**
    *  @brief
    *    Get host
    *
    *  @return
    *    Host name
    */
    const eastl::string & host() const;

    /**
    *  @brief
    *    Get user name
    *
    *  @return
    *    User name
    */
    const eastl::string & username() const;

    /**
    *  @brief
    *    Get password
    *
    *  @return
    *    Password
    */
    const eastl::string & password() const;


protected:
    /**
    *  @brief
    *    Analyze url
    */
    void analyze() const;


protected:
            eastl::string m_url;      ///< Url string
    mutable bool          m_analyzed; ///< 'true' if Url has been analyzed, else 'false'
    mutable eastl::string m_protocol; ///< Protocol part (including "://")
    mutable eastl::string m_location; ///< Location (everything after the protocol part)
    mutable eastl::string m_address;  ///< Address (including username, password, and hostname)
    mutable eastl::string m_path;     ///< Path (everything after the address)
    mutable eastl::string m_login;    ///< Login part (username and password)
    mutable eastl::string m_host;     ///< Host name
    mutable eastl::string m_username; ///< User name
    mutable eastl::string m_password; ///< Password
};


} // namespace cppfs
