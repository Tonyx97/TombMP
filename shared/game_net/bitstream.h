#pragma once

#include <slikenet/statistics.h>
#include <slikenet/peerinterface.h>
#include <slikenet/BitStream.h>

#include "messages.h"

struct bit_streaming
{
protected:

	SLNet::RakPeerInterface* peer = nullptr;

	SLNet::Packet* current_packet = nullptr;

	SLNet::BitStream* current_bs = nullptr;

	std::atomic_bool bs_ready = false;

public:

	static constexpr size_t PACKET_INIT_SIZE()	{ return 3; }

	uint16_t get_packet_id(SLNet::Packet* p, SLNet::BitStream* bs_in)
	{
		if (!p)
			return UINT16_MAX;

		current_packet = p;
		current_bs = bs_in;

		bs_in->IgnoreBytes(sizeof(SLNet::MessageID));

		if (p->data[0] == ID_ENGINE_PACKET_TYPE)
		{
			uint16_t packet_id = 0;
			bs_in->Read(packet_id);
			return packet_id;
		}

		return (uint16_t)p->data[0];
	}

	void reset_bs()								{ current_bs = nullptr; }
	
	SLNet::Packet* get_current_packet()			{ return current_packet; }
	SLNet::BitStream* get_current_bs()			{ return current_bs; }
	SLNet::RakPeerInterface* get_peer()			{ return peer; }

	template <typename... A>
	bool send_packet_reliability(ePacketID msg_id, PacketReliability reliability, const A&... args)
	{
		if (!peer)
			return false;

		SLNet::BitStream bs {};

		if (msg_id > ID_ENGINE_PACKET_TYPE)
		{
			bs.Write<SLNet::MessageID>(ID_ENGINE_PACKET_TYPE);
			bs.Write<ePacketID>(msg_id);
		}
		else bs.Write<SLNet::MessageID>(msg_id);

		push_args_to_bs(&bs, std::forward<const A&>(args)...);

		return !!peer->Send(&bs, HIGH_PRIORITY, reliability, 0, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	template <typename... A>
	bool send_packet(ePacketID msg_id, const A&... args)
	{
		if (!peer)
			return false;

		SLNet::BitStream bs {};

		if (msg_id > ID_ENGINE_PACKET_TYPE)
		{
			bs.Write<SLNet::MessageID>(ID_ENGINE_PACKET_TYPE);
			bs.Write<ePacketID>(msg_id);
		}
		else bs.Write<SLNet::MessageID>(msg_id);

		push_args_to_bs(&bs, std::forward<const A&>(args)...);

		return !!peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	}

	template <typename... A>
	bool send_packet_broadcast_ex(ePacketID msg_id, const SLNet::SystemAddress& bc_client, bool broadcast, const A&... args)
	{
		if (!peer)
			return false;

		SLNet::BitStream bs;

		if (msg_id > ID_ENGINE_PACKET_TYPE)
		{
			bs.Write<SLNet::MessageID>(ID_ENGINE_PACKET_TYPE);
			bs.Write<ePacketID>(msg_id);
		}
		else bs.Write<SLNet::MessageID>(msg_id);

		push_args_to_bs(&bs, args...);

		return !!peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, bc_client, broadcast);
	}

	template <typename... A>
	bool send_packet_broadcast(ePacketID msg_id, const A&... args)
	{
		return send_packet_broadcast_ex(msg_id, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true, args...);
	}

	SLNet::BitStream* create_packet(ePacketID msg_id)
	{
		auto bs = new SLNet::BitStream();

		if (msg_id > ID_ENGINE_PACKET_TYPE)
		{
			bs->Write<SLNet::MessageID>(ID_ENGINE_PACKET_TYPE);
			bs->Write<ePacketID>(msg_id);
		}
		else bs->Write<SLNet::MessageID>(msg_id);

		return bs;
	}

	template <typename... A>
	bool send_packet_broadcast(SLNet::BitStream* bs, const SLNet::SystemAddress& bc_client = SLNet::UNASSIGNED_SYSTEM_ADDRESS, bool broadcast = true, const A&... args)
	{
		push_args_to_bs(bs, args...);

		const bool ok = peer->Send(bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, bc_client, broadcast);

		delete bs;

		return ok;
	}

	template <typename... A>
	bool send_packet(SLNet::BitStream* bs, const A&... args)
	{
		push_args_to_bs(bs, args...);

		const bool ok = peer->Send(bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);

		delete bs;

		return ok;
	}

	template <typename... A>
	bool send_packet_reliability(SLNet::BitStream* bs, PacketReliability reliability, const A&... args)
	{
		push_args_to_bs(bs, args...);

		const bool ok = peer->Send(bs, HIGH_PRIORITY, reliability, 0, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);

		delete bs;

		return ok;
	}

	void extract_args_from_bs() {}
	void extract_args_from_bs(SLNet::BitStream*) {}

	template <typename T, typename... A>
	void extract_args_from_bs(SLNet::BitStream* bs, T& a1, A&... args)
	{
		bs->ReadCompressed<T>(a1);
		extract_args_from_bs(bs, std::forward<A&>(args)...);
	}

	void push_args_to_bs()						{}
	void push_args_to_bs(SLNet::BitStream*)		{}

	template <typename T, typename... A>
	void push_args_to_bs(SLNet::BitStream* bs, const T& a1, A&&... args)
	{
		bs->WriteCompressed<T>(a1);
		push_args_to_bs(bs, std::forward<A>(args)...);
	}

	template <typename... A>
	void read_packet(SLNet::BitStream* bs, A&... args)
	{
		if (!peer)
			return;

		extract_args_from_bs(bs, std::forward<A&>(args)...);
	}

	template <typename... A>
	void read_packet_ex(A&... args)
	{
		if (!peer)
			return;

		extract_args_from_bs(current_bs, std::forward<A&>(args)...);
	}
};