
#include "hulk/core/logger.h"

hulk::logger* hulk::logger::_instance = 0;

void hulk::logger_cleanup()
{
    delete &hulk::logger::instance();
}
