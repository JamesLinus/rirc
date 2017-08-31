#include "mode.h"

/* TODO:
 *  - channel mode (secret, private, etc) flags can be toggled,
 *  - p can never be set when s is set in private/secret case, if this message
 *    is received it is silently ignored (RFC2811 section 4.2.6)
 *
 *  - received MODE messages should check the target, either:
 *		- current user   (usermode)
 *		- a channel name (chanmode, possibly with additional params)
 *
 *	- safe channels ('!' prefix) (see RFC2811)
 *
 *
 */

static char mode_get_prefix(struct mode_config*, char, char);

void
mode_config_defaults(struct mode_config *m)
{
	/* Initialize a mode_config to the RFC2812, RFC2811 defaults
	 *
	 * Chanmodes (RFC2811, section 4)
	 *
	 *   O - give "channel creator" status;
	 *   o - give/take channel operator privilege;
	 *   v - give/take the voice privilege;
	 *
	 *   a - toggle the anonymous channel flag;
	 *   i - toggle the invite-only channel flag;
	 *   m - toggle the moderated channel;
	 *   n - toggle the no messages to channel from clients on the outside;
	 *   q - toggle the quiet channel flag;
	 *   p - toggle the private channel flag;
	 *   s - toggle the secret channel flag;
	 *   r - toggle the server reop channel flag;
	 *   t - toggle the topic settable by channel operator only flag;
	 *
	 *   k - set/remove the channel key (password);
	 *   l - set/remove the user limit to channel;
	 *
	 *   b - set/remove ban mask to keep users out;
	 *   e - set/remove an exception mask to override a ban mask;
	 *   I - set/remove an invitation mask to automatically override the
	 *       invite-only flag;
	 *
	 * Usermodes (RFC2118, section 3.1.5)
	 *
	 *   a - user is flagged as away;
	 *   i - marks a users as invisible;
	 *   w - user receives wallops;
	 *   r - restricted user connection;
	 *   o - operator flag;
	 *   O - local operator flag;
	 *   s - marks a user for receipt of server notices.
	 *
	 * Note: PREFIX and CHANMODES are ubiquitous additions to the IRC
	 *       protocol given by numeric 005 (RPL_ISUPPORT). As such,
	 *       they've been interpreted here in terms of A,B,C,D subcategories
	 *       for the sake of default settings. Numeric 319 (RPL_WHOISCHANNELS)
	 *       states chanmode user prefixes map o,v to @,+ respectively.
	 */

	*m = (struct mode_config)
	{
		.chanmodes = "OovaimnqpsrtklbeI",
		.usermodes = "aiwroOs",
		.CHANMODES = {
			.A = "beI",
			.B = "k",
			.C = "l",
			.D = "aimnqpsrtO"
		},
		.PREFIX = {
			.F = "ov",
			.T = "@+"
		}
	};
}

static char
mode_get_prefix(struct mode_config *m, char prefix, char mode)
{
	/* Return the most prescedent user prefix, given a current prefix
	 * and a new mode flag */

	int from = 0, to = 0;

	char *prefix_f = m->PREFIX.F,
	     *prefix_t = m->PREFIX.T;

	while (*prefix_f && *prefix_f++ != mode)
		from++;

	while (*prefix_t && *prefix_t++ != prefix)
		to++;

	return (from < to) ? m->PREFIX.T[from] : prefix;
}
