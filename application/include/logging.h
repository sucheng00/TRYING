#pragma once

#include <rcutils/logging_macros.h>

const char *logging_get_logger_name(void);

#define LOG_INFO(...)                                                          \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_INFO_NAMED(__log_logger_name, __VA_ARGS__);            \
	} while (0)
#define LOG_INFO_ONCE(...)                                                     \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_INFO_ONCE_NAMED(__log_logger_name, __VA_ARGS__);       \
	} while (0)
#define LOG_INFO_EXPRESSION(expression, ...)                                   \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_INFO_EXPRESSION_NAMED(expression, __log_logger_name,   \
			                                  __VA_ARGS__);                    \
	} while (0)
#define LOG_INFO_FUNCTION(function, ...)                                       \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_INFO_FUNCTION_NAMED(function, __log_logger_name,       \
			                                __VA_ARGS__);                      \
	} while (0)
#define LOG_INFO_SKIPFIRST(...)                                                \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_INFO_SKIPFIRST_NAMED(__log_logger_name, __VA_ARGS__);  \
	} while (0)
#define LOG_INFO_THROTTLE(get_time_point_value, duration, ...)                 \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_INFO_THROTTLE_NAMED(get_time_point_value, duration,    \
			                                __log_logger_name, __VA_ARGS__);   \
	} while (0)
#define LOG_INFO_SKIPFIRST_THROTTLE(get_time_point_value, duration, ...)       \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_INFO_SKIPFIRST_THROTTLE_NAMED(                         \
			    get_time_point_value, duration, __log_logger_name,             \
			    __VA_ARGS__);                                                  \
	} while (0)

#define LOG_WARNING(...)                                                       \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_WARNING_NAMED(__log_logger_name, __VA_ARGS__);         \
	} while (0)
#define LOG_WARNING_ONCE(...)                                                  \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_WARNING_ONCE_NAMED(__log_logger_name, __VA_ARGS__);    \
	} while (0)
#define LOG_WARNING_EXPRESSION(expression, ...)                                \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_WARNING_EXPRESSION_NAMED(                              \
			    expression, __log_logger_name, __VA_ARGS__);                   \
	} while (0)
#define LOG_WARNING_FUNCTION(function, ...)                                    \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_WARNING_FUNCTION_NAMED(function, __log_logger_name,    \
			                                   __VA_ARGS__);                   \
	} while (0)
#define LOG_WARNING_SKIPFIRST(...)                                             \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_WARNING_SKIPFIRST_NAMED(__log_logger_name,             \
			                                    __VA_ARGS__);                  \
	} while (0)
#define LOG_WARNING_THROTTLE(get_time_point_value, duration, ...)              \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_WARNING_THROTTLE_NAMED(get_time_point_value, duration, \
			                                   __log_logger_name,              \
			                                   __VA_ARGS__);                   \
	} while (0)
#define LOG_WARNING_SKIPFIRST_THROTTLE(get_time_point_value, duration, ...)    \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_WARNING_SKIPFIRST_THROTTLE_NAMED(                      \
			    get_time_point_value, duration, __log_logger_name,             \
			    __VA_ARGS__);                                                  \
	} while (0)

#define LOG_ERROR(...)                                                         \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_ERROR_NAMED(__log_logger_name, __VA_ARGS__);           \
	} while (0)
#define LOG_ERROR_ONCE(...)                                                    \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_ERROR_ONCE_NAMED(__log_logger_name, __VA_ARGS__);      \
	} while (0)
#define LOG_ERROR_EXPRESSION(expression, ...)                                  \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_ERROR_EXPRESSION_NAMED(expression, __log_logger_name,  \
			                                   __VA_ARGS__);                   \
	} while (0)
#define LOG_ERROR_FUNCTION(function, ...)                                      \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_ERROR_FUNCTION_NAMED(function, __log_logger_name,      \
			                                 __VA_ARGS__);                     \
	} while (0)
#define LOG_ERROR_SKIPFIRST(...)                                               \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_ERROR_SKIPFIRST_NAMED(__log_logger_name, __VA_ARGS__); \
	} while (0)
#define LOG_ERROR_THROTTLE(get_time_point_value, duration, ...)                \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_ERROR_THROTTLE_NAMED(get_time_point_value, duration,   \
			                                 __log_logger_name, __VA_ARGS__);  \
	} while (0)
#define LOG_ERROR_SKIPFIRST_THROTTLE(get_time_point_value, duration, ...)      \
	do {                                                                       \
		const char *__log_logger_name = logging_get_logger_name();             \
		if (__log_logger_name != NULL)                                         \
			RCUTILS_LOG_ERROR_SKIPFIRST_THROTTLE_NAMED(                        \
			    get_time_point_value, duration, __log_logger_name,             \
			    __VA_ARGS__);                                                  \
	} while (0)
