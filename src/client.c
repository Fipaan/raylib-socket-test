#include "client_socket.h"
#include <raylib.h>
#include <raymath.h>

#define HISTORY_CAPACITY 10
#define SPACING 5
#define OFFSET (SPACING + 2)

typedef struct {
	char msgs[HISTORY_CAPACITY][BUFFER_SIZE + 3];
	size_t count;
} Messages;

typedef struct {
	char id[ID_SIZE + 1];
	char username[USERNAME_SIZE + 1];
} Binding;

typedef struct {
	Binding* bindings;
	size_t count;
} Bindings;

typedef struct {
	bool sending, post, input;
} Sending_State;

typedef struct {
	int screen_width, screen_height;
	Vector2 screen_pos;
	Color back_color;
	Messages history_messages;
	Sending_State user_input, nickname;
	char *display_text, *username_msg;
	size_t username_shifting;
	Bindings b;
} Env;

Env env = {
	.screen_width = 1920 / 2,
	.screen_height = 1080 / 2,
};

const char* get_binding(const char* id) {
	for(size_t i = 0; i < env.b.count; ++i) {
		if(memcmp(env.b.bindings[i].id, id, ID_SIZE + 1) == 0) {
			return env.b.bindings[i].username;
		}
	}
	return NULL;
}

void new_binding(const char* id, const char* username) {
	env.b.count++;
	env.b.bindings = realloc(env.b.bindings, env.b.count * sizeof(*env.b.bindings));
	Binding* b = &env.b.bindings[env.b.count - 1];
	memcpy(b->id, id, ID_SIZE + 1);
	memcpy(b->username, username, strlen(username) + 1);
}

void add_new_message(bool is_reader) {
	char* msg_with_sender;
	int ignore;
	if(is_reader) {
		const char* usr = get_binding(tmp_msg.id);
		ignore = asprintf(&msg_with_sender, "[%s] %s", usr ? usr : tmp_msg.id, tmp_msg.msg);
	} else {
		ignore = asprintf(&msg_with_sender, ">>>>> %s", tmp_msg.msg);
	}
	(void) ignore;
	if(env.history_messages.count < HISTORY_CAPACITY) {
		memcpy(env.history_messages.msgs[env.history_messages.count++], msg_with_sender, strlen(msg_with_sender) + 1);
	} else {
		for(size_t i = 1; i < HISTORY_CAPACITY; ++i) {
			memcpy(env.history_messages.msgs[i - 1], env.history_messages.msgs[i], BUFFER_SIZE + 3);
		}
		memcpy(env.history_messages.msgs[HISTORY_CAPACITY - 1], msg_with_sender, strlen(msg_with_sender) + 1);
	}
}

bool valid_message() {
	return memcmp(tmp_msg.id, me.id, ID_SIZE) != 0;
}

int cursor = 0;
int username_cursor = 0;

bool char_in_str(const char c, const char* str) {
	for(size_t i = 0; i < strlen(str); ++i) {
		if(c == str[i]) return true;
	}
	return false;
}

void handle_username_input() {
	int key = GetKeyPressed();
	int ch = GetCharPressed();
	while (key || ch) {
		if (key == KEY_ENTER && username_cursor > 0) {
			env.username_msg[username_cursor] = '\0';
			env.nickname.sending = true;
			memcpy(me.username, env.username_msg + env.username_shifting, USERNAME_SIZE);
		}
		else if(key == KEY_BACKSPACE && username_cursor > 0) {
			env.username_msg[username_cursor--] = '\0';
			env.username_msg[username_cursor] = '_';
		} else if ((size_t) username_cursor < env.username_shifting + USERNAME_SIZE && char_in_str(ch, valid_chars_id)) {
			env.username_msg[username_cursor++] = ch;
			env.username_msg[username_cursor] = '_';
		}
		key = GetKeyPressed();
		ch = GetCharPressed();
	}
}

void handle_input() {
	int key = GetKeyPressed();
	int ch = GetCharPressed();
	while (key || ch) {
		if (key == KEY_ENTER && cursor > 0) {
			env.display_text[cursor] = '\0';
			env.user_input.sending = true;
		}
		else if(key == KEY_BACKSPACE && cursor > 0) {
			env.display_text[cursor--] = '\0';
			env.display_text[cursor] = '_';
		} else if (ch >= 32 && ch < 127 && cursor < MSG_SIZE) {
			env.display_text[cursor++] = ch;
			env.display_text[cursor] = '_';
		}
		key = GetKeyPressed();
		ch = GetCharPressed();
	}
}

const unsigned long wait_time = 20ul; // In ms
unsigned long awaiting_time = 0ul;
clock_t init_time;

unsigned long modify_init_time() {
	unsigned long new_time = init_time;
	do {
		new_time = clock();
	} while((new_time - init_time) % wait_time != 0);
	return new_time;
}

int main(int argc, char *argv[]) {	
	//GO_REBUILD_URSELF(argc, argv);	
	InitWindow(0, 0, "Ultimate socket test!");
	ToggleFullscreen();	env.screen_width = GetScreenWidth();
	env.screen_height = GetScreenHeight();
	ToggleFullscreen();
	SetWindowSize(env.screen_width / 2, env.screen_height);
	env.screen_pos.y = 0;
	if(argc == 1) {
		env.screen_pos.x = env.screen_width / 4.0f;
	} else {
		if(strlen(argv[1]) == 1) {
			if(*argv[1] == 'l') {
				env.screen_pos.x = 0.0f;
			} else if(*argv[1] == 'r') {
				env.screen_pos.x = env.screen_width / 2.0f;
			} else {
				ERROR("unknown flag, type 'l' 'r' or nothing");
				exit(1);
			}
		} else {
			ERROR("unknown flags, type 'l' 'r' or nothing");
			exit(1);
		}
	}
	SetWindowPosition(env.screen_pos.x, env.screen_pos.y);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	env.username_msg = calloc(1, MSG_SIZE);
	memcpy(env.username_msg, "Username...", strlen("Username..."));
	socket_init();
	init_time = modify_init_time();
	env.display_text = calloc(1, MSG_SIZE + 2);
	env.back_color = GetColor(0x181818FF);
	Vector2 pos = {
		.x = 50.0f,
		.y = 50.0f
	};
	Rectangle nickname_checkbox = {
		.x = pos.x,
		.y = pos.y + 25,
		.width = 300,
		.height = 24
	};
	
	Rectangle input_checkbox = {
		.x = nickname_checkbox.x - OFFSET,
		.y = nickname_checkbox.y + 50 - OFFSET,
		.width = nickname_checkbox.width + 2 * OFFSET,
		.height = nickname_checkbox.height + 2 * OFFSET
	};
	while(!WindowShouldClose()) {
		BeginDrawing();
		awaiting_time = (clock() - init_time) * 1.0f / CLOCKS_PER_SEC * 1000;
		if(awaiting_time > wait_time) {
			init_time = clock();
			socket_loop(env.user_input.sending, env.nickname.sending, env.display_text);
			if(env.user_input.sending) {
				env.user_input.sending = false;
				env.user_input.post = true;
			}
			if(env.nickname.sending) {
				env.nickname.sending = false;
				env.nickname.post = true;
			}
		}
		ClearBackground(env.back_color);
		DrawRectangleLinesEx(input_checkbox, 5.0f, GRAY);
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			if(env.user_input.input && !CheckCollisionPointRec(GetMousePosition(), input_checkbox)) {
				env.user_input.input = false;
			} else if(!env.user_input.sending && CheckCollisionPointRec(GetMousePosition(), input_checkbox)) {
				env.user_input.input = true;
				env.display_text[0] = '_';
			} else if(!env.nickname.post && !env.nickname.input && CheckCollisionPointRec(GetMousePosition(), nickname_checkbox)) {
				env.nickname.input = true;
				env.username_shifting = strlen("Username: ");
				username_cursor = env.username_shifting;
				env.username_msg[username_cursor] = '_';
				memcpy(env.username_msg, "Username: ", strlen("Username: ") + 1);
			}
		}
		
		if(env.nickname.input && !env.user_input.input) handle_username_input();
		if (env.user_input.input) handle_input();
		
		DrawText(env.username_msg, nickname_checkbox.x + OFFSET, nickname_checkbox.y + OFFSET, 24, RAYWHITE);
		DrawText(env.display_text, input_checkbox.x + OFFSET, input_checkbox.y + OFFSET, 24, RAYWHITE);
		if (reading_mode) {
			reading_mode = false;
			add_new_message(valid_message());
			if(env.user_input.post) {
				env.user_input.post = false;
				memset(env.display_text, '\0', BUFFER_SIZE);
				env.display_text[0] = '_';
				cursor = 0;
			}
		} else if (username_reading_mode) {
			username_reading_mode = false;
			new_binding(tmp_msg.id, tmp_msg.msg);
		}
		for(size_t i = 0; i < env.history_messages.count; ++i) {
			DrawText(env.history_messages.msgs[env.history_messages.count - 1 - i], 
					 input_checkbox.x + 5 + 2,
					 input_checkbox.y + (input_checkbox.height + 5) * (i + 1),
					 24, RAYWHITE);
		}
		EndDrawing();
	}
	socket_close();
	return 0;
}
