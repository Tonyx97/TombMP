#pragma once

class debugger
{
private:

	std::vector<std::string> messages;
	
	float curr = 0.f;

	int scroll_dir = 0,
		base = 0;

	bool enabled = false,
		 scrolling = false;

public:

	static constexpr float SIZE_X = 700.f;
	static constexpr float SIZE_Y = 200.f;

	void update();
	void scroll_up();
	void scroll_down();
	void clear()								{ messages.clear(); base = 0; }
	void add_client_msg(const std::string& v)	{ messages.push_back(std::to_string(msgs_count()) + ". ##FF0000FF" + v); }
	void add_server_msg(const std::string& v)	{ messages.push_back(std::to_string(msgs_count()) + ". ##FFA530FF" + v); }
	void toggle()								{ enabled = !enabled; }
	void enable()								{ enabled = true; }
	void disable()								{ enabled = false; }

	bool is_enabled() const						{ return enabled; }

	int msgs_count() const						{ return static_cast<int>(messages.size()); }

	static void error_callback(class script* s, const std::string& err);
};

inline std::unique_ptr<debugger> g_debugger;