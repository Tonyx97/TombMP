import utils;
import prof;

#include <shared/defs.h>
#include <shared/net/net.h>
#include <shared/scripting/resource.h>
#include <shared/scripting/resource_system.h>

#include <argument_parser/argument_parser.h>
#include <ui/ui.h>

#include "client.h"
#include "cmd.h"

#include "game/player.h"

#include "net_handlers/hl_net_defs.h"

#include "game/handlers/hl_defs.h"

#include <mp/game/level.h>

client::client()
{
	peer = SLNet::RakPeerInterface::GetInstance();

	net_stats.resize(1024);
}

client::~client()
{
	if (!peer)
		return;

	peer->Shutdown(100);

	SLNet::RakPeerInterface::DestroyInstance(peer);
}

bool client::init()
{
	if (!peer)
		return false;

	if ((nickname = g_arg_parser->get_arg("nickname")).length() <= GNS_GLOBALS::MIN_NAME_LENGTH)
		return false;

	SLNet::SocketDescriptor socket_desc(0, 0);
	
	socket_desc.socketFamily = AF_INET;

	if (peer->Startup(1, &socket_desc, 1) != SLNet::RAKNET_STARTED)
		return false;

	peer->SetOccasionalPing(true);
	peer->SetSplitMessageProgressInterval(100);

	return true;
}

bool client::connect()
{
	used_ip = g_arg_parser->get_arg("ip");
	used_pass = g_arg_parser->get_arg("pass");

	if (used_ip.length() < 8)
		return false;

	prof::print(RED, "used_ip '{}'", used_ip.c_str());
	prof::print(RED, "used_pass '{}'", used_pass.c_str());

	if (!used_pass.compare("NO_PASS"))
		return (peer->Connect(used_ip.c_str(), tr::net::GAME_SERVER_PORT_INT, nullptr, 0) == SLNet::CONNECTION_ATTEMPT_STARTED);
	else return (peer->Connect(used_ip.c_str(), tr::net::GAME_SERVER_PORT_INT, used_pass.c_str(), used_pass.length()) == SLNet::CONNECTION_ATTEMPT_STARTED);
}

bool client::dispatch_packets(int sleep_time)
{
	if (net_stat_level != -1 && sys_address != SLNet::UNASSIGNED_SYSTEM_ADDRESS)
		if (SLNet::RakNetStatistics statistics; peer->GetStatistics(peer->GetSystemAddressFromIndex(0), &statistics))
			SLNet::StatisticsToString(&statistics, net_stats.data(), net_stat_level);
	
	for (auto p = peer->Receive(); p; peer->DeallocatePacket(p), p = peer->Receive())
	{
		auto is_packet_in_range = [&](auto pid, auto min, auto max)
		{
			return (pid > min && pid < max);
		};

		SLNet::BitStream bs_in(p->data, p->length, false);

		switch (const auto packet_id = get_packet_id(p, &bs_in))
		{
		case ID_DISCONNECTION_NOTIFICATION:
			connected = bs_ready = false;
			break;
		case ID_CONNECTION_BANNED:
			last_err = CONN_ERR_BANNED;
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			last_err = CONN_ERR_ATTEMPT_FAILED;
			break;
		case ID_INVALID_PASSWORD:
			last_err = CONN_ERR_INVALID_PASSWORD;
			break;
		case ID_CONNECTION_LOST:
			last_err = CONN_ERR_CONN_LOST;
			connected = bs_ready = false;
			prof::critical_error("Connection lost");
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			sv_sys_address = p->systemAddress;

			const auto curr_guid = peer->GetMyGUID();

			id = curr_guid.g;
			sys_address = peer->GetSystemAddressFromGuid(curr_guid);
			
			connected = bs_ready = true;

			prof::print(GREEN, "Connected to {} ({:#x})", sv_sys_address.ToString(true, ':'), peer->GetMyGUID().g);

			break;
		}
		default:
		{
			if (packet_id == ID_DISCONNECTION_NOTIFICATION || packet_id == ID_CONNECTION_LOST ||
				is_packet_in_range(packet_id, ID_NET_PLAYER_PACKET_BEGIN, ID_NET_PLAYER_PACKET_END))
			{
				net_player_handlers::handle_packet(packet_id);
			}
			else if (is_packet_in_range(packet_id, ID_FILE_BEGIN, ID_FILE_END))
				file_handlers::handle_packet(packet_id);
			else if (is_packet_in_range(packet_id, ID_SERVER_PACKET_BEGIN, ID_SERVER_PACKET_END))
				server_handlers::handle_packet(packet_id);
			else if (is_packet_in_range(packet_id, ID_RESOURCE_PACKET_BEGIN, ID_RESOURCE_PACKET_END))
				resource_handlers::handle_packet(packet_id);
			else if (is_packet_in_range(packet_id, ID_LEVEL_PACKET_BEGIN, ID_LEVEL_PACKET_END))
				level_handlers::handle_packet(packet_id);
			else if (ready)
			{
				if (is_packet_in_range(packet_id, ID_PLAYER_PACKET_BEGIN, ID_PLAYER_PACKET_END))
					player_handlers::handle_packet(packet_id);
				else if (is_packet_in_range(packet_id, ID_PROJECTILE_PACKET_BEGIN, ID_PROJECTILE_PACKET_END))
					projectile_handlers::handle_packet(packet_id);
				else if (is_packet_in_range(packet_id, ID_FX_PACKET_BEGIN, ID_FX_PACKET_END))
					fx_handlers::handle_packet(packet_id);
				else if (is_packet_in_range(packet_id, ID_AUDIO_PACKET_BEGIN, ID_AUDIO_PACKET_END))
					audio_handlers::handle_packet(packet_id);
				else if (is_packet_in_range(packet_id, ID_ITEM_PACKET_BEGIN, ID_ITEM_PACKET_END))
					entity_handlers::handle_packet(packet_id);
				else if (is_packet_in_range(packet_id, ID_SYNC_BEGIN, ID_SYNC_END))
					sync_handlers::handle_packet(packet_id);
			}
		}
		}

		reset_bs();
	}

	if (sleep_time != -1)
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));

	return connected;
}

void client::join(bool spawn_ready)
{
	gns::net_player::join info;

	info.name = nickname.c_str();
	info.ready = spawn_ready;

	if (send_packet(ID_NET_PLAYER_JOIN, info))
		set_ready_status(spawn_ready);
}

void client::add_resource_script(const std::string& rsrc_name, const std::string& script)
{
	resources_to_load[rsrc_name].push_back(script);
}

void client::sync_resource_all()
{
	auto bs = create_packet(ID_RESOURCE_SYNC_ALL);

	std::vector<gns::resource::info> file_list;

	g_resource->get_list_of_files([&](const std::string& rsrc, const std::string& filename, FILE_HASH hash)
	{
		prof::print(RED, "sending '{}' '{}' {:#x}", rsrc.c_str(), filename.c_str(), hash);

		file_list.emplace_back(rsrc.c_str(), filename.c_str(), hash);
	});

	bs->Write(int(file_list.size()));

	for (auto& v : file_list)
		bs->Write(std::move(v));

	send_packet(bs);
}

void client::sync_resource_all_status()
{
	send_packet(ID_RESOURCE_SYNC_ALL_STATUS);
}

void client::sync_with_players()
{
	send_packet(ID_PLAYER_SYNC_PLAYERS);
}

void client::sync_level_entities(const std::vector<std::pair<int, int16_t>>& level_entities)
{
	auto bs = create_packet(ID_SYNC_LEVEL_ENTITIES);

	const auto& spawns = g_level->get_player_spawns();

	bs->Write(int(spawns.size()));

	for (const auto& spawn_loc : spawns)
		bs->Write(spawn_loc);

	bs->Write(NUMBER_OBJECTS);

	for (int i = 0; i < NUMBER_OBJECTS; ++i)
		bs->Write(bool(objects[i].loaded));

	bs->Write(int(level_entities.size()));

	for (const auto& [subtype, index] : level_entities)
		bs->Write(gns::sync::level_entity_initial_basic_sync { .subtype = subtype, .id = index });

	send_packet(bs);
}

void client::sync_spawned_entities()
{
	send_packet(ID_SYNC_SPAWNED_ENTITIES);
}

bool client::add_file_data(const std::string& filename, std::vector<char>& data, int total_size, FILE_HASH hash)
{
	auto it = ft.received_files_data.find(filename);
	auto data_size = int(data.size());

	if (it != ft.received_files_data.end())
	{
		auto& file_info = it->second;

		file_info.data.insert(file_info.data.end(), data.begin(), data.end());
		file_info.bytes_rcved += data_size;

		return file_info.bytes_rcved >= total_size;
	}
	else
	{
		ft.received_files_data.insert({ filename, { .data = std::move(data), .hash = hash, .size = total_size, .bytes_rcved = data_size } });
		return data_size >= total_size;
	}
}

bool client::save_file(const std::string& filename)
{
	if (auto it = ft.received_files_data.find(filename); it != ft.received_files_data.end())
	{
		const auto& info = it->second;

		auto paths = utils::string::split(filename, "\\");

		const bool path_ok = paths.size() > 1;

		if (path_ok)
		{
			const auto& rsrc_name = paths[0];

			std::string acc_path = resource_system::RESOURCES_PATH;

			for (const auto& folder : std::span(paths).subspan(0, paths.size() - 1))
			{
				acc_path += folder + '\\';

				if (!std::filesystem::is_directory(acc_path))
					std::filesystem::create_directory(acc_path);
			}

			auto fullpath = resource_system::RESOURCES_PATH + filename;

			if (auto file = std::ofstream(fullpath, std::ios::binary))
			{
				auto& data = info.data;

				file.write(data.data(), data.size());
			}

			utils::file::set_file_modified_time(fullpath, info.hash);

			if (std::string dummy_rsrc, subpath; utils::string::split_left(filename, dummy_rsrc, subpath, '\\'))
				save_resource_file(rsrc_name, subpath);

			prof::print(CYAN, "'{}' saved", filename.c_str());
		}

		ft.received_files_data.erase(it);

		return path_ok;
	}

	return false;
}

void client::save_resource_file(const std::string& rsrc, const std::string& file)
{
	ft.rsrcs_file_list[rsrc].push_back(file);
}

bool client::set_name(const std::string& val)
{
	if (val.size() <= GNS_GLOBALS::MIN_NAME_LENGTH || val.size() >= GNS_GLOBALS::MAX_NAME_LENGTH - 1)
		return false;

	gns::net_player::name_bc info {};

	info.old_name = nickname.c_str();
	info.new_name = val.c_str();

	nickname = val;

	send_packet(ID_NET_PLAYER_NAME, info);

	return true;
}

const std::vector<std::string>& client::get_rsrc_file_list(const std::string& rsrc)
{
	return ft.rsrcs_file_list[rsrc];
}

int client::get_ping()
{
	return peer->GetLastPing(sv_sys_address);
}

void client::on_resource_action(resource* rsrc, resource_action action, script_action_result res, bool done)
{
}