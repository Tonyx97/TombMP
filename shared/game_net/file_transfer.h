#pragma once

class file_batch
{
public:

	struct file_transfer_info
	{
		std::string filename;

		std::vector<char> data;

		FILE_HASH hash = 0;

		int data_offset = 0,
			size = 0;

		file_transfer_info(const std::string& filename, std::vector<char>& data, FILE_HASH hash) :
			filename(filename),
			data(std::move(data)),
			hash(hash) { size = int(this->data.size()); }
	};

private:

	gns::file::file_transfer_data info {};

	std::stack<file_transfer_info*> files_data;

	SLNet::SystemAddress target;

	file_transfer_info* curr_file = nullptr;

public:

	file_batch(const SLNet::SystemAddress& target) : target(target)
	{
		info.bytes_sent = info.files_sent = 0;
	}

	~file_batch()
	{
		while (!files_data.empty())
			delete_and_remove_top_file_data();
	}

	void delete_and_remove_top_file_data()
	{
		auto top = files_data.top();
		delete top;
		files_data.pop();
	}

	void next_file()
	{
		if (is_empty())
			return;

		delete_and_remove_top_file_data();

		if (files_data.empty())
		{
			curr_file = nullptr;
			return;
		}

		update_transfer_info();
	}

	void update_transfer_info()
	{
		if (curr_file = files_data.top())
		{
			info.filename = curr_file->filename.c_str();
			info.file_size = curr_file->size;
		}
	}

	void begin();
	void end();
	void increase_bytes_sent(int v)											{ info.bytes_sent += v; }
	void increase_files_sent(int v = 1)										{ info.files_sent += v; }
	void add_file_data(file_transfer_info* fti)
	{
		files_data.push(fti);
		info.total_size += fti->size;
		info.total_files++;
	}
	
	bool is_empty() const													{ return files_data.empty(); }

	int get_total_size() const												{ return info.total_size; }
	int get_total_files() const												{ return info.total_files; }

	file_transfer_info* get_curr_file() const								{ return curr_file; }

	SLNet::SystemAddress get_target() const									{ return target; }

	const gns::file::file_transfer_data& get_info() const					{ return info; }

	static constexpr int MAX_DELTA()										{ return 1024 * 1024; }

};

class file_transfer
{
private:

#if defined(GAME_SERVER)
	std::vector<file_batch*> batches;
#endif


public:

	file_transfer();
	~file_transfer();

	void dispatch();
	void add_batch(const SLNet::SystemAddress& target, const std::vector<file_info>& files);
};