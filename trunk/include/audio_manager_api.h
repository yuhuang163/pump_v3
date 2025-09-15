#ifndef __AUDIO_MANAGER_API_H__
#define __AUDIO_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#define AUDIO_DATA_FRAME_SIZE            1024

/// 通过文件路径接口
#define PLAY_AUDIO_SOUND_TASK(_audio_manager, _user_data, _tag, _filename, _cb) \
    __PLAY_AUDIO_SOUND_TASK(_audio_manager, _user_data, _tag, _filename, 0, _cb, 0, 0)

#define PLAY_AUDIO_SOUND_TASK_EXIT(_audio_manager, _user_data, _tag, _filename, _cb) \
    __PLAY_AUDIO_SOUND_TASK(_audio_manager, _user_data, _tag, _filename, 0, _cb, 1, 0)

#define PLAY_MUSIC_FILE_TASK(_audio_manager, _user_data, _tag, _filename, _cb) \
    __PLAY_AUDIO_SOUND_TASK(_audio_manager, _user_data, _tag, _filename, 0, _cb, 0, 1)

/// 通过资源ID接口.
#define PLAY_AUDIO_SOUND_ID_TASK(_audio_manager, _user_data, _tag, _sound_id, _cb) \
    __PLAY_AUDIO_SOUND_TASK(_audio_manager, _user_data, _tag, NULL, _sound_id, _cb, 0, 0)

#define PLAY_AUDIO_SOUND_ID_TASK_EXIT(_audio_manager, _user_data, _tag, _sound_id, _cb) \
    __PLAY_AUDIO_SOUND_TASK(_audio_manager, _user_data, _tag, NULL, _sound_id, _cb, 1, 0)

#define __PLAY_AUDIO_SOUND_TASK(_audio_manager, _user_data, _tag, _filename, _sound_id, _cb, _exit_when_play_finish, _is_music_task) \
do { \
    audio_sound_prop_t __ads_task = {0}; \
   __ads_task.user_data = (_user_data); \
   __ads_task.tag = (_tag); \
   __ads_task.filename = (_filename == NULL ? "#null" : _filename); \
   __ads_task.cb = (play_finish_cb_fptr)(_cb); \
   __ads_task.sound_id = (_sound_id); \
   __ads_task.exit_when_play_finish = (_exit_when_play_finish); \
   __ads_task.is_music_task = (_is_music_task); \
   LOG_AUDIO("=>play_audio_sound: id=%d(tag=%s),is_music_task: %d", _sound_id, _tag, _is_music_task); \
   (_audio_manager)->play_audio_sound(&__ads_task); \
} while (0)

typedef enum {
    AUDIO_STREAM_TYPE_WAVE_GENERATOR = 0,
    AUDIO_STREAM_TYPE_WAVE_FILE,
} audio_play_stream_type_e;

struct audio_sound_prop;
struct audio_manager_context;
typedef int (*play_finish_cb_fptr)(struct audio_sound_prop *ads_obj, void *user_data);
typedef void (*on_play_finish_cb_fptr)(struct audio_manager_context *ctx, const char *filename, int flag);
typedef void (*audio_sound_cb)(void *user_data);

typedef struct audio_sound_prop {
    const char *tag;
    const char *filename;
    void *user_data;
    play_finish_cb_fptr cb;
    uint16_t sound_id;
    uint8_t is_pending;
    uint8_t exit_when_play_finish;
    uint8_t is_music_task;
} audio_sound_prop_t;

typedef struct {
    uint32_t addr_offset;
    uint16_t decode_index;
    audio_sound_prop_t sound_prop;
    void *pause_user_data;
    audio_sound_cb pause_audio_sound_cb;
} audio_sound_ctrl_t;

/// 依赖模块
struct storage_manager_context;
struct resource_manager_context;
struct rt_device_node;
/// 描述音频管理上下文对象
typedef struct audio_manager_context {
/// public
    void (*set_audio_filepath)(const char *filepath);
    on_play_finish_cb_fptr on_play_notify_cb;
    void (*set_play_audio_type)(audio_play_stream_type_e type);
    int (*play_audio_sound)(audio_sound_prop_t *ads_task);
    int (*is_on_sound_playing)(void);
    bool force_play_music;      /// 语音提醒设置为关闭时，仍可播放音乐
    int (*add_audio_play_task)(list_head_t *head, int index);

/// private
    struct __audio_private {
        uint8_t *audio_buffer;
        char *audio_filepath;
        struct rt_device_node *audio_dev;
        audio_play_stream_type_e play_wave_type;
        uint16_t sample_rate; /// 采样率
        audio_sound_prop_t ads_task; /// 记录当前需要播放的语音任务.
        bool is_playing;
        audio_sound_ctrl_t audio_sound_ctrl;
        struct resource_manager_context *rm;
    } priv;
} audio_manager_context_t;

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */


#ifdef __cplusplus
}
#endif

#endif 


