
#ifndef _hulk_core_logger_h_
#define _hulk_core_logger_h_

#include <map>
#include <sstream>
#include <iostream>
#include <cstring>

namespace hulk {
namespace core {

// -----------------------------------------------------------------------------
#define LOG_DEBUG( l, m ) { std::stringstream __ss; __ss << std::endl << "DBG " << ::hulk::core::filename( __FILE__ ) << ":" << __LINE__ << " - " << m; l.write( ::hulk::core::log::DEBUG, __ss.str() ); }
#define LOG_INFO( l, m )  { std::stringstream __ss; __ss << std::endl << "INF " << ::hulk::core::filename( __FILE__ ) << ":" << __LINE__ << " - " << m; l.write( ::hulk::core::log::INFO, __ss.str() );  }
#define LOG_ERROR( l, m ) { std::stringstream __ss; __ss << std::endl << "ERR " << ::hulk::core::filename( __FILE__ ) << ":" << __LINE__ << " - " << m; l.write( ::hulk::core::log::ERROR, __ss.str() ); }

inline const char* filename( const char* s ) {
    return strrchr( s, '/' ) ? strrchr( s, '/' ) + 1 : s;
}

// -----------------------------------------------------------------------------
class log
{
public:
    typedef enum {
        DEBUG, INFO, WARN, ERROR
    } level;

    log( level l )
    {
        _level = l;
    }

    virtual void write( level l, const std::string& s )
    {
        if( l >= _level ) std::cout << s << std::endl;
    }

private:
    level _level;
};

// -----------------------------------------------------------------------------
class logger
{
public:
    static logger& instance()
    {
        if( !_instance ) {
            _instance = new logger;
        }
        return *_instance;
    }

    log& get( const std::string& id )
    {
        logmap::const_iterator it = _logs.find( id );
        if( it == _logs.end() ) {
            return _default_log;
        } else {
            return *it->second;
        }
    }

private:
    logger() : _default_log( log::DEBUG ) {}

    typedef std::map< std::string, log* > logmap;

    log _default_log;
    logmap _logs;

    static logger* _instance;
};

}
}

#endif
