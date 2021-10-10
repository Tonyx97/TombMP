import utils;
import prof;

#include <shared/defs.h>
#include <shared/scripting/resource_system.h>
#include <shared/scripting/resource.h>

#include <mp/client.h>
#include <ui/ui.h>

#include "hl_net_defs.h"

void file_handlers::handle_packet(uint16_t pid)
{
	switch (pid)
	{
	case ID_FILE_BEGIN_TRANSFER:	return on_begin_transfer();
	case ID_FILE_END_TRANSFER:		return on_end_transfer();
	case ID_FILE_LIST:				return on_file_list();
	case ID_FILE_TRANSFER:			return on_transfer();
	}
}

void file_handlers::on_begin_transfer()
{
	gns::file::file_transfer_data info; g_client->read_packet_ex(info);

	prof::print(YELLOW, "Download Started");

	g_client->begin_download();
	g_ui->begin_download_bar(float(info.total_size));
}

void file_handlers::on_end_transfer()
{
	prof::print(YELLOW, "Download Ended");
	g_ui->end_download_bar();
	g_client->end_download();

	if (g_client->is_ready())
		g_client->sync_resource_all_status();
}

void file_handlers::on_file_list()
{
	auto bs = g_client->get_current_bs();

	bool all_files_up_to_date = false; bs->Read(all_files_up_to_date);

	int size = 0; bs->Read(size);
	if (size <= 0)
		return;

	for (int i = 0; i < size; ++i)
	{
		gns_string<char, GNS_GLOBALS::MAX_RESOURCE_NAME_LENGTH> filename; bs->Read(filename);

		std::string rsrc, subpath;

		if (!utils::string::split_left(*filename, rsrc, subpath, '\\'))
			continue;

		g_client->save_resource_file(rsrc, subpath);

		prof::print(GREEN, "{} from {} is up to date", subpath, rsrc);
	}

	if (all_files_up_to_date)
	{
		g_client->cancel_wait();
		g_client->sync_resource_all_status();
	}
}

void file_handlers::on_transfer()
{
	auto bs = g_client->get_current_bs();

	gns::file::file_transfer_data info; bs->Read(info);

	std::vector<char> data(info.delta);

	bs->Read(data.data(), info.delta);

	std::string filename = *info.filename;

	if (g_client->add_file_data(filename, data, info.file_size, info.file_hash))
		g_client->save_file(filename);

	g_ui->set_download_bar_progress(float(info.bytes_sent));

	prof::print(YELLOW, "Transfering {} ({} / {} bytes) | ({} / {} files) | ({} / {} bytes of this file)...",
		*info.filename,
		info.bytes_sent,
		info.total_size,
		info.files_sent,
		info.total_files,
		info.file_bytes_transferred,
		info.file_size);
}