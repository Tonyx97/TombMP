	#pragma once

class ms;

class client_base
{
protected:

	std::future<void> future;

	tr::net::socket_info si;

	std::string ip;

	ms* sv = nullptr;

	int version = -1;

	bool server_type = false;

private:


public:

	bool is_server() const { return server_type; }

	virtual bool is_waiting_exit() = 0;
	virtual bool compare_ip(const std::string& other) = 0;
};

class client_sv : public client_base
{
private:

public:

	client_sv(ms* sv, SOCKET s, const std::string& ip);
	~client_sv();
	
	static constexpr int32_t VER_MAJOR = 1,
							 VER_MINOR = 0;

	void dispatcher();

	bool is_waiting_exit() override;
	bool compare_ip(const std::string& other) override;

	static constexpr uint32_t VERSION()		{ return (VER_MAJOR << 8) | VER_MINOR; }
	static constexpr int MAX_TIMEOUT_MS()	{ return 0; }
};

class server_sv : public client_base
{
private:

	std::mutex mtx;

	std::string public_ip,
				name,
				gamemode,
				map,
				players_list;

	int players_count = 0,
		max_players = 0;

	bool sharing_enabled = false;

public:

	server_sv(ms* sv, SOCKET s, const std::string& ip);
	~server_sv();

	static constexpr int32_t VER_MAJOR = 1,
							 VER_MINOR = 0;

	void dispatcher();
	void lock();
	void unlock();

	std::string get_all_info();

	bool is_waiting_exit() override;
	bool compare_ip(const std::string& other) override;

	SOCKET get_socket() const								{ return si.s; }

	static constexpr uint32_t VERSION()						{ return (VER_MAJOR << 8) | VER_MINOR; }
	static constexpr int MAX_TIMEOUT_MS()					{ return 60000; }
};