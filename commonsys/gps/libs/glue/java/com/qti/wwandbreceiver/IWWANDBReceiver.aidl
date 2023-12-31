/* ======================================================================
*  Copyright (c) 2017, 2020 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wwandbreceiver;

import java.com.qti.wwandbreceiver.IWWANDBReceiverResponseListener;
import java.com.qti.wwandbreceiver.BSLocationData;
import java.com.qti.wwandbreceiver.BSSpecialInfo;
import java.util.List;
import android.app.PendingIntent;

interface IWWANDBReceiver {

    boolean registerResponseListener(in IWWANDBReceiverResponseListener callback);

    void requestBSList (in int expireInDays);

    void pushWWANDB (in List<BSLocationData> locationData,
                    in List<BSSpecialInfo> specialData,
                    in int daysValid);

    void removeResponseListener(in IWWANDBReceiverResponseListener callback);
    boolean registerResponseListenerExt(in IWWANDBReceiverResponseListener callback,
                                        in PendingIntent notifyIntent);
}
