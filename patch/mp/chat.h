#pragma once

class chat
{
private:

	std::vector<std::wstring> cmd_history;

	std::vector<std::string> chat_list;

	std::wstring curr_msg;

	std::mutex chat_list_mtx;

	float text_size = 18.f,
		  curr = 0.f,
		  max_sx = 390.f;

	int scroll_dir = 0,
		cmd_history_index = 0;

	bool typing = false,
		 scrolling = false,
		 enabled = false;

public:

	chat();

	void key_input(uint32_t key, bool pressed);
	void add_char(wchar_t c);
	void remove_char();
	void remove_word();
	void paste_text();
	void begin_typing();
	void end_typing(bool send = true);
	void add_chat_msg(const std::wstring& msg);
	void add_chat_msg(const std::string& msg);
	void scroll_up();
	void scroll_down();

	void update();
	void enable()									{ enabled = true; }
	void disable()									{ enabled = false; }
	
	bool is_typing() const							{ return typing; }

	int msgs_count() const							{ return static_cast<int>(chat_list.size()); }

	size_t max_length() const;
};

inline std::unique_ptr<chat> g_chat;