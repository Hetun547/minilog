#include <log_worker.h>

#define error(LOGGER, FORMAT, ...) \
minilog::write_log(LOGGER, LOG_LEVEL::ERROR, __FILE__, __LINE__, FORMAT, __VA_ARGS__)

#define warning(LOGGER, FORMAT, ...) \
minilog::write_log(LOGGER, LOG_LEVEL::WARNING, __FILE__, __LINE__, FORMAT, __VA_ARGS__)

#define info(LOGGER, FORMAT, ...) \
minilog::write_log(LOGGER, LOG_LEVEL::INFO, __FILE__, __LINE__, FORMAT, __VA_ARGS__)

#define debug(LOGGER, FORMAT, ...) \
minilog::write_log(LOGGER, LOG_LEVEL::DEBUG, __FILE__, __LINE__, FORMAT, __VA_ARGS__)