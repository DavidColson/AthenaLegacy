
#pragma once


#include <istream>


namespace cppfs
{


/**
*  @brief
*    Input stream
*/
class InputStream : public std::istream
{
public:
    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] sb
    *    Stream buffer (must NOT be null!)
    *
    *  @remarks
    *    The input stream takes ownership over the stream buffer
    */
    explicit InputStream(std::streambuf * sb);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~InputStream();


protected:
    std::streambuf * m_sb; ///< The associated stream buffer
};


} // namespace cppfs
