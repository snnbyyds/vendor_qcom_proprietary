/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "auto_audio_ext"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include "AutoAudioDaemon.h"
#include "auto_audio_ext.h"

extern "C" {

static struct auto_audio *g_auto_audio = NULL;

const char *g_audio_route[MAX_SESSION][ROUTE_MAX] = {
#ifdef PLATFORM_MSMNILE
    {"SEC_TDM_RX_7 Port Mixer TERT_TDM_TX_7", "1"},
    {"QUAT_TDM_RX_7 Port Mixer QUIN_TDM_TX_7", "1"}
#elif PLATFORM_MSMSTEPPE
    {"QUIN_TDM_RX_7 Port Mixer TERT_TDM_TX_7", "1"},
    {"QUAT_TDM_RX_7 Port Mixer QUAT_TDM_TX_7", "1"}
#else
    {"", ""},
    {"", ""}
#endif
};

const int g_audio_pcm[MAX_SESSION][PCM_MAX] = {
#ifdef PLATFORM_MSMNILE
    {TERT_TDM_TX_HOSTLESS, SEC_TDM_RX_HOSTLESS},
    {QUIN_TDM_TX_HOSTLESS, QUAT_TDM_RX_HOSTLESS}
#elif PLATFORM_MSMSTEPPE
    {TERT_TDM_TX_HOSTLESS, QUIN_TDM_RX_HOSTLESS},
    {QUAT_TDM_TX_HOSTLESS, QUAT_TDM_RX_HOSTLESS}
#else
    {0, 0},
    {0, 0}
#endif
};

//! The name of the wakelock to use for the audio daemon.
static const char* wakelock_name = "audio_daemon";
//! The file descriptor to wake lock.
static int wake_lock_fd = -1;
//! The file descriptor to wake unlock.
static int wake_unlock_fd = -1;
//! The reference count for wake lock.
static int wakelock_level = 0;
//! The mutex for wake lock.
static pthread_mutex_t wakelock_mutex;

/* wakelock related interfaces */
void wakelock_init() {
    const char *wake_lock_path = "/sys/power/wake_lock";
    const char *wake_unlock_path = "/sys/power/wake_unlock";
    bool success = false;

    if ((wake_lock_fd = open(wake_lock_path, O_WRONLY | O_APPEND)) < 0) {
        ALOGE("Open %s failed : %s", wake_lock_path, strerror(errno));
    } else if ((wake_unlock_fd = open(wake_unlock_path, O_WRONLY | O_APPEND)) < 0) {
        close(wake_lock_fd);
        ALOGE("Open %s failed : %s", wake_unlock_path, strerror(errno));
    } else {
        success = true;
    }

    if (!success) {
        wake_lock_fd = -1;
        wake_unlock_fd = -1;
    }

    pthread_mutex_init(&wakelock_mutex, NULL);
}

void wakelock_deinit() {
    if (wake_lock_fd >= 0) {
        close(wake_lock_fd);
        wake_lock_fd = -1;
    }

    if (wake_unlock_fd >= 0) {
        close(wake_unlock_fd);
        wake_unlock_fd = -1;
    }

    pthread_mutex_destroy(&wakelock_mutex);
}

void wakelock_acquire() {
    const size_t len = strlen(wakelock_name);
    ssize_t result = 0;

    pthread_mutex_lock(&wakelock_mutex);
    if (wake_lock_fd < 0) {
        ALOGE("Failed to acquire wakelock due to invalid file descriptor");
    } else {
        wakelock_level++;
        if (wakelock_level == 1) {
            result = write(wake_lock_fd, wakelock_name, len);
            if (result < 0) {
                ALOGE("Failed to acquire wakelock with error %s", strerror(errno));
            } else if (result != static_cast<ssize_t>(len)) {
                ALOGE("Wrote incomplete id to wakelock file descriptor");
            } else {
                ALOGD("Wakelock is acquired");
            }
        }
    }
    pthread_mutex_unlock(&wakelock_mutex);
}

void wakelock_release() {
    const size_t len = strlen(wakelock_name);
    ssize_t result = 0;

    pthread_mutex_lock(&wakelock_mutex);
    if (wake_unlock_fd < 0) {
        ALOGE("Failed to release wakelock due to invalid file descriptor");
    } else {
        --wakelock_level;
        if (wakelock_level == 0) {
            result = write(wake_unlock_fd, wakelock_name, len);
            if (result < 0) {
                ALOGE("Failed to release wakelock with error %s", strerror(errno));
            } else if (result != static_cast<ssize_t>(len)) {
                ALOGE("Wrote incomplete id to wakeunlock file descriptor");
            } else {
                ALOGD("Wakelock is released");
            }
        }
    }
    pthread_mutex_unlock(&wakelock_mutex);
}

#ifdef LINUX_ENABLED
int32_t auto_audio_ext_set_snd_card(int snd_card)
{
    int32_t ret = 0;
    struct snd_card_info *snd_info = NULL;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL, not initialized", __func__);
        return -EINVAL;
    }

    if (snd_card >= MAX_SND_CARD) {
        ALOGE("%s: Invalid snd card num %d", __func__, snd_card);
        return -EINVAL;
    }

    snd_info = &g_auto_audio->snd_card[snd_card];

    snd_info->snd_card = snd_card;

    if (snd_info->mixer) {
        ALOGD("%s: Mixer already opened, closing existing mixer", __func__);
        snd_mixer_close(snd_info->mixer);
    }

    ret = snd_mixer_open(&snd_info->mixer, 0);
    if (ret) {
        ALOGE("%s: Failed to open mixer", __func__);
        return -ret;
    }

    ALOGD("%s: snd card %d, mixer %p", __func__,
          snd_info->snd_card, (void *)snd_info->mixer);

    return ret;
}

static int32_t auto_audio_ext_set_mixer_ctl(const char *ctl_name, int ctl_value)
{
    int32_t ret = 0;
    int32_t card_index = -1;
    char device[32];
    static snd_ctl_t *handle = NULL;
    snd_ctl_elem_id_t *id = NULL;
    snd_ctl_elem_value_t *control = NULL;

    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_value_alloca(&control);

    if (snd_card_next(&card_index) < 0 || card_index < 0) {
        ALOGE("%s: Failed to find sound card", __func__);
        return EINVAL;
    }
    snprintf(device, sizeof(device), "hw:%d", card_index);

    if ((ret = snd_ctl_open(&handle, device, 0)) < 0) {
        ALOGE("%s: Control %s open error: %s", __func__, device, snd_strerror(ret));
        goto exit;
    }

    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, ctl_name);

    snd_ctl_elem_value_set_id(control, id);
    if ((ret = snd_ctl_elem_read(handle, control)) < 0) {
        ALOGE("%s: Cannot read the given element from control", __func__);
        goto exit;
    }

    snd_ctl_elem_value_set_integer(control, 0, ctl_value);
    if ((ret = snd_ctl_elem_write(handle, control)) < 0) {
        ALOGE("%s: Cannot write the given element from control", __func__);
        goto exit;
    }

    return ret;

exit:
    if (handle)
        snd_ctl_close(handle);

    return ret;
}

static int32_t auto_audio_ext_open_start_pcm(int card_index,
                                             int pcm_dev,
                                             snd_pcm_t *pcm_handle,
                                             snd_pcm_stream_t pcm_dir)
{
    int32_t ret = -1;
    char device[32];

    snprintf(device, sizeof(device), "hw:%d,%d", card_index, pcm_dev);

    ret = snd_pcm_open(&pcm_handle, device, pcm_dir, 0);
    if (ret) {
        ALOGE("%s: snd_pcm_open failed for direction(%d): %d",
              __func__, pcm_dir, snd_strerror(ret));
        goto exit;
    }

    ret = snd_pcm_set_params(pcm_handle,
                             SND_PCM_FORMAT_S16_LE,
                             SND_PCM_ACCESS_RW_INTERLEAVED,
                             SND_PCM_DEFAULT_CHAN,
                             SND_PCM_DEFAULT_RATE,
                             SND_PCM_DEFAULT_SRSP,
                             SND_PCM_DEFAULT_LATE);
    if (ret) {
        ALOGE("%s: snd_pcm_set_params failed for direction(%d): %d",
              __func__, pcm_dir, snd_strerror(ret));
        goto exit;
    }

    if (snd_pcm_state(pcm_handle) == SND_PCM_STATE_PREPARED) {
        ret = snd_pcm_start(pcm_handle);
        if (ret) {
            ALOGE("%s: snd_pcm_start failed for SND_PCM_STREAM_PLAYBACK: %d",
                  __func__, snd_strerror(ret));
            goto exit;
        }
    }

exit:
    return ret;
}

/* Note: Due to ADP H/W design, SoC TERT/SEC TDM CLK and FSYNC lines are
 * both connected with CODEC and a single master is needed to provide
 * consistent CLK and FSYNC to slaves, hence configuring SoC TERT TDM as
 * single master and bring up a dummy hostless from TERT to SEC to ensure
 * both slave SoC SEC TDM and CODEC are driven upon system boot.
 */
int32_t auto_audio_ext_enable_hostless(int card_index)
{
    int32_t ret = 0;
    int i = 0, j = 0;
    struct snd_card_info *snd_info = NULL;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL", __func__);
        return -EINVAL;
    }

    if (card_index >= MAX_SND_CARD) {
        ALOGE("%s: Invalid snd card %d", __func__, card_index);
        return -EINVAL;
    }

    snd_info = &g_auto_audio->snd_card[card_index];

    pthread_mutex_lock(&snd_info->lock);

    ALOGI("%s: ENTER", __func__);

    if (!snd_info->mixer) {
        ALOGE("%s: mixer is NULL", __func__);
        ret = -EINVAL;
        goto exit;
    }

    for (i = 0; i < MAX_SESSION; i++) {
        if (snd_info->hostless[i].enable) {
            ALOGD("%s: hostless %d is already enabled", __func__, i);
            continue;
        }

        ret = auto_audio_ext_set_mixer_ctl(g_audio_route[i][ROUTE_MIXER],
                                           atoi(g_audio_route[i][ROUTE_VALUE]));
        if (ret) {
            ALOGE("%s: auto_audio_ext_set_mixer_ctl failed: %d", __func__, ret);
            goto error;
        }

        ret = auto_audio_ext_open_start_pcm(card_index,
                                            g_audio_pcm[i][PCM_OUTPUT],
                                            snd_info->hostless[i].pcm_rx,
                                            SND_PCM_STREAM_PLAYBACK);
        if (ret) {
            ALOGE("%s: auto_audio_ext_open_start_pcm failed for PLAYBACK: %d",
                  __func__, ret);
            goto error;
        }

        ret = auto_audio_ext_open_start_pcm(card_index,
                                            g_audio_pcm[i][PCM_INPUT],
                                            snd_info->hostless[i].pcm_tx,
                                            SND_PCM_STREAM_CAPTURE);
        if (ret) {
            ALOGE("%s: auto_audio_ext_open_start_pcm failed for CAPTURE: %d",
                  __func__, ret);
            goto error;
        }

        ALOGV("%s: hostless[i] is enabled.", __func__, i);
        snd_info->hostless[i].enable = true;
    }

    ALOGI("%s: EXIT", __func__);

    pthread_mutex_unlock(&snd_info->lock);
    return ret;

error:
    for (j = i; j >= 0; j--) {
        if (snd_info->hostless[j].pcm_rx)
            snd_pcm_close(snd_info->hostless[j].pcm_rx);

        if (snd_info->hostless[j].pcm_tx)
            snd_pcm_close(snd_info->hostless[j].pcm_tx);

        auto_audio_ext_set_mixer_ctl(g_audio_route[j][ROUTE_MIXER], 0);

        snd_info->hostless[j].enable = false;
    }

exit:
    ALOGI("%s: EXIT", __func__);

    pthread_mutex_unlock(&snd_info->lock);
    return ret;
}

void auto_audio_ext_disable_hostless(int card_index)
{
    int i = 0;
    struct snd_card_info *snd_info = NULL;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL", __func__);
        return;
    }

    if (card_index >= MAX_SND_CARD) {
        ALOGE("%s: Invalid snd card %d", __func__, card_index);
        return;
    }

    snd_info = &g_auto_audio->snd_card[card_index];

    pthread_mutex_lock(&snd_info->lock);

    ALOGI("%s: ENTER", __func__);

    if (!snd_info->mixer) {
        ALOGE("%s: mixer is NULL", __func__);
        goto exit;
    }

    for (i = 0; i < MAX_SESSION; i++) {
        if (!snd_info->hostless[i].enable) {
            ALOGD("%s: hostless %d is already disabled", __func__, i);
            continue;
        }

        if (snd_info->hostless[i].pcm_tx) {
            snd_pcm_close(snd_info->hostless[i].pcm_tx);
            snd_info->hostless[i].pcm_tx = NULL;
        }
        if (snd_info->hostless[i].pcm_rx) {
            snd_pcm_close(snd_info->hostless[i].pcm_rx);
            snd_info->hostless[i].pcm_rx = NULL;
        }

        if (auto_audio_ext_set_mixer_ctl(g_audio_route[i][ROUTE_MIXER], 0)) {
            ALOGE("%s: auto_audio_ext_set_mixer_ctl failed",
                  __func__);
        }

        snd_info->hostless[i].enable = false;
    }

exit:
    ALOGI("%s: EXIT", __func__);
    pthread_mutex_unlock(&snd_info->lock);
}
#else
int32_t auto_audio_ext_set_snd_card(int snd_card)
{
    int32_t ret = 0;
    struct snd_card_info *info = NULL;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL, not initialized",
                __func__);
        return -EINVAL;
    }

    if (snd_card >= MAX_SND_CARD) {
        ALOGE("%s: Invalid snd card %d",
                __func__, snd_card);
        return -EINVAL;
    }

    info = &g_auto_audio->snd_card[snd_card];

    info->snd_card = snd_card;

    if (info->mixer) {
        ALOGD("%s: Mixer already opened, closing existing mixer", __func__);
        mixer_close(info->mixer);
    }

    info->mixer = mixer_open(snd_card);
    if (!info->mixer) {
        ALOGE("%s: Failed to open mixer", __func__);
        return -EINVAL;
    }

    ALOGD("%s: snd card %d, mixer %p", __func__,
            info->snd_card, (void *)info->mixer);

    return ret;
}

static int32_t auto_audio_ext_set_mixer_ctl(struct mixer *mixer, const char *name, int value)
{
    int32_t ret = 0;
    struct mixer_ctl *ctl = NULL;

    if (!mixer || !name) {
        ALOGE("%s: mixer ctl is NULL", __func__);
        return -EINVAL;
    }

    ctl = mixer_get_ctl_by_name(mixer, name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s", __func__, name);
        return -EINVAL;
    }

    ret = mixer_ctl_set_value(ctl, 0, value);
    if (ret) {
        ALOGE("%s: Could not set ctl for mixer cmd - %d", __func__, ret);
        return ret;
    }

    return ret;
}

/* Note: Due to ADP H/W design, SoC TERT/SEC TDM CLK and FSYNC lines are
 * both connected with CODEC and a single master is needed to provide
 * consistent CLK and FSYNC to slaves, hence configuring SoC TERT TDM as
 * single master and bring up a dummy hostless from TERT to SEC to ensure
 * both slave SoC SEC TDM and CODEC are driven upon system boot.
 */
int32_t auto_audio_ext_enable_hostless(int snd_card)
{
    int32_t ret = 0;
    int i = 0, j = 0;
    struct snd_card_info *info = NULL;

    struct pcm_config pcm_config = {
        .channels = 1,
        .rate = 48000,
        .period_size = 240,
        .period_count = 2,
        .format = PCM_FORMAT_S16_LE,
        .start_threshold = 0,
        .stop_threshold = INT_MAX,
        .avail_min = 0,
    };

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL",
                __func__);
        return -EINVAL;
    }

    if (snd_card >= MAX_SND_CARD) {
        ALOGE("%s: Invalid snd card %d",
                __func__, snd_card);
        return -EINVAL;
    }

    info = &g_auto_audio->snd_card[snd_card];

    pthread_mutex_lock(&info->lock);

    ALOGI("%s: ENTER", __func__);

    if (!info->mixer) {
        ALOGE("%s: mixer is NULL",
                __func__);
        ret = -EINVAL;
        goto exit;
    }

    for (i = 0; i < MAX_SESSION; i++) {
        if (info->hostless[i].enable) {
            ALOGD("%s: hostless %d is already enabled",
                    __func__, i);
            continue;
        }

        ret = auto_audio_ext_set_mixer_ctl(info->mixer,
                                        g_audio_route[i][ROUTE_MIXER],
                                        atoi(g_audio_route[i][ROUTE_VALUE]));
        if (ret) {
            ALOGE("%s: auto_audio_ext_set_mixer_ctl failed - %d",
                    __func__, ret);
            goto error;
        }

        info->hostless[i].pcm_tx = pcm_open(info->snd_card,
                                        g_audio_pcm[i][PCM_INPUT],
                                        PCM_IN, &pcm_config);
        if (info->hostless[i].pcm_tx &&
            !pcm_is_ready(info->hostless[i].pcm_tx)) {
            ALOGE("%s: %s", __func__,
                pcm_get_error(info->hostless[i].pcm_tx));
            ret = -EIO;
            goto error;
        }
        info->hostless[i].pcm_rx = pcm_open(info->snd_card,
                                        g_audio_pcm[i][PCM_OUTPUT],
                                        PCM_OUT, &pcm_config);
        if (info->hostless[i].pcm_rx &&
            !pcm_is_ready(info->hostless[i].pcm_rx)) {
            ALOGE("%s: %s", __func__,
                pcm_get_error(info->hostless[i].pcm_rx));
            ret = -EIO;
            goto error;
        }

        if (pcm_start(info->hostless[i].pcm_tx) < 0) {
            ALOGE("%s: pcm start for pcm tx failed", __func__);
            ret = -EIO;
            goto error;
        }
        if (pcm_start(info->hostless[i].pcm_rx) < 0) {
            ALOGE("%s: pcm start for pcm rx failed", __func__);
            ret = -EIO;
            goto error;
        }

        info->hostless[i].enable = true;
    }

    ALOGI("%s: EXIT", __func__);
    pthread_mutex_unlock(&info->lock);
    return ret;

error:
    for (j = i; j >= 0; j--) {
        if (info->hostless[j].pcm_rx)
            pcm_close(info->hostless[j].pcm_rx);
        if (info->hostless[j].pcm_tx)
            pcm_close(info->hostless[j].pcm_tx);
        auto_audio_ext_set_mixer_ctl(info->mixer,
                                    g_audio_route[j][ROUTE_MIXER], 0);
        info->hostless[j].enable = false;
    }
exit:
    ALOGI("%s: EXIT", __func__);
    pthread_mutex_unlock(&info->lock);
    return ret;
}

void auto_audio_ext_disable_hostless(int snd_card)
{
    int i = 0;
    struct snd_card_info *info = NULL;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL",
                __func__);
        return;
    }

    if (snd_card >= MAX_SND_CARD) {
        ALOGE("%s: Invalid snd card %d",
                __func__, snd_card);
        return;
    }

    info = &g_auto_audio->snd_card[snd_card];

    pthread_mutex_lock(&info->lock);

    ALOGI("%s: ENTER", __func__);

    if (!info->mixer) {
        ALOGE("%s: mixer is NULL",
                __func__);
        goto exit;
    }

    for (i = 0; i < MAX_SESSION; i++) {
        if (!info->hostless[i].enable) {
            ALOGD("%s: hostless %d is already disabled",
                    __func__, i);
            continue;
        }

        if (info->hostless[i].pcm_tx) {
            pcm_close(info->hostless[i].pcm_tx);
            info->hostless[i].pcm_tx = NULL;
        }
        if (info->hostless[i].pcm_rx) {
            pcm_close(info->hostless[i].pcm_rx);
            info->hostless[i].pcm_rx = NULL;
        }

        if (auto_audio_ext_set_mixer_ctl(info->mixer,
                                        g_audio_route[i][ROUTE_MIXER], 0)) {
            ALOGE("%s: auto_audio_ext_set_mixer_ctl failed",
                    __func__);
        }

        info->hostless[i].enable = false;
    }

exit:
    ALOGI("%s: EXIT", __func__);
    pthread_mutex_unlock(&info->lock);
}
#endif

int32_t auto_audio_ext_enable_hostless_all(void)
{
    int i = 0, j = 0;
    int32_t ret = 0;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL", __func__);
        return -EINVAL;
    }

    for (i = 0; i < MAX_SND_CARD; i++) {
        if (!g_auto_audio->snd_card[i].mixer)
            continue;
        ret = auto_audio_ext_enable_hostless(i);
        if (ret) {
            ALOGE("%s: auto_audio_ext_enable_hostless() failed", __func__);
            goto error;
        }
    }
    return ret;

error:
    for (j = i; j >= 0; j--) {
        if (!g_auto_audio->snd_card[j].mixer)
            continue;
        auto_audio_ext_disable_hostless(j);
    }
    return ret;
}

void auto_audio_ext_disable_hostless_all(void)
{
    int i = 0;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL", __func__);
        return;
    }

    for (i = 0; i < MAX_SND_CARD; i++) {
        if (!g_auto_audio->snd_card[i].mixer)
            continue;
        auto_audio_ext_disable_hostless(i);
    }
}

int32_t auto_audio_ext_init(void)
{
    int32_t ret = 0;
    int i = 0;

    if (g_auto_audio != NULL) {
        ALOGD("%s: auto audio already initialized",
                __func__);
        return ret;
    }

    g_auto_audio = (struct auto_audio *)calloc(1, sizeof(struct auto_audio));
    if (g_auto_audio == NULL) {
        ALOGE("%s: Memory allocation failed for auto audio",
                __func__);
        return -ENOMEM;
    }
    memset(g_auto_audio, 0, sizeof(struct auto_audio));

    pthread_mutex_init(&g_auto_audio->lock, NULL);

    for (i = 0; i < MAX_SND_CARD; i++) {
        pthread_mutex_init(&g_auto_audio->snd_card[i].lock, NULL);
    }

    wakelock_init();

    return ret;
}

#ifdef LINUX_ENABLED
void auto_audio_ext_deinit(void)
{
    int i = 0, j = 0;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL, cannot deinitialize", __func__);
        return;
    }

    for (i = 0; i < MAX_SND_CARD; i++) {
        if (!g_auto_audio->snd_card[i].mixer)
            continue;

        for (j = 0; j < MAX_SESSION; j++) {
            if (g_auto_audio->snd_card[i].hostless[j].pcm_tx)
                snd_pcm_close(g_auto_audio->snd_card[i].hostless[j].pcm_tx);

            if (g_auto_audio->snd_card[i].hostless[j].pcm_rx)
                snd_pcm_close(g_auto_audio->snd_card[i].hostless[j].pcm_rx);

            auto_audio_ext_set_mixer_ctl(g_audio_route[j][ROUTE_MIXER], 0);
        }

        snd_mixer_close(g_auto_audio->snd_card[i].mixer);
        pthread_mutex_destroy(&g_auto_audio->snd_card[i].lock);
    }

    pthread_mutex_destroy(&g_auto_audio->lock);
    wakelock_deinit();
    free(g_auto_audio);

    return;
}
#else
void auto_audio_ext_deinit(void)
{
    int i = 0, j = 0;

    if (!g_auto_audio) {
        ALOGE("%s: auto audio is NULL, cannot deinitialize",
                __func__);
        return;
    }

    for (i = 0; i < MAX_SND_CARD; i++) {
        if (!g_auto_audio->snd_card[i].mixer)
            continue;

        for (j = 0; j < MAX_SESSION; j++) {
            if (g_auto_audio->snd_card[i].hostless[j].pcm_tx)
                pcm_close(g_auto_audio->snd_card[i].hostless[j].pcm_tx);

            if (g_auto_audio->snd_card[i].hostless[j].pcm_rx)
                pcm_close(g_auto_audio->snd_card[i].hostless[j].pcm_rx);

            auto_audio_ext_set_mixer_ctl(g_auto_audio->snd_card[i].mixer,
                                        g_audio_route[j][ROUTE_MIXER], 0);
        }

        mixer_close(g_auto_audio->snd_card[i].mixer);

        pthread_mutex_destroy(&g_auto_audio->snd_card[i].lock);
    }

    pthread_mutex_destroy(&g_auto_audio->lock);

    wakelock_deinit();

    free(g_auto_audio);

    return;
}
#endif

}
