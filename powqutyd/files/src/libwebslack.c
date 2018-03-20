/*
 * libwebslack is a library to send messages, using slacks incoming webhooks
 * Copyright (C) 2017  Stefan Venz
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <curl/curl.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libwebslack.h"

#define TERM_CHAR 1
#define JSON_SYMBOLS 32
#define PAYLOAD_SYMBOLS 8
#define CHANNEL 7
#define USERNAME 8
#define TEXT 4
#define ICON_EMOJI 10

static int check_parameters(struct team_info *ti) {

	if ((ti->channel == NULL) || (strlen(ti->channel) == 0)) {
		printf("[ERROR] channel not set\n");
		return EXIT_FAILURE;
	}

	if ((ti->username == NULL) || (strlen(ti->username) == 0)) {
		printf("[ERROR] username not set\n");
		return EXIT_FAILURE;
	}

	if ((ti->text == NULL) ||(strlen(ti->text) == 0)) {
		printf("[ERROR] text not set\n");
		return EXIT_FAILURE;
	}

	if ((ti->emoji == NULL) || (strlen(ti->emoji) == 0)) {
		snprintf(ti->emoji, (strlen(":ghost:") + 1), ":ghost:");
	}

	if ((ti->webhook_url == NULL) || (strlen(ti->webhook_url) == 0)) {
		printf("[ERROR] url not set\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int send_message(struct team_info *ti) {
	CURL *curl;
	CURLcode res = -1;
	int ret;

	if (check_parameters(ti)) {
		printf("cant send message\n");
		return EXIT_FAILURE;
	}

	uint8_t channel_len = sizeof(char) * CHANNEL + strlen(ti->channel);
	uint8_t user_len = sizeof(char) * USERNAME + strlen(ti->username);
	uint16_t text_len = sizeof(char) * TEXT + strlen(ti->text);
	uint8_t emoji_len = sizeof(char) * ICON_EMOJI + strlen(ti->emoji);
	uint32_t final_len = channel_len + user_len + text_len + emoji_len +
		sizeof(char) * (JSON_SYMBOLS + PAYLOAD_SYMBOLS + TERM_CHAR);
	char *str = malloc(final_len);

	ret = snprintf(str, final_len, "payload={\"channel\": \"%s\", \"username\":"
		 " \"%s\", \"text\": \"%s\", \"icon_emoji\": \"%s\"}\"",
		 ti->channel,ti->username, ti->text, ti->emoji);
	if ((ret < 0) || ((uint32_t)ret > final_len)) {
		printf("\nsomething went wrong creating the payload\n");
#if DEBUG
		printf("ret: %d final_len: %d\n, str: %s\n strlen: %d\n",
			ret, final_len, str, strlen(str));
#endif
	}

	curl_global_init(CURL_GLOBAL_ALL);
	
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, ti->webhook_url);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	printf("\n");
	curl_global_cleanup();
	free(str);
	str = NULL;
	if (res != CURLE_OK) {
		printf("curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int set_webhook_url(struct team_info *ti, const char *webhook_url) {
	int ret = 0;

	if (webhook_url == NULL) {
		printf("URL has to be set\n");
		return EXIT_FAILURE;
	}

	if (strlen(webhook_url) > MAX_HOOK_LENGTH) {
		printf("webhooks may only be %d characters, but %s has %d\n",
		       (int)MAX_HOOK_LENGTH, webhook_url,
		       (int)strlen(webhook_url));
		return EXIT_FAILURE;
	}

	ret = snprintf(ti->webhook_url, MAX_HOOK_LENGTH + 1, "%s", webhook_url);
	if ((ret >= MAX_HOOK_LENGTH + 1) || (ret < 0)) {
		printf("something went wrong setting the webhook URL\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int set_channel(struct team_info *ti, const char *channel) {
	int ret = 0;

	if (channel == NULL) {
		printf("channel needs to be set\n");
		return EXIT_FAILURE;
	}

	if (strlen(channel) > MAX_CHANNEL_LENGTH) {
		printf("channels may only have %d characters but %s has %d\n",
		       (int)MAX_CHANNEL_LENGTH, channel, (int)strlen(channel));
		return EXIT_FAILURE;
	}

	ret = snprintf(ti->channel,MAX_CHANNEL_LENGTH + 1, "%s", channel);
	if ((ret >= MAX_CHANNEL_LENGTH + 1) || (ret < 0)) {
		printf("something went wrong setting the username\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int set_username(struct team_info *ti, const char *username) {
	int ret = 0;

	if (username == NULL) {
		printf("username has to be set\n");
		return 1;
	}

	if (strlen(username) >  MAX_USER_LENGTH) {
		printf("usernames may only be %d characters, but %s has %d\n",
		       (int)MAX_USER_LENGTH, username,
		       (int)strlen(username));
		return 1;
	}

	ret = snprintf(ti->username, MAX_USER_LENGTH + 1, "%s", username);
	if ((ret >= (MAX_USER_LENGTH + 1)) || (ret < 0)) {
		printf("something went wrong setting the username\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int set_message(struct team_info *ti, const char *text) {
	int ret = 0;

	if (text == NULL) {
		printf("text must be set\n");
		return EXIT_FAILURE;
	}

	if (strlen(text) > MAX_TEXT_LENGTH) {
		printf("text may only be %d characters, but %s has %d\n",
			(int)MAX_TEXT_LENGTH, text, (int)strlen(text));
		return EXIT_FAILURE;
	}

	ret = snprintf(ti->text, MAX_TEXT_LENGTH + 1, "%s", text);
	if ((ret >= (MAX_TEXT_LENGTH + 1)) || (ret < 0)) {
		printf("something went wrong setting the text\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int set_emoji(struct team_info *ti, const char *emoji) {
	int ret = 0;

	if (emoji == NULL) {
		printf("emoji has to be set\n");
		return EXIT_FAILURE;
	}

	if (strlen(emoji) > MAX_EMOJI_LENGTH) {
		printf("emojis may only have %d characters, but %s has %d\n",
			(int)MAX_EMOJI_LENGTH, emoji, (int)strlen(emoji));
		return EXIT_FAILURE;
	}

	ret = snprintf(ti->emoji, MAX_EMOJI_LENGTH + 1, "%s", emoji);
	if ((ret >= (MAX_EMOJI_LENGTH + 1)) || (ret < 0)) {
		printf("something went wrong setting the emoji\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

