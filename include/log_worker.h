#pragma once
#include <string>
#include <filesystem>
#include <ctime>
#include <tuple>


enum LOG_LEVEL
{
	UNKNOWN_LEVEL,
	DEBUG,
	INFO,
	WARNING,
	ERROR
};


/// @brief max length for single log msg, changing it as your wish			   	
constexpr int single_log_max_length = 4*1024;



class minilog
{
public:
	/// @brief 
	/// @param log_dir dir path for log
	/// @param single_log_max_length 
	/// @param out_time out_time_days
	bool static init(const char* log_dir, uint32_t out_time = 0);

	template <typename... Args>
	void static write_log(const char* logger, LOG_LEVEL level,char* file_name, int log_line, const char* log_formation, Args...args);

	static std::filesystem::path log_dir_path;
	static int log_out_time_days;
private:
	void static push_log(std::tuple<const std::filesystem::path, const char*> log_data);

};

template<typename ...Args>
inline void minilog::write_log(const char* logger, LOG_LEVEL level, char* file_name, int log_line, const char* log_formation, Args ...args)
{
	try {
		auto now = std::chrono::system_clock::now();
		std::time_t cur_time_t = std::chrono::system_clock::to_time_t(now);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		ms %= 1000;

		tm cur_time;
		localtime_s(&cur_time, &cur_time_t);

		constexpr auto log_file_name_length = 1024; // "%s_%02d_%02d_%02d.log"
		constexpr auto log_with_date_info_length = single_log_max_length + 1024;

		char log_file_name[log_file_name_length] = { 0 };
		char log_data[single_log_max_length] = { 0 };
		char log_data_with_date_info[log_with_date_info_length] = { 0 };

#ifdef NDEBUG
		if (level == LOG_LEVEL::DEBUG)
			return;
#endif // 



		char* str_level;
		switch (level) {
		case LOG_LEVEL::DEBUG:
			str_level = "debug";
			break;
		case LOG_LEVEL::INFO:
			str_level = "info";
			break;
		case LOG_LEVEL::WARNING:
			str_level = "warning";
			break;
		case LOG_LEVEL::ERROR:
			str_level = "error";
			break;
		default:
			str_level = "unknown";
			break;
		}

		sprintf(log_file_name, "%s_%02d_%02d_%02d.log", logger, (cur_time.tm_year + 1900) % 100, cur_time.tm_mon + 1, cur_time.tm_mday);
		sprintf(log_data, log_formation, args...);
		while (log_data[strlen(log_data) - 1] == '\n')
			log_data[strlen(log_data) - 1] = '\0';
		sprintf(log_data_with_date_info, "%s %02d:%02d:%02d.%03d %s %d\n %s: %s ...\n", logger, cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec, (int)ms, file_name, log_line, str_level, (char*)log_data);

		auto log_file_path = log_dir_path / std::filesystem::path(log_file_name);
		push_log(std::make_tuple(log_file_path, (const char*)log_data_with_date_info));

	}
	catch (std::exception& ex) {
		throw(ex.what());
	}
	 
}


