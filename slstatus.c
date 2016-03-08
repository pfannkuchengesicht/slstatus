/* See LICENSE file for copyright and license details. */

/* global libraries */
#include <alsa/asoundlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

/* local libraries */
#include "config.h"

#include "config_reader.h"

/* functions */
void setstatus(char *str);
char *battery();
char *cpu_temperature();
char *cpu_usage();
char *datetime();
char *ram_usage();
char *smprintf(char *fmt, ...);
char *volume();
char *wifi_signal();

/* global variables */
static Display *dpy;

/* set statusbar (WM_NAME) */
void
setstatus(char *str)
{
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
}

/* battery percentage */
char *
battery()
{
    int battery_now, battery_full, battery_perc;
    FILE *fp;

    /* open battery now file */
    if (!(fp = fopen(batterynowfile, "r"))) {
        fprintf(stderr, "Error opening battery file.");
        exit(1);
    }

    /* read value */
    fscanf(fp, "%i", &battery_now);

    /* close battery now file */
    fclose(fp);

    /* open battery full file */
    if (!(fp = fopen(batteryfullfile, "r"))) {
        fprintf(stderr, "Error opening battery file.");
        exit(1);
    }

    /* read value */
    fscanf(fp, "%i", &battery_full);

    /* close battery full file */
    fclose(fp);

    /* calculate percent */
    battery_perc = battery_now / (battery_full / 100);

    /* return batt_perc as string */
    return smprintf("%d%%", battery_perc);
}

/* cpu temperature */
char *
cpu_temperature()
{
    int temperature;
    FILE *fp;

    /* open temperature file */
    if (!(fp = fopen(tempfile, "r"))) {
        fprintf(stderr, "Could not open temperature file.\n");
        exit(1);
    }

    /* extract temperature */
    fscanf(fp, "%d", &temperature);

    /* close temperature file */
    fclose(fp);

    /* return temperature in degrees */
    return smprintf("%d°C", temperature / 1000);
}

/* cpu percentage */
char *
cpu_usage()
{
    int cpu_perc;
    long double a[4], b[4];
    FILE *fp;

    /* open stat file */
    if (!(fp = fopen("/proc/stat","r"))) {
        fprintf(stderr, "Error opening stat file.");
        exit(1);
    }

    /* read values */
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);

    /* close stat file */
    fclose(fp);

    /* wait a second (for avg values) */
    sleep(1);

    /* open stat file */
    if (!(fp = fopen("/proc/stat","r"))) {
        fprintf(stderr, "Error opening stat file.");
        exit(1);
    }

    /* read values */
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);

    /* close stat file */
    fclose(fp);

    /* calculate avg in this second */
    cpu_perc = 100 * ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));

    /* return cpu_perc as string */
    return smprintf("%d%%", cpu_perc);
}

/* date and time */
char *
datetime()
{
    time_t tm;
    size_t bufsize = 19;
    char *buf = malloc(bufsize);

    /* get time in format */
    time(&tm);
    if(!strftime(buf, bufsize, timeformat, localtime(&tm))) {
        fprintf(stderr, "Strftime failed.\n");
        exit(1);
    }

    /* return time */
    return buf;
}

/* ram percentage */
char *
ram_usage()
{
    int ram_perc;
    long total, free, available;
    FILE *fp;

    /* open meminfo file */
    if (!(fp = fopen("/proc/meminfo", "r"))) {
        fprintf(stderr, "Error opening meminfo file.");
        exit(1);
    }

    /* read the values */
    fscanf(fp, "MemTotal: %ld kB\n", &total);
    fscanf(fp, "MemFree: %ld kB\n", &free);
    fscanf(fp, "MemAvailable: %ld kB\n", &available);

    /* close meminfo file */
    fclose(fp);

    /* calculate percentage */
    ram_perc = 100 * (total - available) / total;

    /* return ram_perc as string */
    return smprintf("%d%%",ram_perc);
}

/* smprintf function */
char *
smprintf(char *fmt, ...)
{
    va_list fmtargs;
    char *ret;
    int len;

    va_start(fmtargs, fmt);
    len = vsnprintf(NULL, 0, fmt, fmtargs);
    va_end(fmtargs);

    ret = malloc(++len);
    if (ret == NULL) {
        fprintf(stderr, "Malloc error.");
        exit(1);
    }

    va_start(fmtargs, fmt);
    vsnprintf(ret, len, fmt, fmtargs);
    va_end(fmtargs);

    return ret;
}

/* alsa volume percentage */
char *
volume()
{
    int mute = 0;
    long vol = 0, max = 0, min = 0;
        
    /* get volume from alsa */
    snd_mixer_t *handle;
    snd_mixer_elem_t *pcm_mixer, *mas_mixer;
    snd_mixer_selem_id_t *vol_info, *mute_info;
    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, soundcard);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);
    snd_mixer_selem_id_malloc(&vol_info);
    snd_mixer_selem_id_malloc(&mute_info);
    snd_mixer_selem_id_set_name(vol_info, channel);
    snd_mixer_selem_id_set_name(mute_info, channel);
    pcm_mixer = snd_mixer_find_selem(handle, vol_info);
    mas_mixer = snd_mixer_find_selem(handle, mute_info);
    snd_mixer_selem_get_playback_volume_range((snd_mixer_elem_t *)pcm_mixer,
                    &min, &max);
    snd_mixer_selem_get_playback_volume((snd_mixer_elem_t *)pcm_mixer,
                    SND_MIXER_SCHN_MONO, &vol);
    snd_mixer_selem_get_playback_switch(mas_mixer, SND_MIXER_SCHN_MONO,
                    &mute);
    if (vol_info)
        snd_mixer_selem_id_free(vol_info);
    if (mute_info)
        snd_mixer_selem_id_free(mute_info);
    if (handle)
        snd_mixer_close(handle);

    /* return the string (mute) */
    if (!mute)
        return "mute";
    else
        return smprintf("%d%%", (vol * 100) / max);
}

/* wifi percentage */
char *
wifi_signal()
{
    int bufsize = 255;
    int strength;
    char buf[bufsize];
    char *datastart;
    char path_start[16] = "/sys/class/net/";
    char path_end[11] = "/operstate";
    char path[32];
    char status[5];
    FILE *fp;

    /* generate the path name */
    strcat(path, path_start);
    strcat(path, wificard);
    strcat(path, path_end);

    /* open wifi file */
    if(!(fp = fopen(path, "r"))) {
        fprintf(stderr, "Error opening wifi operstate file.");
        exit(1);
    }

    /* read the status */
    fgets(status, 5, fp);

    /* close wifi file */
    fclose(fp);

    /* check if interface down */
    if(strcmp(status, "up\n") != 0){
        return "n/a";
    }

    /* open wifi file */
    if (!(fp = fopen("/proc/net/wireless", "r"))) {
        fprintf(stderr, "Error opening wireless file.");
        exit(1);
    }

    /* extract the signal strength */
    fgets(buf, bufsize, fp);
    fgets(buf, bufsize, fp);
    fgets(buf, bufsize, fp);
    if ((datastart = strstr(buf, "wlp3s0:")) != NULL) {
        datastart = strstr(buf, ":");
        sscanf(datastart + 1, " %*d   %d  %*d  %*d        %*d      %*d      %*d      %*d      %*d        %*d", &strength);
    }

    /* close wifi file */
    fclose(fp);

    /* return strength in percent */
    return smprintf("%d%%", strength);
}

/* main function */
int
main()
{
    char status[1024];

    Config_t *cfg = open_config("/etc/slstatus.conf");

    KeyValue_t *kv = NULL;

    while((kv = read_next(cfg)) != NULL) {
        if(strcmp(kv->key, "snd.soundcard") == 0) {
            soundcard = malloc(strlen(kv->value) + 1);
            strncpy(soundcard, kv->value, strlen(kv->value));
        } else if(strcmp(kv->key, "snd.channel") == 0) {
            channel = malloc(strlen(kv->value) + 1);
            strncpy(channel, kv->value, strlen(kv->value));
        } else if(strcmp(kv->key, "wifi") == 0) {
            wificard = malloc(strlen(kv->value) + 1);
            strncpy(wificard, kv->value, strlen(kv->value));
        } else if(strcmp(kv->key, "bat.now") == 0) {
            batterynowfile = malloc(strlen(kv->value) + 1);
            strncpy(batterynowfile, kv->value, strlen(kv->value));
        } else if(strcmp(kv->key, "bat.full") == 0) {
            batteryfullfile = malloc(strlen(kv->value) + 1);
            strncpy(batteryfullfile, kv->value, strlen(kv->value));
        } else if(strcmp(kv->key, "cpu.temp") == 0) {
            tempfile = malloc(strlen(kv->value) + 1);
            strncpy(tempfile, kv->value, strlen(kv->value));
        } /*else if(strcmp(kv->key, "timeformat") == 0) {
            timeformat = malloc(strlen(kv->value) + 1);
            strncpy(timeformat, kv->value, strlen(kv->value));
        }*/
    }

    if(soundcard == NULL) {
        exit(1);
    }

    if(channel == NULL) {
        exit(1);
    }

    if(wificard == NULL) {
        exit(1);
    }

    if(batterynowfile == NULL) {
        exit(1);
    }

    if(batteryfullfile == NULL) {
        exit(1);
    }

    if(tempfile == NULL) {
        exit(1);
    }
    
    printf("Soundcard: %s\n", soundcard);
    printf("channel: %s\n", channel);
    printf("Wifi: %s\n", wificard);
    printf("Bat now: %s\n", batterynowfile);
    printf("Bat full: %s\n", batteryfullfile);
    printf("CPU temp: %s\n", tempfile);

    close_config(cfg);

    /* open display */
    if (!(dpy = XOpenDisplay(0x0))) {
        fprintf(stderr, "Cannot open display!\n");
        exit(1);
    }

    /* return status every second */	
    for (;;) {
        sprintf(status, FORMATSTRING, ARGUMENTS);
        setstatus(status);
    }

    /* close display */
    XCloseDisplay(dpy);

    free(soundcard);
    free(channel);
    free(wificard);
    free(batterynowfile);
    free(batteryfullfile);
    free(tempfile);

    /* exit successfully */
    return 0;
}
