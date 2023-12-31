/******************************************************************************
  @file    OptsHandler.cpp
  @brief   Implementation for handling operations

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#define ATRACE_TAG ATRACE_TAG_ALWAYS
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <sched.h>
#include <cutils/android_filesystem_config.h>

#define LOG_TAG "ANDR-PERF-OPTSHANDLER"
#include <log/log.h>

#include "ioctl.h"
#include "Request.h"
#include "MpctlUtils.h"
#include "OptsData.h"
#include "ResourceQueues.h"
#include "Target.h"
#include "ResourceInfo.h"
#include "EventQueue.h"
#include "OptsHandler.h"
#include "PerfController.h"
#include "BoostConfigReader.h"
#include <cutils/trace.h>

#define ENABLE_PREFER_IDLE  1
#define WRITE_MIGRATE_COST  0

#define DISABLE_PC_LATENCY  1
#define MAX_LPM_BIAS_HYST 100
#define DEFAULT_LPM_BIAS_HYST 0
#define MPCTL_NO_ROOT_NAME "PERFD-EW-DISP"

//Assumes max # cpus == 32
#define CPUSET_STR_SIZE 86

#define RELEASE_LOCK 0
#define ACQUIRE_LOCK 1
#define BASE_10 10

#define ACQ 1
#define REL 0
#define UP 1
#define DOWN 0
typedef enum {
    UPDOWN,
    DOWNUP
} MIG_SEQ;

OptsHandler OptsHandler::mOptsHandler;

namespace {
    pthread_t id_ka_thread;
    pthread_t noroot_tid;
    bool tKillFlag = true;
    bool isThreadAlive = false;
    bool isNorootThreadAlive = false;
    pthread_mutex_t ka_mutex = PTHREAD_MUTEX_INITIALIZER;

    android::sp<IPasrManager> mPasrManager = nullptr;
    bool pasr_enabled = false;

    EventQueue NRevqueue;
}

OptsHandler::OptsHandler() {
    InitializeOptsTable();
}

OptsHandler::~OptsHandler() {
}

int OptsHandler::Init() {

    int retval = 0;
    mPasrManager = IPasrManager::getService(PASR_SERVICE_NAME);

    if (mPasrManager && !access(MEM_OFFLINE_NODE, F_OK))
        pasr_enabled = true;

    NRevqueue.GetDataPool().SetCBs(Alloccb, Dealloccb);
    retval = pthread_create(&noroot_tid, NULL, noroot_thread, NULL);

    if (retval != 0) {
        QLOGE("Could not create noroot_thread");
    } else {
        retval = pthread_setname_np(noroot_tid, MPCTL_NO_ROOT_NAME);
        if (retval != 0) {
            QLOGE("Failed to name no_root Thread: %s", MPCTL_NO_ROOT_NAME);
        }
    }

    return retval;
}

// callbacks for EventQueue
void *OptsHandler::Alloccb() {
    void *mem = (void *) new(std::nothrow) noroot_args;
    return mem;
}

void OptsHandler::Dealloccb(void *mem) {
    if (mem) {
        delete (noroot_args *)mem;
    }
}

void *OptsHandler::noroot_thread(void *args) {
    int retval = 0;
    int cmd;
    noroot_args *msg = NULL;
    bool trace_flag = Target::getCurTarget().isTraceEnabled();
    char trace_buf[TRACE_BUF_SZ] = {0};

    /* We must drop the root privileges before opening the DRM node */
    if (setuid(AID_SYSTEM)) {
        QLOGE("Failed to drop root UID: %s. Cannot open the DRM node", strerror(errno));
        isNorootThreadAlive = false;
        pthread_exit(NULL);
    }

    isNorootThreadAlive = true;

    /* Main loop */
    for (;;) {
        EventData *evData = NRevqueue.Wait();

        if (!evData || !evData->mEvData) {
            continue;
        }

        cmd = evData->mEvType;
        msg = (noroot_args *)evData->mEvData;
        if (trace_flag) {
            snprintf(trace_buf, TRACE_BUF_SZ,"disp_early_wakeup: %d ", cmd);
            ATRACE_BEGIN(trace_buf);
        }

        switch (cmd) {
            case DISPLAY_EARLY_WAKEUP_HINT:
                retval = early_wakeup_ioctl(msg->val);
                msg->retval = retval;
                break;
            default:
                QLOGE("Unsupported cmd: %d", cmd);
                break;
        }
        if (trace_flag) {
            ATRACE_END();
        }

        NRevqueue.GetDataPool().Return(evData);
    }
    return NULL;
}

void OptsHandler::InitializeOptsTable() {
/* Assuming that all the information related to resources, such as CompareOpts., are dependend only the
resource and not on targets. If this assumption changes it's better to move mOptsTable to TargetInit.
 * Assigning default ApplyOts and ResetOpts function to all the resources and also CompareOpts to
 higher_is_better by default. If any resource does not follow the default types, they need to be changed.*/

    int idx = 0;
    for (idx = 0; idx < MAX_MINOR_RESOURCES; idx++) {
        mOptsTable[idx] = {modify_sysfsnode, modify_sysfsnode, higher_is_better};
    }

    /* changing CompareOpts to lower_is_better basing on the resource index, which is
       the sum of it's major group start id and minor id. */
    mOptsTable[POWER_COLLAPSE_START_INDEX + L2_POWER_COLLAPSE_PERF_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[SCHED_START_INDEX + SCHED_STATIC_CPU_PWR_COST_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[SCHED_START_INDEX + SCHED_RESTRICT_CLUSTER_SPILL_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[SCHED_START_INDEX + SCHED_FREQ_AGGR_THRESHOLD_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[SCHED_START_INDEX + SCHED_FORCE_LB_ENABLE].mCompareOpts = lower_is_better;
    mOptsTable[INTERACTIVE_START_INDEX + INTERACTIVE_TIMER_RATE_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[INTERACTIVE_START_INDEX + INTERACTIVE_TIMER_SLACK_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[INTERACTIVE_START_INDEX + INTERACTIVE_MAX_FREQ_HYSTERESIS_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[INTERACTIVE_START_INDEX + SCHEDUTIL_HISPEED_LOAD_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[ONDEMAND_START_INDEX + OND_SAMPLING_RATE_OPCODE].mCompareOpts = lower_is_better;
    mOptsTable[GPU_START_INDEX + GPU_MIN_POWER_LEVEL].mCompareOpts = lower_is_better;

    /* changing CompareOpts to always_apply basing on the resource index.*/
    mOptsTable[SCHED_START_INDEX + SCHED_WINDOW_TICKS_UPDATE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_CPUSET_TOP_APP_OPCODE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_CPUSET_FOREGROUND_OPCODE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_CPUSET_SYSTEM_BACKGROUND_OPCODE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_CPUSET_BACKGROUND_OPCODE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHEDTUNE_PREFER_IDLE_OPCODE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_TUNE_PREFER_IDLE_FOREGROUND].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_BUSY_HYSTERSIS_CPU_MASK_OPCODE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_BUSY_HYSTERESIS_ENABLE_COLOC_CPUS].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_COLOC_BIAS_HYST].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + SCHED_WINDOW_STATS_POLICY].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + STUNE_TOPAPP_SCHEDTUNE_COLOCATE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + CPUCTL_TOPAPP_UCLAMP_LATENCY_SENSITIVE].mCompareOpts = always_apply;
    mOptsTable[SCHED_START_INDEX + CPUCTL_FG_UCLAMP_LATENCY_SENSITIVE].mCompareOpts = always_apply;
    mOptsTable[INTERACTIVE_START_INDEX + SCHEDUTIL_PREDICTIVE_LOAD_OPCODE].mCompareOpts = always_apply;
    mOptsTable[MISC_START_INDEX + STORAGE_CLK_SCALING_DISABLE_OPCODE].mCompareOpts = always_apply;

    /* changing ApplyOts and ResetOpts functions to call different function.*/
    mOptsTable[DISPLAY_START_INDEX + DISPLAY_OFF_OPCODE] = {dummy, dummy, higher_is_better};

    mOptsTable[POWER_COLLAPSE_START_INDEX + POWER_COLLAPSE_OPCODE] =
                        {pmQoS_cpu_dma_latency, pmQoS_cpu_dma_latency, lower_is_better};

    /* cpu freq */
    mOptsTable[CPUFREQ_START_INDEX + CPUFREQ_MIN_FREQ_OPCODE] = {cpu_options, cpu_options, higher_is_better};
    mOptsTable[CPUFREQ_START_INDEX + CPUFREQ_MAX_FREQ_OPCODE] = {cpu_options, cpu_options, higher_is_better};

    /* sched group */
    mOptsTable[SCHED_START_INDEX + SCHED_BOOST_OPCODE] = {set_sched_boost, reset_sched_boost, always_apply};
    mOptsTable[SCHED_START_INDEX + SCHED_UPMIGRATE_OPCODE] = {migrate, migrate, lower_is_better};
    mOptsTable[SCHED_START_INDEX + SCHED_DOWNMIGRATE_OPCODE] = {migrate, migrate, lower_is_better};
    mOptsTable[SCHED_START_INDEX + SCHED_GROUP_OPCODE] = {add_sched_group, reset_sched_group, always_apply};
    mOptsTable[SCHED_START_INDEX + SCHED_FREQ_AGGR_GROUP_OPCODE] =
                        {sched_add_freq_aggr_group, sched_reset_freq_aggr_group, always_apply};
    mOptsTable[SCHED_START_INDEX + SCHED_TASK_BOOST] =
                        {sched_task_boost, sched_reset_task_boost, add_in_order};
    mOptsTable[SCHED_START_INDEX + SCHED_ENABLE_TASK_BOOST_RENDERTHREAD] =
                        {sched_enable_task_boost_renderthread, sched_reset_task_boost,
                         add_in_order_fps_based_taskboost};
    mOptsTable[SCHED_START_INDEX + SCHED_DISABLE_TASK_BOOST_RENDERTHREAD] =
                        {sched_reset_task_boost, sched_reset_task_boost,
                         add_in_order_fps_based_taskboost};
    mOptsTable[SCHED_START_INDEX + SCHED_LOW_LATENCY] =
                        {sched_low_latency, sched_reset_low_latency, add_in_order};
    mOptsTable[SCHED_START_INDEX + SCHED_COLOC_BUZY_HYST_CPU_PCT] =
                        {sched_coloc_busy_hyst_cpu_busy_pct, sched_coloc_busy_hyst_cpu_busy_pct, lower_is_better};
    mOptsTable[SCHED_START_INDEX + SCHED_COLOC_BUZY_HYST_CPU_NS] =
                        {sched_coloc_busy_hyst_cpu_ns, sched_coloc_busy_hyst_cpu_ns, higher_is_better};
    mOptsTable[SCHED_START_INDEX + SCHED_UP_DOWN_MIGRATE] =
                        {migrate_action_apply, migrate_action_release, migrate_lower_is_better};
    mOptsTable[SCHED_START_INDEX + SCHED_GROUP_UP_DOWN_MIGRATE] =
                        {grp_migrate_action_apply, grp_migrate_action_release, migrate_lower_is_better};

    /* hotplug */
    mOptsTable[CORE_HOTPLUG_START_INDEX + CORE_HOTPLUG_MIN_CORE_ONLINE_OPCODE] =
                        {lock_min_cores, lock_min_cores, higher_is_better};
    mOptsTable[CORE_HOTPLUG_START_INDEX + CORE_HOTPLUG_MAX_CORE_ONLINE_OPCODE] =
                        {lock_max_cores, lock_max_cores, higher_is_better};

    /* cpubw hwmon */
    mOptsTable[CPUBW_HWMON_START_INDEX + CPUBW_HWMON_HYST_OPT_OPCODE] =
                        {cpubw_hwmon_hyst_opt, cpubw_hwmon_hyst_opt, higher_is_better};

    /* video */
    mOptsTable[VIDEO_START_INDEX + VIDEO_ENCODE_PB_HINT] =
                        {handle_vid_encplay_hint, handle_vid_encplay_hint, higher_is_better};
    mOptsTable[VIDEO_START_INDEX + VIDEO_DECODE_PB_HINT] =
                        {handle_vid_decplay_hint, handle_vid_decplay_hint, higher_is_better};
    mOptsTable[VIDEO_START_INDEX + VIDEO_DISPLAY_PB_HINT] =
                        {handle_disp_hint, handle_disp_hint, higher_is_better};

    /* display */
    mOptsTable[VIDEO_START_INDEX + DISPLAY_EARLY_WAKEUP_HINT] =
                        {handle_early_wakeup_hint, handle_early_wakeup_hint, always_apply};

    /* ksm */
    mOptsTable[KSM_START_INDEX + KSM_ENABLE_DISABLE_OPCODE] =
                        {disable_ksm, enable_ksm, higher_is_better};
    mOptsTable[KSM_START_INDEX + KSM_SET_RESET_OPCODE] =
                        {set_ksm_param, reset_ksm_param, higher_is_better};

    /* gpu */
    mOptsTable[GPU_START_INDEX + GPU_DISABLE_GPU_NAP_OPCODE] =
                        {gpu_disable_gpu_nap, gpu_disable_gpu_nap, higher_is_better};
    mOptsTable[GPU_START_INDEX + GPU_IS_APP_FG_OPCODE] = {gpu_is_app_fg, dummy, always_apply};
    mOptsTable[GPU_START_INDEX + GPU_IS_APP_BG_OPCODE] = {gpu_is_app_bg, dummy, always_apply};

    /* miscellaneous */
    mOptsTable[MISC_START_INDEX + UNSUPPORTED_OPCODE] = {unsupported, unsupported, higher_is_better};
    mOptsTable[MISC_START_INDEX + IRQ_BAL_OPCODE] = {irq_balancer, irq_balancer, higher_is_better};
    mOptsTable[MISC_START_INDEX + NET_KEEP_ALIVE_OPCODE] = {keep_alive, dummy, higher_is_better};
    mOptsTable[MISC_START_INDEX + PID_AFFINE] = {set_pid_affine, reset_pid_affine, always_apply};
    mOptsTable[MISC_START_INDEX + DISABLE_PASR] = {perfmode_entry_pasr, perfmode_exit_pasr, always_apply};
    mOptsTable[MISC_START_INDEX + FPS_HYST_OPCODE] = {handle_fps_hyst, handle_fps_hyst, higher_is_better};

    /*llcbw hwmon*/
    mOptsTable[LLCBW_HWMON_START_INDEX + LLCBW_HWMON_HYST_OPT_OPCODE] =
                        {llcbw_hwmon_hyst_opt, llcbw_hwmon_hyst_opt, higher_is_better};

    mOptsTable[LLCBW_HWMON_START_INDEX + LLCC_DDR_BW_HYST_OPT] =
                        {llcbw_hwmon_hyst_opt, llcbw_hwmon_hyst_opt, higher_is_better};

    mOptsTable[CPUBW_HWMON_START_INDEX + CPU_LLCC_BW_HYST_OPT] =
                        {cpubw_hwmon_hyst_opt, cpubw_hwmon_hyst_opt, higher_is_better};


    /* memlat */
    mOptsTable[MEMLAT_START_INDEX + L3_MEMLAT_MINFREQ_OPCODE] = {l3_min_freq, l3_min_freq, higher_is_better};
    mOptsTable[MEMLAT_START_INDEX + L3_MEMLAT_MAXFREQ_OPCODE] = {l3_max_freq, l3_max_freq, higher_is_better};


    /* npubw_llcbw_ddr hwmon */
    mOptsTable[NPU_START_INDEX + NPU_LLC_DDR_HWMON_HYST_OPT_OPCODE] =
                        {npubw_hwmon_hyst_opt, npubw_hwmon_hyst_opt, higher_is_better};
    /*npu_llcbw hwmon*/
    mOptsTable[NPU_START_INDEX + NPU_LLCBW_HWMON_HYST_OPT_OPCODE] =
                        {npu_llcbw_hwmon_hyst_opt, npu_llcbw_hwmon_hyst_opt, higher_is_better};
    mOptsTable[SCHED_START_INDEX + SCHED_LOAD_BOOST_OPCODE].mCompareOpts = higher_is_better_negative;
}

int OptsHandler::ApplyOpt(Resource &resObj) {
    int ret = FAILED;
    unsigned short idx = resObj.qindex;
    OptsData &dataObj = OptsData::getInstance();
    if (idx < MAX_MINOR_RESOURCES) {
        ret = mOptsTable[idx].mApplyOpts(resObj, dataObj);
    } else {
        QLOGE("Failed to call apply optimization for 0x%x", resObj.qindex);
    }
    return ret;
}

int OptsHandler::ResetOpt(Resource &resObj) {
    int ret = FAILED;
    unsigned short idx = resObj.qindex;
    OptsData &dataObj = OptsData::getInstance();
    if (idx < MAX_MINOR_RESOURCES) {
        ret = mOptsTable[idx].mResetOpts(resObj, dataObj);
    } else {
        QLOGE("Failed to call reset optimization for 0x%x", idx);
    }
    return ret;
}

int OptsHandler::CompareOpt(int qindx, unsigned int reqVal, unsigned int curVal) {
    int ret = ADD_NEW_REQUEST;

    //First resource id present
    if (qindx >=0 && qindx < MAX_MINOR_RESOURCES) {
        ret = mOptsTable[qindx].mCompareOpts(reqVal, curVal);
    } else {
        QLOGE("Cannot find a compareOpt function");
        return FAILED;
    }
    return ret;
}

/*Function for reqval overflow check during multiplication*/
unsigned int OptsHandler::MultiplyReqval(unsigned int rval, unsigned int factor) {
    unsigned int fin_rval;
    if (rval < (UINT_MAX/factor)) {
       fin_rval = rval * factor;
    } else {
       fin_rval = UINT_MAX;
    }
    return fin_rval;
}

int OptsHandler::ValidateClusterAndCore(int cluster, int core, int resourceStatus) {
    Target &t = Target::getCurTarget();
    TargetConfig &tc = TargetConfig::getTargetConfig();
    int supportedCore = -1, core_no = -1;

    //First check cluster and core are in valid range
    if ((cluster < 0) || (cluster > tc.getNumCluster())) {
        QLOGE("Invalid cluster no. %d", cluster);
        return FAILED;
    }
    if ((core < 0) || (core > t.getLastCoreIndex(cluster))) {
        QLOGE("Invalid core no. %d", core);
        return FAILED;
    }

    //Second check resource is supported on that core or not
    core_no = t.getFirstCoreIndex(cluster);
    switch (resourceStatus) {
        case SYNC_CORE: //Resource is sync core based, only first core of cluster is supported
            if ((core != 0) && (core != core_no)) {
                QLOGE("Core %d is not supported for this perflock resource, instead use core 0/%d", core, core_no);
                return FAILED;
            }
            break;
       case ASYNC_CORE: //Resource is async core based, all core accepted
            QLOGI("Core %d is supported for this perflock resource", core);
            break;
       case CORE_INDEPENDENT: //Resource does not depend on core, only first core of perf cluster is accepted
            supportedCore = t.getFirstCoreOfPerfCluster();
            if ((supportedCore == -1) || (core != supportedCore)) {
                QLOGE("Core %d is not supported for this perflock resource, instead use core %d", core, supportedCore);
                return FAILED;
            }
            break;
       default:
            QLOGE("Invalid resource based core status");
            return FAILED;
    }
    return SUCCESS;
}

/*All possible modifications to the requested value and any pre acquire node
updations can to be done in this function and finally update the acqval with
the string with which the requested node needs to be acquired.*/
int OptsHandler::CustomizeRequestValue(Resource &r, OptsData &d, char *acqval) {
    unsigned int reqval = r.value;
    int idx = r.qindex, rc = FAILED, cpu = -1, setval = FAILED;
    char *tmp_here = NULL, tmp_node[NODE_MAX] = "";
    bool valid_bound = true;
    unsigned int valCluster1 = 0, valCluster2 = 0;
    int hwmon_max_freq_index = 0, hwmon_min_freq_index = 0;
    int kernelMajor = 0, kernelMinor = 0;
    TargetConfig &tc = TargetConfig::getTargetConfig();
    Target &target = Target::getCurTarget();

    switch (idx) {
    case POWER_COLLAPSE_START_INDEX + L2_POWER_COLLAPSE_PERF_OPCODE:
    case SCHED_START_INDEX + SCHED_SET_FREQ_AGGR_OPCODE:
    case SCHED_START_INDEX + SCHED_ENABLE_THREAD_GROUPING_OPCODE:
    case SCHED_START_INDEX + SCHED_RESTRICT_CLUSTER_SPILL_OPCODE:
    case SCHED_START_INDEX + SCHEDTUNE_PREFER_IDLE_OPCODE:
    case SCHED_START_INDEX + SCHED_TUNE_PREFER_IDLE_FOREGROUND:
    case SCHED_START_INDEX + STUNE_TOPAPP_SCHEDTUNE_COLOCATE:
    case SCHED_START_INDEX + CPUCTL_TOPAPP_UCLAMP_LATENCY_SENSITIVE:
    case SCHED_START_INDEX + CPUCTL_FG_UCLAMP_LATENCY_SENSITIVE:
    case ONDEMAND_START_INDEX + OND_IO_IS_BUSY_OPCODE:
    case ONDEMAND_START_INDEX + OND_ENABLE_STEP_UP:
    case INTERACTIVE_START_INDEX + INTERACTIVE_BOOST_OPCODE:
    case INTERACTIVE_START_INDEX + INTERACTIVE_IO_IS_BUSY_OPCODE:
    case INTERACTIVE_START_INDEX + SCHEDUTIL_PREDICTIVE_LOAD_OPCODE:
        reqval = !!reqval;
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case SCHED_START_INDEX + SCHED_MIGRATE_COST_OPCODE:
        snprintf(acqval, NODE_MAX, "%d", WRITE_MIGRATE_COST);
        break;

    case SCHED_START_INDEX + SCHED_CPUSET_TOP_APP_OPCODE:
    case SCHED_START_INDEX + SCHED_CPUSET_SYSTEM_BACKGROUND_OPCODE:
    case SCHED_START_INDEX + SCHED_CPUSET_BACKGROUND_OPCODE:
        tmp_here = cpuset_bitmask_to_str(reqval);
        if (tmp_here != NULL) {
            strlcpy(acqval, tmp_here, NODE_MAX);
            delete[] tmp_here;
        } else {
            QLOGE("Failed to get the cpuset bitmask for requested %u value", reqval);
            return FAILED;
        }
        break;

    case SCHED_START_INDEX + SCHED_CPUSET_FOREGROUND_OPCODE:
        /* Assumption, foreground/boost/cpus and foreground/cpus have same
        bitmask all the times. During acquire first update foreground/boost/cpus
        follwed by foreground/cpus and during release first release foreground/cpus
        foolowed by foreground/boost/cpus */
        tmp_here = cpuset_bitmask_to_str(reqval);
        if (tmp_here != NULL) {
            strlcpy(acqval, tmp_here, NODE_MAX);
            FWRITE_STR(SCHED_FOREGROUND_BOOST, acqval, strlen(acqval), rc);
            QLOGI("Updated %s with %s, return value %d", SCHED_FOREGROUND_BOOST, acqval, rc);
            delete[] tmp_here;
        } else {
            QLOGE("Failed to get the cpuset bitmask for requested value = %u", reqval);
            return FAILED;
        }
        break;

    case SCHED_START_INDEX + SCHEDTUNE_BOOST_OPCODE:
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case POWER_COLLAPSE_START_INDEX + LPM_BIAS_HYST_OPCODE:
        kernelMajor = tc.getKernelMajorVersion();
        kernelMinor = tc.getKernelMinorVersion();

        if (reqval <= 0 || reqval >= MAX_LPM_BIAS_HYST) {
            QLOGI("Requested value is %u out of limits, resetting to default bias of %d",
                                                        reqval, DEFAULT_LPM_BIAS_HYST);
            reqval = DEFAULT_LPM_BIAS_HYST;
        }

        if ((kernelMajor == 4 && kernelMinor >= 19) || kernelMajor > 4) {
            /* Value passed is in milliseconds; convert to nanoseconds */
            reqval = MultiplyReqval(reqval, 1000000);
        }
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    //todo: check this
    case INTERACTIVE_START_INDEX + INTERACTIVE_USE_SCHED_LOAD_OPCODE:
    //todo: need to ask power team to send a value instead of setting it to zero over here
    case INTERACTIVE_START_INDEX + INTERACTIVE_USE_MIGRATION_NOTIF_OPCODE:
    /* turn off ignore_hispeed_on_notify */
    case INTERACTIVE_START_INDEX + INTERACTIVE_IGNORE_HISPEED_NOTIF_OPCODE:
    /* Removes input boost during keypress scenarios */
    case MISC_START_INDEX + INPUT_BOOST_RESET_OPCODE:
        snprintf(acqval, NODE_MAX, "%d", 0);
        break;

    case INTERACTIVE_START_INDEX + SCHEDUTIL_HISPEED_FREQ_OPCODE:
    case INTERACTIVE_START_INDEX + INTERACTIVE_HISPEED_FREQ_OPCODE:
        if (reqval > 0 ) {
            reqval = MultiplyReqval(reqval, FREQ_MULTIPLICATION_FACTOR);
            reqval = d.find_next_cpu_frequency(r.core, reqval);
        }
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case ONDEMAND_START_INDEX + OND_SYNC_FREQ_OPCODE:
    case ONDEMAND_START_INDEX + OND_OPIMAL_FREQ_OPCODE:
        if (reqval > 0) {
            reqval = MultiplyReqval(reqval, FREQ_MULTIPLICATION_FACTOR);
            reqval = d.find_next_avail_freq(reqval);
        }
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case SCHED_START_INDEX + SCHED_PREFER_IDLE_OPCODE:
        snprintf(acqval, NODE_MAX, "%d", ENABLE_PREFER_IDLE);
        break;

    case CPUBW_HWMON_START_INDEX + CPUBW_HWMON_MAXFREQ_OPCODE:
        if (reqval > 0) {
            reqval = MultiplyReqval(reqval, 100);
            setval = d.find_next_cpubw_available_freq(reqval);
        }
        if (setval == FAILED) {
            QLOGE("Error! Perflock failed, invalid freq value %d", setval);
            return FAILED;
        }
        snprintf(acqval, NODE_MAX, "%d", setval);
        /* read hwmon min_freq */
        hwmon_min_freq_index = CPUBW_HWMON_START_INDEX + CPUBW_HWMON_MINFREQ_OPCODE;
        FREAD_STR(d.sysfsnode_path[hwmon_min_freq_index], tmp_node, NODE_MAX, rc);
        if (rc < 0) {
            QLOGE("Failed to read %s", d.sysfsnode_path[hwmon_min_freq_index]);
            return FAILED;
        }
        /*max_freq cannot be lower than min_freq, if reqval
        is lower than minfreq, request is denied*/
        valid_bound = minBoundCheck(d.sysfsnode_storage[idx], tmp_node, acqval, d.cpubw_avail_freqs[d.cpubw_avail_freqs_n-1]);
        if (!valid_bound) {
            QLOGE("Min bounds check failed");
            return FAILED;
        }
        break;

    /*new opcode for cpu-llcc-ddr-bw max_freq node*/
    case LLCBW_HWMON_START_INDEX + LLCC_DDR_BW_MAX_FREQ:
        if (reqval > 0) {
            reqval = MultiplyReqval(reqval, 100);
            setval = d.find_next_llccbw_available_freq(reqval);
        }
        if (setval == FAILED) {
            QLOGE("Error! Perflock failed, invalid freq value %d", setval);
            return FAILED;
        }
        snprintf(acqval, NODE_MAX, "%d", setval);
        /* read hwmon min_freq */
        hwmon_min_freq_index = LLCBW_HWMON_START_INDEX + LLCC_DDR_BW_MIN_FREQ;
        FREAD_STR(d.sysfsnode_path[hwmon_min_freq_index], tmp_node, NODE_MAX, rc);
        if (rc < 0) {
            QLOGE("Failed to read %s", d.sysfsnode_path[hwmon_min_freq_index]);
            return FAILED;
        }
        /*max_freq cannot be lower than min_freq, if reqval
        is lower than minfreq, request is denied*/
        valid_bound = minBoundCheck(d.sysfsnode_storage[idx], tmp_node, acqval, d.llcbw_avail_freqs[d.llcbw_avail_freqs_n-1]);
        if (!valid_bound) {
            QLOGE("Min bounds check failed");
            return FAILED;
        }
        break;

     /*new opcode for cpu-llcc-ddr-bw min_freq node*/
     case LLCBW_HWMON_START_INDEX + LLCC_DDR_BW_MIN_FREQ:
         if (reqval > 0) {
             reqval = MultiplyReqval(reqval, 100);
             tmp_here = get_devfreq_new_val(reqval, d.llcbw_avail_freqs, d.llcbw_avail_freqs_n, d.llcbw_maxfreq_path);
         }
         if (tmp_here != NULL) {
             strlcpy(acqval, tmp_here, NODE_MAX);
             free(tmp_here);
         } else {
             QLOGE("Unable to find the new devfreq_val for the requested value %u", reqval);
             return FAILED;
         }
         break;

     /*new opcode for cpu-cpu-llcc-bw min_freq node*/
     case CPUBW_HWMON_START_INDEX + CPU_LLCC_BW_MIN_FREQ:
         if (reqval > 0) {
             reqval = MultiplyReqval(reqval, 100);
             tmp_here = get_devfreq_new_val(reqval, d.cpubw_avail_freqs, d.cpubw_avail_freqs_n, d.cpubw_maxfreq_path);
         }
         if (tmp_here != NULL) {
             strlcpy(acqval, tmp_here, NODE_MAX);
             free(tmp_here);
         } else {
             QLOGE("Unable to find the new devfreq_val for the requested value %u", reqval);
             return FAILED;
         }
         break;

    case CPUBW_HWMON_START_INDEX + CPUBW_HWMON_MINFREQ_OPCODE:
        if (reqval > 0) {
            reqval = MultiplyReqval(reqval, 100);
            tmp_here = get_devfreq_new_val(reqval, d.cpubw_avail_freqs, d.cpubw_avail_freqs_n, d.cpubw_maxfreq_path);
        }
        if (tmp_here != NULL) {
            strlcpy(acqval, tmp_here, NODE_MAX);
            free(tmp_here);
        } else {
            QLOGE("Unable to find the new devfreq_val for the requested value %u", reqval);
            return FAILED;
        }
        break;

    case INTERACTIVE_START_INDEX + INTERACTIVE_ABOVE_HISPEED_DELAY_OPCODE:
        if (reqval == 0xFE) {
            snprintf(acqval, NODE_MAX, "19000 1400000:39000 1700000:19000");
        } else if (reqval == 0xFD) {
            snprintf(acqval, NODE_MAX, "%d", 19000);
        } else {
            reqval = MultiplyReqval(reqval, 10000);
            snprintf(acqval, NODE_MAX, "%d", reqval);
        }
        break;

    case INTERACTIVE_START_INDEX + INTERACTIVE_TARGET_LOADS_OPCODE:
        if (reqval == 0xFE) {
            snprintf(acqval, NODE_MAX, "85 1500000:90 1800000:70");
        } else {
            snprintf(acqval, NODE_MAX, "%d", reqval);
        }
        break;

    case SCHED_START_INDEX + SCHED_MOSTLY_IDLE_FREQ_OPCODE:
        if (reqval > 0) {
            reqval = MultiplyReqval(reqval, FREQ_MULTIPLICATION_FACTOR);
            if ((cpu = get_online_core(0, target.getLastCoreIndex(0)))!= FAILED) {
                 valCluster1 =  d.find_next_cpu_frequency(cpu, reqval);
                QLOGI("valCluster1=%d", valCluster1);
            }
            if ((cpu = get_online_core(target.getLastCoreIndex(0)+1, target.getLastCoreIndex(1))) != FAILED) {
                 valCluster2 =  d.find_next_cpu_frequency(cpu, reqval);
                QLOGI("valCluster2=%d", valCluster2);
            }
        }

        if (d.node_type[idx] == UPDATE_CORES_PER_CLUSTER) {
            snprintf(acqval, NODE_MAX, "%d,%d", valCluster1, valCluster2);
        } else {
            snprintf(acqval, NODE_MAX, "%d", valCluster1);
        }
        break;

    case INTERACTIVE_START_INDEX + INTERACTIVE_BOOSTPULSE_DURATION_OPCODE:
    case INTERACTIVE_START_INDEX + INTERACTIVE_MIN_SAMPLE_TIME_OPCODE:
    case INTERACTIVE_START_INDEX + INTERACTIVE_TIMER_RATE_OPCODE:
    case INTERACTIVE_START_INDEX + INTERACTIVE_TIMER_SLACK_OPCODE:
    case INTERACTIVE_START_INDEX + INTERACTIVE_MAX_FREQ_HYSTERESIS_OPCODE:
    case ONDEMAND_START_INDEX + OND_SAMPLING_RATE_OPCODE: // 0xff - (data & 0xff); //todo
        if (reqval > 0) {
            reqval = MultiplyReqval(reqval, TIMER_MULTIPLICATION_FACTOR);
        }
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case GPU_START_INDEX + GPU_MIN_FREQ_OPCODE:
    case GPU_START_INDEX + GPU_MAX_FREQ_OPCODE:
        /* find the nearest >= available freq lvl, after multiplying
        * input value with 1000000 */
        reqval = MultiplyReqval(reqval, 1000000);
        reqval = target.findNextGpuFreq(reqval);
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case GPU_START_INDEX + GPU_BUS_MIN_FREQ_OPCODE:
    case GPU_START_INDEX + GPU_BUS_MAX_FREQ_OPCODE:
        /* find the nearest >= available freq lvl */
        reqval = target.findNextGpuBusFreq(reqval);
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case ONDEMAND_START_INDEX + NOTIFY_ON_MIGRATE:
        if (reqval != 1) {
            reqval = 0;
        }
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case MISC_START_INDEX + STORAGE_CLK_SCALING_DISABLE_OPCODE:
        if (reqval == 1) {
            reqval = 0;
        } else {
            reqval = 1;
        }
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;

    case LLCBW_HWMON_START_INDEX + LLCBW_HWMON_MINFREQ_OPCODE:
        if (reqval > 0) {
            reqval = MultiplyReqval(reqval, 100);
            tmp_here = get_devfreq_new_val(reqval, d.llcbw_avail_freqs, d.llcbw_avail_freqs_n, d.llcbw_maxfreq_path);
        }
        if (tmp_here != NULL) {
            strlcpy(acqval, tmp_here, NODE_MAX);
            free(tmp_here);
        } else {
            QLOGE("Unable to find the new devfreq_val for the requested value %u", reqval);
            return FAILED;
        }
        break;

    default:
        snprintf(acqval, NODE_MAX, "%d", reqval);
        break;
    }

    return SUCCESS;
}

/*Based on resource type, we select which all nodes to be updated. Default case
is to update the given single node with value node_strg.*/
int OptsHandler::update_node_param(int node_type, const char node[NODE_MAX],
                                            char node_strg[NODE_MAX], int node_strg_l) {
    int rc = -1, i = 0;
    char tmp_node[NODE_MAX] = "", tmp_s[NODE_MAX] = "";
    char *pch = NULL;
    Target &target = Target::getCurTarget();
    TargetConfig &tc = TargetConfig::getTargetConfig();
    unsigned int valCluster1 = 0, valCluster2 = 0;

    switch (node_type) {
    case UPDATE_ALL_CORES:
        /* In current implementation we are considering all cores should have same
        value. If this assumption changes, code needs to be updated.*/
        strlcpy(tmp_node, node, NODE_MAX);
        for (i = 0; i < tc.getTotalNumCores(); i++) {
            tmp_node[CPU_INDEX] = i + '0';
            rc = change_node_val(tmp_node, node_strg, node_strg_l);
        }
        break;

    case UPDATE_CORES_PER_CLUSTER:
        /* During acquire call, cores of diffrent cluster are changed with different value.
        and on release all the cores are reset to previously stored value from core 0. */
        valCluster1 = strtol(node_strg, NULL, 0);
        pch = strchr(node_strg, ',');
        if(pch != NULL) {
            valCluster2 = strtol(pch+1, NULL, 0);
        } else {
            valCluster2 = valCluster1;
        }

        strlcpy(tmp_node, node, NODE_MAX);
        snprintf(tmp_s, NODE_MAX, "%d", valCluster1);
        for (i = 0; i <= target.getLastCoreIndex(0); i++) {
            tmp_node[CPU_INDEX] = i + '0';
            rc = change_node_val(tmp_node, tmp_s, strlen(tmp_s));
        }
        snprintf(tmp_s, NODE_MAX, "%d", valCluster2);
        for (i = target.getLastCoreIndex(0)+1; i <= target.getLastCoreIndex(1); i++) {
            tmp_node[CPU_INDEX] = i + '0';
            rc = change_node_val(tmp_node, tmp_s, strlen(tmp_s));
        }
        break;

    default:
        rc = change_node_val(node, node_strg, node_strg_l);
        break;
    }
    return rc;
}

/* Wrapper functions for function pointers */
int OptsHandler::dummy(Resource &r,  OptsData &d) {
    return 0;
}

/* Unsupported resource opcode*/
int OptsHandler::unsupported(Resource &r, OptsData &d) {
    QLOGE("Error: This resource is not supported");
    return FAILED;
}

/*For each request, we first validate the cores and clusters and then basing on resource type,
we get the node storage pointers for that resource. Also necessary changes are made to node
path basing on core and cluster of request.*/
int OptsHandler::GetNodeStorageLink(Resource &r, OptsData &d, char **node_storage, int **node_storage_length) {
    int idx = r.qindex, rc = FAILED;
    unsigned char minor = r.minor;
    int cpu = -1, core = r.core;
    Target &t = Target::getCurTarget();
    TargetConfig &tc = TargetConfig::getTargetConfig();

    switch(d.node_type[idx]) {
    case SELECT_CORE_TO_UPDATE:
    /* we find the node storage paths basing on the requested core.*/
        rc = ValidateClusterAndCore(r.cluster, r.core, ASYNC_CORE);
        if (rc == FAILED) {
            QLOGE("Request on invalid core or cluster");
            return FAILED;
        }
        d.sysfsnode_path[idx][CPU_INDEX] = core + '0';
        if (idx == (SCHED_START_INDEX + SCHED_STATIC_CPU_PWR_COST_OPCODE)) {
            *node_storage = d.sch_cpu_pwr_cost_s[core];
            *node_storage_length = &d.sch_cpu_pwr_cost_sl[core];
        }
        else if (idx == (SCHED_START_INDEX + SCHED_LOAD_BOOST_OPCODE)) {
            *node_storage = d.sch_load_boost_s[core];
            *node_storage_length = &d.sch_load_boost_sl[core];
        }
        break;

    case INTERACTIVE_NODE:
    /*If target's govinstance type is of cluster based, then we get the node storage
      paths of first online cpu in requested cluster and returns failed if all cores in
      that cluster are offline. Default case is to return the cluster 0 node storage paths.*/
        if (CLUSTER_BASED_GOV_INSTANCE == tc.getGovInstanceType()) {
            rc = ValidateClusterAndCore(r.cluster, r.core, SYNC_CORE);
            if (rc == FAILED) {
                QLOGE("Request on invalid core or cluster");
                return FAILED;
            }

            if (CLUSTER0 == r.cluster) {
                if ((cpu = get_online_core(0, t.getLastCoreIndex(0))) != FAILED) {
                    d.sysfsnode_path[idx][CPU_INDEX] = cpu + '0';
                    *node_storage = d.cluster0_interactive_node_storage[minor];
                    *node_storage_length = &d.cluster0_interactive_node_storage_length[minor];
                } else {
                    QLOGE("Error! No core is online for cluster %d", r.cluster);
                    return FAILED;
                }
            } else {
                if ((cpu = get_online_core(t.getFirstCoreIndex(r.cluster), t.getLastCoreIndex(r.cluster))) != FAILED) {
                    d.sysfsnode_path[idx][CPU_INDEX] = cpu + '0';
                    if (r.cluster == 1) {
                      *node_storage = d.cluster1_interactive_node_storage[minor];
                      *node_storage_length = &d.cluster1_interactive_node_storage_length[minor];
                    } else if (r.cluster == 2) {
                      *node_storage = d.cluster2_interactive_node_storage[minor];
                      *node_storage_length = &d.cluster2_interactive_node_storage_length[minor];
                    }
                } else {
                    QLOGE("Error! No core is online for cluster %d", r.cluster);
                    return FAILED;
                }
            }
        } else {
            rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
            if (rc == FAILED) {
                QLOGE("Request on invalid core or cluster");
                return FAILED;
            }
            *node_storage = d.cluster0_interactive_node_storage[minor];
            *node_storage_length = &d.cluster0_interactive_node_storage_length[minor];
        }
        break;

        case MEM_LAT_NODE:

        if (CLUSTER_BASED_GOV_INSTANCE == tc.getGovInstanceType()) {
            rc = ValidateClusterAndCore(r.cluster, r.core, SYNC_CORE);
            if (rc == FAILED) {
                QLOGE("Request on invalid core or cluster of mem_lat");
                return FAILED;
            }

            int MEM_CPU_INDEX = ((string)d.sysfsnode_path[idx]).find_first_of("0123456789");

            if (CLUSTER0 == r.cluster) {
               d.sysfsnode_path[idx][MEM_CPU_INDEX] = '0';
               *node_storage = d.memlat_minfreq_node_strg[r.cluster][minor];
               *node_storage_length = &d.memlat_minfreq_node_strg_len[r.cluster][minor];
            } else {
                cpu = t.getLastCoreIndex(r.cluster-1)+1;
                d.sysfsnode_path[idx][MEM_CPU_INDEX] = cpu + '0';
                *node_storage = d.memlat_minfreq_node_strg[r.cluster][minor];
                *node_storage_length = &d.memlat_minfreq_node_strg_len[r.cluster][minor];
            }
        } else {
               rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
               if (rc == FAILED) {
                   QLOGE("Request on invalid core or cluster");
                   return FAILED;
                }
                *node_storage = d.memlat_minfreq_node_strg[CLUSTER0][minor];
                *node_storage_length = &d.memlat_minfreq_node_strg_len[CLUSTER0][minor];
        }
    break;

    default:
        rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
        if (rc == FAILED) {
            QLOGE("Request on invalid core or cluster");
            return FAILED;
        }

        *node_storage = d.sysfsnode_storage[idx];
        *node_storage_length = &d.sysfsnode_storage_length[idx];
        break;
    }

    return SUCCESS;
}

/*An optional function, which is called after every acquire/release call for
handling all requests post node updations.*/
void OptsHandler::CustomizePostNodeUpdation(Resource &r, OptsData &d, int perflock_call,
                                            char node_strg[NODE_MAX], int node_strg_l) {
    int idx = r.qindex, rc;
    if (perflock_call == ACQUIRE_LOCK) {
        switch(idx) {
        case SCHED_START_INDEX + SCHED_SET_FREQ_AGGR_OPCODE:
            d.mSchedFreqAggrGroupData.checkSchedFreqAggrPresence();
            break;
        }
    } else if (perflock_call == RELEASE_LOCK) {
        switch(idx) {
        case SCHED_START_INDEX + SCHED_CPUSET_FOREGROUND_OPCODE:
            /* Only for foreground cpuset: restore foreground/boost/cpus
            after restoring foreground/cpus with all cpus bitmask */
            FWRITE_STR(SCHED_FOREGROUND_BOOST, node_strg, node_strg_l, rc);
            QLOGI("Updated %s with %s, return value %d", SCHED_FOREGROUND_BOOST, node_strg, rc);
            break;

        case SCHED_START_INDEX + SCHED_SET_FREQ_AGGR_OPCODE:
            d.mSchedFreqAggrGroupData.checkSchedFreqAggrPresence();
            break;
        }
    }
}

/*A common function to handle all the generic perflock calls.*/
int OptsHandler::modify_sysfsnode(Resource &r, OptsData &d) {
    int idx = r.qindex, rc = FAILED;
    char tmp_s[NODE_MAX]= "";
    unsigned int reqval = r.value;
    TargetConfig &tc = TargetConfig::getTargetConfig();
    char *node_storage = NULL;
    int *node_storage_length = NULL;

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource %s not supported", d.sysfsnode_path[idx]);
        return FAILED;
    }

    /*The first step for any request is to validate it and then get the Node storage paths.
    For acqiure call these node storage paths are used to store the current value of nodes
    and for release call the nodes are updated with values in these storage paths.*/
    rc = GetNodeStorageLink(r, d, &node_storage, &node_storage_length);

    /*Release call, release happens only if we have any previously stored value.
      For only interactive type nodes, on release failure we wait on poll_thread.*/
    if (reqval == MAX_LVL) {
        QLOGI("Perflock release call for resource index = %d, path = %s, from function = %s",
                                                        idx, d.sysfsnode_path[idx], __func__);

        if (rc != FAILED && *node_storage_length > 0) {
            rc = update_node_param(d.node_type[idx], d.sysfsnode_path[idx], node_storage, *node_storage_length);
            CustomizePostNodeUpdation(r, d, RELEASE_LOCK, node_storage, *node_storage_length);
            *node_storage_length = -1;
        } else if (rc == FAILED) {
            QLOGE("Unable to find the correct node storage pointers for resource index=%d, node path=%s",
                                                        idx, d.sysfsnode_path[idx]);
        } else {
            QLOGE("perf_lock_rel: failed for %s as previous value is not stored", d.sysfsnode_path[idx]);
        }

        if(d.node_type[idx] == INTERACTIVE_NODE) {
            if (rc < 0 && CLUSTER_BASED_GOV_INSTANCE == tc.getGovInstanceType()) {
                signal_chk_poll_thread(d.sysfsnode_path[idx], rc);
            }
        }
        return rc;
    }

    /*steps followed for Acquire call
      1. Ensuring that we got the node storage pointers correctly.
      2. Storing the previous node value, only if it is not already stored.
      3. Customizing the requested value.
      4. Updating all the required nodes basing on the opcode type.*/

    QLOGI("Perflock Acquire call for resource index = %d, path = %s, from function = %s",
                                                        idx, d.sysfsnode_path[idx], __func__);
    QLOGI("Requested value = %d", reqval);
    if (rc == FAILED) {
        QLOGE("Unable to find the correct node storage pointers for resource index=%d, node path=%s",
                                                        idx, d.sysfsnode_path[idx]);
        return FAILED;
    }

    if (*node_storage_length <= 0) {
        *node_storage_length = save_node_val(d.sysfsnode_path[idx], node_storage);
        if (*node_storage_length <= 0) {
            QLOGE("Failed to read %s", d.sysfsnode_path[idx]);
            return FAILED;
        }
    }

    rc = CustomizeRequestValue(r, d, tmp_s);
    if (rc == SUCCESS) {
        QLOGI("After customizing, the request reqval=%s", tmp_s);
        rc = update_node_param(d.node_type[idx], d.sysfsnode_path[idx], tmp_s, strlen(tmp_s));
        CustomizePostNodeUpdation(r, d, ACQUIRE_LOCK, tmp_s, strlen(tmp_s));
        return rc;
    } else {
        QLOGE("Error! Perflock failed, invalid request value %d", reqval);
        return FAILED;
    }
}

int OptsHandler::pmQoS_cpu_dma_latency(Resource &r, OptsData &d) {
    int rc = FAILED;
    int idx = r.qindex;
    static char pmqos_cpu_dma_s[NODE_MAX];// pm qos cpu dma latency string
    unsigned int latency = r.value;
    static int fd = FAILED;
    unsigned int min_pmqos_latency = 0;
    Target &t = Target::getCurTarget();

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("%s:Request on invalid core or cluster", __func__);
        return FAILED;
    }
    /*Set minimum latency(floor) to WFI for the target.
     * Clients thus do not need to explicitly mention the WFI latency
     * & continue to use the existing value of 0x1 to disable PC and use WFI.
     * If a client gives an explicit value > WFI, then
     * client knows what it is doing and hence honor.
     */
     min_pmqos_latency = t.getMinPmQosLatency();
     if (latency < min_pmqos_latency) {
         QLOGI("%s:Requested Latency=0x%x", __func__, latency);
         QLOGI("%s:Target Min WFI Latency is =0x%x, setting same", __func__,min_pmqos_latency);
         latency = min_pmqos_latency;
     } else {
         QLOGI("%s:Client latency=0x%x > trgt min latency=0x%x", __func__, latency,min_pmqos_latency);
         QLOGI("No Override with min latency for target");
     }
    /*If there is a lock acquired, then we close, to release
     *that fd and open with new value.Doing this way
     * in this case actually handles concurrency.
     */
    if (fd >= 0) { //close the device node that was opened.
        rc = close(fd);
        fd = FAILED;
        QLOGI("%s:Released the PMQos Lock and dev node closed, rc=%d", __func__, rc);
    }

    /*Check if Perf lock request to acquire(!= MAX_LVL)*/
    if (latency != MAX_LVL) {
        //FWRITE_STR not used since it closes the fd. This shouldnt be done in this case
        ///kernel/msm-4.4/Documentation/power/pm_qos_interface.txt#95
        fd = open(d.sysfsnode_path[idx], O_WRONLY);
        if (fd >= 0) {
           snprintf(pmqos_cpu_dma_s, NODE_MAX, "%x", latency);
           rc = write(fd, pmqos_cpu_dma_s, strlen(pmqos_cpu_dma_s));// Write the CPU DMA Latency value
           if (rc < 0) {
               QLOGE("%s:Writing to the PM QoS node failed=%d", __func__, rc);
               close(fd);
               fd = FAILED;
           } else {
               //To remove the user mode request for a target value simply close the device node
               QLOGI("%s:Sucessfully applied PMQos Lock for fd=%d, latency=0x%s, rc=%d", __func__, fd, pmqos_cpu_dma_s, rc);
           }
       } else {
           rc = FAILED;
           QLOGE("%s:Failed to Open PMQos Lock for fd=%d, latency=0x%s, rc=%d", __func__, fd, pmqos_cpu_dma_s, rc);
       }
    }
    return rc;
}

/* CPU options */
int OptsHandler::cpu_options(Resource &r,  OptsData &d) {
    int rc = -1;
    int i = 0;
    int idx = r.qindex;
    char fmtStr[NODE_MAX];
    unsigned int reqval  = r.value;
    unsigned int valToUpdate;
    char nodeToUpdate[NODE_MAX];
    int min_freq = (CPUFREQ_MIN_FREQ_OPCODE == r.minor) ? 1 : 0;
    int cpu = -1;
    static bool is_sync_cores; //Updating a single core freq, updates all cores freq.
    Target &t = Target::getCurTarget();
    TargetConfig &tc = TargetConfig::getTargetConfig();
    int startCpu, endCpu, cluster, coresInCluster;

    is_sync_cores = tc.isSyncCore();
    cpu = r.core;
    if (r.cluster >= tc.getNumCluster()) {
        QLOGI("Invalid Cluster id=%d", r.cluster);
        return FAILED;
    }
    /* For sync_cores, where changing frequency for one core changes, ignore the requests
     * for resources other than for core0 and core4. This is to ensure that we may not end
     * up loosing concurrency support for cpufreq perflocks.
     */

    cluster = t.getClusterForCpu(cpu, startCpu, endCpu);
    if ((startCpu < 0) || (endCpu < 0) || (cluster < 0)) {
        QLOGE("Could not find a cluster corresponding the core %d", cpu);
        return FAILED;
    }
    if (is_sync_cores && (cpu > startCpu && cpu <= endCpu)) {
        QLOGW("Warning: Resource [%d, %d] not supported for core %d. Instead use resource for core %d", r.major, r.minor,cpu,startCpu);
        return FAILED;
    }
    /* Calculate the value that needs to be updated to KPM.
     * If lock is being released (reqVal == 0) then
     * update with 0 (for min_freq) and cpu max reset value (for max_freq)
     * If lock is being acquired then reqVal is multiplied
     * with 100000 and updated by finding next closest frequency.
     * */
    if (reqval == MAX_LVL) {
        QLOGI("Releasing the node %s", d.sysfsnode_path[idx]);
        valToUpdate = 0;
        strlcpy(nodeToUpdate, d.sysfsnode_path[idx], strlen(d.sysfsnode_path[idx])+1);
        if (!min_freq){
            valToUpdate = tc.getCpuMaxFreqResetVal(cluster);
        }
    } else {
        reqval = MultiplyReqval(reqval, FREQ_MULTIPLICATION_FACTOR);
        valToUpdate = d.find_next_cpu_frequency(r.core, reqval);
        strlcpy(nodeToUpdate, d.sysfsnode_path[idx], strlen(d.sysfsnode_path[idx])+1);
    }
    QLOGI("Freq value to be updated %d", valToUpdate);

    /* Construct the formatted string with which KPM node is updated
     */
    coresInCluster = tc.getCoresInCluster(cluster);
    if (coresInCluster < 0){
        QLOGE("Cores in a cluster cannot be %d", coresInCluster);
        return FAILED;
    }

    if (cluster == 0) {
        rc = update_freq_node(0, t.getLastCoreIndex(0), valToUpdate, fmtStr, NODE_MAX);
        if (rc == FAILED) {
            QLOGE("Perflock failed, frequency format string is not proper");
            return FAILED;
        }
    } else if (cluster < MAX_CLUSTER) {
        rc = update_freq_node(t.getLastCoreIndex(cluster - 1)+1, t.getLastCoreIndex(cluster), valToUpdate, fmtStr, NODE_MAX);
        if (rc == FAILED) {
            QLOGE("Peflock failed, frequency format string is not proper");
            return FAILED;
        }
    } else {
        QLOGE("Cluster Id %d not supported\n",cluster);
        return FAILED;
    }

    /* Finally update the KPM Node. The cluster based node is updated in
     * fmtStr, and KPM node is available in nodeToUpdate.
     * */
     FWRITE_STR(nodeToUpdate, fmtStr, strlen(fmtStr), rc);
     QLOGI("Updated %s with %s return value %d",nodeToUpdate , fmtStr, rc );
     return rc;
}

/* Set sched boost */
int OptsHandler::set_sched_boost(Resource &r, OptsData &d) {
    int rc = -1;
    int idx = r.qindex;
    char tmp_s[NODE_MAX];
    unsigned int reqval = r.value;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.sysfsnode_path[idx], tmp_s, rc);
    return rc;
}

/* Reset sched boost */
int OptsHandler::reset_sched_boost(Resource &r, OptsData &d) {
    char tmp_s[NODE_MAX];
    int reqval = r.value;
    int idx = r.qindex;
    int rc = -1;
    int kernelMajor = 0;
    int kernelMinor = 0;
    TargetConfig &tc = TargetConfig::getTargetConfig();

    kernelMajor = tc.getKernelMajorVersion();
    kernelMinor = tc.getKernelMinorVersion();

    if ((kernelMajor <= 4 && kernelMinor <= 9)) {
        reqval = 0;
    } else {
        reqval = -reqval;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_rel: updated %s with %s return value %d", d.sysfsnode_path[idx], tmp_s, rc);
    return rc;
}

static thread_group get_threads_of_interest_for_sched_group(int tid) {
    char name[NODE_MAX], data[NODE_MAX];
    int rc, *tids = new(std::nothrow) int[HWUI_SCHED_GROUP_SIZE], num_tids = 0;
    thread_group t;
    t.tids = NULL;
    t.num_tids = 0;

    if (tids != nullptr)
        memset(tids, 0, HWUI_SCHED_GROUP_SIZE * sizeof(int));
     else
         return t;

    memset(data, 0, NODE_MAX);
    memset(name, 0, NODE_MAX);
    snprintf(name, NODE_MAX, "/proc/%d/task", tid);
    DIR* proc_dir = opendir(name);
    if (proc_dir) {
        tids[0] = tid;
        num_tids++;
        struct dirent *ent;
        while (((ent = readdir(proc_dir)) != NULL) && (num_tids < 5)) {
            if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
                memset(name, 0, NODE_MAX);
                snprintf(name, NODE_MAX, "/proc/%s/status", ent->d_name);
                memset(data, 0, NODE_MAX);
                FREAD_STR(name, data, NODE_MAX - 1, rc)
                if (sched_group_data::isHWUIThread(data)) {
                    tids[num_tids] = atoi(ent->d_name);
                    num_tids++;
                }
            }
        }
        closedir(proc_dir);
    }
    /*
     * Apply sched group only for HWUI Threads.
     */
    if (!num_tids ||
               (num_tids < (HWUI_SCHED_GROUP_SIZE - 1))) {
        num_tids = 0;
        delete[] tids;
        tids = NULL;
        QLOGI("sched_group NOT Found");
    }

    t.tids = tids;
    t.num_tids = num_tids;
    return t;
}

static void write_sched_group(thread_group t, int enable) {
    if (!t.num_tids)
        return;
    char value[NODE_MAX];
    memset(value, 0, NODE_MAX);
    if (enable) {
        snprintf(value, NODE_MAX, "%d", t.tids[0]);
    } else
        value[0] = '0';
    int len = strlen(value), rc;

    for (int i = 0; i < t.num_tids; i++) {
        if (t.tids[i] != 0) {
            char file_name[NODE_MAX];
            memset(file_name, 0, NODE_MAX);
            snprintf(file_name, NODE_MAX, "/proc/%d/sched_group_id",
                                                             t.tids[i]);
            FWRITE_STR(file_name, value, len, rc);
            QLOGI("sched_group_id for tid = %d is %s", t.tids[i], value);
        }
    }
}

/* Add sched group */
int OptsHandler::add_sched_group(Resource &r, OptsData &d) {
    int rc = 0;
    char *bptr;
    char value[NODE_MAX];

    if (d.mSchedGroupData.Found(r.value))
        return rc;

    thread_group t = get_threads_of_interest_for_sched_group(r.value);
    d.mSchedGroupData.Add(r.value, t);
    write_sched_group(t, 1);
    return rc;
}

/* Reset sched group */
int OptsHandler::reset_sched_group(Resource &r, OptsData &d) {
    int rc = 0;
    char *bptr;
    char value[NODE_MAX];

    if (!d.mSchedGroupData.Found(r.value))
        return rc;

    thread_group t = d.mSchedGroupData.Get(r.value);
    if (t.tids) {
        write_sched_group(t, 0);
        delete[] t.tids;
        t.tids = 0;
        t.num_tids = 0;
    }
    d.mSchedGroupData.Remove(r.value);
    return rc;
}



int OptsHandler::write_sched_freq_aggr_group(int tid, bool enable) {
    DIR *dir;
    struct dirent *readDir;
    char file_name[NODE_MAX], dir_name[NODE_MAX], pid_str[NODE_MAX];
    int fd, rc=-1,rc1=-1;

    memset(dir_name, 0, NODE_MAX);
    snprintf(dir_name, NODE_MAX, "/proc/%d/task", tid);

    memset(pid_str, 0, NODE_MAX);
    if (enable) {
        snprintf(pid_str, NODE_MAX, "%d", tid);
    } else {
        pid_str[0] = '0';
    }

    /*find all tasks related to passed TID and set sched_group_id*/
    if((dir = opendir(dir_name))) {
        while((readDir = readdir(dir))) {
            memset(file_name, 0, NODE_MAX);
            snprintf(file_name, NODE_MAX, "/proc/%s/sched_group_id", readDir->d_name);
            fd = open(file_name, O_WRONLY);
            if (fd >= 0) {
                rc1 = write(fd, pid_str, strlen(pid_str));// set sgid
                close(fd);
            }
        } //while(readDir)
        closedir(dir);
    } else { //if(opendir)
        QLOGE("%s, opendir() failed on /proc/%d/task", __FUNCTION__, tid);
        rc=-1;
        return rc;
    }
    /*Scheduler group_id inheritance feature ensures setting of sched_group_id for any
      new child thread getting created after sched_group_id is set for the App process.
      Need to check if there exists any window where above while loop may miss settting
      sched_group_id for any child thread created during above mentioned boundary*/
    rc=0;
    return rc;
}

/* Add sched freq aggr group */
int OptsHandler::sched_add_freq_aggr_group(Resource &r, OptsData &d) {
    int rc = 0, instanceRefCount=1;
    bool enableFA = true;

    QLOGI("%s, pid=%d",__FUNCTION__, r.value);

    if (d.mSchedFreqAggrGroupData.Found(r.value)) {
        /*tid entry already exists, increment instanceRefCount and exit
          as Frequency Aggregation is already enabled on tid*/
        QLOGI("%s, tid %d already in hastable", __FUNCTION__, r.value);
        d.mSchedFreqAggrGroupData.IncrementInsRefCntVal(r.value);
        return rc;
    }
    /*Add instanceRefCount as 1 using tid as key*/
    d.mSchedFreqAggrGroupData.Add(r.value, instanceRefCount);
    write_sched_freq_aggr_group(r.value, enableFA);
    return rc;
}

char *OptsHandler::cpuset_bitmask_to_str(unsigned int bitmask) {
    int i = 0;
    int str_index = 0;
    char *cpuset_str = new(std::nothrow) char[CPUSET_STR_SIZE];
    if(cpuset_str != NULL) {
        memset(cpuset_str, '\0', CPUSET_STR_SIZE);
        while(bitmask != 0) {
            if(bitmask & 1 && str_index < CPUSET_STR_SIZE - 1) {
                int chars_copied = snprintf(cpuset_str + str_index, CPUSET_STR_SIZE - str_index, "%d,", i);
                str_index += chars_copied;
            }
            bitmask = bitmask >> 1;
            i++;
        }
        if(str_index > 0) {
            cpuset_str[str_index-1] = '\0';
        }
    }
    return cpuset_str;
}

/* Reset sched freq aggr group */
int OptsHandler::sched_reset_freq_aggr_group(Resource &r, OptsData &d) {
    int rc = 0, instanceRefCount;
    bool disableFA = false;

    QLOGI("%s, pid=%d",__FUNCTION__, r.value);

    if (!d.mSchedFreqAggrGroupData.Found(r.value)) {
        QLOGI("%s, tid %d not found in hastable", __FUNCTION__, r.value);
        return rc;
    }

    if ((instanceRefCount = d.mSchedFreqAggrGroupData.Get(r.value)) == 1) {
        write_sched_freq_aggr_group(r.value, disableFA);
        d.mSchedFreqAggrGroupData.Remove(r.value);
    } else if (instanceRefCount > 1) {
        /*more reference to tid entry exists, decrement instanceRefCount
          and exit*/
        d.mSchedFreqAggrGroupData.DecrementInsRefCntVal(r.value);
    }
    return rc;
}

int OptsHandler::l3_min_freq(Resource &r,  OptsData &d) {
    int rc = FAILED;
    char* setval_Cluster[MAX_CLUSTER] = {NULL};
    unsigned int reqval = r.value;
    static unsigned int stored_l3_min_freq = 0;
    int count = 0;
    Target &t = Target::getCurTarget();
    TargetConfig &tc =  TargetConfig::getTargetConfig();

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if(rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }
    if(reqval == MAX_LVL) {

        for(int j = 0; j < tc.getNumCluster(); j++) {
            if(d.l3_minf_sl[j] > 0) {
                FWRITE_STR(d.l3_minfreq_path[j], d.l3_minf_s[j], d.l3_minf_sl[j], rc);
                QLOGI("perf_lock_rel_l3Cluster%d: updated %s with %s return value %d",j, d.l3_minfreq_path, d.l3_minf_s[j], rc);
            }
        }
        stored_l3_min_freq = 0;
        return rc;
    }

    if(reqval > 0) {
        reqval = MultiplyReqval(reqval, 100000);
        for(int j = 0; j < tc.getNumCluster(); j++) {
            setval_Cluster[j] = get_devfreq_new_val(reqval, d.l3_avail_freqs[j], d.l3_avail_freqs_n[j], d.l3_maxfreq_path[j]);
            if(!stored_l3_min_freq) {
                d.l3_minf_sl[j] = save_node_val(d.l3_minfreq_path[j], d.l3_minf_s[j]);
                if(d.l3_minf_sl[j] > 0)
                    count++;
            }
            if(setval_Cluster[j] != NULL) {
                FWRITE_STR(d.l3_minfreq_path[j], setval_Cluster[j], strlen(setval_Cluster[j]), rc);
                QLOGI("perf_lock_acq_l3Cluster%d: updated %s with %s return value %d",j, d.l3_minfreq_path[j], setval_Cluster[j], rc);
                free(setval_Cluster[j]);
            }
        }
    }


    if(count == tc.getNumCluster())
        stored_l3_min_freq = 1;

    return rc;
}

int OptsHandler::l3_max_freq(Resource &r,  OptsData &d) {
    int rc = FAILED;
    char* setval_Cluster[MAX_CLUSTER] = {NULL};
    unsigned int reqval = r.value;
    static unsigned int stored_l3_max_freq = 0;
    int count = 0;
    Target &t = Target::getCurTarget();
    TargetConfig &tc =  TargetConfig::getTargetConfig();

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if(rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }
    if(reqval == MAX_LVL) {

        for(int j = 0; j < tc.getNumCluster(); j++) {
            if(d.l3_maxf_sl[j] > 0) {
                FWRITE_STR(d.l3_maxfreq_path[j], d.l3_maxf_s[j], d.l3_maxf_sl[j], rc);
                QLOGI("perf_lock_rel_l3Cluster%d: updated %s with %s return value %d",j, d.l3_maxfreq_path, d.l3_maxf_s[j], rc);
            }
        }
        stored_l3_max_freq = 0;
        return rc;
    }

    if(reqval > 0) {
        reqval = MultiplyReqval(reqval, 100000);
        for(int j = 0; j < tc.getNumCluster(); j++) {
            setval_Cluster[j] = get_devfreq_new_val(reqval, d.l3_avail_freqs[j], d.l3_avail_freqs_n[j], d.l3_maxfreq_path[j]);
            if(!stored_l3_max_freq) {
                d.l3_maxf_sl[j] = save_node_val(d.l3_maxfreq_path[j], d.l3_maxf_s[j]);
                if(d.l3_maxf_sl[j] > 0)
                    count++;
            }
            if(setval_Cluster[j] != NULL) {
                FWRITE_STR(d.l3_maxfreq_path[j], setval_Cluster[j], strlen(setval_Cluster[j]), rc);
                QLOGI("perf_lock_acq_l3Cluster%d: updated %s with %s return value %d",j, d.l3_maxfreq_path[j], setval_Cluster[j], rc);
                free(setval_Cluster[j]);
            }
        }
    }


    if(count == tc.getNumCluster())
        stored_l3_max_freq = 1;

    return rc;
}


int OptsHandler::npubw_hwmon_hyst_opt(Resource &r, OptsData &d) {
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_hyst_opt = 0;
    unsigned int reqval = r.value;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if (reqval == MAX_LVL) {
        if (d.npubw_hc_sl > 0) {
           FWRITE_STR(d.npubw_hwmon_hyst_count_path, d.npubw_hc_s, d.npubw_hc_sl, rc);
           if (rc < 0) {
               QLOGE("Failed to write %s", d.npubw_hwmon_hyst_count_path);
               return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.npubw_hwmon_hyst_count_path, d.npubw_hc_s, rc);
        }
        if (d.npubw_hm_sl > 0) {
            FWRITE_STR(d.npubw_hwmon_hist_memory_path, d.npubw_hm_s, d.npubw_hm_sl, rc);
            if (rc < 0) {
               QLOGE("Failed to write %s", d.npubw_hwmon_hist_memory_path);
               return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.npubw_hwmon_hist_memory_path, d.npubw_hm_s, rc);
        }
        if (d.npubw_hl_sl > 0) {
            FWRITE_STR(d.npubw_hwmon_hyst_length_path, d.npubw_hl_s, d.npubw_hl_sl, rc);
            if (rc < 0) {
               QLOGE("Failed to write %s", d.npubw_hwmon_hyst_length_path);
               return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.npubw_hwmon_hyst_length_path, d.npubw_hl_s, rc);
        }
        stored_hyst_opt = 0;

        return rc;
    }

    if (!stored_hyst_opt) {
        FREAD_STR(d.npubw_hwmon_hyst_count_path, d.npubw_hc_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.npubw_hc_sl = strlen(d.npubw_hc_s);
        } else {
            QLOGE("Failed to read %s", d.npubw_hwmon_hyst_count_path);
            return FAILED;
        }
        FREAD_STR(d.npubw_hwmon_hist_memory_path, d.npubw_hm_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.npubw_hm_sl = strlen(d.npubw_hm_s);
        } else {
            QLOGE("Failed to read %s", d.npubw_hwmon_hist_memory_path);
            return FAILED;
        }
        FREAD_STR(d.npubw_hwmon_hyst_length_path, d.npubw_hl_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.npubw_hl_sl = strlen(d.npubw_hl_s);
        } else {
            QLOGE("Failed to read %s", d.npubw_hwmon_hyst_length_path);
            return FAILED;
        }
        stored_hyst_opt = 1;
    }

    snprintf(tmp_s, NODE_MAX, "0");
    FWRITE_STR(d.npubw_hwmon_hyst_count_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.npubw_hwmon_hyst_count_path, tmp_s, rc);
    FWRITE_STR(d.npubw_hwmon_hist_memory_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.npubw_hwmon_hist_memory_path, tmp_s, rc);
    FWRITE_STR(d.npubw_hwmon_hyst_length_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.npubw_hwmon_hyst_length_path, tmp_s, rc);
    return rc;
}

int OptsHandler::npu_llcbw_hwmon_hyst_opt(Resource &r, OptsData &d) {
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_hyst_opt = 0;
    unsigned int reqval = r.value;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if (reqval == MAX_LVL) {
        if (d.npu_llcbw_hc_sl > 0) {
            FWRITE_STR(d.npu_llcbw_hwmon_hyst_count_path, d.npu_llcbw_hc_s, d.npu_llcbw_hc_sl, rc);
            if (rc < 0) {
                QLOGE("Failed to write %s", d.npu_llcbw_hwmon_hyst_count_path);
                return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.npu_llcbw_hwmon_hyst_count_path, d.npu_llcbw_hc_s, rc);
        }
        if (d.npu_llcbw_hm_sl > 0) {
            FWRITE_STR(d.npu_llcbw_hwmon_hist_memory_path, d.npu_llcbw_hm_s, d.npu_llcbw_hm_sl, rc);
            if (rc < 0) {
                QLOGE("Failed to write %s", d.npu_llcbw_hwmon_hist_memory_path);
                return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.npu_llcbw_hwmon_hist_memory_path, d.npu_llcbw_hm_s, rc);
        }
        if (d.npu_llcbw_hl_sl > 0) {
            FWRITE_STR(d.npu_llcbw_hwmon_hyst_length_path, d.npu_llcbw_hl_s, d.npu_llcbw_hl_sl, rc);
            if (rc < 0) {
                QLOGE("Failed to write %s", d.npu_llcbw_hwmon_hyst_length_path);
                return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.npu_llcbw_hwmon_hyst_length_path, d.npu_llcbw_hl_s, rc);
        }
        stored_hyst_opt = 0;

        return rc;
    }

    if (!stored_hyst_opt) {
        FREAD_STR(d.npu_llcbw_hwmon_hyst_count_path, d.npu_llcbw_hc_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.npu_llcbw_hc_sl = strlen(d.npu_llcbw_hc_s);
        } else {
            QLOGE("Failed to read %s", d.npu_llcbw_hwmon_hyst_count_path);
            return FAILED;
        }
        FREAD_STR(d.npu_llcbw_hwmon_hist_memory_path, d.npu_llcbw_hm_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.npu_llcbw_hm_sl = strlen(d.npu_llcbw_hm_s);
        } else {
            QLOGE("Failed to read %s", d.npu_llcbw_hwmon_hist_memory_path);
            return FAILED;
        }
        FREAD_STR(d.npu_llcbw_hwmon_hyst_length_path, d.npu_llcbw_hl_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.npu_llcbw_hl_sl = strlen(d.npu_llcbw_hl_s);
        } else {
            QLOGE("Failed to read %s", d.npu_llcbw_hwmon_hyst_length_path);
            return FAILED;
        }
        stored_hyst_opt = 1;
    }

    snprintf(tmp_s, NODE_MAX, "0");
    FWRITE_STR(d.npu_llcbw_hwmon_hyst_count_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.npu_llcbw_hwmon_hyst_count_path, tmp_s, rc);
    FWRITE_STR(d.npu_llcbw_hwmon_hist_memory_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.npu_llcbw_hwmon_hist_memory_path, tmp_s, rc);
    FWRITE_STR(d.npu_llcbw_hwmon_hyst_length_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.npu_llcbw_hwmon_hyst_length_path, tmp_s, rc);
    return rc;
}


int OptsHandler::cpubw_hwmon_hyst_opt(Resource &r, OptsData &d) {
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_hyst_opt = 0;
    unsigned int reqval = r.value;
    int idx = r.qindex;

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource %s not supported", d.sysfsnode_path[idx]);
        return FAILED;
    }

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if (reqval == MAX_LVL) {
        if (d.cpubw_hc_sl > 0) {
           FWRITE_STR(d.cpubw_hwmon_hyst_count_path, d.cpubw_hc_s, d.cpubw_hc_sl, rc);
           if (rc < 0) {
               QLOGE("Failed to write %s", d.cpubw_hwmon_hyst_count_path);
               return FAILED;
            }
        }
        if (d.cpubw_hm_sl > 0) {
            FWRITE_STR(d.cpubw_hwmon_hist_memory_path, d.cpubw_hm_s, d.cpubw_hm_sl, rc);
            if (rc < 0) {
               QLOGE("Failed to write %s", d.cpubw_hwmon_hist_memory_path);
               return FAILED;
            }
        }
        if (d.cpubw_hl_sl > 0) {
            FWRITE_STR(d.cpubw_hwmon_hyst_length_path, d.cpubw_hl_s, d.cpubw_hl_sl, rc);
            if (rc < 0) {
               QLOGE("Failed to write %s", d.cpubw_hwmon_hyst_length_path);
               return FAILED;
            }
        }
        stored_hyst_opt = 0;

        return rc;
    }

    if (!stored_hyst_opt) {
        FREAD_STR(d.cpubw_hwmon_hyst_count_path, d.cpubw_hc_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.cpubw_hc_sl = strlen(d.cpubw_hc_s);
        } else {
            QLOGE("Failed to read %s", d.cpubw_hwmon_hyst_count_path);
            return FAILED;
        }
        FREAD_STR(d.cpubw_hwmon_hist_memory_path, d.cpubw_hm_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.cpubw_hm_sl = strlen(d.cpubw_hm_s);
        } else {
            QLOGE("Failed to read %s", d.cpubw_hwmon_hist_memory_path);
            return FAILED;
        }
        FREAD_STR(d.cpubw_hwmon_hyst_length_path, d.cpubw_hl_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.cpubw_hl_sl = strlen(d.cpubw_hl_s);
        } else {
            QLOGE("Failed to read %s", d.cpubw_hwmon_hyst_length_path);
            return FAILED;
        }
        stored_hyst_opt = 1;
    }

    snprintf(tmp_s, NODE_MAX, "0");
    FWRITE_STR(d.cpubw_hwmon_hyst_count_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.cpubw_hwmon_hyst_count_path, tmp_s, rc);
    FWRITE_STR(d.cpubw_hwmon_hist_memory_path, tmp_s, strlen(tmp_s), rc);
     QLOGI("perf_lock_acq: updated %s with %s return value %d", d.cpubw_hwmon_hist_memory_path, tmp_s, rc);
    FWRITE_STR(d.cpubw_hwmon_hyst_length_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.cpubw_hwmon_hyst_length_path, tmp_s, rc);
    return rc;
}

int OptsHandler::llcbw_hwmon_hyst_opt(Resource &r, OptsData &d) {
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_hyst_opt = 0;
    unsigned int reqval = r.value;
    int idx = r.qindex;

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource %s not supported", d.sysfsnode_path[idx]);
        return FAILED;
    }

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if (reqval == MAX_LVL) {
        if (d.llcbw_hc_sl > 0) {
            FWRITE_STR(d.llcbw_hwmon_hyst_count_path, d.llcbw_hc_s, d.llcbw_hc_sl, rc);
            if (rc < 0) {
                QLOGE("Failed to write %s", d.llcbw_hwmon_hyst_count_path);
                return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.llcbw_hwmon_hyst_count_path, d.llcbw_hc_s, rc);
        }
        if (d.llcbw_hm_sl > 0) {
            FWRITE_STR(d.llcbw_hwmon_hist_memory_path, d.llcbw_hm_s, d.llcbw_hm_sl, rc);
            if (rc < 0) {
                QLOGE("Failed to write %s", d.llcbw_hwmon_hist_memory_path);
                return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.llcbw_hwmon_hist_memory_path, d.llcbw_hm_s, rc);
        }
        if (d.llcbw_hl_sl > 0) {
            FWRITE_STR(d.llcbw_hwmon_hyst_length_path, d.llcbw_hl_s, d.llcbw_hl_sl, rc);
            if (rc < 0) {
                QLOGE("Failed to write %s", d.llcbw_hwmon_hyst_length_path);
                return FAILED;
            }
            QLOGI("perf_lock_rel: updated %s with %s return value %d", d.llcbw_hwmon_hyst_length_path, d.llcbw_hl_s, rc);
        }
        stored_hyst_opt = 0;

        return rc;
    }

    if (!stored_hyst_opt) {
        FREAD_STR(d.llcbw_hwmon_hyst_count_path, d.llcbw_hc_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.llcbw_hc_sl = strlen(d.llcbw_hc_s);
        } else {
            QLOGE("Failed to read %s", d.llcbw_hwmon_hyst_count_path);
            return FAILED;
        }
        FREAD_STR(d.llcbw_hwmon_hist_memory_path, d.llcbw_hm_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.llcbw_hm_sl = strlen(d.llcbw_hm_s);
        } else {
            QLOGE("Failed to read %s", d.llcbw_hwmon_hist_memory_path);
            return FAILED;
        }
        FREAD_STR(d.llcbw_hwmon_hyst_length_path, d.llcbw_hl_s, NODE_MAX, rc);
        if (rc >= 0) {
            d.llcbw_hl_sl = strlen(d.llcbw_hl_s);
        } else {
            QLOGE("Failed to read %s", d.llcbw_hwmon_hyst_length_path);
            return FAILED;
        }
        stored_hyst_opt = 1;
    }

    snprintf(tmp_s, NODE_MAX, "0");
    FWRITE_STR(d.llcbw_hwmon_hyst_count_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.llcbw_hwmon_hyst_count_path, tmp_s, rc);
    FWRITE_STR(d.llcbw_hwmon_hist_memory_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.llcbw_hwmon_hist_memory_path, tmp_s, rc);
    FWRITE_STR(d.llcbw_hwmon_hyst_length_path, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", d.llcbw_hwmon_hyst_length_path, tmp_s, rc);
    return rc;
}

/* Function to handle single layer display hint
 * state = 0, is not single layer
 * state = 1, is single layer
 */
int OptsHandler::handle_disp_hint(Resource &r, OptsData &d) {
    unsigned int reqval = r.value;
    int rc = FAILED;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    d.mHintData.disp_single_layer = reqval;
    QLOGI("Display sent layer=%d", d.mHintData.disp_single_layer);

    /* Video is started, but display sends multiple
     * layer hint, so rearm timer, handled as
     * condition2 in slvp callback
     */
    if (d.mHintData.slvp_perflock_set == 1 && d.mHintData.disp_single_layer != 1) {
        QLOGI("Display to rearm timer,layer=%d",d.mHintData.disp_single_layer);

        /* rearm timer here, release handle in SLVP callback */
        rearm_slvp_timer(&d.mHintData);
    }

    return 0;
}

/* Function to receive video playback hint
 * state = 0, video stopped
 * state = 1, single instance of video
 */
int OptsHandler::handle_vid_decplay_hint(Resource &r, OptsData &d) {
    unsigned int reqval = r.value;
    int rc = FAILED;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    d.mHintData.vid_hint_state = reqval;

    /* timer is created only once here on getting video hint */
    if (d.mHintData.vid_hint_state == 1 && !d.mHintData.timer_created) {
        QLOGI("Video sent hint, create timer");
        return vid_create_timer(&d.mHintData);
    } else {
        /* only rearm here, handle conditions in SLVP callback */
        QLOGI("Video rearm timer");
        rearm_slvp_timer(&d.mHintData);
    }

    return 0;
}

/* Function to recieve video encode hint
 * for WFD use case.
 */
int OptsHandler::handle_vid_encplay_hint(Resource &r, OptsData &d) {
    unsigned int reqval = r.value;
    int rc = FAILED;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    /* reqval - 1, encode start
     * reqval - 0, encode stop
     */
    d.mHintData.vid_enc_start = reqval;
    /* rearm timer if encode is atarted
     * handle condition4 in callback */
    if (d.mHintData.slvp_perflock_set == 1) {
        rearm_slvp_timer(&d.mHintData);
    }

    return 0;
}

int OptsHandler::disable_ksm(Resource &r, OptsData &d) {
    int rc = FAILED;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    return d.toggle_ksm_run(0);
}

int OptsHandler::enable_ksm(Resource &r, OptsData &d) {
    int rc = FAILED;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    return d.toggle_ksm_run(1);
}

int OptsHandler::set_ksm_param(Resource &r, OptsData &d) {
    int rc = 0;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if(d.is_ksm_supported == 0)
    {
       char sleep_time[PROPERTY_VALUE_MAX];
       char scan_page[PROPERTY_VALUE_MAX];
       memset(sleep_time, 0, sizeof(sleep_time));
       memset(scan_page, 0, sizeof(scan_page));

       strlcpy(sleep_time, "20", PROPERTY_VALUE_MAX);
       strlcpy(scan_page, "300", PROPERTY_VALUE_MAX);

       FWRITE_STR(d.ksm_param_sleeptime, sleep_time, strlen(sleep_time), rc);
       FWRITE_STR(d.ksm_param_pages_to_scan, scan_page, strlen(scan_page), rc);
    }
    return rc;
}

int OptsHandler::reset_ksm_param(Resource &r, OptsData &d) {
    int rc = 0;

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if(d.is_ksm_supported == 0)
    {
       FWRITE_STR(d.ksm_param_sleeptime, d.ksm_sleep_millisecs, strlen(d.ksm_sleep_millisecs), rc);
       FWRITE_STR(d.ksm_param_pages_to_scan, d.ksm_pages_to_scan, strlen(d.ksm_pages_to_scan), rc);
    }
    return rc;
}

int OptsHandler::lock_min_cores(Resource &r,  OptsData &d) {
    Target &t = Target::getCurTarget();
    TargetConfig &tc = TargetConfig::getTargetConfig();

    if (d.core_ctl_present && t.isResourceSupported(r.major, r.minor)) {
        int rc = -1, coresInCluster, cluster = -1;
        char tmp_s[NODE_MAX];
        unsigned int reqval = r.value;
        static int stored_val[MAX_CLUSTER] = {0};

        cluster = r.cluster;
        snprintf(d.core_ctl_min_cpu_node, NODE_MAX, CORE_CTL_MIN_CPU, t.getFirstCoreIndex(cluster));

        if (reqval == MAX_LVL) {
            d.min_cores[cluster] = atoi(d.core_ctl_min_s[cluster]);
            FWRITE_STR(d.core_ctl_min_cpu_node, d.core_ctl_min_s[cluster], d.core_ctl_min_sl[cluster], rc);
            QLOGI("perf_lock_rel: updating %s with %s", d.core_ctl_min_cpu_node, d.core_ctl_min_s[cluster]);
            stored_val[cluster] = 0;
            return rc;
        }

        if (!stored_val[cluster]) {
            FREAD_STR(d.core_ctl_min_cpu_node, d.core_ctl_min_s[cluster], NODE_MAX, rc);
            if (rc >= 0) {
                d.core_ctl_min_sl[cluster] = strlen(d.core_ctl_min_s[cluster]);
                QLOGI("%s read with %s return value %d", d.core_ctl_min_cpu_node, d.core_ctl_min_s[cluster], rc);
            } else {
                QLOGE("Failed to read %s", d.core_ctl_min_cpu_node);
                return FAILED;
            }
            stored_val[cluster] = 1;
        }

        d.min_cores[cluster] = reqval;
        coresInCluster = tc.getCoresInCluster(cluster);
        if ((coresInCluster >= 0) && (d.min_cores[cluster] > coresInCluster)) {
            d.min_cores[cluster] = coresInCluster;
        }

        if (d.min_cores[cluster] > d.max_cores[cluster]) {
            d.min_cores[cluster] = d.max_cores[cluster];
        }

        snprintf(tmp_s, NODE_MAX, "%d", d.min_cores[cluster]);
        FWRITE_STR(d.core_ctl_min_cpu_node, tmp_s, strlen(tmp_s), rc);
        QLOGI("perf_lock_acq: updating %s with %s", d.core_ctl_min_cpu_node, tmp_s);
        return rc;
    }
    else {
        QLOGI("lock_min_cores perflock is not supported");
        return FAILED;
    }
    return 0;
}

int OptsHandler::lock_max_cores(Resource &r,  OptsData &d) {
    char tmp_s[NODE_MAX];
    Target &t = Target::getCurTarget();
    TargetConfig &tc = TargetConfig::getTargetConfig();
    unsigned int reqval = r.value;
    static int stored_val[MAX_CLUSTER] = {0};
    int rc = FAILED, cluster = -1;

    if (d.core_ctl_present > 0) {
        cluster = r.cluster;
        snprintf(d.core_ctl_max_cpu_node, NODE_MAX, CORE_CTL_MAX_CPU, t.getFirstCoreIndex(cluster));

        if (reqval == MAX_LVL) {
           /* Update max_core first otherwise min_core wiil not update */
            d.max_cores[cluster] = atoi(d.core_ctl_max_s[cluster]);
            FWRITE_STR(d.core_ctl_max_cpu_node, d.core_ctl_max_s[cluster], d.core_ctl_max_sl[cluster], rc);
            QLOGI("perf_lock_rel: updating %s with %s ", d.core_ctl_max_cpu_node, d.core_ctl_max_s[cluster]);
            if (d.min_cores[cluster] >= 0) {
               snprintf(d.core_ctl_min_cpu_node, NODE_MAX, CORE_CTL_MIN_CPU, t.getFirstCoreIndex(cluster));
               snprintf(tmp_s, NODE_MAX, "%d", d.min_cores[cluster]);
               FWRITE_STR(d.core_ctl_min_cpu_node, tmp_s, strlen(tmp_s), rc);
               QLOGI("perf_lock_rel: updating %s with %s ", d.core_ctl_min_cpu_node, tmp_s);
               stored_val[cluster] = 0;
            }
            return rc;
       }

       if (((int)reqval < tc.getMinCoreOnline()) || ((int)reqval > tc.getCoresInCluster(cluster))) {
            QLOGE("Error: perf-lock failed, invalid no. of cores requested to be online");
            return FAILED;
       }

        if (!stored_val[cluster]) {
            FREAD_STR(d.core_ctl_max_cpu_node, d.core_ctl_max_s[cluster], NODE_MAX, rc);
            if (rc >= 0) {
                d.core_ctl_max_sl[cluster] = strlen(d.core_ctl_max_s[cluster]);
                QLOGI("%s read with %s return value %d", d.core_ctl_max_cpu_node, d.core_ctl_max_s[cluster], rc);
                stored_val[cluster] = 1;
            } else {
                QLOGE("Failed to read %s", d.core_ctl_max_cpu_node);
                return FAILED;
            }
       }

        d.max_cores[cluster] = reqval;
        snprintf(tmp_s, NODE_MAX, "%d", d.max_cores[cluster]);
        FWRITE_STR(d.core_ctl_max_cpu_node, tmp_s, strlen(tmp_s), rc);
        QLOGI("perf_lock_acq: updating %s with %s ", d.core_ctl_max_cpu_node, tmp_s);
        return rc;
    }
    else if (d.kpm_hotplug_support > 0) {
        if (tc.getCoresInCluster(r.cluster) <= 0) {
            QLOGW("Warning: Cluster %d does not exist, resource is not supported", r.cluster);
           return FAILED;
        }

        if (reqval == MAX_LVL) {
            if (r.cluster == 0) {
               d.lock_max_clust0 = -1;
            } else if (r.cluster == 1) {
               d.lock_max_clust1 = -1;
            }
        } else {
            d.max_cores[r.cluster] = reqval;

            if (d.max_cores[r.cluster] > tc.getCoresInCluster(r.cluster)) {
               QLOGE("Error! perf-lock failed, invalid no. of cores requested to be online");
               return FAILED;
            }

            if (r.cluster == 0) {
               d.lock_max_clust0 = d.max_cores[r.cluster];
            } else if (r.cluster == 1) {
               d.lock_max_clust1 = d.max_cores[r.cluster];
            }
       }

       snprintf(tmp_s, NODE_MAX, "%d:%d", d.lock_max_clust0, d.lock_max_clust1);
       char kpmSysNode[NODE_MAX];
       memset(kpmSysNode, 0, sizeof(kpmSysNode));
       PerfDataStore *store = PerfDataStore::getPerfDataStore();
       store->GetSysNode(KPM_MAX_CPUS, kpmSysNode);

       QLOGE("Write %s into %s", tmp_s, kpmSysNode);
       FWRITE_STR(kpmSysNode, tmp_s, strlen(tmp_s), rc);
       return rc;
    }
    else
       return FAILED;
    return 0;
}

int OptsHandler::handle_early_wakeup_hint(Resource &r, OptsData &d) {
    unsigned int reqVal = r.value;
    int displayId = d.getEarlyWakeupDispId();
    EventData *evData;
    noroot_args *msg;

    /* perf_lock_rel */
    QLOGI("drmIOCTL reqVal: %d displayId: %d", reqVal, displayId);
    /* perf_lock_rel */
    if (reqVal == MAX_LVL) {
        QLOGI("drmIOCTL perf_lock_rel");
        return SUCCESS;
    }

    if (!isNorootThreadAlive) {
        QLOGE("Can't process request: noroot_thread is not running");
        return FAILED;
    }

    evData = NRevqueue.GetDataPool().Get();

    if (!evData || !(evData->mEvData)) {
        QLOGE("Event data pool ran empty");
        return FAILED;
    }

    msg = (noroot_args *)evData->mEvData;
    msg->val = displayId;
    msg->retval = 0;
    evData->mEvType = DISPLAY_EARLY_WAKEUP_HINT;
    /* perf_lock_acq */
    QLOGI("calling drmIOCTLLib reqVal: %d", reqVal);
    NRevqueue.Wakeup(evData);
    return SUCCESS;
}

/* Disable GPU Nap */
int OptsHandler::gpu_disable_gpu_nap(Resource &r, OptsData &d) {
    int rc = FAILED;
    int stored_gpu_idle_timer = 0;
    unsigned int reqval = r.value;
    char tmp_s1[NODE_MAX];
    char tmp_s2[NODE_MAX];
    int ret[4] = {-1,-1,-1,-1};
    static char gpu_nap_s[4][NODE_MAX]; // input value node
    static int  gpu_nap_sl[4] = {-1,-1,-1,-1}; // input value string length
    static unsigned int stored_gpu_nap = 0;

    Target &target = Target::getCurTarget();

    rc = ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }
    rc = FAILED;
    if (reqval == MAX_LVL) {
        if (gpu_nap_sl[0] > 0 && gpu_nap_sl[1] > 0 && gpu_nap_sl[2] && gpu_nap_sl[3]) {
            FWRITE_STR(GPU_FORCE_RAIL_ON, gpu_nap_s[0], gpu_nap_sl[0], ret[0]);
            FWRITE_STR(GPU_FORCE_CLK_ON, gpu_nap_s[1], gpu_nap_sl[1], ret[1]);
            FWRITE_STR(GPU_IDLE_TIMER, gpu_nap_s[2], gpu_nap_sl[2], ret[2]);
            FWRITE_STR(GPU_FORCE_NO_NAP, gpu_nap_s[3], gpu_nap_sl[3], ret[3]);
            QLOGI("perf_lock_rel: updated %s with %s return value %d", GPU_FORCE_RAIL_ON, gpu_nap_s[0], ret[0]);
            QLOGI("perf_lock_rel: updated %s with %s return value %d", GPU_FORCE_CLK_ON, gpu_nap_s[1], ret[1]);
            QLOGI("perf_lock_rel: updated %s with %s return value %d", GPU_IDLE_TIMER, gpu_nap_s[2], ret[2]);
            QLOGI("perf_lock_rel: updated %s with %s return value %d", GPU_FORCE_NO_NAP, gpu_nap_s[3], ret[3]);
            stored_gpu_nap = 0;
        }
        rc = ret[0] && ret[1] && ret[2] && ret[3];
        return rc;
    }

    if (!stored_gpu_nap) {
        FREAD_STR(GPU_FORCE_RAIL_ON, gpu_nap_s[0], NODE_MAX, ret[0]);
        FREAD_STR(GPU_FORCE_CLK_ON, gpu_nap_s[1], NODE_MAX, ret[1]);
        FREAD_STR(GPU_IDLE_TIMER, gpu_nap_s[2], NODE_MAX, ret[2]);
        FREAD_STR(GPU_FORCE_NO_NAP, gpu_nap_s[3], NODE_MAX, ret[3]);
        if (ret[0] > 0 && ret[1] > 0 && ret[2] > 0 && ret[3] > 0) {
            QLOGI("%s read with %s return value %d", GPU_FORCE_RAIL_ON, gpu_nap_s[0], ret[0]);
            QLOGI("%s read with %s return value %d", GPU_FORCE_CLK_ON, gpu_nap_s[1], ret[1]);
            QLOGI("%s read with %s return value %d", GPU_IDLE_TIMER, gpu_nap_s[2], ret[2]);
            QLOGI("%s read with %s return value %d", GPU_FORCE_NO_NAP, gpu_nap_s[3], ret[3]);
            gpu_nap_sl[0] = strlen(gpu_nap_s[0]);
            gpu_nap_sl[1] = strlen(gpu_nap_s[1]);
            gpu_nap_sl[2] = strlen(gpu_nap_s[2]);
            gpu_nap_sl[3] = strlen(gpu_nap_s[3]);
            stored_gpu_nap = 1;
            stored_gpu_idle_timer = strtod(gpu_nap_s[2], NULL);
        } else {
            if (ret[0] < 0)
                QLOGE("Failed to read %s", GPU_FORCE_RAIL_ON);
            if (ret[1] < 0)
                QLOGE("Failed to read %s", GPU_FORCE_CLK_ON);
            if (ret[2] < 0)
                QLOGE("Failed to read %s", GPU_IDLE_TIMER);
            if (ret[3] < 0)
                QLOGE("Failed to read %s", GPU_FORCE_NO_NAP);
            return FAILED;
        }
    }

    snprintf(tmp_s1, NODE_MAX, "%d", 1);
    snprintf(tmp_s2, NODE_MAX, "%d", reqval);

    FWRITE_STR(GPU_FORCE_RAIL_ON, tmp_s1, strlen(tmp_s1), ret[0]);
    FWRITE_STR(GPU_FORCE_CLK_ON, tmp_s1, strlen(tmp_s1), ret[1]);
    if (reqval > stored_gpu_idle_timer) {
        FWRITE_STR(GPU_IDLE_TIMER, tmp_s2, strlen(tmp_s2), ret[2]);
    }
    FWRITE_STR(GPU_FORCE_NO_NAP, tmp_s1, strlen(tmp_s1), ret[3]);

    QLOGI("perf_lock_acq: updated %s with %s return value %d", GPU_FORCE_RAIL_ON, tmp_s1, ret[0]);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", GPU_FORCE_CLK_ON, tmp_s1, ret[1]);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", GPU_IDLE_TIMER, tmp_s2, ret[2]);
    QLOGI("perf_lock_acq: updated %s with %s return value %d", GPU_FORCE_NO_NAP, tmp_s1, ret[3]);

    if (ret[0] < 0 && ret[1] < 0 && ret[2] < 0 && ret[3] < 0) {
        QLOGE("Acquiring GPU nap lock failed\n");
        return FAILED;
    }
    rc = ret[0] && ret[1] && ret[2] && ret[3];
    return rc;
}

/* Set irq_balancer */
int OptsHandler::irq_balancer(Resource &r, OptsData &d) {
    int rc = 0;
    int i = 0;
    int tmp_s[MAX_CPUS];
    unsigned int reqval = r.value;
    Target &target = Target::getCurTarget();
    TargetConfig &tc = TargetConfig::getTargetConfig();

    rc = ValidateClusterAndCore(r.cluster, r.core, SYNC_CORE);
    if (rc == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if (reqval == MAX_LVL) {
        if (d.irq_bal_sp) {
            send_irq_balance(d.irq_bal_sp, NULL);
            d.stored_irq_balancer = 0;
            free(d.irq_bal_sp);
            d.irq_bal_sp = NULL;
        }
        return rc;
    }

    if (reqval > 0) {
        for (i = 0; i < tc.getTotalNumCores(); i++)
            tmp_s[i] = (reqval & (1 << i)) >> i;

        if (!d.stored_irq_balancer) {
            if (!d.irq_bal_sp) {
                d.irq_bal_sp = (int *) malloc(sizeof(*d.irq_bal_sp) * MAX_CPUS);
            }
            send_irq_balance(tmp_s, &d.irq_bal_sp);
            d.stored_irq_balancer = 1;
        } else {
            send_irq_balance(tmp_s, NULL);
        }
    }

    return rc;
}

int OptsHandler::sched_coloc_busy_hyst_cpu_busy_pct(Resource &r, OptsData &d) {
    unsigned short idx = r.qindex;
    char *node_storage = NULL;
    int *node_storage_length = NULL;
    Target &t = Target::getCurTarget();
    int core_start = -1, core_end = -1;

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource %s not supported", d.sysfsnode_path[idx]);
        return FAILED;
    }

    if (ValidateClusterAndCore(r.cluster, r.core, SYNC_CORE) == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    core_start = t.getFirstCoreIndex(r.cluster);
    core_end = t.getLastCoreIndex(r.cluster);

    if(core_start == -1 || core_end == -1) {
        return FAILED;
    }

    node_storage = d.sch_coloc_busy_hyst_cpu_busy_pct_s[r.cluster];
    node_storage_length = &d.sch_coloc_busy_hyst_cpu_busy_pct_sl[r.cluster];

    return multiValNodeFunc(r, d, node_storage, node_storage_length, core_start, core_end);
}

int OptsHandler::sched_coloc_busy_hyst_cpu_ns(Resource &r, OptsData &d) {
    unsigned short idx = r.qindex;
    char *node_storage = NULL;
    int *node_storage_length = NULL;
    Target &t = Target::getCurTarget();
    int core_start = -1, core_end = -1;

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource %s not supported", d.sysfsnode_path[idx]);
        return FAILED;
    }

    if (ValidateClusterAndCore(r.cluster, r.core, SYNC_CORE) == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    core_start = t.getFirstCoreIndex(r.cluster);
    core_end = t.getLastCoreIndex(r.cluster);

    if(core_start == -1 || core_end == -1) {
        return FAILED;
    }

    node_storage = d.sch_coloc_busy_hyst_cpu_ns_s[r.cluster];
    node_storage_length = &d.sch_coloc_busy_hyst_cpu_ns_sl[r.cluster];

    return multiValNodeFunc(r, d, node_storage, node_storage_length, core_start, core_end);
}

int OptsHandler::multiValNodeFunc(Resource &r, OptsData &d, char *node_storage, int *node_storage_length, int core_start, int core_end) {
    int rc;
    unsigned short idx = r.qindex;
    char tmp_s[NODE_MAX]= "";
    unsigned int reqval = r.value;
    char *pos;
    int old_value = -1;

    if(node_storage == NULL || node_storage_length == NULL) {
        QLOGE("Perflock node storage error");
        return FAILED;
    }

    if (reqval == MAX_LVL) {
        QLOGI("Perflock release call for resource index = %d, path = %s, from \
                function = %s", idx, d.sysfsnode_path[idx], __func__);

        if (*node_storage_length > 0) {
                int core_index = 0;
                old_value = -1;
                char *old_val_s = strtok_r(node_storage, "\t", &pos);
                if (!old_val_s)
                    return FAILED;
                if(core_index >= core_start && core_index <= core_end) {
                    old_value = strtol(old_val_s, NULL, BASE_10);
                }
                core_index++;
                while ((old_val_s = strtok_r(NULL, "\t", &pos)) && old_value == -1) {
                    if(core_index >= core_start && core_index <= core_end) {
                        old_value = strtol(old_val_s, NULL, BASE_10);
                    }
                    core_index++;
                }
            if (old_value == -1)
                return FAILED;
            parseMultiValNode(old_value, tmp_s,
                    d.sysfsnode_path[idx], core_start, core_end);
            rc = change_node_val(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s));
            *node_storage_length = -1;
            return rc;
        }
        else
            QLOGE("Unable to find the correct node storage pointers for \
                    resource index=%d, node path=%s", idx, d.sysfsnode_path[idx]);
    }
    else {
        if (*node_storage_length <= 0) {
            *node_storage_length = save_node_val(d.sysfsnode_path[idx],
                    node_storage);
            if (*node_storage_length <= 0) {
                QLOGE("Failed to read %s", d.sysfsnode_path[idx]);
                return FAILED;
            }
        }
        /* extract other values within the node */
        if (parseMultiValNode(reqval, tmp_s,
                    d.sysfsnode_path[idx], core_start, core_end) == -1) {
            QLOGE("Failed to parse migration value(s)");
            return FAILED;
        }

        return change_node_val(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s));
    }
    return FAILED;
}

int OptsHandler::migrate(Resource &r, OptsData &d) {
    int rc;
    unsigned short idx = r.qindex;
    char tmp_s[NODE_MAX]= "";
    unsigned int reqval = r.value;
    char *node_storage = NULL, *pos;
    int *node_storage_length = NULL;

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource %s not supported", d.sysfsnode_path[idx]);
        return FAILED;
    }

    if (ValidateClusterAndCore(r.cluster, r.core, SYNC_CORE) == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    if (idx == SCHED_START_INDEX + SCHED_UPMIGRATE_OPCODE) {
        node_storage = d.sch_upmigrate_s[r.cluster];
        node_storage_length = &d.sch_upmigrate_sl[r.cluster];
    }
    else if (idx == SCHED_START_INDEX + SCHED_DOWNMIGRATE_OPCODE) {
        node_storage = d.sch_downmigrate_s[r.cluster];
        node_storage_length = &d.sch_downmigrate_sl[r.cluster];
    }
    else {
        QLOGE("Resource index %d: , not supported in function: %s", idx,
              __func__);
        return FAILED;
    }

    if (reqval == MAX_LVL) {
        QLOGI("Perflock release call for resource index = %d, path = %s, from \
              function = %s", idx, d.sysfsnode_path[idx], __func__);

        if (*node_storage_length > 0) {
            if (r.cluster <= CLUSTER1) {
                char *gold_val = strtok_r(node_storage, "\t", &pos);
                if (!gold_val)
                    return FAILED;
                strlcpy(tmp_s, gold_val, NODE_MAX);
            }
            else if (r.cluster == CLUSTER2) {
                int prime_val;
                char *prime_str = strtok_r(node_storage, "\t", &pos);
                prime_str = strtok_r(nullptr, "\t", &pos);

                if (!prime_str)
                    return FAILED;
                prime_val = strtol(prime_str, NULL, BASE_10);
                parse_mig_vals(r.cluster, prime_val, tmp_s,
                               d.sysfsnode_path[idx]);
            }
            rc = change_node_val(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s));
            *node_storage_length = -1;
            return rc;
        }
        else
            QLOGE("Unable to find the correct node storage pointers for \
                  resource index=%d, node path=%s", idx, d.sysfsnode_path[idx]);
    }
    else {
        if (*node_storage_length <= 0) {
            *node_storage_length = save_node_val(d.sysfsnode_path[idx],
                                                 node_storage);
            if (*node_storage_length <= 0) {
                QLOGE("Failed to read %s", d.sysfsnode_path[idx]);
                return FAILED;
            }
        }
        if (r.cluster == CLUSTER2) {
            /* extract other values within the node */
            if (parse_mig_vals(r.cluster, reqval, tmp_s,
                d.sysfsnode_path[idx]) == -1) {
                QLOGE("Failed to parse migration value(s)");
                return FAILED;
            }
        }
        else
            snprintf(tmp_s, NODE_MAX, "%d", reqval);

        return change_node_val(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s));
    }
    return FAILED;
}

/*support for combined upmigrate downmigrate opcode*/
int OptsHandler::value_percluster(char node_val[NODE_MAX], int cluster) {
     int value = FAILED;
     char *pos;

     if (cluster <= CLUSTER1) {
         char *gold_val = strtok_r(node_val, "\t", &pos);
         if (!gold_val)
             return FAILED;
         value = strtol(gold_val, NULL, BASE_10);
     }
     else if (cluster == CLUSTER2) {
         char *prime_str = strtok_r(node_val, "\t", &pos);
         prime_str = strtok_r(nullptr, "\t", &pos);
         if (!prime_str)
              return FAILED;
         value = strtol(prime_str, NULL, BASE_10);
     }
     return value;
}

/*Read and parse current upmigarte downmigarte values on the nodes*/
int OptsHandler::read_curmigrate_val(const char *node_path, int cluster) {
    if (node_path == NULL)
        return FAILED;

    char node_val[NODE_MAX] = "";

    int node_val_length = save_node_val(node_path, node_val);
    if (node_val_length <= 0) {
        QLOGE("Failed to read %s", node_path);
        return FAILED;
    }
    return value_percluster(node_val, cluster);
}

/* Migarate acquire call func
*  upmigrate_val = 31:16 bit in reqval
*  downmigrate_val = 15:0 bit in reqval
*  eg: reqval = 0x003C0032 is concatinated value with
*               0x32 for downmigrate, 0x3C for upmigrate.
*/
int OptsHandler::migrate_action_apply(Resource &r, OptsData &d) {
    unsigned short idx = r.qindex;
    uint32_t reqval = r.value;
    int rc = FAILED;

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource %s not supported", d.sysfsnode_path[idx]);
        return FAILED;
    }
    if (ValidateClusterAndCore(r.cluster, r.core, SYNC_CORE) == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    //parse request value
    short reqUp_val = (short) (reqval >> 16);
    short reqDown_val = (short) reqval;
    if (reqUp_val < 0 || reqDown_val < 0) {
        QLOGE("Invalid migrate request values passed");
        return FAILED;
    }

    if ((d.sched_upmigrate[0] == '\0') && (d.sched_downmigrate[0] == '\0')) {
       snprintf(d.sched_upmigrate, NODE_MAX, d.sysfsnode_path[idx], "sched_upmigrate");
       snprintf(d.sched_downmigrate, NODE_MAX, d.sysfsnode_path[idx], "sched_downmigrate");
    }

    //determine order in which nodes are to be updated
    MIG_SEQ seq = UPDOWN;
    int curUp_val = 0, curDown_val = 0;
    curUp_val = read_curmigrate_val(d.sched_upmigrate, r.cluster);
    curDown_val = read_curmigrate_val(d.sched_downmigrate, r.cluster);
    if (curUp_val == FAILED || curDown_val == FAILED) {
        QLOGE("Unable to read default migrate values");
        return FAILED;
    }

    if (reqUp_val && reqDown_val && reqUp_val > reqDown_val) {//both vals passed
        seq = (reqUp_val < curDown_val) ? DOWNUP : UPDOWN;
    }
    else {
        QLOGI("Unable to apply, please pass valid values");
        return FAILED;
    }

    d.upmigrate_val[r.cluster] = reqUp_val;
    d.downmigrate_val[r.cluster] = reqDown_val;

    //action is called accordingly based on seq
    if (seq == UPDOWN) {
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_upmigrate);
        rc = migrate_action(r, d, ACQ, UP, d.upmigrate_val[r.cluster]);
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_downmigrate);
        rc =  migrate_action(r, d, ACQ, DOWN, d.downmigrate_val[r.cluster]);
    }
    else {
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_downmigrate);
        rc =  migrate_action(r, d, ACQ, DOWN, d.downmigrate_val[r.cluster]);
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_upmigrate);
        rc = migrate_action(r, d, ACQ, UP, d.upmigrate_val[r.cluster]);
    }
    return rc;
}

/*Release call for up down migrate*/
int OptsHandler::migrate_action_release(Resource &r, OptsData &d) {
    unsigned short idx = r.qindex;
    int rc = FAILED;

    //get upmigrate def value
    char up_node_val[NODE_MAX];
    strlcpy(up_node_val, d.sch_upmigrate_s[r.cluster], NODE_MAX);
    int upmig_def_value = value_percluster(up_node_val, r.cluster);
    int curDown_val = read_curmigrate_val(d.sched_downmigrate, r.cluster);

    //if up reset value is less than cur down, release down first
    if (upmig_def_value < curDown_val) {
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_downmigrate);
        rc = migrate_action(r, d, REL, DOWN, MAX_LVL);
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_upmigrate);
        rc = migrate_action(r, d, REL, UP, MAX_LVL);
    } else {
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_upmigrate);
        rc = migrate_action(r, d, REL, UP, MAX_LVL);
        snprintf(d.sysfsnode_path[idx], NODE_MAX, "%s", d.sched_downmigrate);
        rc = migrate_action(r, d, REL, DOWN, MAX_LVL);
    }
    return rc;
}

/* Migrate nodes are update here based on acq or rel call
*  @action = Either its a acquire or a release(ACQ/REL)
*  @flag = Indicates upmigrate node or downmigrate node
*  @reqval = Respective val after bit operation
*/
int OptsHandler::migrate_action(Resource &r, OptsData &d, int action, int flag, unsigned int reqval) {
    int rc;
    unsigned short idx = r.qindex;
    char tmp_s[NODE_MAX]= "";
    char *node_storage = NULL, *pos;
    int *node_storage_length = NULL;

    if (flag == UP) {
        node_storage = d.sch_upmigrate_s[r.cluster];
        node_storage_length = &d.sch_upmigrate_sl[r.cluster];
    }
    else if (flag == DOWN) {
        node_storage = d.sch_downmigrate_s[r.cluster];
        node_storage_length = &d.sch_downmigrate_sl[r.cluster];
    }
    else {
        QLOGE("Resource index %d: , not supported in function: %s", idx,
              __func__);
        return FAILED;
    }

    if (action == REL) {
        QLOGI("Perflock release call for resource index = %d, path = %s, from \
              function = %s", idx, d.sysfsnode_path[idx], __func__);

        if (*node_storage_length > 0) {
            if (r.cluster <= CLUSTER1) {
                char *gold_val = strtok_r(node_storage, "\t", &pos);
                if (!gold_val)
                    return FAILED;
                strlcpy(tmp_s, gold_val, NODE_MAX);
            }
            else if (r.cluster == CLUSTER2) {
                int prime_val;
                char *prime_str = strtok_r(node_storage, "\t", &pos);
                prime_str = strtok_r(nullptr, "\t", &pos);

                if (!prime_str)
                    return FAILED;
                prime_val = strtol(prime_str, NULL, BASE_10);
                parse_mig_vals(r.cluster, prime_val, tmp_s,
                               d.sysfsnode_path[idx]);
            }
            rc = change_node_val(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s));
            *node_storage_length = -1;
            return rc;
        }
        else
            QLOGE("Unable to find the correct node storage pointers for \
                  resource index=%d, node path=%s", idx, d.sysfsnode_path[idx]);
    }
    else {
        if (*node_storage_length <= 0) {
            *node_storage_length = save_node_val(d.sysfsnode_path[idx],
                                                 node_storage);
            if (*node_storage_length <= 0) {
                QLOGE("Failed to read %s", d.sysfsnode_path[idx]);
                return FAILED;
            }
        }
        if (r.cluster == CLUSTER2) {
            /* extract other values within the node */
            if (parse_mig_vals(r.cluster, reqval, tmp_s,
                d.sysfsnode_path[idx]) == -1) {
                QLOGE("Failed to parse migration value(s)");
                return FAILED;
            }
        }
        else
            snprintf(tmp_s, NODE_MAX, "%d", reqval);

        return change_node_val(d.sysfsnode_path[idx], tmp_s, strlen(tmp_s));
    }
    return FAILED;
}

int OptsHandler::keep_alive(Resource &r, OptsData &d) {
    int rc = -1;
    tKillFlag = (r.value == 0) ? false : true;

    QLOGI("keep alive tKillFlag:%d\n", tKillFlag);
    pthread_mutex_lock(&ka_mutex);
    if (!isThreadAlive) {
        if(!tKillFlag) {
            rc = pthread_create(&id_ka_thread, NULL, keep_alive_thread, NULL);
            if (rc!=0)
            {
                QLOGE("Unable to create keepAlive thread, and error code is %d\n",rc);
            }
        }
    }
    pthread_mutex_unlock(&ka_mutex);
    return rc;
}

void* OptsHandler::keep_alive_thread(void*) {
    int len;
    int i;
    int tmp;
    char cmd[50];

    while (true) {
        pthread_mutex_lock(&ka_mutex);
        if (!tKillFlag) {
            isThreadAlive = true;
            pthread_mutex_unlock(&ka_mutex);
            len = sizeof (dnsIPs) / sizeof (*dnsIPs);
            i = rand()%len;
            snprintf(cmd, 30, "ping -c 1 %s",dnsIPs[i]);
            system(cmd);
            QLOGI("Hello KeepAlive~\n");
            sleep(5);
        } else {
            isThreadAlive = false;
            pthread_mutex_unlock(&ka_mutex);
            break;
        }
    }
    QLOGI("Keep Alive Thread has gone.~~~\n");
    pthread_exit(0);
    return NULL;
}

int OptsHandler::perfmode_entry_pasr(Resource &r, OptsData &d) {

    uint32_t src = (uint32_t)PasrSrc_1_1::PASR_SRC_PERF;
    uint32_t pri = (uint32_t)PasrPriority::PASR_PRI_CRITICAL;
    uint32_t state = (uint32_t)PasrState::MEMORY_ONLINE;

    if (!pasr_enabled)
        return SUCCESS;    //nothing to do for pasr

    /* Enter into critical state for Online */
    android::hardware::Return<PasrStatus> status =
                        mPasrManager->stateEnter_1_1(src, pri, state);

    if(!status.isOk()) {
        ALOGE("perfmode_entry_pasr: HIDL call failed!");
        return FAILED;
    }

    if(status != PasrStatus::SUCCESS) {
        ALOGE("Entering critical online state for pasrmanager failed!");
        return FAILED;
    }

    /* Attempt to Online all blocks */
    status = mPasrManager->attemptOnlineAll_1_1(src, pri);
    if(status != PasrStatus::ONLINE) {
        ALOGE("Attempting to online all blocks failed!");
        return FAILED;
    }

    ALOGD("Onlined all blocks for perf mode entry");
    return SUCCESS;
}

int OptsHandler::perfmode_exit_pasr(Resource &r, OptsData &d) {

    uint32_t src = (uint32_t)PasrSrc_1_1::PASR_SRC_PERF;

    if (!pasr_enabled)
        return SUCCESS;    //nothing to do for pasr

    /* Exit from critical state for Online */
    android::hardware::Return<PasrStatus> status =
                                mPasrManager->stateExit_1_1(src);

    if(!status.isOk()) {
        ALOGE("perfmode_exit_pasr: HIDL call failed!");
        return FAILED;
    }

    if(status != PasrStatus::SUCCESS) {
        ALOGE("Exiting from critical online state for pasrmanager failed!");
        return FAILED;
    }

    ALOGD("Exit from perf mode successful for pasrmanager");
    return SUCCESS;
}

/* Group Migarate acquire call func
*  upmigrate_val = 31:16 bit in reqval
*  downmigrate_val = 15:0 bit in reqval
*  eg: reqval = 0x003C0032 is concatinated value with
*               0x32 for downmigrate, 0x3C for upmigrate.
*/
int OptsHandler::grp_migrate_action_apply(Resource &r, OptsData &d) {
    unsigned short idx = r.qindex;
    uint32_t reqval = r.value;
    int rc = FAILED;
    char tmp_s[NODE_MAX]= "";
    int curUp_val = 0, curDown_val = 0, ret[2] = {-1};

    if (!d.is_supported[idx]) {
        QLOGE("Perflock resource combined group up/down migrate not supported");
        return FAILED;
    }
    if (ValidateClusterAndCore(r.cluster, r.core, CORE_INDEPENDENT) == FAILED) {
        QLOGE("Request on invalid core or cluster");
        return FAILED;
    }

    //parse request value
    short reqUp_val = (short) (reqval >> 16);
    short reqDown_val = (short) reqval;
    if (reqUp_val < 0 || reqDown_val < 0) {
        QLOGE("Invalid migrate request values passed");
        return FAILED;
    }

    if ((d.sched_grp_upmigrate[0] == '\0') && (d.sched_grp_downmigrate[0] == '\0')) {
       snprintf(d.sched_grp_upmigrate, NODE_MAX, d.sysfsnode_path[idx], "sched_group_upmigrate");
       snprintf(d.sched_grp_downmigrate, NODE_MAX, d.sysfsnode_path[idx], "sched_group_downmigrate");
    }

    //determine order in which nodes are to be updated
    MIG_SEQ seq = UPDOWN;
    FREAD_STR(d.sched_grp_upmigrate, tmp_s, NODE_MAX, ret[0]);
    curUp_val = atoi(tmp_s);
    FREAD_STR(d.sched_grp_downmigrate, tmp_s, NODE_MAX, ret[1]);
    curDown_val = atoi(tmp_s);
    if (ret[0] == FAILED || ret[1] == FAILED) {
        QLOGE("Unable to read default migrate values");
        return FAILED;
    }
    if (reqUp_val && reqDown_val && reqUp_val >= reqDown_val) {//both vals passed
        seq = (reqUp_val < curDown_val) ? DOWNUP : UPDOWN;
    }
    else {
        QLOGI("Unable to apply, please pass valid values");
        return FAILED;
    }

    //store default value, will be required during release
    if(d.grp_upmigrate_sl <= 0) {
        d.grp_upmigrate_sl = save_node_val(d.sched_grp_upmigrate, d.grp_upmigrate_s);
        if(d.grp_upmigrate_sl < 0)
            QLOGE("Unable to store default value for %s", d.sched_grp_upmigrate);
        QLOGI("d.grp_upmigrate_s:%s d.grp_upmigrate_sl:%d", d.grp_upmigrate_s, d.grp_upmigrate_sl);
    }
    if(d.grp_downmigrate_sl <= 0) {
        d.grp_downmigrate_sl = save_node_val(d.sched_grp_downmigrate, d.grp_downmigrate_s);
        if(d.grp_downmigrate_sl < 0)
            QLOGE("Unable to store default value for %s", d.sched_grp_downmigrate);
        QLOGI("d.grp_downmigrate_s:%s d.grp_downmigrate_sl:%d", d.grp_downmigrate_s, d.grp_downmigrate_sl);
    }

    //action is called accordingly based on seq
    if (seq == UPDOWN) {
        snprintf(tmp_s, NODE_MAX, "%d", reqUp_val);
        FWRITE_STR(d.sched_grp_upmigrate, tmp_s, strlen(tmp_s), rc);
        QLOGI("updating %s with %s", d.sched_grp_upmigrate, tmp_s);
        snprintf(tmp_s, NODE_MAX, "%d", reqDown_val);
        QLOGI("updating %s with %s", d.sched_grp_downmigrate, tmp_s);
        FWRITE_STR(d.sched_grp_downmigrate, tmp_s, strlen(tmp_s), rc);
    }
    else {
        snprintf(tmp_s, NODE_MAX, "%d", reqDown_val);
        FWRITE_STR(d.sched_grp_downmigrate, tmp_s, strlen(tmp_s), rc);
        QLOGI("updating %s with %s", d.sched_grp_downmigrate, tmp_s);
        snprintf(tmp_s, NODE_MAX, "%d", reqUp_val);
        FWRITE_STR(d.sched_grp_upmigrate, tmp_s, strlen(tmp_s), rc);
        QLOGI("updating %s with %s", d.sched_grp_upmigrate, tmp_s);
    }
    return rc;
}

/*Release call for Group up down migrate*/
int OptsHandler::grp_migrate_action_release(Resource &r, OptsData &d) {
    unsigned short idx = r.qindex;
    int rc = FAILED;
    char tmp_s[NODE_MAX]= "";

    //get group upmigrate def value
    char up_node_val[NODE_MAX], down_node_val[NODE_MAX];
    strlcpy(up_node_val, d.grp_upmigrate_s, NODE_MAX);
    int upmig_def_value = atoi(up_node_val);

    //get group downmigrate current value
    int curDown_val, ret;
    FREAD_STR(d.sched_grp_downmigrate, down_node_val, NODE_MAX, ret);
    curDown_val = atoi(down_node_val);

    //if up reset value is less than cur down, release down first
    if (d.grp_upmigrate_sl > 0 && d.grp_downmigrate_sl > 0) {
        if (upmig_def_value < curDown_val) {
            snprintf(tmp_s, NODE_MAX, "%s", d.grp_downmigrate_s);
            FWRITE_STR(d.sched_grp_downmigrate, tmp_s, strlen(tmp_s), rc);
            QLOGI("updating %s with %s", d.sched_grp_downmigrate, tmp_s);
            snprintf(tmp_s, NODE_MAX, "%s", d.grp_upmigrate_s);
            FWRITE_STR(d.sched_grp_upmigrate, tmp_s, strlen(tmp_s), rc);
            QLOGI("updating %s with %s", d.sched_grp_upmigrate, tmp_s);
        } else {
            snprintf(tmp_s, NODE_MAX, "%s", d.grp_upmigrate_s);
            FWRITE_STR(d.sched_grp_upmigrate, tmp_s, strlen(tmp_s), rc);
            QLOGI("updating %s with %s", d.sched_grp_upmigrate, tmp_s);
            snprintf(tmp_s, NODE_MAX, "%s", d.grp_downmigrate_s);
            FWRITE_STR(d.sched_grp_downmigrate, tmp_s, strlen(tmp_s), rc);
            QLOGI("updating %s with %s", d.sched_grp_downmigrate, tmp_s);
        }
        d.grp_downmigrate_sl = 0;
        d.grp_upmigrate_sl = 0;
    }
    return rc;
}


/* Return ADD_AND_UPDATE_REQUEST if reqVal is greater than curVal
 * Return PEND_REQUEST if reqVal is less or equal to curVal
 * */
int OptsHandler::higher_is_better(unsigned int reqLevel, unsigned int curLevel) {
    int ret;

    if (reqLevel > curLevel) {
        ret = ADD_AND_UPDATE_REQUEST;
    } else {
        ret = PEND_REQUEST;
    }

    QLOGI("higher is better called , returning %d", ret);
    return ret;
}

int OptsHandler::lower_is_better_negative(unsigned int reqLevel_s, unsigned int curLevel_s) {
    int ret;
    int reqLevel = reqLevel_s, curLevel = curLevel_s;

    if (reqLevel < curLevel) {
        ret = ADD_AND_UPDATE_REQUEST;
    } else {
        ret = PEND_REQUEST;
    }

    QLOGI("lower is better called , returning %d", ret);
    return ret;
}

int OptsHandler::higher_is_better_negative(unsigned int reqLevel_s, unsigned int curLevel_s) {
    int ret;
    int reqLevel = reqLevel_s, curLevel = curLevel_s;

    if (reqLevel > curLevel) {
        ret = ADD_AND_UPDATE_REQUEST;
    } else {
        ret = PEND_REQUEST;
    }

    QLOGI("higher is better called , returning %d", ret);
    return ret;
}

/* Return ADD_AND_UPDATE_REQUEST if reqVal is lower than curVal
 * Return PEND_REQUEST if reqVal is less or equal to curVal
 * */
int OptsHandler::lower_is_better(unsigned int reqLevel, unsigned int curLevel) {
    int ret;

    if (reqLevel < curLevel) {
        ret = ADD_AND_UPDATE_REQUEST;
    } else {
        ret = PEND_REQUEST;
    }

    QLOGI("lower_is_better called, returning %d", ret);
    return ret;
}

/* Return ADD_AND_UPDATE_REQUEST if parsed reqVal is lower than curVal
 * Return PEND_REQUEST if parsed reqVal is less or equal to curVal
 * */
int OptsHandler::migrate_lower_is_better(unsigned int reqLevel, unsigned int curLevel) {
    int ret;

    short reqUp_val = (short) (reqLevel >> 16);
    short reqDown_val = (short) reqLevel;
    short curUp_val = (short) (curLevel >> 16);
    short curDown_val = (short) curLevel;

    if (reqUp_val < curUp_val && reqDown_val < curDown_val) {
        ret = ADD_AND_UPDATE_REQUEST;
    } else {
        ret = PEND_REQUEST;
    }

    QLOGI("migrate_lower_is_better called, returning %d", ret);
    return ret;
}

/* ALways return ADD_AND_UPDATE_REQUEST
 */
int OptsHandler::always_apply(unsigned int reqLevel, unsigned int curLevel) {

    QLOGI("always_apply called, returning %d", ADD_AND_UPDATE_REQUEST);
    return ADD_AND_UPDATE_REQUEST;
}

/* Used by SCHED_TASK_BOOST, node path should be /proc/<tid>/sched_boost
 * use this help to record SCHED_TASK_BOOST perflock in order by tid.
 */
int OptsHandler::add_in_order(unsigned int reqLevel, unsigned int curLevel) {
    int ret;

    if (reqLevel == curLevel) {
        ret = EQUAL_ADD_IN_ORDER;
    } else if (reqLevel > curLevel) {
        ret = ADD_IN_ORDER;
    } else {
        ret = PEND_ADD_IN_ORDER;
    }

    QLOGI("add_in_order called, returning %d", ret);
    return ret;
}

/* Used by SCHED_ENABLE_TASK_BOOST_RENDERTHREAD & SCHED_DISABLE_TASK_BOOST_RENDERTHREAD,
 * node path is /proc/<tid>/sched_boost
 */
int OptsHandler::add_in_order_fps_based_taskboost(unsigned int reqLevel, unsigned int curLevel)
{
    QLOGI("add_in_order_fps_based_taskboost: Returning ADD_IN_ORDER");
    return ADD_IN_ORDER;
}

/*
 * Obtains all migration values within a node except that at @index.
 * @index: Index of supplied migration value.
 * @mig_val: Migration value supplied.
 * @err: Used for error checking.
 * @acqval: Output buffer where parsed values are stored.
 * @inbuf: Path where values are parsed from.
 */
int OptsHandler::parseMultiValNode(const uint mig_val, char *acqval,
                                const char *inbuf, int core_start, int core_end) {
    int rc = -1;
    char *pos;
    char node_buff[NODE_MAX];
    const size_t kmax_mig_len = 10; /* max migration value is 10 digits */
    uint i = 0;

    if (inbuf)
        FREAD_STR(inbuf, node_buff, NODE_MAX, rc);

    if (!inbuf || rc < 0) {
        QLOGE("Failed to read %s", inbuf);

        return FAILED;
    }

    char *val = strtok_r(node_buff, "\t", &pos);

    if (!val) {
        QLOGE("Failed to parse migration values");
        return FAILED;
    }

    if (i >= core_start && i <= core_end) {
        snprintf(acqval, NODE_MAX, "%u", mig_val);
        rc = 0;
    }
    else
        strlcpy(acqval, val, NODE_MAX);
    i++;

    while (val = strtok_r(nullptr, "\t", &pos)) {

        strlcat(acqval, "\t", NODE_MAX);

        if(i >= core_start && i <= core_end) {
            char val_str[kmax_mig_len + 1];
            snprintf(val_str, kmax_mig_len + 1, "%u", mig_val);
            strlcat(acqval, val_str, NODE_MAX);
            rc = 0;
        }
        else
            strlcat(acqval, val, NODE_MAX);
        i++;
    }
    return rc;
}

/*
 * Obtains all migration values within a node except that at @index.
 * @index: Index of supplied migration value.
 * @mig_val: Migration value supplied.
 * @err: Used for error checking.
 * @acqval: Output buffer where parsed values are stored.
 * @inbuf: Path where values are parsed from.
 */
int OptsHandler::parse_mig_vals(uint cluster, const uint mig_val, char *acqval,
                                const char *inbuf) {
    int rc = -1;
    char node_buff[NODE_MAX];
    const size_t kmax_mig_len = 4; /* max migration value is 4 digits */

    if (inbuf)
        FREAD_STR(inbuf, node_buff, NODE_MAX, rc);

    if (!inbuf || rc < 0) {
        QLOGE("Failed to read %s", inbuf);

        return -1;
    }

    char *pos;
    char *val = strtok_r(node_buff, "\t", &pos);

    if (!val) {
        QLOGE("Failed to parse migration values");

        return -1;
    }
    uint i = 0;

    int index = cluster ? cluster - 1 : cluster; // cluster 1 = index 0 and so forth

    if (i == index) {
        snprintf(acqval, NODE_MAX, "%u", mig_val);
        rc = 0;
    }
    else
        strlcpy(acqval, val, NODE_MAX);
    i++;

    while (val = strtok_r(nullptr, "\t", &pos)) {
        strlcat(acqval, "\t", NODE_MAX);

        if (i == index) {
            char val_str[kmax_mig_len + 1];
            snprintf(val_str, kmax_mig_len + 1, "%u", mig_val);
            strlcat(acqval, val_str, NODE_MAX);
            rc = 0;
        }
        else
            strlcat(acqval, val, NODE_MAX);
        i++;
    }
    return rc;
}

/**
 The value /proc/<pid>/sched_boost
 0:disable task boost
 1:task will prefer to use gold or super big core according to the cpu state
 2:task will prefer to use super big core, need un-isolate it first
 3:similar as affinity without inheritable, force task to use the max capability core
*/
int OptsHandler::sched_task_boost(Resource &r, OptsData &d) {
    int rc = FAILED;
    int idx = r.qindex;
    char node_path[NODE_MAX] = "";
    snprintf(node_path, NODE_MAX, d.sysfsnode_path[idx], r.value);
    char tmp_s[NODE_MAX] = "";
    snprintf(tmp_s, NODE_MAX, "%d", TASK_BOOST_STRICT_MAX);
    rc =  update_node_param(d.node_type[idx], node_path, tmp_s, strlen(tmp_s));
    if (rc < 0)
       QLOGE("can't boost task %d", r.value);
    return rc;
}

int OptsHandler::sched_reset_task_boost(Resource &r, OptsData &d) {
    int rc = FAILED;
    int idx = r.qindex;
    char node_path[NODE_MAX] = "";
    snprintf(node_path, NODE_MAX, d.sysfsnode_path[idx], r.value);
    char tmp_s[NODE_MAX] = "";
    snprintf(tmp_s, NODE_MAX, "%d", 0);
    rc =  update_node_param(d.node_type[idx], node_path, tmp_s, strlen(tmp_s));
    if (rc < 0)
       QLOGE("can't reset task %d", r.value);
    else
       QLOGI("Reset task %d", r.value);
    return rc;
}

/* Apply task boost to top-app's render thread tid */
int OptsHandler::sched_enable_task_boost_renderthread(Resource &r, OptsData &d) {
    int rc = FAILED;
    int idx = r.qindex;
    char node_path[NODE_MAX] = "";
    snprintf(node_path, NODE_MAX, d.sysfsnode_path[idx], r.value);
    char tmp_s[NODE_MAX] = "";
    snprintf(tmp_s, NODE_MAX, "%d", TASK_BOOST_ON_MID);
    rc =  update_node_param(d.node_type[idx], node_path, tmp_s, strlen(tmp_s));
    if (rc < 0)
       QLOGE("can't boost task %d", r.value);
    else
       QLOGI("Boosted task %d", r.value);
    return rc;
}

int OptsHandler::gpu_is_app_fg(Resource &r, OptsData &d) {
    int rc = FAILED;
    int idx = r.qindex;
    char tmp_s[NODE_MAX] = "";
    char fg_node_path[NODE_MAX] = "";

    snprintf(fg_node_path, NODE_MAX, d.sysfsnode_path[idx], r.value);
    if (access(fg_node_path, F_OK) != -1) {
       snprintf(tmp_s, NODE_MAX, "%s", "foreground");
       rc =  update_node_param(d.node_type[idx], fg_node_path, tmp_s, strlen(tmp_s));
       if (rc < 0)
          QLOGE("Failed to update %s with %s return value %d",fg_node_path,tmp_s,rc);
       else
          QLOGI("perf_lock_acq: updated %s with %s return value %d",fg_node_path,tmp_s,rc);
    }
    return rc;
}

int OptsHandler::gpu_is_app_bg(Resource &r, OptsData &d) {
     int rc = FAILED;
     int idx = r.qindex;
     char tmp_s[NODE_MAX] = "";
     char bg_node_path[NODE_MAX] = "";

     snprintf(bg_node_path, NODE_MAX, d.sysfsnode_path[idx], r.value);
     if (access(bg_node_path, F_OK) != -1) {
        snprintf(tmp_s, NODE_MAX, "%s", "background");
        rc =  update_node_param(d.node_type[idx], bg_node_path, tmp_s, strlen(tmp_s));
        if (rc < 0)
           QLOGE("Failed to update %s with %s return value %d",bg_node_path,tmp_s,rc);
        else
           QLOGI("perf_lock_acq: updated %s with %s return value %d",bg_node_path,tmp_s,rc);
     }
     return rc;
}

/* Set sched sysfs node: /proc/<pid>/sched_low_latency
   Boost specific low latency tasks to ensure they get into the runqueue
   as fast as possible.
*/
int OptsHandler::sched_low_latency(Resource &r, OptsData &d) {
    int rc = FAILED;
    int idx = r.qindex;
    char node_path[NODE_MAX] = "";
    snprintf(node_path, NODE_MAX, d.sysfsnode_path[idx], r.value);
    char tmp_s[NODE_MAX] = "";
    snprintf(tmp_s, NODE_MAX, "%d", 1);
    rc =  update_node_param(d.node_type[idx], node_path, tmp_s, strlen(tmp_s));
    if (rc < 0)
       QLOGE("can't set low latency for tid %d", r.value);
    return rc;
}

int OptsHandler::sched_reset_low_latency(Resource &r, OptsData &d) {
    int rc = FAILED;
    int idx = r.qindex;
    char node_path[NODE_MAX] = "";
    snprintf(node_path, NODE_MAX, d.sysfsnode_path[idx], r.value);
    char tmp_s[NODE_MAX] = "";
    snprintf(tmp_s, NODE_MAX, "%d", 0);
    rc =  update_node_param(d.node_type[idx], node_path, tmp_s, strlen(tmp_s));
    if (rc < 0)
       QLOGE("can't reset low latency for tid %d", r.value);
    else
       QLOGI("Reset low latency for tid %d", r.value);
    return rc;
}


int OptsHandler::set_pid_affine(Resource &r, OptsData &d) {
    FILE *fgSet;
    char buff[NODE_MAX];
    int rc = 0;
    int len = 0;
    int cpu = -1, cluster = -1;
    int startCpu = -1, endCpu  = -1;
    Target &t = Target::getCurTarget();
    if (d.hwcPid != 0) {
        fgSet = fopen(FOREGROUND_TASK_NODE, "a+");
        if (fgSet == NULL) {
            QLOGE("Cannot open/create foreground cgroup file\n");
            return 0;
        }
        QLOGI("writig hwc pid:%d on node:%s\n", d.hwcPid, FOREGROUND_TASK_NODE);
        snprintf(buff, NODE_MAX, "%d", d.hwcPid);
        len = strlen(buff);
        if (len >= (NODE_MAX-1)) {
            fclose(fgSet);
            return 0;
        }
        buff[len+1] = '\0';
        rc = fwrite(buff, sizeof(char), len+1, fgSet);
        fclose(fgSet);
    }
    if (d.sfPid != 0) {
        fgSet = fopen(FOREGROUND_TASK_NODE, "a+");
        if (fgSet == NULL) {
            QLOGE("Cannot open/create foreground cgroup file\n");
            return 0;
        }
        memset(buff, 0, NODE_MAX);
        QLOGI("writig sf pid:%d on node:%s\n", d.sfPid, FOREGROUND_TASK_NODE);
        snprintf(buff, NODE_MAX, "%d", d.sfPid);
        len = strlen(buff);
        if (len >= (NODE_MAX-1)) {
            fclose(fgSet);
            return 0;
        }
        buff[len+1] = '\0';
        rc = fwrite(buff, sizeof(char), len+1, fgSet);
        fclose(fgSet);
    }
    cpu_set_t set;

    CPU_ZERO(&set);
    //setting affinity to Gold Cluster
    cpu = t.getFirstCoreOfPerfCluster();
    if (cpu < 0) {
        QLOGE("Couldn't find perf cluster");
        return FAILED;
    }
    cluster = t.getClusterForCpu(cpu, startCpu, endCpu);
    if ((startCpu < 0) || (endCpu < 0) || (cluster < 0)) {
        QLOGE("Could not find a cluster corresponding the core %d", cpu);
        return FAILED;
    }
    for (int  i = startCpu; i <= endCpu; i++) {
        CPU_SET(i, &set);
    }
    rc = sched_setaffinity(d.hwcPid, sizeof(cpu_set_t), &set);
    rc = sched_setaffinity(d.sfPid, sizeof(cpu_set_t), &set);

    return SUCCESS;
}

int OptsHandler::reset_pid_affine(Resource &r, OptsData &d) {
    FILE *pFile;
    char buff[NODE_MAX];
    int rc = 0;
    int len = 0;
    if (d.hwcPid != 0) {
        pFile = fopen(SYSBG_TASK_NODE, "a+");
        if (pFile == NULL) {
            QLOGE("Cannot open/create system background cgroup file\n");
            return 0;
        }
        QLOGI("writig hwc pid:%d on node:%s\n", d.hwcPid, FOREGROUND_TASK_NODE);
        snprintf(buff, NODE_MAX, "%d", d.hwcPid);
        len = strlen(buff);
        if (len >= (NODE_MAX-1)) {
            fclose(pFile);
            return 0;
        }
        buff[len+1] = '\0';
        rc = fwrite(buff, sizeof(char), len+1, pFile);
        fclose(pFile);
    }
    if (d.sfPid != 0) {
        pFile = fopen(SYSBG_TASK_NODE, "a+");
        if (pFile == NULL) {
            QLOGE("Cannot open/create system background cgroup file\n");
            return 0;
        }
        memset(buff, 0, NODE_MAX);
        QLOGI("writig sf pid:%d on node:%s\n", d.sfPid, FOREGROUND_TASK_NODE);
        snprintf(buff, NODE_MAX, "%d", d.sfPid);
        len = strlen(buff);
        if (len >= (NODE_MAX-1)) {
            fclose(pFile);
            return 0;
        }
        buff[len+1] = '\0';
        rc = fwrite(buff, sizeof(char), len+1, pFile);
        fclose(pFile);
    }
    return SUCCESS;
}

int OptsHandler::handle_fps_hyst(Resource &r, OptsData &d) {
    unsigned int reqVal = r.value;

    if (reqVal == MAX_LVL) { /* perf_lock_rel */
        QLOGI("OptsHandler::handle_fps_hyst perf_lock_rel reqVal: %d", reqVal);
        d.set_fps_hyst_time(-1.0f);
    }
    else {  /* perf_lock_acq */
        QLOGI("OptsHandler::handle_fps_hyst perf_lock_acq reqVal: %d", reqVal);
        d.set_fps_hyst_time(reqVal);
    }
    return 0;
}
