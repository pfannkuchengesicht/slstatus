/* See LICENSE file for copyright and license details. */

/* alsa sound */
static char *soundcard = NULL;
static char *channel = NULL;

/* cpu temperature */
static char *tempfile = NULL;

/* wifi */
static char *wificard = NULL;

/* battery */
static char *batterynowfile = NULL;
static char *batteryfullfile = NULL;

/* time */
static const char timeformat[] = "%y-%m-%d %H:%M:%S";

/* statusbar */
#define FORMATSTRING "wifi %4s | bat %4s | cpu %4s %3s | ram %3s | vol %4s | %3s"
#define ARGUMENTS wifi_signal(), battery(), cpu_usage(), cpu_temperature(), ram_usage(), volume(), datetime()
