#include "log_worker.h"
#include <queue>
#include <shared_mutex>
#include <condition_variable>
#include <map>
#include <thread>
#include <fstream>
using namespace std::chrono_literals;
namespace fs = std::filesystem;
std::filesystem::path minilog::log_dir_path = fs::current_path();
int minilog::log_out_time_days = 0;

static int writer_thread_count = 3;

class log_pool;
class log_cleaner
{
public:
	log_cleaner(log_pool* pool);
	log_cleaner(log_cleaner&&) = delete;
	log_cleaner(log_cleaner&) = delete;
	~log_cleaner();

	void clear_log_by_time();

private:
	log_pool* pool = nullptr;
	std::thread worker;

};
class log_writer
{
public:
	log_writer(log_pool* pool);
	log_writer(log_writer&) = delete;
	log_writer(log_writer&&) = delete;
	~log_writer();

private:
	void loop_write();
	bool write(const std::filesystem::path& file_path, const char* log_data);

	log_pool* pool = nullptr;
	std::vector<std::thread> vect_worker;
};

class log_pool
{
public:
	log_pool(log_pool&) = delete;
	log_pool(log_pool&&) = delete;
	~log_pool();

	void init();

	static log_pool* getinstance() {
		return &instace;
	}

	void push(std::tuple<const std::filesystem::path, const  char*>& log_data);
	std::tuple<const std::filesystem::path, const char*> get_log_data();
private:
	friend class log_writer;
	friend class log_cleaner;

	log_pool() = default;
	static log_pool instace;

	std::queue<std::tuple<const std::filesystem::path, const char*>> pool;
	std::mutex mtx_pool;
	std::condition_variable cv_pool;
	std::atomic_bool b_has_log = false;
	std::atomic_bool b_quit = false;

	log_writer* writer;
	log_cleaner* cleaner;

};

log_pool log_pool::instace = log_pool();


log_pool::~log_pool()
{

	b_quit = true;
	cv_pool.notify_all();
	if (writer != nullptr) {
		delete writer;
		writer = nullptr;
	}
	if (cleaner != nullptr) {
		delete cleaner;
		cleaner = nullptr;
	}
}

void log_pool::init()
{
	writer = new log_writer(this);
	cleaner = new log_cleaner(this);
}

void log_pool::push(std::tuple<const std::filesystem::path, const char*>& log_data)
{
	std::unique_lock<std::mutex> locker(mtx_pool);
	pool.push(std::move(log_data));
	b_has_log = true;
	cv_pool.notify_one();
}

std::tuple<const std::filesystem::path, const char*> log_pool::get_log_data()
{
	if (pool.empty()) {
		b_has_log = false;
		return std::tuple<char*, char*>(nullptr, nullptr);
	}

	auto log_data = pool.front();
	pool.pop();
	if (pool.empty())
		b_has_log = false;
	return log_data;
}

log_writer::log_writer(log_pool* pool)
	: pool(pool)
{
	for (int i = 0; i < writer_thread_count; ++i) {
		vect_worker.push_back(std::move(std::thread(&log_writer::loop_write, this)));
	}
}

log_writer::~log_writer()
{
	for (int i = 0; i < writer_thread_count; ++i) {
		vect_worker[i].join();
	}
	pool = nullptr;
}

void log_writer::loop_write()
{
	try {
		for (;;) {
			if (pool->b_quit && pool->pool.empty())
				break;
			std::unique_lock<std::mutex> locker(pool->mtx_pool);

			while (!(pool->b_has_log || pool->b_quit))
				pool->cv_pool.wait(locker);
			if (pool->b_quit && pool->pool.empty())
				break;
			auto log_path_data = pool->get_log_data();
			locker.unlock();

			auto file_path = std::get<0>(log_path_data);
			auto log_data = std::get<1>(log_path_data);
			if (log_data == nullptr || file_path.empty())
				continue;


			int iretry = 0;
			while (!write(file_path, log_data)) {
				iretry++;
				std::this_thread::sleep_for(1ms);
				if (iretry >= 5000) {
					continue;
				}
			}
		}
	}
	catch (std::exception& ex) {
		throw(ex.what());
	}
}

bool log_writer::write(const std::filesystem::path& file_path, const char* log_data)
{
	std::fstream fstream;
	fstream.open(file_path, std::ios::out | std::ios_base::app | std::ios_base::binary);
	if (!fstream.is_open())
		return false;
	fstream << "thread: " << std::to_string(*(uint32_t*)&std::this_thread::get_id()) << "  ";
	fstream << log_data;
	fstream.close();
	return true;
}



bool minilog::init(const char* log_dir, uint32_t out_time)
{
	minilog::log_out_time_days = out_time;
	minilog::log_dir_path = fs::path(log_dir);
	fs::create_directories(fs::path(log_dir));
	if(!fs::exists(minilog::log_dir_path))
		return false;
	
	log_pool::getinstance()->init();
	return true;
}

void minilog::push_log(std::tuple<const std::filesystem::path, const char*> log_data)
{
	log_pool::getinstance()->push(log_data);

}

log_cleaner::log_cleaner(log_pool* pool)
	: pool(pool)
{
	if(minilog::log_out_time_days > 0)
		worker = std::thread(&log_cleaner::clear_log_by_time, this);
}

log_cleaner::~log_cleaner()
{
	if (minilog::log_out_time_days > 0)
		worker.join();
	pool = nullptr;
}

void log_cleaner::clear_log_by_time()
{
	if (minilog::log_out_time_days <= 0)
		return;

	auto last_check_point = std::chrono::system_clock::now() - 4h;
	for (;;) {
		std::this_thread::sleep_for(50ms);
		if (pool->b_quit)
			break;
		auto now = std::chrono::system_clock::now();
		if (now - last_check_point > 1h) {

			if (fs::exists(minilog::log_dir_path) && fs::is_directory(minilog::log_dir_path)) {
				for (const auto& entry : fs::directory_iterator(minilog::log_dir_path)) {

					if (pool->b_quit)
						break;

					if (entry.is_directory())
						continue;
					auto file_extension = entry.path().extension();
					if (file_extension != fs::path(".log"))
						continue;

					auto duration = std::filesystem::file_time_type::clock::now() - entry.last_write_time();
					auto differ = std::chrono::duration_cast<std::chrono::hours>(duration);
					if (differ >= std::chrono::hours(minilog::log_out_time_days * 24)) {
						std::filesystem::remove(entry.path());
					}
				}
			}
			
			last_check_point = std::chrono::system_clock::now();
		}
	}
}
