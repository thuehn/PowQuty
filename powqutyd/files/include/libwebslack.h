/*
 * libwebslack is library to use Slacks incoming webhooks in a C programm
 *
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
#ifndef _LIBWEBSLACK_H_
#define _LIBWEBSLACK_H_

#define MAX_HOOK_LENGTH 512
#define MAX_CHANNEL_LENGTH 22
#define MAX_USER_LENGTH 21
#define MAX_TEXT_LENGTH 1024
#define MAX_EMOJI_LENGTH 512

struct team_info {
	char webhook_url[MAX_HOOK_LENGTH + 1];
	char channel[MAX_CHANNEL_LENGTH + 1];
	char username[MAX_USER_LENGTH + 1];
	char text[MAX_TEXT_LENGTH + 1];
	char emoji[MAX_EMOJI_LENGTH + 1];
};

/*
 * set webhook to use
 * @param team_info: team struct
 * @param webhook_url: webhook url of your team to use
 * return 0 on success, else 1
 */
int set_webhook_url(struct team_info *team_info, const char *webhook_url);

/*
 * set channel to post to
 * channel names may only be 22 letters in length
 * @param team_info: team struct
 * @param channel: channel to post to
 * return 0 on success, else 1
 */
int set_channel(struct team_info *team_info, const char *channel);

/*
 * set the user name for a team struct
 * The username may only contain lowercase letters, numbers, hyphens,
 * periods and underscores
 * @param team_info: team struct
 * @param username: username to set
 * return 0 on success, else 1
 */
int set_username(struct team_info *team_info, const char *username);

/*
 * set message to send
 * @param team_info: team to send to
 * @param text: send this text
 * return 0 on success, else 1
 */
int set_message(struct team_info *team_info, const char *text);

/*
 * Set emoji icon
 * @param team_info: team struct
 * @param emoji: emoji name
 * return 0 on success, else 1
 */
int set_emoji(struct team_info *team_info, const char *emoji);

/*
 * send message to team
 * @param team_info: team info struct
 * @param return: 0 on success, else 1
 */
int send_message(struct team_info *team_info);

#endif /*_LIBWEBSLACK_H_ */

