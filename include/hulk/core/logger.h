#ifndef _hulk_core_logger_h_
#define _hulk_core_logger_h_

#include <iostream>

#define LOG_DEBUG( msg ) std::cerr << "DBG " << __FILE__ << ":" << __LINE__ << " - " << msg << std::endl
#define LOG_INFO( msg )  std::cerr << "INF " << __FILE__ << ":" << __LINE__ << " - " << msg << std::endl
#define LOG_ERROR( msg ) std::cerr << "ERR " << __FILE__ << ":" << __LINE__ << " - " << msg << std::endl

#endif
