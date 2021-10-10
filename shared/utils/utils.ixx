module;

#include <shared/defs.h>

#include <TlHelp32.h>
#include <Psapi.h>

export module utils;

export namespace utils
{
	inline std::mt19937_64 mt;

	namespace rtti
	{
		template <typename T, typename Ty>
		T* safe_cast(Ty ptr)
		{
			if (!ptr)
				return nullptr;

			return (T::check_class(ptr) ? static_cast<T*>(ptr) : nullptr);
		}
	}

	namespace time
	{
		tm* get_tm_date()
		{
			auto tick = ::time(0);
			return localtime(&tick);
		}

		std::string get_str_date(char s1, char s2, char s3)
		{
			auto date = get_tm_date();

			auto month = date->tm_mon + 1;

			auto month_prefix = (month < 10 ? "0" : "") + std::to_string(month),
				 day_prefix = (date->tm_mday < 10 ? "0" : "") + std::to_string(date->tm_mday),
				 hour_prefix = (date->tm_hour < 10 ? "0" : "") + std::to_string(date->tm_hour),
				 minute_prefix = (date->tm_min < 10 ? "0" : "") + std::to_string(date->tm_min),
				 second_prefix = (date->tm_sec < 10 ? "0" : "") + std::to_string(date->tm_sec);

			return day_prefix + s1 + month_prefix + s1 + std::to_string(1900 + date->tm_year) + s3 + hour_prefix + s2 + minute_prefix + s2 + second_prefix;
		}

		std::string get_str_time(char s1)
		{
			auto date = get_tm_date();

			auto hour_prefix = (date->tm_hour < 10 ? "0" : "") + std::to_string(date->tm_hour),
				 minute_prefix = (date->tm_min < 10 ? "0" : "") + std::to_string(date->tm_min),
				 second_prefix = (date->tm_sec < 10 ? "0" : "") + std::to_string(date->tm_sec);

			return hour_prefix + s1 + minute_prefix + s1 + second_prefix;
		}
	}

	namespace file
	{
		size_t get_size(std::ifstream& file)
		{
			file.seekg(0, file.end);
			auto length = file.tellg();
			file.seekg(0, file.beg);

			return static_cast<size_t>(length);
		}

		size_t get_size(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::binary);
			if (!file)
				return std::string::npos;

			file.seekg(0, file.end);
			auto length = file.tellg();
			file.seekg(0, file.beg);

			return static_cast<size_t>(length);
		}

		template <typename T>
		std::vector<char> read_file(const T& filename)
		{
			std::ifstream file(filename, std::ios::binary);
			if (!file)
				return {};

			size_t file_size = get_size(file);
			if (file_size <= 0)
			{
				file.close();
				return {};
			}

			std::vector<char> data;

			data.resize(file_size);

			file.read(data.data(), file_size);
			file.close();

			return data;
		}


		uint64_t hash_file_simple(const std::string& filename)
		{
			auto file = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (!file || file == INVALID_HANDLE_VALUE)
				return 0;

			FILETIME modified;

			GetFileTime(file, nullptr, nullptr, &modified);
			CloseHandle(file);

			return *(uint64_t*)&modified;
		}

		bool set_file_modified_time(const std::string& filename, uint64_t value)
		{
			auto file = CreateFileA(filename.c_str(), FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, 0, nullptr);
			if (!file || file == INVALID_HANDLE_VALUE)
				return false;

			auto ft = *(FILETIME*)&value;

			SetFileTime(file, nullptr, nullptr, &ft);
			CloseHandle(file);

			return true;
		}
	}

	namespace string
	{
		inline std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> string_converter;

		std::vector<std::string> split(const std::string& str, char delimiter)
		{
			std::stringstream ss(str);

			std::vector<std::string> out;

			std::string curr;

			while (std::getline(ss, curr, delimiter))
				out.push_back(curr);

			return out;
		}

		bool split_left(const std::string& str, std::string& left, std::string& right, char delimiter)
		{
			if (auto delimiter_pos = str.find(delimiter); delimiter_pos != std::string::npos)
			{
				std::string temp = str;

				left = temp.substr(0, delimiter_pos);
				right = temp.substr(delimiter_pos + 1);

				return true;
			}

			return false;
		}

		std::wstring color_to_string(uint32_t color)
		{
			std::wstringstream ss;

			ss << L"##" << std::hex << std::setw(8) << std::setfill(L'0') << color;

			return ss.str();
		}

		bool bool_from_str(const std::string& str)
		{
			return !str.compare("true");
		}

		namespace impl
		{
			template <typename T>
			constexpr size_t constexpr_strlen(const T* val)
			{
				size_t size = 0;
				auto p = val;
				while (*p++ != 0) ++size;
				return size;
			}

			template <typename Tx, typename Ty>
			auto split_str(const Tx& str, Ty delimitator_beg, Ty delimitator_end)
			{
				std::boyer_moore_horspool_searcher bms(delimitator_beg, delimitator_end);
				std::vector<Tx> words;

				const auto str_end = str.end();

				auto curr_it = str.begin();

				for (auto&& [beg, end] = bms(str.begin(), str_end);
					beg != str_end;
					curr_it = end, std::tie(beg, end) = bms(end, str_end))
				{
					if (const auto& word = Tx(curr_it, beg); !word.empty())
						words.push_back(word);
				}

				if (curr_it != str_end)
					words.push_back(Tx(curr_it, str_end));

				return words;
			}
		}

		template <typename Tx>
		auto split(const Tx& str, const Tx& delimiter)
		{
			return impl::split_str(str, delimiter.begin(), delimiter.end());
		}

		template <typename Tx>
		auto split(const Tx& str, const char* delimiter)
		{
			return impl::split_str(str, delimiter, delimiter + std::strlen(delimiter));
		}

		template <typename Tx>
		auto split(const Tx& str, const wchar_t* delimiter)
		{
			return impl::split_str(str, delimiter, delimiter + std::wcslen(delimiter));
		}

		std::wstring convert(const std::string& str)
		{
			return string_converter.from_bytes(str);
		}

		std::string convert(const std::wstring& str)
		{
			return string_converter.to_bytes(str);
		}

		void clear_resize(std::string& str, size_t new_size)
		{
			str.clear();
			str.resize(new_size);
		}
	}

	namespace container
	{
		template <typename T, typename Fn>
		void for_each_safe(T& container, const Fn& fn)
		{
			for (auto it = container.begin(); it != container.end();)
			{
				if (fn(&(*it)))
					++it;
				else it = container.erase(it);
			}
		}

		template <typename T, typename Fn>
		void for_each_safe_ptr(T& container, const Fn& fn)
		{
			for (auto it = container.begin(); it != container.end();)
			{
				if (fn(*it))
					++it;
				else it = container.erase(it);
			}
		}
	}

	namespace hash
	{
		// Jenkins hashing algorithm
		template <typename T, std::enable_if_t<std::is_same_v<T, const char*>>* = nullptr>
		constexpr auto JENKINS(T word)
		{
			uint32_t hash = 0;

			do
			{
				hash += *word;
				hash += (hash << 10);
				hash ^= (hash >> 6);

			} while (*word++);

			hash += (hash << 3);
			hash ^= (hash >> 11);
			hash += (hash << 15);

			return hash;
		}

		// Jenkins hashing algorithm (std::string)
		template <typename T, std::enable_if_t<std::is_same_v<T, std::string>>* = nullptr>
		constexpr auto JENKINS(const T& str)
		{
			uint32_t hash = 0;

			for (const auto& c : str)
			{
				hash += c;
				hash += (hash << 10);
				hash ^= (hash >> 6);
			}

			hash += (hash << 3);
			hash ^= (hash >> 11);
			hash += (hash << 15);

			return hash;
		}

		constexpr uint32_t MURMUR(const char* word, int len)
		{
			auto temp_word_ptr = word;

			if (len == -1)
				do ++len; while (*temp_word_ptr++);

			uint32_t h1 = 0x19977991,
				c1 = 0xcc9e2d51,
				c2 = 0x1b873593;

			const auto nblocks = len / 4;
			const auto data = (const uint8_t*)word;
			const auto blocks = (const uint32_t*)(data + nblocks * 4);

			for (auto i = -nblocks; i; ++i)
			{
				uint32_t k1 = blocks[i];

				k1 *= c1;
				k1 = _rotl(k1, 15);
				k1 *= c2;

				h1 ^= k1;
				h1 = _rotl(h1, 13);
				h1 = h1 * 5 + 0xe6546b64;
			}

			const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);

			uint32_t k1 = 0;

			switch (len & 3)
			{
			case 3: k1 ^= tail[2] << 16;
			case 2: k1 ^= tail[1] << 8;
			case 1: k1 ^= tail[0];
				k1 *= c1; k1 = _rotl(k1, 15); k1 *= c2; h1 ^= k1;
			};

			h1 ^= len;
			h1 ^= h1 >> 16;
			h1 *= 0x85ebca6b;
			h1 ^= h1 >> 13;
			h1 *= 0xc2b2ae35;
			h1 ^= h1 >> 16;

			return h1;
		}

		inline void hash_combine(size_t& seed) {}

		template <typename T, typename... A>
		inline void hash_combine(size_t& seed, const T& v, A... rest)
		{
			seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
			hash_combine(seed, rest...);
		}
	}

	namespace mem
	{
		template <typename T, typename Y>
		void move(Y& dest, const T& src)
		{
			*(T*)&dest = src;
		}

		template <typename T, typename Y>
		void move_ex(Y& dest, const T& src)
		{
			dest = *(Y*)&src;
		}

		template <typename T, typename Y>
		T as(const Y& v)
		{
			return *(T*)&v;
		}

		void for_each_process(const std::function<bool(uint32_t pid, const char* name)>& fn, const std::vector<std::string>& ignored_processes)
		{
			PROCESSENTRY32 process_entry;

			process_entry.dwSize = sizeof(PROCESSENTRY32);

			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

			if (!Process32First(snapshot, &process_entry))
			{
				CloseHandle(snapshot);
				return;
			}

			do {
				std::string exe_name(process_entry.szExeFile);

				std::transform(exe_name.begin(), exe_name.end(), exe_name.begin(), ::tolower);

				if (std::find_if(ignored_processes.begin(), ignored_processes.end(), [&](const std::string& str)
					{
						return !str.compare(exe_name);
					}) == ignored_processes.end())
				{
					if (fn(process_entry.th32ProcessID, exe_name.c_str()))
						break;
				}
			} while (Process32Next(snapshot, &process_entry));

			CloseHandle(snapshot);
		}

		void for_each_module(uint32_t pid, const std::function<bool(uintptr_t base_addr, uint32_t size, const char* name)>& fn, const std::vector<std::string>& ignored_mods)
		{
			MODULEENTRY32 mod_entry;

			mod_entry.dwSize = sizeof(MODULEENTRY32);

			HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

			if (!Module32First(snapshot, &mod_entry))
			{
				CloseHandle(snapshot);
				return;
			}

			do {
				std::string mod_name(mod_entry.szModule);

				std::transform(mod_name.begin(), mod_name.end(), mod_name.begin(), ::tolower);

				if (std::find_if(ignored_mods.begin(), ignored_mods.end(), [&](const std::string& str)
					{
						return !str.compare(mod_name);
					}) == ignored_mods.end())
				{
					if (fn((uintptr_t)mod_entry.modBaseAddr, mod_entry.modBaseSize, (const char*)mod_entry.szModule))
						break;
				}
			} while (Module32Next(snapshot, &mod_entry));

			CloseHandle(snapshot);
		}
	}

	namespace rand
	{
		template <typename T>
		T rand_int(T min, T max) { return std::uniform_int_distribution<T>(min, max)(mt); }
	}

	namespace win
	{
		std::wstring get_clipboard_text()
		{
			if (!OpenClipboard(nullptr))
				return {};

			HANDLE data = GetClipboardData(CF_UNICODETEXT);
			if (!data)
				return {};

			auto text = static_cast<wchar_t*>(GlobalLock(data));
			if (!text)
				return {};

			std::wstring out(text);

			GlobalUnlock(data);

			CloseClipboard();

			return out;
		}
	}
}