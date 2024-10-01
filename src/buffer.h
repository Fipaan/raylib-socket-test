#include <stddef.h>
#include <stdbool.h>
extern int asprintf(char **restrict ptr, const char *restrict fmt, ...);
#define  INFO(format, ...) printf(" [INFO] " format "\n", ##__VA_ARGS__)
#define  WARN(format, ...) printf(" [WARN] " format "\n", ##__VA_ARGS__)
#define ERROR(format, ...) printf("[ERROR] " format "\n", ##__VA_ARGS__)
#define DEBUG(format, ...) printf("[DEBUG] " format "\n", ##__VA_ARGS__)

typedef struct {
	char* data;
	size_t length;
} Buffer;

typedef struct {
	Buffer* words;
	size_t amount;
} Words;

#ifdef WORDS_IMPLEMENTATION
#include <string.h>
#include <stdio.h>
#include <assert.h>
char* next_word(char* stream, const char* end, bool* is_multi_word, size_t* psize) {
	// If not correct stream -> NULL
	if(stream == NULL || end == NULL || stream >= end || is_multi_word == NULL) return NULL;
	const char prev_char = *is_multi_word ? '\'' : ' ';
	// '(w)ord1   word2' -> 'word1( )  word2'
	while(stream != end && *stream != prev_char) {
		stream++;
	}
	// If previous word - multi-word -> "'word1(')  word2" -> "'word1'( )  word2"
	stream += *is_multi_word;
	// 'word1( )  word2' -> 'word1   (w)ord2'
	while(stream != end && *stream == ' ') {
		stream++;
	}
	// 'wordlast   ' -> NULL
	if(stream >= end) return NULL;
	size_t tmp_size = 0;
	// "word1   (')word2'" -> "word1   '(w)ord2'"
	*is_multi_word = *stream == '\'';
	stream += *is_multi_word;
	const char checking_char = *is_multi_word ? '\'' : ' ';
	// 'word1   (w)ord2 ...' -> 'word1   word2( )...' (getting word size)
	while(stream + tmp_size != end && stream[tmp_size] != checking_char) {
			tmp_size++;
	}
	// Handle this situation: "word1   'word2"
	if(*is_multi_word && stream + tmp_size == end) {
		WARN("Trailing single-quote");
	}
	if(psize != NULL) *psize = tmp_size;
	return stream;
}

char* start_word(char* start, const char* end, bool* is_multi_word, size_t* psize) {
	if(start[0] == ' ') {
		return next_word(start, end, is_multi_word, psize);
	}
	const bool is_first_multi_word = start[0] == '\'';
	const char prev_char = is_first_multi_word ? '\'' : ' ';
	start += is_first_multi_word;
	if(psize != NULL) {
		size_t size = 0;
		while(start + size < end && start[size] != prev_char) {
			size++;
		}
		*psize = size;
	}
	return start;
}

void* malloccpy_(void* src, size_t size) {
	void* dest = malloc(size);
	memcpy(dest, src, size);
	return dest;
}

void* malloccpy3(void* src, size_t size, size_t delta_alloc) {
	void* dest = malloc(size + delta_alloc);
	memcpy(dest, src, size);
	return dest;
}

Words parse_string(char* str) {
	Words w = {0};
	// Nothing to parse
	if(str == NULL) return w;
	const size_t len = strlen(str);
	// Empty string
	if(len == 0) return w;
	const char* end = str + len;
	bool is_spacing = false;
	char* cursor = start_word(str, end, &is_spacing, NULL);
	// Count words
	while(cursor != NULL) {
		w.amount++;
		if(strchr(cursor, is_spacing ? '\'' : ' ') == NULL) {
			break;
		}
		cursor = next_word(cursor, end, &is_spacing, NULL);
	}
	// If no words. For example: '     '
	if(w.amount == 0) return w;
	w.words = calloc(sizeof(Buffer), w.amount);
	is_spacing = false;
	size_t curr_size;
	size_t index = 0;
	// Add words to structure
	for(cursor = start_word(str, end, &is_spacing, &curr_size); 
		cursor != NULL;
		cursor = next_word(cursor, end, &is_spacing, &curr_size)) 
	{	
		assert(index < w.amount);
		w.words[index].length = curr_size;
		w.words[index].data = malloccpy3(cursor, curr_size, 1);
		w.words[index].data[curr_size] = '\0';
		index++;
	}
	return w;
}

typedef struct {
	Buffer name, msg;
} Message;

void move_from_words(Buffer* dest_buf, Words w, size_t index) {
	dest_buf->data = realloc(dest_buf->data, w.words[index].length + 1);
	dest_buf->length = w.words[index].length;
	memcpy(dest_buf->data, w.words[index].data, dest_buf->length + 1);
}

void dump_words(Words w) {
	puts("{");
	for(size_t i = 0; i < w.amount; ++i) {
		printf("\t%zu: (%zu) \"%s\", \n", i, w.words[i].length, w.words[i].data);
	}
	puts("}");
}

#define CLIENT_NAME_SIZE 10
#define MESSAGE_LEN_SIZE 50

bool parse_message(Message* rmsg, Words w) {
	if(w.amount < 2) {
		WARN("Too few words! (%zu)", w.amount);
		return false;
	}
	if(w.amount > 2) {
		WARN("Too many words! (%zu)", w.amount);
		dump_words(w);
		return false;
	}
	if(w.words[0].length > CLIENT_NAME_SIZE) {
		WARN("Too long name! (%zu)", w.words[0].length);
		return false;
	}
	if(w.words[1].length > MESSAGE_LEN_SIZE) {
		WARN("Too long msg! (%zu)", w.words[1].length);
		return false;
	};
	move_from_words(&rmsg->name, w, 0);
	move_from_words(&rmsg->msg, w, 1);
	return true;
}

char* sended_message(Message message) {
	char* res;
	asprintf(&res, "%s '%s'", message.name.data, message.msg.data);
	return res;
}
#endif /* WORDS_IMPLEMENTATION */
