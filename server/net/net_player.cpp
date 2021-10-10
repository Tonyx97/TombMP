import utils;
import prof;

#include <shared/defs.h>

#include <shared/scripting/resource_system.h>
#include <shared/scripting/resource.h>

#include <server/server.h>

#include "net_player.h"

net_player::net_player()
{
}

net_player::~net_player()
{
	logout();
}

void net_player::sync_all_resources()
{
	g_server->send_packet_broadcast_ex(ID_RESOURCE_SYNC_ALL, sys_address, false);
}

void net_player::sync_all_resources_status()
{
	if (auto bs = g_server->create_all_resources_sync_status_packet())
		g_server->send_packet_broadcast(bs, sys_address, false);
}

void net_player::add_file_to_check(gns::resource::info& info)
{
	std::string rsrc_name = *info.rsrc,
				filename = *info.name;

	if (auto it = ft.rsrc_files_to_check.find(rsrc_name); it != ft.rsrc_files_to_check.end())
		it->second.insert({ filename, std::move(info) });
	else ft.rsrc_files_to_check.insert({ rsrc_name, { { filename, std::move(info) } } });
}

void net_player::compare_and_send_files()
{
	std::vector<file_info> files_to_send;
	std::vector<std::string> files_ready;

	g_resource->for_each_resource([&](resource* rsrc)
	{
		const auto& rsrc_name = rsrc->get_folder();

		if (auto rsrc_it = ft.rsrc_files_to_check.find(rsrc_name); rsrc_it != ft.rsrc_files_to_check.end())
		{
			const auto& rsrc_files = rsrc_it->second;

			rsrc->for_each_client_file([&](const file_info& info)
			{
				if (auto it = rsrc_files.find(info.name); it != rsrc_files.end())
				{
					auto& cmp_info = it->second;

					if (cmp_info.hash == info.hash)
						files_ready.push_back(rsrc->get_filename(info.name));
					else files_to_send.emplace_back(rsrc_name, rsrc->get_filename(info.name), info.hash);
				}
				else files_to_send.emplace_back(rsrc_name, rsrc->get_filename(info.name), info.hash);

				return false;
			});
		}
		else
		{
			rsrc->for_each_client_file([&](const file_info& info)
			{
				files_to_send.emplace_back(rsrc_name, rsrc->get_filename(info.name), info.hash);
				return false;
			});
		}
	});

	ft.rsrc_files_to_check.clear();

	if (!files_ready.empty())
	{
		auto bs = g_server->create_packet(ID_FILE_LIST);

		bs->Write(files_to_send.empty());
		bs->Write(int(files_ready.size()));

		for (const auto& filename : files_ready)
			bs->Write(gns_string<char, GNS_GLOBALS::MAX_RESOURCE_NAME_LENGTH>(filename.c_str()));

		g_server->send_packet_broadcast(bs, sys_address, false);
	}

	if (!files_to_send.empty())
	{
		auto ft = g_server->get_ft();

		ft->add_batch(sys_address, files_to_send);
	}
}

void net_player::send_notification(const std::string& text, uint32_t color)
{
	gns::net_player::notification out_info { text.c_str(), color };

	g_server->send_packet_broadcast_ex(ID_NET_PLAYER_NOTIFICATION, sys_address, false, out_info);
}

bool net_player::login(const std::string& user, const std::string& pass, bool& invalid_pass)
{
	if (!g_server->verify_user(user, pass, flags, invalid_pass))
		return false;

	g_server->set_user_logged_in(user_db_name = user);

	return true;
}

bool net_player::logout()
{
	const bool was_logged_in = is_logged_in();

	flags = NET_PLAYER_FLAG_NONE;

	if (was_logged_in)
		g_server->set_user_logged_out(user_db_name);

	return was_logged_in;
}

int net_player::get_ping() const
{
	return g_server->get_system_ping(sys_address);
}