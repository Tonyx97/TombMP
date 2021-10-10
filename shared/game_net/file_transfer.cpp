import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>

#include <server/server.h>

#include "file_transfer.h"

void file_batch::begin()
{
	update_transfer_info();

	g_server->send_packet_broadcast_ex(ID_FILE_BEGIN_TRANSFER, target, false, info);
}

void file_batch::end()
{
	g_server->send_packet_broadcast_ex(ID_FILE_END_TRANSFER, target, false, info);
}

file_transfer::file_transfer()
{
}

file_transfer::~file_transfer()
{
}

void file_transfer::dispatch()
{
	utils::container::for_each_safe_ptr(batches, [&](file_batch* batch)
	{
		auto target = batch->get_target();
		auto file = batch->get_curr_file();

		auto& data = file->data;

		if (file->data_offset < file->size)
		{
			int delta = file->size - file->data_offset > file_batch::MAX_DELTA() ? file_batch::MAX_DELTA() : file->size - file->data_offset,
				old_data_offset = file->data_offset;

			file->data_offset += delta;

			batch->increase_bytes_sent(delta);

			const bool transfer_completed = file->data_offset >= file->size;

			if (transfer_completed)
				batch->increase_files_sent();

			auto bs = g_server->create_packet(ID_FILE_TRANSFER);

			auto batch_info = batch->get_info();

			batch_info.file_bytes_transferred = file->data_offset;
			batch_info.delta = delta;
			batch_info.file_hash = file->hash;

			bs->Write(batch_info);
			bs->Write(data.data() + old_data_offset, delta);

			if (g_server->send_packet_broadcast(bs, target, false) && transfer_completed)
				batch->next_file();
		}

		if (batch->is_empty())
		{
			batch->end();

			delete batch;

			return false;
		}

		return true;
	});
}

void file_transfer::add_batch(const SLNet::SystemAddress& target, const std::vector<file_info>& files)
{
	if (files.empty())
		return;

	auto batch = new file_batch(target);

	for (const auto& [rsrc, file, hash] : files)
		if (auto file_data = utils::file::read_file(resource_system::RESOURCES_PATH + file); !file_data.empty())
			batch->add_file_data(new file_batch::file_transfer_info(file, file_data, hash));

	if (batch->is_empty())
		return;

	batch->begin();

	batches.push_back(batch);
}