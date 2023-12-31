/*
 * Copyright (c) 2016-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.qti.server.wigig.p2p;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.net.DhcpResultsParcelable;
import android.net.InterfaceConfiguration;
import android.net.InetAddresses;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.net.NetworkInfo;
import android.net.NetworkUtils;
import android.net.ip.IpClientUtil;
import android.net.ip.IIpClient;
import android.net.ip.IpClientCallbacks;
import android.net.shared.ProvisioningConfiguration;
import com.qualcomm.qti.wigig.IActionListener;
import com.qualcomm.qti.wigig.WpsInfo;
import com.qualcomm.qti.wigig.p2p.IPeerListListener;
import com.qualcomm.qti.wigig.p2p.IConnectionInfoListener;
import com.qualcomm.qti.wigig.p2p.IGroupInfoListener;
import com.qualcomm.qti.wigig.p2p.IPersistentGroupInfoListener;
import com.qualcomm.qti.wigig.p2p.IHandoverMessageListener;
import com.qualcomm.qti.wigig.p2p.WifiP2pConfig;
import com.qualcomm.qti.wigig.p2p.WifiP2pDevice;
import com.qualcomm.qti.wigig.p2p.WifiP2pDeviceList;
import com.qualcomm.qti.wigig.p2p.WifiP2pGroup;
import com.qualcomm.qti.wigig.p2p.WifiP2pGroupList;
import com.qualcomm.qti.wigig.p2p.WifiP2pGroupList.GroupDeleteListener;
import com.qualcomm.qti.wigig.p2p.WifiP2pInfo;
import com.qualcomm.qti.wigig.p2p.WifiP2pProvDiscEvent;
import com.qualcomm.qti.wigig.p2p.WifiP2pWfdInfo;
import com.qualcomm.qti.wigig.p2p.nsd.IServiceResponseListener;
import com.qualcomm.qti.wigig.p2p.nsd.IDnsSdServiceResponseListener;
import com.qualcomm.qti.wigig.p2p.nsd.IDnsSdTxtRecordListener;
import com.qualcomm.qti.wigig.p2p.nsd.IUpnpServiceResponseListener;
import com.qualcomm.qti.wigig.p2p.nsd.WifiP2pServiceInfo;
import com.qualcomm.qti.wigig.p2p.nsd.WifiP2pDnsSdServiceInfo;
import com.qualcomm.qti.wigig.p2p.nsd.WifiP2pServiceRequest;
import com.qualcomm.qti.wigig.p2p.nsd.WifiP2pServiceResponse;
import com.qualcomm.qti.wigig.p2p.nsd.WifiP2pDnsSdServiceResponse;
import com.qualcomm.qti.wigig.p2p.nsd.WifiP2pUpnpServiceResponse;
import com.qualcomm.qti.wigig.p2p.IWigigP2pManager;
import com.qualcomm.qti.wigig.p2p.WigigP2pManager;
import android.os.Binder;
import android.os.Bundle;
import android.os.ConditionVariable;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.INetworkManagementService;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Slog;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.TextView;

import com.android.internal.util.AsyncChannel;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
import com.android.server.SystemService;
import com.qualcomm.qti.server.wigig.WigigMonitor;
import com.qualcomm.qti.server.wigig.WigigNative;
import com.qualcomm.qti.server.wigig.WigigStateMachine;
import com.qualcomm.qti.server.wigig.util.ExternalCallbackTracker;
import com.qualcomm.qti.server.wigig.WLog;
import com.qualcomm.qti.wigig.WigigManager;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;


/**
 * WigigP2pService includes a state machine to perform WiGig p2p operations. Applications
 * communicate with this service to issue device discovery and connectivity requests
 * through the WigigP2pManager interface. The state machine communicates with the wigig
 * driver through wpa_supplicant and handles the event responses through WigigMonitor.
 *
 * Note that the term Wigig when used without a p2p suffix refers to the client mode
 * of Wigig operation
 * @hide
 */
public class WigigP2pServiceImpl extends IWigigP2pManager.Stub {
    private static final String TAG = "WigigP2pService";
    // TODO restore default to false before production
    private static boolean DBG = true;
    private static final String NETWORKTYPE = "WIGIG_P2P";

    // for getting resources for WIFI dialogs
    private static final String WIFI_RES_PACKAGE = "com.android.wifi.resources";

    private Context mWifiResContext;
    private Resources mWifiRes;

    private Context mContext;
    private String mInterface;

    INetworkManagementService mNwService;
    private volatile IIpClient mIpClient;
    private DhcpResultsParcelable mDhcpResults;

    private P2pStateMachine mP2pStateMachine;
    private AsyncChannel mReplyChannel = new AsyncChannel();
    private AsyncChannel mWigigChannel;

    private static final Boolean JOIN_GROUP = true;
    private static final Boolean FORM_GROUP = false;

    private static final Boolean RELOAD = true;
    private static final Boolean NO_RELOAD = false;

    /* Two minutes comes from the wpa_supplicant setting */
    private static final int GROUP_CREATING_WAIT_TIME_MS = 120 * 1000;
    private static int mGroupCreatingTimeoutIndex = 0;

    private static final int DISABLE_P2P_WAIT_TIME_MS = 5 * 1000;
    private static int mDisableP2pTimeoutIndex = 0;

    /* Set a two minute discover timeout to avoid STA scans from being blocked */
    private static final int DISCOVER_TIMEOUT_S = 120;

    /* Idle time after a peer is gone when the group is torn down */
    private static final int GROUP_IDLE_TIME_S = 10;

    private static final int BASE = WigigManager.BASE_WIGIG_P2P_SERVICE;

    /* Delayed message to timeout group creation */
    public static final int GROUP_CREATING_TIMED_OUT        =   BASE + 1;

    /* User accepted a peer request */
    private static final int PEER_CONNECTION_USER_ACCEPT    =   BASE + 2;
    /* User rejected a peer request */
    private static final int PEER_CONNECTION_USER_REJECT    =   BASE + 3;
    /* User wants to disconnect wigig in favour of p2p */
    private static final int DROP_WIGIG_USER_ACCEPT          =   BASE + 4;
    /* User wants to keep his wifi connection and drop p2p */
    private static final int DROP_WIGIG_USER_REJECT          =   BASE + 5;
    /* Delayed message to timeout p2p disable */
    public static final int DISABLE_P2P_TIMED_OUT           =   BASE + 6;


    /* Commands to the WigigStateMachine */
    public static final int P2P_CONNECTION_CHANGED          =   BASE + 11;

    /* These commands are used to temporarily disconnect wigig when we detect
     * a frequency conflict which would make it impossible to have with p2p
     * and wigig active at the same time.
     *
     * If the user chooses to disable wigig temporarily, we keep wigig disconnected
     * until the p2p connection is done and terminated at which point we will
     * bring back wigig up
     *
     * DISCONNECT_WIGIG_REQUEST
     *      msg.arg1 = 1 enables temporary disconnect and 0 disables it.
     */
    public static final int DISCONNECT_WIGIG_REQUEST         =   BASE + 12;
    public static final int DISCONNECT_WIGIG_RESPONSE        =   BASE + 13;

    public static final int SET_MIRACAST_MODE               =   BASE + 14;

    // During dhcp (and perhaps other times) we can't afford to drop packets
    // but Discovery will switch our channel enough we will.
    //   msg.arg1 = ENABLED for blocking, DISABLED for resumed.
    //   msg.arg2 = msg to send when blocked
    //   msg.obj  = StateMachine to send to when blocked
    public static final int BLOCK_DISCOVERY                 =   BASE + 15;

    // Messages for interaction with IpClient.
    private static final int IPM_PRE_DHCP_ACTION            =   BASE + 30;
    private static final int IPM_POST_DHCP_ACTION           =   BASE + 31;
    private static final int IPM_DHCP_RESULTS               =   BASE + 32;
    private static final int IPM_PROVISIONING_SUCCESS       =   BASE + 33;
    private static final int IPM_PROVISIONING_FAILURE       =   BASE + 34;

    public static final int ENABLED                         = 1;
    public static final int DISABLED                        = 0;

    private static final int IPCLIENT_TIMEOUT_MS = 10_000;

    private final boolean mP2pSupported;

    private WifiP2pDevice mThisDevice = new WifiP2pDevice();

    /* When a group has been explicitly created by an app, we persist the group
     * even after all clients have been disconnected until an explicit remove
     * is invoked */
    private boolean mAutonomousGroup;

    /* Invitation to join an existing p2p group */
    private boolean mJoinExistingGroup;

    /* Track whether we are in p2p discovery. This is used to avoid sending duplicate
     * broadcasts
     */
    private boolean mDiscoveryStarted;
    /* Track whether servcice/peer discovery is blocked in favor of other wigig actions
     * (notably dhcp)
     */
    private boolean mDiscoveryBlocked;

    /*
     * remember if we were in a scan when it had to be stopped
     */
    private boolean mDiscoveryPostponed = false;

    private NetworkInfo mNetworkInfo;

    private boolean mTemporarilyDisconnectedWigig = false;

    private boolean mRadioRequested = false;

    /* The transaction Id of service discovery request */
    private byte mServiceTransactionId = 0;

    /* Service discovery request ID of wpa_supplicant.
     * null means it's not set yet. */
    private String mServiceDiscReqId;

    /* clients(application) information list. */
    private HashMap<Messenger, ClientInfo> mClientInfoList = new HashMap<Messenger, ClientInfo>();

    private final ExternalCallbackTracker<IActionListener> mProcessingActionListeners;
    private final ExternalCallbackTracker<IPeerListListener> mProcessingPeerListListeners;
    private final ExternalCallbackTracker<IConnectionInfoListener> mProcessingConnectionInfoListeners;
    private final ExternalCallbackTracker<IGroupInfoListener> mProcessingGroupInfoListeners;
    private final ExternalCallbackTracker<IPersistentGroupInfoListener> mProcessingPersistentGroupInfoListeners;
    private final ExternalCallbackTracker<IHandoverMessageListener> mProcessingHandoverMessageListeners;

    private int mCallbackIdentifier = 0;

    /* Is chosen as a unique address to avoid conflict with
       the ranges defined in Tethering.java */
    private static final String SERVER_ADDRESS = "172.16.52.1";
    private static final String[] SERVER_DHCP_RANGE = { "172.16.52.2", "172.16.52.254" };

    /**
     * Error code definition.
     * see the Table.8 in the WiFi Direct specification for the detail.
     */
    public static enum P2pStatus {
        /* Success. */
        SUCCESS,

        /* The target device is currently unavailable. */
        INFORMATION_IS_CURRENTLY_UNAVAILABLE,

        /* Protocol error. */
        INCOMPATIBLE_PARAMETERS,

        /* The target device reached the limit of the number of the connectable device.
         * For example, device limit or group limit is set. */
        LIMIT_REACHED,

        /* Protocol error. */
        INVALID_PARAMETER,

        /* Unable to accommodate request. */
        UNABLE_TO_ACCOMMODATE_REQUEST,

        /* Previous protocol error, or disruptive behavior. */
        PREVIOUS_PROTOCOL_ERROR,

        /* There is no common channels the both devices can use. */
        NO_COMMON_CHANNEL,

        /* Unknown p2p group. For example, Device A tries to invoke the previous persistent group,
         *  but device B has removed the specified credential already. */
        UNKNOWN_P2P_GROUP,

        /* Both p2p devices indicated an intent of 15 in group owner negotiation. */
        BOTH_GO_INTENT_15,

        /* Incompatible provisioning method. */
        INCOMPATIBLE_PROVISIONING_METHOD,

        /* Rejected by user */
        REJECTED_BY_USER,

        /* Unknown error */
        UNKNOWN;

        public static P2pStatus valueOf(int error) {
            switch(error) {
            case 0 :
                return SUCCESS;
            case 1:
                return INFORMATION_IS_CURRENTLY_UNAVAILABLE;
            case 2:
                return INCOMPATIBLE_PARAMETERS;
            case 3:
                return LIMIT_REACHED;
            case 4:
                return INVALID_PARAMETER;
            case 5:
                return UNABLE_TO_ACCOMMODATE_REQUEST;
            case 6:
                return PREVIOUS_PROTOCOL_ERROR;
            case 7:
                return NO_COMMON_CHANNEL;
            case 8:
                return UNKNOWN_P2P_GROUP;
            case 9:
                return BOTH_GO_INTENT_15;
            case 10:
                return INCOMPATIBLE_PROVISIONING_METHOD;
            case 11:
                return REJECTED_BY_USER;
            default:
                return UNKNOWN;
            }
        }
    }

    /**
     * Handles client connections
     */
    private class ClientHandler extends Handler {

        ClientHandler(android.os.Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
              case WigigP2pManager.SET_DEVICE_NAME:
              case WigigP2pManager.SET_WFD_INFO:
              case WigigP2pManager.DISCOVER_PEERS:
              case WigigP2pManager.STOP_DISCOVERY:
              case WigigP2pManager.CONNECT:
              case WigigP2pManager.CANCEL_CONNECT:
              case WigigP2pManager.CREATE_GROUP:
              case WigigP2pManager.REMOVE_GROUP:
              case WigigP2pManager.START_LISTEN:
              case WigigP2pManager.STOP_LISTEN:
              case WigigP2pManager.SET_CHANNEL:
              case WigigP2pManager.START_WPS:
              case WigigP2pManager.ADD_LOCAL_SERVICE:
              case WigigP2pManager.REMOVE_LOCAL_SERVICE:
              case WigigP2pManager.CLEAR_LOCAL_SERVICES:
              case WigigP2pManager.DISCOVER_SERVICES:
              case WigigP2pManager.ADD_SERVICE_REQUEST:
              case WigigP2pManager.REMOVE_SERVICE_REQUEST:
              case WigigP2pManager.CLEAR_SERVICE_REQUESTS:
              case WigigP2pManager.REQUEST_PEERS:
              case WigigP2pManager.REQUEST_CONNECTION_INFO:
              case WigigP2pManager.REQUEST_GROUP_INFO:
              case WigigP2pManager.DELETE_PERSISTENT_GROUP:
              case WigigP2pManager.REQUEST_PERSISTENT_GROUP_INFO:
                mP2pStateMachine.sendMessage(Message.obtain(msg));
                break;
              default:
                WLog.v(TAG, "ClientHandler.handleMessage ignoring msg=" + msg);
                break;
            }
        }
    }
    private ClientHandler mClientHandler;

    public WigigP2pServiceImpl(Context context) {
        mContext = context;

        //STOPSHIP: get this from native side
        mInterface = "p2p-dev-wigig0";
        mNetworkInfo = new NetworkInfo(ConnectivityManager.TYPE_WIFI_P2P, 0, NETWORKTYPE, "");

        mP2pSupported = mContext.getPackageManager().hasSystemFeature(
                PackageManager.FEATURE_WIFI_DIRECT);

        try {
            mWifiResContext = mContext.createPackageContext(WIFI_RES_PACKAGE,
                Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
        } catch (Exception e) {
            Slog.e(TAG, "exception in createPackageContext: " + e);
            // this will cause Wigig services to fail loading
            throw new RuntimeException(e);
        }

        mWifiRes = mWifiResContext.getResources();

        mThisDevice.primaryDeviceType = mWifiRes.getString(
                getWifiResId("string", "config_wifi_p2p_device_type"));

        HandlerThread wigigP2pThread = new HandlerThread("WigigP2pService");
        wigigP2pThread.start();
        mClientHandler = new ClientHandler(wigigP2pThread.getLooper());

        mProcessingActionListeners = new ExternalCallbackTracker<>(mClientHandler);
        mProcessingPeerListListeners = new ExternalCallbackTracker<>(mClientHandler);
        mProcessingConnectionInfoListeners = new ExternalCallbackTracker<>(mClientHandler);
        mProcessingGroupInfoListeners = new ExternalCallbackTracker<>(mClientHandler);
        mProcessingPersistentGroupInfoListeners = new ExternalCallbackTracker<>(mClientHandler);
        mProcessingHandoverMessageListeners = new ExternalCallbackTracker<>(mClientHandler);

        mP2pStateMachine = new P2pStateMachine(TAG, wigigP2pThread.getLooper(), mP2pSupported);
        mP2pStateMachine.start();
    }

    public void onBootPhase(int phase) {
        if (phase == SystemService.PHASE_SYSTEM_SERVICES_READY) {
            connectivityServiceReady();
        }
    }

    public void connectivityServiceReady() {
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNwService = INetworkManagementService.Stub.asInterface(b);
    }

    private int getWifiResId(String category, String name) {
        if (mWifiRes == null) {
            Slog.e(TAG, "no WIFI resources, fail to get " + category + "." + name);
            return -1;
        }
        return mWifiRes.getIdentifier(name, category, WIFI_RES_PACKAGE);
    }


    private void enforceAccessPermission() {
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.ACCESS_WIFI_STATE,
                "WigigP2pService");
    }

    private void enforceChangePermission() {
        mContext.enforceCallingOrSelfPermission(android.Manifest.permission.CHANGE_WIFI_STATE,
                "WigigP2pService");
    }

    private void enforceConnectivityInternalPermission() {
        mContext.enforceCallingOrSelfPermission(
                android.Manifest.permission.CONNECTIVITY_INTERNAL,
                "WigigP2pService");
    }

    private int checkConnectivityInternalPermission() {
        return mContext.checkCallingOrSelfPermission(
                android.Manifest.permission.CONNECTIVITY_INTERNAL);
    }

    private int checkLocationHardwarePermission() {
        return mContext.checkCallingOrSelfPermission(
                android.Manifest.permission.LOCATION_HARDWARE);
    }

    private void enforceConnectivityInternalOrLocationHardwarePermission() {
        if (checkConnectivityInternalPermission() != PackageManager.PERMISSION_GRANTED
                && checkLocationHardwarePermission() != PackageManager.PERMISSION_GRANTED) {
            enforceConnectivityInternalPermission();
        }
    }

    private void stopIpClient() {
        if (mIpClient != null) {
            try {
                mIpClient.stop();
            } catch (RemoteException e) {
                Slog.e(TAG, "Error stopping IpClient", e);
            }
            mIpClient = null;
        }
        mDhcpResults = null;
    }

    private class IpClientCallback extends IpClientCallbacks {
        private final ConditionVariable mWaitForCreationCv = new ConditionVariable(false);

        @Override
        public void onIpClientCreated(IIpClient ipClient) {
            mIpClient = ipClient;
            mWaitForCreationCv.open();
        }

        @Override
        public void onPreDhcpAction() {
            mP2pStateMachine.sendMessage(IPM_PRE_DHCP_ACTION);
        }
        @Override
        public void onPostDhcpAction() {
            mP2pStateMachine.sendMessage(IPM_POST_DHCP_ACTION);
        }
        @Override
        public void onNewDhcpResults(DhcpResultsParcelable dhcpResults) {
            mP2pStateMachine.sendMessage(IPM_DHCP_RESULTS, dhcpResults);
        }
        @Override
        public void onProvisioningSuccess(LinkProperties newLp) {
            mP2pStateMachine.sendMessage(IPM_PROVISIONING_SUCCESS);
        }
        @Override
        public void onProvisioningFailure(LinkProperties newLp) {
            mP2pStateMachine.sendMessage(IPM_PROVISIONING_FAILURE);
        }

        boolean awaitCreation() {
            return mWaitForCreationCv.block(IPCLIENT_TIMEOUT_MS);
        }
    }

    private void startIpClient(String ifname) {
        stopIpClient();

        IpClientCallback clientCallback = new IpClientCallback();
        IpClientUtil.makeIpClient(mContext, ifname, clientCallback);
        if (!clientCallback.awaitCreation()) {
            Slog.e(TAG, "Timeout waiting for IpClient");
        }

        final ProvisioningConfiguration config =
                new ProvisioningConfiguration.Builder()
                          .withoutIPv6()
                          .withoutIpReachabilityMonitor()
                          .withPreDhcpAction(30 * 1000)
                          .withProvisioningTimeoutMs(36 * 1000)
                          .build();
        try {
            mIpClient.startProvisioning(config.toStableParcelable());
        } catch (RemoteException e) {
            Slog.e(TAG, "Error starting provisioning for IpClient", e);
        }
    }

    /**
     * Get a reference to handler. This is used by a client to establish
     * an AsyncChannel communication with WigigP2pService
     */
    public Messenger getMessenger() {
        enforceAccessPermission();
        enforceChangePermission();
        return new Messenger(mClientHandler);
    }

    /**
     * Get a reference to handler. This is used by a WigigStateMachine to establish
     * an AsyncChannel communication with P2pStateMachine
     * @hide
     */
    public Messenger getP2pStateMachineMessenger() {
        enforceConnectivityInternalOrLocationHardwarePermission();
        enforceAccessPermission();
        enforceChangePermission();
        return new Messenger(mP2pStateMachine.getHandler());
    }

    /** This is used to provide information to drivers to optimize performance depending
     * on the current mode of operation.
     * 0 - disabled
     * 1 - source operation
     * 2 - sink operation
     *
     * As an example, the driver could reduce the channel dwell time during scanning
     * when acting as a source or sink to minimize impact on miracast.
     */
    public void setMiracastMode(int mode) {
        enforceConnectivityInternalPermission();
        mP2pStateMachine.sendMessage(SET_MIRACAST_MODE, mode);
    }

    private void sendActionListenerFailure(int callbackIdentifier, int reason) {
        IActionListener actionListener;
        synchronized (mProcessingActionListeners) {
            actionListener = mProcessingActionListeners.remove(callbackIdentifier);
        }
        if (actionListener != null) {
            try {
                actionListener.onFailure(reason);
            } catch (RemoteException e) {
                // no-op (client may be dead, nothing to be done)
            }
        }
    }

    private void sendActionListenerSuccess(int callbackIdentifier) {
        IActionListener actionListener;
        synchronized (mProcessingActionListeners) {
            actionListener = mProcessingActionListeners.remove(callbackIdentifier);
        }
        if (actionListener != null) {
            try {
                actionListener.onSuccess();
            } catch (RemoteException e) {
                // no-op (client may be dead, nothing to be done)
            }
        }
    }

    private void sendPeersAvailable(int callbackIdentifier, WifiP2pDeviceList peers) {
        IPeerListListener peerListListener;
        synchronized (mProcessingPeerListListeners) {
            peerListListener = mProcessingPeerListListeners.remove(callbackIdentifier);
        }
        if (peerListListener != null) {
            try {
                peerListListener.onPeersAvailable(peers);
            } catch (RemoteException e) {
                // no-op (client may be dead, nothing to be done)
            }
        }
    }

    private void sendConnectionInfoAvailable(int callbackIdentifier, WifiP2pInfo info) {
        IConnectionInfoListener connectionInfoListener;
        synchronized (mProcessingConnectionInfoListeners) {
            connectionInfoListener = mProcessingConnectionInfoListeners.remove(callbackIdentifier);
        }
        if (connectionInfoListener != null) {
            try {
                connectionInfoListener.onConnectionInfoAvailable(info);
            } catch (RemoteException e) {
                // no-op (client may be dead, nothing to be done)
            }
        }
    }

    private void sendGroupInfoAvailable(int callbackIdentifier, WifiP2pGroup group) {
        IGroupInfoListener groupInfoListener;
        synchronized (mProcessingGroupInfoListeners) {
            groupInfoListener = mProcessingGroupInfoListeners.remove(callbackIdentifier);
        }
        if (groupInfoListener != null) {
            try {
                groupInfoListener.onGroupInfoAvailable(group);
            } catch (RemoteException e) {
                // no-op (client may be dead, nothing to be done)
            }
        }
    }

    private void sendPersistentGroupInfoAvailable(int callbackIdentifier, WifiP2pGroupList groups) {
        IPersistentGroupInfoListener persistentGroupInfoListener;
        synchronized (mProcessingPersistentGroupInfoListeners) {
            persistentGroupInfoListener = mProcessingPersistentGroupInfoListeners.remove(callbackIdentifier);
        }
        if (persistentGroupInfoListener != null) {
            try {
                persistentGroupInfoListener.onPersistentGroupInfoAvailable(groups);
            } catch (RemoteException e) {
                // no-op (client may be dead, nothing to be done)
            }
        }
    }

    private void sendHandoverMessageAvailable(int callbackIdentifier, String handoverMessage) {
        IHandoverMessageListener handoverMessageListener;
        synchronized (mProcessingHandoverMessageListeners) {
            handoverMessageListener = mProcessingHandoverMessageListeners.remove(callbackIdentifier);
        }
        if (handoverMessageListener != null) {
            try {
                handoverMessageListener.onHandoverMessageAvailable(handoverMessage);
            } catch (RemoteException e) {
                // no-op (client may be dead, nothing to be done)
            }
        }
    }

    public void discoverPeers(IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.DISCOVER_PEERS, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void stopPeerDiscovery(IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.STOP_DISCOVERY, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void connect(WifiP2pConfig config, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.CONNECT, -1, callbackIdentifier, config);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void cancelConnect(IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.CANCEL_CONNECT, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void createGroup(IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.CREATE_GROUP, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void removeGroup(IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.REMOVE_GROUP, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void listen(boolean enable, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(enable ? WigigP2pManager.START_LISTEN : WigigP2pManager.STOP_LISTEN,
                        -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void setWigigP2pChannels(int lc, int oc, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Bundle p2pChannels = new Bundle();
            p2pChannels.putInt("lc", lc);
            p2pChannels.putInt("oc", oc);

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.SET_CHANNEL, -1, callbackIdentifier, p2pChannels);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void startWps(WpsInfo wps, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.START_WPS, -1, callbackIdentifier, wps);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void addLocalService(Messenger client, WifiP2pServiceInfo servInfo, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            if (client == null) {
                sendActionListenerFailure(callbackIdentifier, WigigP2pManager.ERROR);
                return;
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.ADD_LOCAL_SERVICE, -1, callbackIdentifier, servInfo);
            // client is for identification only, we reply through the listeners
            message.replyTo = client;
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void removeLocalService(Messenger client, WifiP2pServiceInfo servInfo, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            if (client == null) {
                sendActionListenerFailure(callbackIdentifier, WigigP2pManager.ERROR);
                return;
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.REMOVE_LOCAL_SERVICE, -1, callbackIdentifier, servInfo);
            // client is for identification only, we reply through the listeners
            message.replyTo = client;
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void clearLocalServices(Messenger client, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            if (client == null) {
                sendActionListenerFailure(callbackIdentifier, WigigP2pManager.ERROR);
                return;
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.CLEAR_LOCAL_SERVICES, -1, callbackIdentifier);
            // client is for identification only, we reply through the listeners
            message.replyTo = client;
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void setServiceResponseListener(Messenger client, IBinder binder,
            IServiceResponseListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            if (binder == null || client == null) {
                Slog.e(TAG, "setServiceResponseListener: null binder or client");
                // nothing to be done, we can't send reply
                return;
            }

            mP2pStateMachine.setServiceResponseListener(client, binder, callback);
        });
    }

    public void setDnsSdServiceResponseListener(Messenger client, IBinder binder,
            IDnsSdServiceResponseListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            if (binder == null || client == null) {
                Slog.e(TAG, "setDnsSdServiceResponseListener: null binder or client");
                // nothing to be done, we can't send reply
                return;
            }

            mP2pStateMachine.setDnsSdServiceResponseListener(client, binder, callback);
        });
    }

    public void setDnsSdTxtRecordListener(Messenger client, IBinder binder,
            IDnsSdTxtRecordListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            if (binder == null || client == null) {
                Slog.e(TAG, "setDnsSdTxtRecordListener: null binder or client");
                // nothing to be done, we can't send reply
                return;
            }

            mP2pStateMachine.setDnsSdTxtRecordListener(client, binder, callback);
        });
    }

    public void setUpnpServiceResponseListener(Messenger client, IBinder binder,
            IUpnpServiceResponseListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            if (binder == null || client == null) {
                Slog.e(TAG, "setUpnpServiceResponseListener: null binder or client");
                // nothing to be done, we can't send reply
                return;
            }

            mP2pStateMachine.setUpnpServiceResponseListener(client, binder, callback);
        });
    }

    public void discoverServices(Messenger client, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            if (client == null) {
                sendActionListenerFailure(callbackIdentifier, WigigP2pManager.ERROR);
                return;
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.DISCOVER_SERVICES, -1, callbackIdentifier);
            // client is for identification only, we reply through the listeners
            message.replyTo = client;
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void addServiceRequest(Messenger client, WifiP2pServiceRequest req, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            if (client == null) {
                sendActionListenerFailure(callbackIdentifier, WigigP2pManager.ERROR);
                return;
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.ADD_SERVICE_REQUEST, -1, callbackIdentifier, req);
            // client is for identification only, we reply through the listeners
            message.replyTo = client;
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void removeServiceRequest(Messenger client, WifiP2pServiceRequest req, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            if (client == null) {
                sendActionListenerFailure(callbackIdentifier, WigigP2pManager.ERROR);
                return;
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.REMOVE_SERVICE_REQUEST, -1, callbackIdentifier, req);
            // client is for identification only, we reply through the listeners
            message.replyTo = client;
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void clearServiceRequests(Messenger client, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            if (client == null) {
                sendActionListenerFailure(callbackIdentifier, WigigP2pManager.ERROR);
                return;
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.CLEAR_SERVICE_REQUESTS, -1, callbackIdentifier);
            // client is for identification only, we reply through the listeners
            message.replyTo = client;
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void requestPeers(IBinder binder, IPeerListListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingPeerListListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.REQUEST_PEERS, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void requestConnectionInfo(IBinder binder, IConnectionInfoListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingConnectionInfoListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.REQUEST_CONNECTION_INFO, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void requestGroupInfo(IBinder binder, IGroupInfoListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingGroupInfoListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.REQUEST_GROUP_INFO, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void setDeviceName(String devName, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            WifiP2pDevice d = new WifiP2pDevice();
            d.deviceName = devName;
            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.SET_DEVICE_NAME, -1, callbackIdentifier, d);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void setWFDInfo(WifiP2pWfdInfo wfdInfo, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.SET_WFD_INFO, -1, callbackIdentifier, wfdInfo);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void deletePersistentGroup(int netId, IBinder binder, IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.DELETE_PERSISTENT_GROUP, netId, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void requestPersistentGroupInfo(IBinder binder, IPersistentGroupInfoListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingPersistentGroupInfoListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.REQUEST_PERSISTENT_GROUP_INFO, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void getNfcHandoverRequest(IBinder binder, IHandoverMessageListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingHandoverMessageListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.GET_HANDOVER_REQUEST, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void getNfcHandoverSelect(IBinder binder, IHandoverMessageListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingHandoverMessageListeners.add(binder, callback, callbackIdentifier);
            }

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.GET_HANDOVER_SELECT, -1, callbackIdentifier);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void initiatorReportNfcHandover(String handoverSelect, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Bundle bundle = new Bundle();
            bundle.putString(WigigP2pManager.EXTRA_HANDOVER_MESSAGE, handoverSelect);

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.INITIATOR_REPORT_NFC_HANDOVER, -1, callbackIdentifier, bundle);
            mP2pStateMachine.sendMessage(message);
        });
    }

    public void responderReportNfcHandover(String handoverRequest, IBinder binder,
            IActionListener callback) {
        enforceAccessPermission();
        mClientHandler.post(() -> {
            int callbackIdentifier = ++mCallbackIdentifier;
            if (callback != null && binder != null) {
                mProcessingActionListeners.add(binder, callback, callbackIdentifier);
            }

            Bundle bundle = new Bundle();
            bundle.putString(WigigP2pManager.EXTRA_HANDOVER_MESSAGE, handoverRequest);

            Message message =
                    mP2pStateMachine.obtainMessage(WigigP2pManager.RESPONDER_REPORT_NFC_HANDOVER, -1, callbackIdentifier, bundle);
            mP2pStateMachine.sendMessage(message);
        });
    }

    @Override
    protected void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.DUMP)
                != PackageManager.PERMISSION_GRANTED) {
            pw.println("Permission Denial: can't dump WigigP2pService from from pid="
                    + Binder.getCallingPid()
                    + ", uid=" + Binder.getCallingUid());
            return;
        }
        mP2pStateMachine.dump(fd, pw, args);
        pw.println("mAutonomousGroup " + mAutonomousGroup);
        pw.println("mJoinExistingGroup " + mJoinExistingGroup);
        pw.println("mDiscoveryStarted " + mDiscoveryStarted);
        pw.println("mNetworkInfo " + mNetworkInfo);
        pw.println("mTemporarilyDisconnectedWigig " + mTemporarilyDisconnectedWigig);
        pw.println("mServiceDiscReqId " + mServiceDiscReqId);
        pw.println();

        final IIpClient ipClient = mIpClient;
        if (ipClient != null) {
            pw.println("mIpClient:");
            IpClientUtil.dumpIpClient(ipClient, fd, pw, args);
        }
    }


    /**
     * Handles interaction with WigigStateMachine
     */
    private class P2pStateMachine extends StateMachine {

        private DefaultState mDefaultState = new DefaultState();
        private P2pNotSupportedState mP2pNotSupportedState = new P2pNotSupportedState();
        private P2pDisablingState mP2pDisablingState = new P2pDisablingState();
        private P2pDisabledState mP2pDisabledState = new P2pDisabledState();
        private P2pEnablingState mP2pEnablingState = new P2pEnablingState();
        private P2pEnabledState mP2pEnabledState = new P2pEnabledState();
        // Idle is when p2p is enabled and ready but no P2P operation
        // (including discovery) is running. Any P2P operation that needs
        // the radio must request it from the Wigig state machine
        private IdleState mIdleState = new IdleState();
        // Inactive is when p2p is enabled with no connectivity
        // Discovery can run in this state
        private InactiveState mInactiveState = new InactiveState();
        // RadioInUseByWigig is when the Wigig radio is in use by a non-P2P operation such
        // as connected station. Any API that needs the radio will fail with
        // BUSY error code
        private RadioInUseByWigigState mRadioInUseByWigigState = new RadioInUseByWigigState();
        private GroupCreatingState mGroupCreatingState = new GroupCreatingState();
        private UserAuthorizingInviteRequestState mUserAuthorizingInviteRequestState
                = new UserAuthorizingInviteRequestState();
        private UserAuthorizingNegotiationRequestState mUserAuthorizingNegotiationRequestState
                = new UserAuthorizingNegotiationRequestState();
        private ProvisionDiscoveryState mProvisionDiscoveryState = new ProvisionDiscoveryState();
        private GroupNegotiationState mGroupNegotiationState = new GroupNegotiationState();
        private FrequencyConflictState mFrequencyConflictState =new FrequencyConflictState();

        private GroupCreatedState mGroupCreatedState = new GroupCreatedState();
        private UserAuthorizingJoinState mUserAuthorizingJoinState = new UserAuthorizingJoinState();
        private OngoingGroupRemovalState mOngoingGroupRemovalState = new OngoingGroupRemovalState();

        // TODO need to align with WIFI implementation, once we have updated
        // implementation for WigigNative and WigigMonitor
        private WigigNative mWigigNative = new WigigNative(mInterface);
        private WigigMonitor mWigigMonitor = new WigigMonitor(this, mWigigNative);

        private final WifiP2pDeviceList mPeers = new WifiP2pDeviceList();
        /* During a connection, supplicant can tell us that a device was lost. From a supplicant's
         * perspective, the discovery stops during connection and it purges device since it does
         * not get latest updates about the device without being in discovery state.
         *
         * From the framework perspective, the device is still there since we are connecting or
         * connected to it. so we keep these devices in a separate list, so that they are removed
         * when connection is cancelled or lost
         */
        private final WifiP2pDeviceList mPeersLostDuringConnection = new WifiP2pDeviceList();
        private final WifiP2pGroupList mGroups = new WifiP2pGroupList(null,
                new GroupDeleteListener() {
            @Override
            public void onDeleteGroup(int netId) {
                if (DBG) logd("called onDeleteGroup() netId=" + netId);
                mWigigNative.removeNetwork(netId);
                mWigigNative.saveConfig();
                sendP2pPersistentGroupsChangedBroadcast();
            }
        });
        private final WifiP2pInfo mWifiP2pInfo = new WifiP2pInfo();
        private WifiP2pGroup mGroup;

        // Saved WifiP2pConfig for an ongoing peer connection. This will never be null.
        // The deviceAddress will be an empty string when the device is inactive
        // or if it is connected without any ongoing join request
        private WifiP2pConfig mSavedPeerConfig = new WifiP2pConfig();

        P2pStateMachine(String name, Looper looper, boolean p2pSupported) {
            super(name, looper);

            addState(mDefaultState);
                addState(mP2pNotSupportedState, mDefaultState);
                addState(mP2pDisablingState, mDefaultState);
                addState(mP2pDisabledState, mDefaultState);
                addState(mP2pEnablingState, mDefaultState);
                addState(mP2pEnabledState, mDefaultState);
                    addState(mIdleState, mP2pEnabledState);
                    addState(mInactiveState, mP2pEnabledState);
                    addState(mRadioInUseByWigigState, mP2pEnabledState);
                    addState(mGroupCreatingState, mP2pEnabledState);
                        addState(mUserAuthorizingInviteRequestState, mGroupCreatingState);
                        addState(mUserAuthorizingNegotiationRequestState, mGroupCreatingState);
                        addState(mProvisionDiscoveryState, mGroupCreatingState);
                        addState(mGroupNegotiationState, mGroupCreatingState);
                        addState(mFrequencyConflictState, mGroupCreatingState);
                    addState(mGroupCreatedState, mP2pEnabledState);
                        addState(mUserAuthorizingJoinState, mGroupCreatedState);
                        addState(mOngoingGroupRemovalState, mGroupCreatedState);

            if (p2pSupported) {
                setInitialState(mP2pDisabledState);
            } else {
                setInitialState(mP2pNotSupportedState);
            }
            setLogRecSize(50);
            setLogOnlyTransitions(true);
            if (DBG) setDbg(true);
        }

    class DefaultState extends State {
        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case AsyncChannel.CMD_CHANNEL_HALF_CONNECTED:
                    if (message.arg1 == AsyncChannel.STATUS_SUCCESSFUL) {
                        if (DBG) logd("Full connection with WigigStateMachine established");
                        mWigigChannel = (AsyncChannel) message.obj;
                    } else {
                        loge("Full connection failure, error = " + message.arg1);
                        mWigigChannel = null;
                    }
                    break;

                case AsyncChannel.CMD_CHANNEL_DISCONNECTED:
                    if (message.arg1 == AsyncChannel.STATUS_SEND_UNSUCCESSFUL) {
                        loge("Send failed, client connection lost");
                    } else {
                        loge("Client connection lost with reason: " + message.arg1);
                    }
                    mWigigChannel = null;
                    break;

                case AsyncChannel.CMD_CHANNEL_FULL_CONNECTION:
                    AsyncChannel ac = new AsyncChannel();
                    ac.connect(mContext, getHandler(), message.replyTo);
                    break;
                case BLOCK_DISCOVERY:
                    mDiscoveryBlocked = (message.arg1 == ENABLED ? true : false);
                    // always reset this - we went to a state that doesn't support discovery so
                    // it would have stopped regardless
                    mDiscoveryPostponed = false;
                    if (mDiscoveryBlocked) {
                        try {
                            StateMachine m = (StateMachine)message.obj;
                            m.sendMessage(message.arg2);
                        } catch (Exception e) {
                            loge("unable to send BLOCK_DISCOVERY response: " + e);
                        }
                    }
                    break;
                case WigigP2pManager.DISCOVER_PEERS:
                case WigigP2pManager.STOP_DISCOVERY:
                case WigigP2pManager.DISCOVER_SERVICES:
                case WigigP2pManager.CONNECT:
                case WigigP2pManager.CANCEL_CONNECT:
                case WigigP2pManager.CREATE_GROUP:
                case WigigP2pManager.REMOVE_GROUP:
                case WigigP2pManager.ADD_LOCAL_SERVICE:
                case WigigP2pManager.REMOVE_LOCAL_SERVICE:
                case WigigP2pManager.CLEAR_LOCAL_SERVICES:
                case WigigP2pManager.ADD_SERVICE_REQUEST:
                case WigigP2pManager.REMOVE_SERVICE_REQUEST:
                case WigigP2pManager.CLEAR_SERVICE_REQUESTS:
                case WigigP2pManager.SET_DEVICE_NAME:
                case WigigP2pManager.DELETE_PERSISTENT_GROUP:
                case WigigP2pManager.START_WPS:
                case WigigP2pManager.SET_WFD_INFO:
                case WigigP2pManager.INITIATOR_REPORT_NFC_HANDOVER:
                case WigigP2pManager.RESPONDER_REPORT_NFC_HANDOVER:
                    sendActionListenerFailure(message.arg2, WigigP2pManager.BUSY);
                    break;
                case WigigP2pManager.REQUEST_PEERS:
                    sendPeersAvailable(message.arg2, new WifiP2pDeviceList(mPeers));
                    break;
                case WigigP2pManager.REQUEST_CONNECTION_INFO:
                    sendConnectionInfoAvailable(message.arg2, new WifiP2pInfo(mWifiP2pInfo));
                    break;
                case WigigP2pManager.REQUEST_GROUP_INFO:
                    sendGroupInfoAvailable(message.arg2,
                        mGroup != null ? new WifiP2pGroup(mGroup) : null);
                    break;
                case WigigP2pManager.REQUEST_PERSISTENT_GROUP_INFO:
                    sendPersistentGroupInfoAvailable(message.arg2, new WifiP2pGroupList(mGroups, null));
                    break;
                case WigigP2pManager.GET_HANDOVER_REQUEST:
                case WigigP2pManager.GET_HANDOVER_SELECT:
                    sendHandoverMessageAvailable(message.arg2, null);
                    break;
                case WigigMonitor.P2P_INVITATION_RESULT_EVENT:
                case WigigMonitor.SCAN_RESULTS_EVENT:
                case WigigMonitor.SUP_CONNECTION_EVENT:
                case WigigMonitor.SUP_DISCONNECTION_EVENT:
                case WigigMonitor.NETWORK_CONNECTION_EVENT:
                case WigigMonitor.NETWORK_DISCONNECTION_EVENT:
                case WigigMonitor.SUPPLICANT_STATE_CHANGE_EVENT:
                case WigigMonitor.AUTHENTICATION_FAILURE_EVENT:
                case WigigMonitor.WPS_SUCCESS_EVENT:
                case WigigMonitor.WPS_FAIL_EVENT:
                case WigigMonitor.WPS_OVERLAP_EVENT:
                case WigigMonitor.WPS_TIMEOUT_EVENT:
                case WigigMonitor.P2P_GROUP_REMOVED_EVENT:
                case WigigMonitor.P2P_DEVICE_FOUND_EVENT:
                case WigigMonitor.P2P_DEVICE_LOST_EVENT:
                case WigigMonitor.P2P_FIND_STOPPED_EVENT:
                case WigigMonitor.P2P_SERV_DISC_RESP_EVENT:
                case PEER_CONNECTION_USER_ACCEPT:
                case PEER_CONNECTION_USER_REJECT:
                case DISCONNECT_WIGIG_RESPONSE:
                case DROP_WIGIG_USER_ACCEPT:
                case DROP_WIGIG_USER_REJECT:
                case GROUP_CREATING_TIMED_OUT:
                case DISABLE_P2P_TIMED_OUT:
                case IPM_PRE_DHCP_ACTION:
                case IPM_POST_DHCP_ACTION:
                case IPM_DHCP_RESULTS:
                case IPM_PROVISIONING_SUCCESS:
                case IPM_PROVISIONING_FAILURE:
                case WigigMonitor.P2P_PROV_DISC_FAILURE_EVENT:
                case SET_MIRACAST_MODE:
                case WigigP2pManager.START_LISTEN:
                case WigigP2pManager.STOP_LISTEN:
                case WigigP2pManager.SET_CHANNEL:
                case WigigStateMachine.CMD_ENABLE_P2P:
                    // Enable is lazy and has no response
                    break;
                case WigigStateMachine.CMD_DISABLE_P2P_REQ:
                    // If we end up handling in default, p2p is not enabled
                    mWigigChannel.sendMessage(WigigStateMachine.CMD_DISABLE_P2P_RSP);
                    break;
                    /* unexpected group created, remove */
                case WigigMonitor.P2P_GROUP_STARTED_EVENT:
                    mGroup = (WifiP2pGroup) message.obj;
                    loge("Unexpected group creation, remove " + mGroup);
                    mWigigNative.p2pGroupRemove(mGroup.getInterface());
                    break;
                // A group formation failure is always followed by
                // a group removed event. Flushing things at group formation
                // failure causes supplicant issues. Ignore right now.
                case WigigMonitor.P2P_GROUP_FORMATION_FAILURE_EVENT:
                    break;
                default:
                    loge("Unhandled message " + message);
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class P2pNotSupportedState extends State {
        @Override
        public boolean processMessage(Message message) {
            switch (message.what) {
                case WigigP2pManager.DISCOVER_PEERS:
                case WigigP2pManager.STOP_DISCOVERY:
                case WigigP2pManager.DISCOVER_SERVICES:
                case WigigP2pManager.CONNECT:
                case WigigP2pManager.CANCEL_CONNECT:
                case WigigP2pManager.CREATE_GROUP:
                case WigigP2pManager.REMOVE_GROUP:
                case WigigP2pManager.ADD_LOCAL_SERVICE:
                case WigigP2pManager.REMOVE_LOCAL_SERVICE:
                case WigigP2pManager.CLEAR_LOCAL_SERVICES:
                case WigigP2pManager.ADD_SERVICE_REQUEST:
                case WigigP2pManager.REMOVE_SERVICE_REQUEST:
                case WigigP2pManager.CLEAR_SERVICE_REQUESTS:
                case WigigP2pManager.SET_DEVICE_NAME:
                case WigigP2pManager.DELETE_PERSISTENT_GROUP:
                case WigigP2pManager.SET_WFD_INFO:
                case WigigP2pManager.START_WPS:
                case WigigP2pManager.START_LISTEN:
                case WigigP2pManager.STOP_LISTEN:
                    sendActionListenerFailure(message.arg2, WigigP2pManager.P2P_UNSUPPORTED);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class P2pDisablingState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            sendMessageDelayed(obtainMessage(DISABLE_P2P_TIMED_OUT,
                    ++mDisableP2pTimeoutIndex, 0), DISABLE_P2P_WAIT_TIME_MS);
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigMonitor.SUP_DISCONNECTION_EVENT:
                    if (DBG) logd("p2p socket connection lost");
                    transitionTo(mP2pDisabledState);
                    break;
                case WigigStateMachine.CMD_ENABLE_P2P:
                case WigigStateMachine.CMD_DISABLE_P2P_REQ:
                    deferMessage(message);
                    break;
                case DISABLE_P2P_TIMED_OUT:
                    if (mDisableP2pTimeoutIndex == message.arg1) {
                        loge("P2p disable timed out");
                        transitionTo(mP2pDisabledState);
                    }
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }

        @Override
        public void exit() {
            mWigigChannel.sendMessage(WigigStateMachine.CMD_DISABLE_P2P_RSP);
        }
    }

    class P2pDisabledState extends State {
       @Override
        public void enter() {
            if (DBG) logd(getName());
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigStateMachine.CMD_ENABLE_P2P:
                    try {
                        mNwService.setInterfaceUp(mInterface);
                    } catch (RemoteException re) {
                        loge("Unable to change interface settings: " + re);
                    } catch (IllegalStateException ie) {
                        loge("Unable to change interface settings: " + ie);
                    }
                    if (mWigigMonitor.startMonitoring()) {
                        transitionTo(mP2pEnablingState);
                    } else {
                        loge("start  monitoring failed, do not transition");
                    }
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class P2pEnablingState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigMonitor.SUP_CONNECTION_EVENT:
                    if (DBG) logd("P2p socket connection successful");
                    transitionTo(mIdleState);
                    break;
                case WigigMonitor.SUP_DISCONNECTION_EVENT:
                    loge("P2p socket connection failed");
                    transitionTo(mP2pDisabledState);
                    break;
                case WigigStateMachine.CMD_ENABLE_P2P:
                case WigigStateMachine.CMD_DISABLE_P2P_REQ:
                    deferMessage(message);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class P2pEnabledState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            sendP2pStateChangedBroadcast(true);
            mNetworkInfo.setIsAvailable(true);
            sendP2pConnectionChangedBroadcast();
            initializeP2pSettings();
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigMonitor.SUP_DISCONNECTION_EVENT:
                    loge("Unexpected loss of p2p socket connection");
                    transitionTo(mP2pDisabledState);
                    break;
                case WigigStateMachine.CMD_ENABLE_P2P:
                    //Nothing to do
                    break;
                case WigigStateMachine.CMD_DISABLE_P2P_REQ:
                    if (mPeers.clear()) {
                        sendPeersChangedBroadcast();
                    }
                    if (mGroups.clear()) sendP2pPersistentGroupsChangedBroadcast();

                    mWigigMonitor.stopMonitoring();
                    transitionTo(mP2pDisablingState);
                    break;
                case WigigP2pManager.SET_DEVICE_NAME:
                {
                    WifiP2pDevice d = (WifiP2pDevice) message.obj;
                    if (d != null && setAndPersistDeviceName(d.deviceName)) {
                        if (DBG) logd("set device name " + d.deviceName);
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                }
                case WigigP2pManager.SET_WFD_INFO:
                {
                    WifiP2pWfdInfo d = (WifiP2pWfdInfo) message.obj;
                    if (d != null && setWfdInfo(d)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                }
                case BLOCK_DISCOVERY:
                    boolean blocked = (message.arg1 == ENABLED ? true : false);
                    if (mDiscoveryBlocked == blocked) break;
                    mDiscoveryBlocked = blocked;
                    if (blocked && mDiscoveryStarted) {
                        mWigigNative.p2pStopFind();
                        mDiscoveryPostponed = true;
                    }
                    if (!blocked && mDiscoveryPostponed) {
                        mDiscoveryPostponed = false;
                        mWigigNative.p2pFind(DISCOVER_TIMEOUT_S);
                    }
                    if (blocked) {
                        try {
                            StateMachine m = (StateMachine)message.obj;
                            m.sendMessage(message.arg2);
                        } catch (Exception e) {
                            loge("unable to send BLOCK_DISCOVERY response: " + e);
                        }
                    }
                    break;
                case WigigP2pManager.DISCOVER_PEERS:
                    if (mDiscoveryBlocked) {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.BUSY);
                        break;
                    }
                    // do not send service discovery request while normal find operation.
                    clearSupplicantServiceRequest();
                    if (mWigigNative.p2pFind(DISCOVER_TIMEOUT_S)) {
                        sendActionListenerSuccess(message.arg2);
                        sendP2pDiscoveryChangedBroadcast(true);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigMonitor.P2P_FIND_STOPPED_EVENT:
                    sendP2pDiscoveryChangedBroadcast(false);
                    break;
                case WigigP2pManager.STOP_DISCOVERY:
                    if (mWigigNative.p2pStopFind()) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.DISCOVER_SERVICES:
                    if (mDiscoveryBlocked) {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.BUSY);
                        break;
                    }
                    if (DBG) logd(getName() + " discover services");
                    if (!updateSupplicantServiceRequest()) {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.NO_SERVICE_REQUESTS);
                        break;
                    }
                    if (mWigigNative.p2pFind(DISCOVER_TIMEOUT_S)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigMonitor.P2P_DEVICE_FOUND_EVENT:
                    WifiP2pDevice device = (WifiP2pDevice) message.obj;
                    if (mThisDevice.deviceAddress.equals(device.deviceAddress)) break;
                    mPeers.updateSupplicantDetails(device);
                    sendPeersChangedBroadcast();
                    break;
                case WigigMonitor.P2P_DEVICE_LOST_EVENT:
                    device = (WifiP2pDevice) message.obj;
                    // Gets current details for the one removed
                    device = mPeers.remove(device.deviceAddress);
                    if (device != null) {
                        sendPeersChangedBroadcast();
                    }
                    break;
                case WigigP2pManager.ADD_LOCAL_SERVICE:
                    if (DBG) logd(getName() + " add service");
                    WifiP2pServiceInfo servInfo = (WifiP2pServiceInfo)message.obj;
                    if (addLocalService(message.replyTo, servInfo)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.REMOVE_LOCAL_SERVICE:
                    if (DBG) logd(getName() + " remove service");
                    servInfo = (WifiP2pServiceInfo)message.obj;
                    removeLocalService(message.replyTo, servInfo);
                    sendActionListenerSuccess(message.arg2);
                    break;
                case WigigP2pManager.CLEAR_LOCAL_SERVICES:
                    if (DBG) logd(getName() + " clear service");
                    clearLocalServices(message.replyTo);
                    sendActionListenerSuccess(message.arg2);
                    break;
                case WigigP2pManager.ADD_SERVICE_REQUEST:
                    if (DBG) logd(getName() + " add service request");
                    if (!addServiceRequest(message.replyTo, (WifiP2pServiceRequest)message.obj)) {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                        break;
                    }
                    sendActionListenerSuccess(message.arg2);
                    break;
                case WigigP2pManager.REMOVE_SERVICE_REQUEST:
                    if (DBG) logd(getName() + " remove service request");
                    removeServiceRequest(message.replyTo, (WifiP2pServiceRequest)message.obj);
                    sendActionListenerSuccess(message.arg2);
                    break;
                case WigigP2pManager.CLEAR_SERVICE_REQUESTS:
                    if (DBG) logd(getName() + " clear service request");
                    clearServiceRequests(message.replyTo);
                    sendActionListenerSuccess(message.arg2);
                    break;
                case WigigMonitor.P2P_SERV_DISC_RESP_EVENT:
                    if (DBG) logd(getName() + " receive service response");
                    List<WifiP2pServiceResponse> sdRespList =
                        (List<WifiP2pServiceResponse>) message.obj;
                    for (WifiP2pServiceResponse resp : sdRespList) {
                        WifiP2pDevice dev =
                            mPeers.get(resp.getSrcDevice().deviceAddress);
                        resp.setSrcDevice(dev);
                        sendServiceResponse(resp);
                    }
                    break;
                case WigigP2pManager.DELETE_PERSISTENT_GROUP:
                   if (DBG) logd(getName() + " delete persistent group");
                   mGroups.remove(message.arg1);
                   sendActionListenerSuccess(message.arg2);
                   break;
                case SET_MIRACAST_MODE:
                    mWigigNative.setMiracastMode(message.arg1);
                    break;
                case WigigP2pManager.START_LISTEN:
                    if (DBG) logd(getName() + " start listen mode");
                    mWigigNative.p2pFlush();
                    if (mWigigNative.p2pExtListen(true, 500, 500)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.STOP_LISTEN:
                    if (DBG) logd(getName() + " stop listen mode");
                    if (mWigigNative.p2pExtListen(false, 0, 0)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    mWigigNative.p2pFlush();
                    break;
                case WigigP2pManager.SET_CHANNEL:
                    Bundle p2pChannels = (Bundle) message.obj;
                    int lc = p2pChannels.getInt("lc", 0);
                    int oc = p2pChannels.getInt("oc", 0);
                    if (DBG) logd(getName() + " set listen and operating channel");
                    if (mWigigNative.p2pSetChannel(lc, oc)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.GET_HANDOVER_REQUEST:
                    sendHandoverMessageAvailable(message.arg2, mWigigNative.getNfcHandoverRequest());
                    break;
                case WigigP2pManager.GET_HANDOVER_SELECT:
                    sendHandoverMessageAvailable(message.arg2, mWigigNative.getNfcHandoverSelect());
                    break;
                default:
                   return NOT_HANDLED;
            }
            return HANDLED;
        }

        @Override
        public void exit() {
            sendP2pDiscoveryChangedBroadcast(false);
            sendP2pStateChangedBroadcast(false);
            mNetworkInfo.setIsAvailable(false);
        }
    }

    class IdleState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            mWigigChannel.sendMessage(WigigStateMachine.CMD_P2P_RELEASED_RADIO);
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigP2pManager.DISCOVER_PEERS:
                case WigigP2pManager.DISCOVER_SERVICES:
                case WigigP2pManager.CONNECT:
                case WigigMonitor.P2P_GO_NEGOTIATION_REQUEST_EVENT:
                case WigigMonitor.P2P_INVITATION_RECEIVED_EVENT:
                case WigigMonitor.P2P_PROV_DISC_SHOW_PIN_EVENT:
                case WigigP2pManager.CREATE_GROUP:
                case WigigMonitor.P2P_GROUP_STARTED_EVENT:
                case WigigP2pManager.START_LISTEN:
                case WigigP2pManager.INITIATOR_REPORT_NFC_HANDOVER:
                case WigigP2pManager.RESPONDER_REPORT_NFC_HANDOVER:
                    mRadioRequested = true;
                    mWigigChannel.sendMessage(WigigStateMachine.CMD_P2P_REQUEST_RADIO);
                    deferMessage(message);
                    break;
                case WigigStateMachine.CMD_P2P_RADIO_STATE_UPDATE:
                    if(mRadioRequested &&
                       message.arg1 == WigigStateMachine.P2P_RADIO_STATE_IN_USE_BY_P2P) {
                        transitionTo(mInactiveState);
                    } else if (message.arg1 == WigigStateMachine.P2P_RADIO_STATE_IN_USE_BY_WIGIG) {
                        transitionTo(mRadioInUseByWigigState);
                    }
                    mRadioRequested = false;
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class RadioInUseByWigigState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            // TODO check if we need to disable persistent groups to prevent supplicant
            // doing P2P stuff in the background
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigP2pManager.DISCOVER_PEERS:
                case WigigP2pManager.DISCOVER_SERVICES:
                case WigigP2pManager.CONNECT:
                case WigigP2pManager.CREATE_GROUP:
                case WigigP2pManager.START_WPS:
                case WigigP2pManager.INITIATOR_REPORT_NFC_HANDOVER:
                case WigigP2pManager.RESPONDER_REPORT_NFC_HANDOVER:
                    sendActionListenerFailure(message.arg2, WigigP2pManager.BUSY);
                    break;
                case WigigStateMachine.CMD_P2P_RADIO_STATE_UPDATE:
                    if (message.arg1 != WigigStateMachine.P2P_RADIO_STATE_IN_USE_BY_WIGIG) {
                        transitionTo(mIdleState);
                    }
                    break;
                default:
                    loge("Unhandled message " + message);
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class InactiveState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            mSavedPeerConfig.invalidate();
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigP2pManager.CONNECT:
                    if (DBG) logd(getName() + " sending connect");
                    WifiP2pConfig config = (WifiP2pConfig) message.obj;
                    if (isConfigInvalid(config)) {
                        loge("Dropping connect requeset " + config);
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                        break;
                    }

                    mAutonomousGroup = false;
                    mWigigNative.p2pStopFind();
                    if (reinvokePersistentGroup(config)) {
                        transitionTo(mGroupNegotiationState);
                    } else {
                        transitionTo(mProvisionDiscoveryState);
                    }
                    mSavedPeerConfig = config;
                    mPeers.updateStatus(mSavedPeerConfig.deviceAddress, WifiP2pDevice.INVITED);
                    sendPeersChangedBroadcast();
                    sendActionListenerSuccess(message.arg2);
                    break;
                case WigigP2pManager.STOP_DISCOVERY:
                    if (mWigigNative.p2pStopFind()) {
                        // When discovery stops in inactive state, flush to clear
                        // state peer data
                        mWigigNative.p2pFlush();
                        mServiceDiscReqId = null;
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigMonitor.P2P_GO_NEGOTIATION_REQUEST_EVENT:
                    config = (WifiP2pConfig) message.obj;
                    if (isConfigInvalid(config)) {
                        loge("Dropping GO neg request " + config);
                        break;
                    }
                    mSavedPeerConfig = config;
                    mAutonomousGroup = false;
                    mJoinExistingGroup = false;
                    transitionTo(mUserAuthorizingNegotiationRequestState);
                    break;
                case WigigMonitor.P2P_INVITATION_RECEIVED_EVENT:
                    WifiP2pGroup group = (WifiP2pGroup) message.obj;
                    WifiP2pDevice owner = group.getOwner();

                    if (owner == null) {
                        loge("Ignored invitation from null owner");
                        break;
                    }

                    config = new WifiP2pConfig();
                    config.deviceAddress = group.getOwner().deviceAddress;

                    if (isConfigInvalid(config)) {
                        loge("Dropping invitation request " + config);
                        break;
                    }
                    mSavedPeerConfig = config;

                    //Check if we have the owner in peer list and use appropriate
                    //wps method. Default is to use PBC.
                    if ((owner = mPeers.get(owner.deviceAddress)) != null) {
                        if (owner.wpsPbcSupported()) {
                            mSavedPeerConfig.wps.setup = WpsInfo.PBC;
                        } else if (owner.wpsKeypadSupported()) {
                            mSavedPeerConfig.wps.setup = WpsInfo.KEYPAD;
                        } else if (owner.wpsDisplaySupported()) {
                            mSavedPeerConfig.wps.setup = WpsInfo.DISPLAY;
                        }
                    }

                    mAutonomousGroup = false;
                    mJoinExistingGroup = true;
                    transitionTo(mUserAuthorizingInviteRequestState);
                    break;
                case WigigMonitor.P2P_INVITATION_ACCEPTED_EVENT:
                    group = (WifiP2pGroup) message.obj;
                    owner = group.getOwner();

                    if (owner == null) {
                        loge("invalid owner in invitation accepted, dropping");
                        break;
                    }

                    config = new WifiP2pConfig();
                    config.deviceAddress = group.getOwner().deviceAddress;

                    if (isConfigInvalid(config)) {
                        loge("Dropping invitation accepted because of invalid config: " + config);
                        break;
                    }
                    mSavedPeerConfig = config;

                    mAutonomousGroup = false;
                    mJoinExistingGroup = true;
                    transitionTo(mGroupNegotiationState);

                    break;
                case WigigMonitor.P2P_PROV_DISC_PBC_REQ_EVENT:
                case WigigMonitor.P2P_PROV_DISC_ENTER_PIN_EVENT:
                    //We let the supplicant handle the provision discovery response
                    //and wait instead for the GO_NEGOTIATION_REQUEST_EVENT.
                    //Handling provision discovery and issuing a p2p_connect before
                    //group negotiation comes through causes issues
                    break;
                case WigigP2pManager.CREATE_GROUP:
                    mAutonomousGroup = true;
                    int netId = message.arg1;
                    boolean ret = false;
                    if (netId == WifiP2pGroup.PERSISTENT_NET_ID) {
                        // check if the go persistent group is present.
                        netId = mGroups.getNetworkId(mThisDevice.deviceAddress);
                        if (netId != -1) {
                            ret = mWigigNative.p2pGroupAdd(netId);
                        } else {
                            ret = mWigigNative.p2pGroupAdd(true);
                        }
                    } else {
                        ret = mWigigNative.p2pGroupAdd(false);
                    }

                    if (ret) {
                        sendActionListenerSuccess(message.arg2);
                        transitionTo(mGroupNegotiationState);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                        // remain at this state.
                    }
                    break;
                case WigigMonitor.P2P_GROUP_STARTED_EVENT:
                    mGroup = (WifiP2pGroup) message.obj;
                    if (DBG) logd(getName() + " group started");

                    // We hit this scenario when a persistent group is reinvoked
                    if (mGroup.getNetworkId() == WifiP2pGroup.PERSISTENT_NET_ID) {
                        mAutonomousGroup = false;
                        deferMessage(message);
                        transitionTo(mGroupNegotiationState);
                    } else {
                        loge("Unexpected group creation, remove " + mGroup);
                        mWigigNative.p2pGroupRemove(mGroup.getInterface());
                    }
                    break;
                case WigigP2pManager.START_LISTEN:
                    if (DBG) logd(getName() + " start listen mode");
                    mWigigNative.p2pFlush();
                    if (mWigigNative.p2pExtListen(true, 500, 500)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.STOP_LISTEN:
                    if (DBG) logd(getName() + " stop listen mode");
                    if (mWigigNative.p2pExtListen(false, 0, 0)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    mWigigNative.p2pFlush();
                    break;
                case WigigP2pManager.SET_CHANNEL:
                    Bundle p2pChannels = (Bundle) message.obj;
                    int lc = p2pChannels.getInt("lc", 0);
                    int oc = p2pChannels.getInt("oc", 0);
                    if (DBG) logd(getName() + " set listen and operating channel");
                    if (mWigigNative.p2pSetChannel(lc, oc)) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.INITIATOR_REPORT_NFC_HANDOVER:
                    String handoverSelect = null;

                    if (message.obj != null) {
                        handoverSelect = ((Bundle) message.obj)
                                .getString(WigigP2pManager.EXTRA_HANDOVER_MESSAGE);
                    }

                    if (handoverSelect != null
                            && mWigigNative.initiatorReportNfcHandover(handoverSelect)) {
                        sendActionListenerSuccess(message.arg2);
                        transitionTo(mGroupCreatingState);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.RESPONDER_REPORT_NFC_HANDOVER:
                    String handoverRequest = null;

                    if (message.obj != null) {
                        handoverRequest = ((Bundle) message.obj)
                                .getString(WigigP2pManager.EXTRA_HANDOVER_MESSAGE);
                    }

                    if (handoverRequest != null
                            && mWigigNative.responderReportNfcHandover(handoverRequest)) {
                        sendActionListenerSuccess(message.arg2);
                        transitionTo(mGroupCreatingState);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigMonitor.P2P_FIND_STOPPED_EVENT:
                    sendP2pDiscoveryChangedBroadcast(false);
                    transitionTo(mIdleState);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class GroupCreatingState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            sendMessageDelayed(obtainMessage(GROUP_CREATING_TIMED_OUT,
                    ++mGroupCreatingTimeoutIndex, 0), GROUP_CREATING_WAIT_TIME_MS);
            // TODO move P2P override to a common "P2P operation running" state when
            // implementing Wigig-P2P state machine handover
            mWigigMonitor.setP2pOverride(true);
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            boolean ret = HANDLED;
            switch (message.what) {
               case GROUP_CREATING_TIMED_OUT:
                    if (mGroupCreatingTimeoutIndex == message.arg1) {
                        if (DBG) logd("Group negotiation timed out");
                        handleGroupCreationFailure();
                        transitionTo(mIdleState);
                    }
                    break;
                case WigigMonitor.P2P_DEVICE_LOST_EVENT:
                    WifiP2pDevice device = (WifiP2pDevice) message.obj;
                    if (!mSavedPeerConfig.deviceAddress.equals(device.deviceAddress)) {
                        if (DBG) {
                            logd("mSavedPeerConfig " + mSavedPeerConfig.deviceAddress +
                                "device " + device.deviceAddress);
                        }
                        // Do the regular device lost handling
                        ret = NOT_HANDLED;
                        break;
                    }
                    // Do nothing
                    if (DBG) logd("Add device to lost list " + device);
                    mPeersLostDuringConnection.updateSupplicantDetails(device);
                    break;
                case WigigP2pManager.DISCOVER_PEERS:
                    /* Discovery will break negotiation */
                    sendActionListenerFailure(message.arg2, WigigP2pManager.BUSY);
                    break;
                case WigigP2pManager.CANCEL_CONNECT:
                    //Do a supplicant p2p_cancel which only cancels an ongoing
                    //group negotiation. This will fail for a pending provision
                    //discovery or for a pending user action, but at the framework
                    //level, we always treat cancel as succeeded and enter
                    //an inactive state
                    mWigigNative.p2pCancelConnect();
                    handleGroupCreationFailure();
                    transitionTo(mIdleState);
                    sendActionListenerSuccess(message.arg2);
                    break;
                case WigigMonitor.P2P_GO_NEGOTIATION_SUCCESS_EVENT:
                    // We hit this scenario when NFC handover is invoked.
                    mAutonomousGroup = false;
                    transitionTo(mGroupNegotiationState);
                    break;
                default:
                    ret = NOT_HANDLED;
            }
            return ret;
        }

        @Override
        public void exit() {
            // TODO move P2P override to a common "P2P operation running" state when
            // implementing Wigig-P2P state machine handover
            mWigigMonitor.setP2pOverride(false);
        }
    }

    class UserAuthorizingNegotiationRequestState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            notifyInvitationReceived();
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            boolean ret = HANDLED;
            switch (message.what) {
                case PEER_CONNECTION_USER_ACCEPT:
                    mWigigNative.p2pStopFind();
                    p2pConnectWithPinDisplay(mSavedPeerConfig);
                    mPeers.updateStatus(mSavedPeerConfig.deviceAddress, WifiP2pDevice.INVITED);
                    sendPeersChangedBroadcast();
                    transitionTo(mGroupNegotiationState);
                   break;
                case PEER_CONNECTION_USER_REJECT:
                    if (DBG) logd("User rejected negotiation " + mSavedPeerConfig);
                    transitionTo(mIdleState);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return ret;
        }

        @Override
        public void exit() {
            //TODO: dismiss dialog if not already done
        }
    }

    class UserAuthorizingInviteRequestState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            notifyInvitationReceived();
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            boolean ret = HANDLED;
            switch (message.what) {
                case PEER_CONNECTION_USER_ACCEPT:
                    mWigigNative.p2pStopFind();
                    if (!reinvokePersistentGroup(mSavedPeerConfig)) {
                        // Do negotiation when persistence fails
                        p2pConnectWithPinDisplay(mSavedPeerConfig);
                    }
                    mPeers.updateStatus(mSavedPeerConfig.deviceAddress, WifiP2pDevice.INVITED);
                    sendPeersChangedBroadcast();
                    transitionTo(mGroupNegotiationState);
                   break;
                case PEER_CONNECTION_USER_REJECT:
                    if (DBG) logd("User rejected invitation " + mSavedPeerConfig);
                    transitionTo(mIdleState);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return ret;
        }

        @Override
        public void exit() {
            //TODO: dismiss dialog if not already done
        }
    }



    class ProvisionDiscoveryState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            mWigigNative.p2pProvisionDiscovery(mSavedPeerConfig);
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            WifiP2pProvDiscEvent provDisc;
            WifiP2pDevice device;
            switch (message.what) {
                case WigigMonitor.P2P_PROV_DISC_PBC_RSP_EVENT:
                    provDisc = (WifiP2pProvDiscEvent) message.obj;
                    device = provDisc.device;
                    if (!device.deviceAddress.equals(mSavedPeerConfig.deviceAddress)) break;

                    if (mSavedPeerConfig.wps.setup == WpsInfo.PBC) {
                        if (DBG) logd("Found a match " + mSavedPeerConfig);
                        p2pConnectWithPinDisplay(mSavedPeerConfig);
                        transitionTo(mGroupNegotiationState);
                    }
                    break;
                case WigigMonitor.P2P_PROV_DISC_ENTER_PIN_EVENT:
                    provDisc = (WifiP2pProvDiscEvent) message.obj;
                    device = provDisc.device;
                    if (!device.deviceAddress.equals(mSavedPeerConfig.deviceAddress)) break;

                    if (mSavedPeerConfig.wps.setup == WpsInfo.KEYPAD) {
                        if (DBG) logd("Found a match " + mSavedPeerConfig);
                        /* we already have the pin */
                        if (!TextUtils.isEmpty(mSavedPeerConfig.wps.pin)) {
                            p2pConnectWithPinDisplay(mSavedPeerConfig);
                            transitionTo(mGroupNegotiationState);
                        } else {
                            mJoinExistingGroup = false;
                            transitionTo(mUserAuthorizingNegotiationRequestState);
                        }
                    }
                    break;
                case WigigMonitor.P2P_PROV_DISC_SHOW_PIN_EVENT:
                    provDisc = (WifiP2pProvDiscEvent) message.obj;
                    device = provDisc.device;
                    if (!device.deviceAddress.equals(mSavedPeerConfig.deviceAddress)) break;

                    if (mSavedPeerConfig.wps.setup == WpsInfo.DISPLAY) {
                        if (DBG) logd("Found a match " + mSavedPeerConfig);
                        mSavedPeerConfig.wps.pin = provDisc.pin;
                        p2pConnectWithPinDisplay(mSavedPeerConfig);
                        notifyInvitationSent(provDisc.pin, device.deviceAddress);
                        transitionTo(mGroupNegotiationState);
                    }
                    break;
                case WigigMonitor.P2P_PROV_DISC_FAILURE_EVENT:
                    loge("provision discovery failed");
                    handleGroupCreationFailure();
                    transitionTo(mIdleState);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class GroupNegotiationState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                // We ignore these right now, since we get a GROUP_STARTED notification
                // afterwards
                case WigigMonitor.P2P_GO_NEGOTIATION_SUCCESS_EVENT:
                case WigigMonitor.P2P_GROUP_FORMATION_SUCCESS_EVENT:
                    if (DBG) logd(getName() + " go success");
                    break;
                case WigigMonitor.P2P_GROUP_STARTED_EVENT:
                    mGroup = (WifiP2pGroup) message.obj;
                    if (DBG) logd(getName() + " group started");

                    if (mGroup.getNetworkId() == WifiP2pGroup.PERSISTENT_NET_ID) {
                        /*
                         * update cache information and set network id to mGroup.
                         */
                        updatePersistentNetworks(NO_RELOAD);
                        String devAddr = mGroup.getOwner().deviceAddress;
                        mGroup.setNetworkId(mGroups.getNetworkId(devAddr,
                                mGroup.getNetworkName()));
                    }

                    if (mGroup.isGroupOwner()) {
                        /* Setting an idle time out on GO causes issues with certain scenarios
                         * on clients where it can be off-channel for longer and with the power
                         * save modes used.
                         *
                         * TODO: Verify multi-channel scenarios and supplicant behavior are
                         * better before adding a time out in future
                         */
                        //Set group idle timeout of 10 sec, to avoid GO beaconing incase of any
                        //failure during 4-way Handshake.
                        if (!mAutonomousGroup) {
                            mWigigNative.setP2pGroupIdle(mGroup.getInterface(), GROUP_IDLE_TIME_S);
                        }
                        startDhcpServer(mGroup.getInterface());
                    } else {
                        mWigigNative.setP2pGroupIdle(mGroup.getInterface(), GROUP_IDLE_TIME_S);
                        startIpClient(mGroup.getInterface());
                        WifiP2pDevice groupOwner = mGroup.getOwner();
                        WifiP2pDevice peer = mPeers.get(groupOwner.deviceAddress);
                        if (peer != null) {
                            // update group owner details with peer details found at discovery
                            groupOwner.updateSupplicantDetails(peer);
                            mPeers.updateStatus(groupOwner.deviceAddress, WifiP2pDevice.CONNECTED);
                            sendPeersChangedBroadcast();
                        } else {
                            // A supplicant bug can lead to reporting an invalid
                            // group owner address (all zeroes) at times. Avoid a
                            // crash, but continue group creation since it is not
                            // essential.
                            logw("Unknown group owner " + groupOwner);
                        }
                    }
                    transitionTo(mGroupCreatedState);
                    break;
                case WigigMonitor.P2P_GO_NEGOTIATION_FAILURE_EVENT:
                    P2pStatus status = (P2pStatus) message.obj;
                    if (status == P2pStatus.NO_COMMON_CHANNEL) {
                        transitionTo(mFrequencyConflictState);
                        break;
                    }
                    /* continue with group removal handling */
                case WigigMonitor.P2P_GROUP_REMOVED_EVENT:
                    if (DBG) logd(getName() + " go failure");
                    handleGroupCreationFailure();
                    transitionTo(mIdleState);
                    break;
                // A group formation failure is always followed by
                // a group removed event. Flushing things at group formation
                // failure causes supplicant issues. Ignore right now.
                case WigigMonitor.P2P_GROUP_FORMATION_FAILURE_EVENT:
                    status = (P2pStatus) message.obj;
                    if (status == P2pStatus.NO_COMMON_CHANNEL) {
                        transitionTo(mFrequencyConflictState);
                        break;
                    }
                    break;
                case WigigMonitor.P2P_INVITATION_RESULT_EVENT:
                    status = (P2pStatus)message.obj;
                    if (status == P2pStatus.SUCCESS) {
                        // invocation was succeeded.
                        // wait P2P_GROUP_STARTED_EVENT.
                        break;
                    }
                    loge("Invitation result " + status);
                    if (status == P2pStatus.UNKNOWN_P2P_GROUP) {
                        // target device has already removed the credential.
                        // So, remove this credential accordingly.
                        int netId = mSavedPeerConfig.netId;
                        if (netId >= 0) {
                            if (DBG) logd("Remove unknown client from the list");
                            removeClientFromList(netId, mSavedPeerConfig.deviceAddress, true);
                        }

                        // Reinvocation has failed, try group negotiation
                        mSavedPeerConfig.netId = WifiP2pGroup.PERSISTENT_NET_ID;
                        p2pConnectWithPinDisplay(mSavedPeerConfig);
                    } else if (status == P2pStatus.INFORMATION_IS_CURRENTLY_UNAVAILABLE) {

                        // Devices setting persistent_reconnect to 0 in wpa_supplicant
                        // always defer the invocation request and return
                        // "information is currently unable" error.
                        // So, try another way to connect for interoperability.
                        mSavedPeerConfig.netId = WifiP2pGroup.PERSISTENT_NET_ID;
                        p2pConnectWithPinDisplay(mSavedPeerConfig);
                    } else if (status == P2pStatus.NO_COMMON_CHANNEL) {
                        transitionTo(mFrequencyConflictState);
                    } else {
                        handleGroupCreationFailure();
                        transitionTo(mIdleState);
                    }
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    class FrequencyConflictState extends State {
        private AlertDialog mFrequencyConflictDialog;
        @Override
        public void enter() {
            if (DBG) logd(getName());
            notifyFrequencyConflict();
        }

        private void notifyFrequencyConflict() {
            logd("Notify frequency conflict");

            AlertDialog dialog = new AlertDialog.Builder(mContext)
                .setMessage(mWifiRes.getString(getWifiResId("string", "wifi_p2p_frequency_conflict_message"),
                        getDeviceName(mSavedPeerConfig.deviceAddress)))
                .setPositiveButton(mWifiRes.getString(getWifiResId("string", "dlg_ok")), new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            sendMessage(DROP_WIGIG_USER_ACCEPT);
                        }
                    })
                .setNegativeButton(mWifiRes.getString(getWifiResId("string", "decline")), new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            sendMessage(DROP_WIGIG_USER_REJECT);
                        }
                    })
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface arg0) {
                            sendMessage(DROP_WIGIG_USER_REJECT);
                        }
                    })
                .create();

            dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
            WindowManager.LayoutParams attrs = dialog.getWindow().getAttributes();
            attrs.privateFlags = WindowManager.LayoutParams.SYSTEM_FLAG_SHOW_FOR_ALL_USERS;
            dialog.getWindow().setAttributes(attrs);
            dialog.show();
            mFrequencyConflictDialog = dialog;
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigMonitor.P2P_GO_NEGOTIATION_SUCCESS_EVENT:
                case WigigMonitor.P2P_GROUP_FORMATION_SUCCESS_EVENT:
                    loge(getName() + "group sucess during freq conflict!");
                    break;
                case WigigMonitor.P2P_GROUP_STARTED_EVENT:
                    loge(getName() + "group started after freq conflict, handle anyway");
                    deferMessage(message);
                    transitionTo(mGroupNegotiationState);
                    break;
                case WigigMonitor.P2P_GO_NEGOTIATION_FAILURE_EVENT:
                case WigigMonitor.P2P_GROUP_REMOVED_EVENT:
                case WigigMonitor.P2P_GROUP_FORMATION_FAILURE_EVENT:
                    // Ignore failures since we retry again
                    break;
                case DROP_WIGIG_USER_REJECT:
                    // User rejected dropping wigig in favour of p2p
                    handleGroupCreationFailure();
                    transitionTo(mIdleState);
                    break;
                case DROP_WIGIG_USER_ACCEPT:
                    // User accepted dropping wigig in favour of p2p
                    mWigigChannel.sendMessage(WigigP2pServiceImpl.DISCONNECT_WIGIG_REQUEST, 1);
                    mTemporarilyDisconnectedWigig = true;
                    break;
                case DISCONNECT_WIGIG_RESPONSE:
                    // Got a response from wigigstatemachine, retry p2p
                    if (DBG) logd(getName() + "Wigig disconnected, retry p2p");
                    transitionTo(mIdleState);
                    sendMessage(WigigP2pManager.CONNECT, mSavedPeerConfig);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }

        public void exit() {
            if (mFrequencyConflictDialog != null) mFrequencyConflictDialog.dismiss();
        }
    }

    class GroupCreatedState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            // TODO move P2P override to a common "P2P operation running" state when
            // implementing Wigig-P2P state machine handover
            mWigigMonitor.setP2pOverride(true);

            // Once connected, peer config details are invalid
            mSavedPeerConfig.invalidate();
            mNetworkInfo.setDetailedState(NetworkInfo.DetailedState.CONNECTED, null, null);

            updateThisDevice(WifiP2pDevice.CONNECTED);

            //DHCP server has already been started if I am a group owner
            if (mGroup.isGroupOwner()) {
                setWifiP2pInfoOnGroupFormation(NetworkUtils.numericToInetAddress(SERVER_ADDRESS));
            }

            // In case of a negotiation group, connection changed is sent
            // after a client joins. For autonomous, send now
            if (mAutonomousGroup) {
                sendP2pConnectionChangedBroadcast();
            }
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigMonitor.AP_STA_CONNECTED_EVENT:
                    WifiP2pDevice device = (WifiP2pDevice) message.obj;
                    String deviceAddress = device.deviceAddress;
                    // Clear timeout that was set when group was started.
                    mWigigNative.setP2pGroupIdle(mGroup.getInterface(), 0);
                    if (deviceAddress != null) {
                        if (mPeers.get(deviceAddress) != null) {
                            mGroup.addClient(mPeers.get(deviceAddress));
                        } else {
                            mGroup.addClient(deviceAddress);
                        }
                        mPeers.updateStatus(deviceAddress, WifiP2pDevice.CONNECTED);
                        if (DBG) logd(getName() + " ap sta connected");
                        sendPeersChangedBroadcast();
                    } else {
                        loge("Connect on null device address, ignore");
                    }
                    sendP2pConnectionChangedBroadcast();
                    break;
                case WigigMonitor.AP_STA_DISCONNECTED_EVENT:
                    device = (WifiP2pDevice) message.obj;
                    deviceAddress = device.deviceAddress;
                    if (deviceAddress != null) {
                        mPeers.updateStatus(deviceAddress, WifiP2pDevice.AVAILABLE);
                        if (mGroup.removeClient(deviceAddress)) {
                            if (DBG) logd("Removed client " + deviceAddress);
                            if (!mAutonomousGroup && mGroup.isClientListEmpty()) {
                                logd("Client list empty, remove non-persistent p2p group");
                                mWigigNative.p2pGroupRemove(mGroup.getInterface());
                                // We end up sending connection changed broadcast
                                // when this happens at exit()
                            } else {
                                // Notify when a client disconnects from group
                                sendP2pConnectionChangedBroadcast();
                            }
                        } else {
                            if (DBG) logd("Failed to remove client " + deviceAddress);
                            for (WifiP2pDevice c : mGroup.getClientList()) {
                                if (DBG) logd("client " + c.deviceAddress);
                            }
                        }
                        sendPeersChangedBroadcast();
                        if (DBG) logd(getName() + " ap sta disconnected");
                    } else {
                        loge("Disconnect on unknown device: " + device);
                    }
                    break;
                case IPM_PRE_DHCP_ACTION:
                    mWigigNative.setP2pPowerSave(mGroup.getInterface(), false);
                    try {
                        mIpClient.completedPreDhcpAction();
                    } catch (RemoteException e) {
                        loge("Error for IpClient completedPreDhcpAction", e);
                    }
                    break;
                case IPM_POST_DHCP_ACTION:
                    mWigigNative.setP2pPowerSave(mGroup.getInterface(), true);
                    break;
                case IPM_DHCP_RESULTS:
                    mDhcpResults = (DhcpResultsParcelable) message.obj;
                    if (DBG) logd("mDhcpResults: " + mDhcpResults);
                    if (mDhcpResults == null) {
                        loge("IP provisioning success but no DHCP results!");
                        break;
                    }
                    setWifiP2pInfoOnGroupFormation(
                        InetAddresses.parseNumericAddress(mDhcpResults.serverAddress));
                    sendP2pConnectionChangedBroadcast();
                    if (mDhcpResults.baseConfiguration == null) {
                        loge("no baseConfiguration in DHCP results, skip routing setup");
                        break;
                    }
                    try {
                        final String ifname = mGroup.getInterface();
                        mNwService.addInterfaceToLocalNetwork(
                                ifname, mDhcpResults.baseConfiguration.getRoutes(ifname));
                    } catch (RemoteException e) {
                        loge("Failed to add iface to local network " + e);
                    } catch (IllegalStateException e) {
                        loge("Unable to add iface to local network " + e);
                    }
                    break;
                case IPM_PROVISIONING_SUCCESS:
                    break;
                case IPM_PROVISIONING_FAILURE:
                    loge("IP provisioning failed");
                    mWigigNative.p2pGroupRemove(mGroup.getInterface());
                    break;
                case WigigP2pManager.REMOVE_GROUP:
                    if (DBG) logd(getName() + " remove group");
                    if (mWigigNative.p2pGroupRemove(mGroup.getInterface())) {
                        transitionTo(mOngoingGroupRemovalState);
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        handleGroupRemoved();
                        transitionTo(mIdleState);
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                /* We do not listen to NETWORK_DISCONNECTION_EVENT for group removal
                 * handling since supplicant actually tries to reconnect after a temporary
                 * disconnect until group idle time out. Eventually, a group removal event
                 * will come when group has been removed.
                 *
                 * When there are connectivity issues during temporary disconnect, the application
                 * will also just remove the group.
                 *
                 * Treating network disconnection as group removal causes race conditions since
                 * supplicant would still maintain the group at that stage.
                 */
                case WigigMonitor.P2P_GROUP_REMOVED_EVENT:
                    if (DBG) logd(getName() + " group removed");
                    handleGroupRemoved();
                    transitionTo(mIdleState);
                    break;
                case WigigMonitor.P2P_DEVICE_LOST_EVENT:
                    device = (WifiP2pDevice) message.obj;
                    //Device loss for a connected device indicates it is not in discovery any more
                    if (mGroup.contains(device)) {
                        if (DBG) logd("Add device to lost list " + device);
                        mPeersLostDuringConnection.updateSupplicantDetails(device);
                        return HANDLED;
                    }
                    // Do the regular device lost handling
                    return NOT_HANDLED;
                case WigigStateMachine.CMD_DISABLE_P2P_REQ:
                    sendMessage(WigigP2pManager.REMOVE_GROUP);
                    deferMessage(message);
                    break;
                    // This allows any client to join the GO during the
                    // WPS window
                case WigigP2pManager.START_WPS:
                    WpsInfo wps = (WpsInfo) message.obj;
                    if (wps == null) {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                        break;
                    }
                    boolean ret = true;
                    if (wps.setup == WpsInfo.PBC) {
                        ret = mWigigNative.startWpsPbc(mGroup.getInterface(), null);
                    } else {
                        if (wps.pin == null) {
                            String pin = mWigigNative.startWpsPinDisplay(mGroup.getInterface());
                            try {
                                Integer.parseInt(pin);
                                notifyInvitationSent(pin, "any");
                            } catch (NumberFormatException ignore) {
                                ret = false;
                            }
                        } else {
                            ret = mWigigNative.startWpsPinKeypad(mGroup.getInterface(),
                                    wps.pin);
                        }
                    }
                    if (ret) {
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    break;
                case WigigP2pManager.CONNECT:
                    WifiP2pConfig config = (WifiP2pConfig) message.obj;
                    if (isConfigInvalid(config)) {
                        loge("Dropping connect requeset " + config);
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                        break;
                    }
                    logd("Inviting device : " + config.deviceAddress);
                    mSavedPeerConfig = config;
                    if (mWigigNative.p2pInvite(mGroup, config.deviceAddress)) {
                        mPeers.updateStatus(config.deviceAddress, WifiP2pDevice.INVITED);
                        sendPeersChangedBroadcast();
                        sendActionListenerSuccess(message.arg2);
                    } else {
                        sendActionListenerFailure(message.arg2, WigigP2pManager.ERROR);
                    }
                    // TODO: figure out updating the status to declined when invitation is rejected
                    break;
                case WigigMonitor.P2P_INVITATION_RESULT_EVENT:
                    P2pStatus status = (P2pStatus)message.obj;
                    if (status == P2pStatus.SUCCESS) {
                        // invocation was succeeded.
                        break;
                    }
                    loge("Invitation result " + status);
                    if (status == P2pStatus.UNKNOWN_P2P_GROUP) {
                        // target device has already removed the credential.
                        // So, remove this credential accordingly.
                        int netId = mGroup.getNetworkId();
                        if (netId >= 0) {
                            if (DBG) logd("Remove unknown client from the list");
                            if (!removeClientFromList(netId,
                                    mSavedPeerConfig.deviceAddress, false)) {
                                // not found the client on the list
                                loge("Already removed the client, ignore");
                                break;
                            }
                            // try invitation.
                            sendMessage(WigigP2pManager.CONNECT, mSavedPeerConfig);
                        }
                    }
                    break;
                case WigigMonitor.P2P_PROV_DISC_PBC_REQ_EVENT:
                case WigigMonitor.P2P_PROV_DISC_ENTER_PIN_EVENT:
                case WigigMonitor.P2P_PROV_DISC_SHOW_PIN_EVENT:
                    WifiP2pProvDiscEvent provDisc = (WifiP2pProvDiscEvent) message.obj;
                    mSavedPeerConfig = new WifiP2pConfig();
                    mSavedPeerConfig.deviceAddress = provDisc.device.deviceAddress;
                    if (message.what == WigigMonitor.P2P_PROV_DISC_ENTER_PIN_EVENT) {
                        mSavedPeerConfig.wps.setup = WpsInfo.KEYPAD;
                    } else if (message.what == WigigMonitor.P2P_PROV_DISC_SHOW_PIN_EVENT) {
                        mSavedPeerConfig.wps.setup = WpsInfo.DISPLAY;
                        mSavedPeerConfig.wps.pin = provDisc.pin;
                    } else {
                        mSavedPeerConfig.wps.setup = WpsInfo.PBC;
                    }
                    transitionTo(mUserAuthorizingJoinState);
                    break;
                case WigigMonitor.P2P_GROUP_STARTED_EVENT:
                    loge("Duplicate group creation event notice, ignore");
                    break;
                case WigigP2pManager.DISCOVER_PEERS:
                    /* currently we do not support discovery while connected */
                    sendActionListenerFailure(message.arg2, WigigP2pManager.BUSY);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }

        public void exit() {
            updateThisDevice(WifiP2pDevice.AVAILABLE);
            resetWifiP2pInfo();
            mNetworkInfo.setDetailedState(NetworkInfo.DetailedState.DISCONNECTED, null, null);
            sendP2pConnectionChangedBroadcast();
            // TODO move P2P override to a common "P2P operation running" state when
            // implementing Wigig-P2P state machine handover
            mWigigMonitor.setP2pOverride(false);
        }
    }

    class UserAuthorizingJoinState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
            notifyInvitationReceived();
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                case WigigMonitor.P2P_PROV_DISC_PBC_REQ_EVENT:
                case WigigMonitor.P2P_PROV_DISC_ENTER_PIN_EVENT:
                case WigigMonitor.P2P_PROV_DISC_SHOW_PIN_EVENT:
                    //Ignore more client requests
                    break;
                case PEER_CONNECTION_USER_ACCEPT:
                    //Stop discovery to avoid failure due to channel switch
                    mWigigNative.p2pStopFind();
                    if (mSavedPeerConfig.wps.setup == WpsInfo.PBC) {
                        mWigigNative.startWpsPbc(mGroup.getInterface(), null);
                    } else {
                        mWigigNative.startWpsPinKeypad(mGroup.getInterface(),
                                mSavedPeerConfig.wps.pin);
                    }
                    transitionTo(mGroupCreatedState);
                    break;
                case PEER_CONNECTION_USER_REJECT:
                    if (DBG) logd("User rejected incoming request");
                    transitionTo(mGroupCreatedState);
                    break;
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }

        @Override
        public void exit() {
            //TODO: dismiss dialog if not already done
        }
    }

    class OngoingGroupRemovalState extends State {
        @Override
        public void enter() {
            if (DBG) logd(getName());
        }

        @Override
        public boolean processMessage(Message message) {
            if (DBG) logd(getName() + message.toString());
            switch (message.what) {
                // Group removal ongoing. Multiple calls
                // end up removing persisted network. Do nothing.
                case WigigP2pManager.REMOVE_GROUP:
                    sendActionListenerSuccess(message.arg2);
                    break;
                // Parent state will transition out of this state
                // when removal is complete
                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    @Override
    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        super.dump(fd, pw, args);
        pw.println("mWifiP2pInfo " + mWifiP2pInfo);
        pw.println("mGroup " + mGroup);
        pw.println("mSavedPeerConfig " + mSavedPeerConfig);
        pw.println();
    }

    private void sendP2pStateChangedBroadcast(boolean enabled) {
        final Intent intent = new Intent(WigigP2pManager.WIGIG_P2P_STATE_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        if (enabled) {
            intent.putExtra(WigigP2pManager.EXTRA_WIGIG_STATE,
                    WigigP2pManager.WIGIG_P2P_STATE_ENABLED);
        } else {
            intent.putExtra(WigigP2pManager.EXTRA_WIGIG_STATE,
                    WigigP2pManager.WIGIG_P2P_STATE_DISABLED);
        }
        mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
    }

    private void sendP2pDiscoveryChangedBroadcast(boolean started) {
        if (mDiscoveryStarted == started) return;
        mDiscoveryStarted = started;

        if (DBG) logd("discovery change broadcast " + started);

        final Intent intent = new Intent(WigigP2pManager.WIGIG_P2P_DISCOVERY_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        intent.putExtra(WigigP2pManager.EXTRA_DISCOVERY_STATE, started ?
                WigigP2pManager.WIGIG_P2P_DISCOVERY_STARTED :
                WigigP2pManager.WIGIG_P2P_DISCOVERY_STOPPED);
        mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
    }

    private void sendThisDeviceChangedBroadcast() {
        final Intent intent = new Intent(WigigP2pManager.WIGIG_P2P_THIS_DEVICE_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        intent.putExtra(WigigP2pManager.EXTRA_WIGIG_P2P_DEVICE, new WifiP2pDevice(mThisDevice));
        mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
    }

    private void sendPeersChangedBroadcast() {
        final Intent intent = new Intent(WigigP2pManager.WIGIG_P2P_PEERS_CHANGED_ACTION);
        intent.putExtra(WigigP2pManager.EXTRA_P2P_DEVICE_LIST, new WifiP2pDeviceList(mPeers));
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
    }

    private void sendP2pConnectionChangedBroadcast() {
        if (DBG) logd("sending p2p connection changed broadcast");
        Intent intent = new Intent(WigigP2pManager.WIGIG_P2P_CONNECTION_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT
                | Intent.FLAG_RECEIVER_REPLACE_PENDING);
        intent.putExtra(WigigP2pManager.EXTRA_WIGIG_P2P_INFO, new WifiP2pInfo(mWifiP2pInfo));
        intent.putExtra(WigigP2pManager.EXTRA_NETWORK_INFO, new NetworkInfo(mNetworkInfo));
        intent.putExtra(WigigP2pManager.EXTRA_WIGIG_P2P_GROUP, new WifiP2pGroup(mGroup));
        mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
        mWigigChannel.sendMessage(WigigP2pServiceImpl.P2P_CONNECTION_CHANGED,
                new NetworkInfo(mNetworkInfo));
    }

    private void sendP2pPersistentGroupsChangedBroadcast() {
        if (DBG) logd("sending p2p persistent groups changed broadcast");
        Intent intent = new Intent(WigigP2pManager.WIGIG_P2P_PERSISTENT_GROUPS_CHANGED_ACTION);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
    }

    private void startDhcpServer(String intf) {
        InterfaceConfiguration ifcg = null;
        try {
            ifcg = mNwService.getInterfaceConfig(intf);
            ifcg.setLinkAddress(new LinkAddress(NetworkUtils.numericToInetAddress(
                        SERVER_ADDRESS), 24));
            ifcg.setInterfaceUp();
            mNwService.setInterfaceConfig(intf, ifcg);
            String[] tetheringDhcpRanges = SERVER_DHCP_RANGE;
            if (mNwService.isTetheringStarted()) {
                if (DBG) logd("Stop existing tethering and restart it");
                mNwService.stopTethering();
            }
            mNwService.tetherInterface(intf);
            mNwService.startTethering(tetheringDhcpRanges);
        } catch (Exception e) {
            loge("Error configuring interface " + intf + ", :" + e);
            return;
        }

        logd("Started Dhcp server on " + intf);
   }

    private void stopDhcpServer(String intf) {
        try {
            mNwService.untetherInterface(intf);
            for (String temp : mNwService.listTetheredInterfaces()) {
                logd("List all interfaces " + temp);
                if (temp.compareTo(intf) != 0) {
                    logd("Found other tethering interfaces, so keep tethering alive");
                    return;
                }
            }
            mNwService.stopTethering();
        } catch (Exception e) {
            loge("Error stopping Dhcp server" + e);
            return;
        } finally {
            logd("Stopped Dhcp server");
        }
    }

    private void notifyP2pEnableFailure() {
        AlertDialog dialog = new AlertDialog.Builder(mContext)
            .setTitle(mWifiRes.getString(getWifiResId("string", "wifi_p2p_dialog_title")))
            .setMessage(mWifiRes.getString(getWifiResId("string", "wifi_p2p_failed_message")))
            .setPositiveButton(mWifiRes.getString(getWifiResId("string", "ok")), null)
            .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        WindowManager.LayoutParams attrs = dialog.getWindow().getAttributes();
        attrs.privateFlags = WindowManager.LayoutParams.SYSTEM_FLAG_SHOW_FOR_ALL_USERS;
        dialog.getWindow().setAttributes(attrs);
        dialog.show();
    }

    private void addRowToDialog(LayoutInflater li, ViewGroup group, int stringId, String value) {
        View row = li.inflate(getWifiResId("layout", "wifi_p2p_dialog_row"), group, false);
        ((TextView) row.findViewById(getWifiResId("id", "name"))).setText(mWifiRes.getString(stringId));
        ((TextView) row.findViewById(getWifiResId("id", "value"))).setText(value);
        group.addView(row);
    }

    private void notifyInvitationSent(String pin, String peerAddress) {
        LayoutInflater li = LayoutInflater.from(mWifiResContext);
        if (li == null) {
            loge("fail to get layout infater, do not show invitation sent dialog");
            return;
        }

        final View textEntryView = li.inflate(getWifiResId("layout", "wifi_p2p_dialog"), null);

        ViewGroup group = (ViewGroup) textEntryView.findViewById(getWifiResId("id", "info"));
        addRowToDialog(li, group, getWifiResId("string", "wifi_p2p_to_message"), getDeviceName(peerAddress));
        addRowToDialog(li, group, getWifiResId("string", "wifi_p2p_show_pin_message"), pin);

        AlertDialog dialog = new AlertDialog.Builder(mContext)
            .setTitle(mWifiRes.getString(getWifiResId("string", "wifi_p2p_invitation_sent_title")))
            .setView(textEntryView)
            .setPositiveButton(mWifiRes.getString(getWifiResId("string", "ok")), null)
            .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        WindowManager.LayoutParams attrs = dialog.getWindow().getAttributes();
        attrs.privateFlags = WindowManager.LayoutParams.SYSTEM_FLAG_SHOW_FOR_ALL_USERS;
        dialog.getWindow().setAttributes(attrs);
        dialog.show();
    }

    private void notifyInvitationReceived() {
        LayoutInflater li = LayoutInflater.from(mWifiResContext);
        if (li == null) {
            loge("fail to get layout infater, do not show invitation dialog");
            return;
        }

        final WpsInfo wps = mSavedPeerConfig.wps;
        final View textEntryView = li.inflate(getWifiResId("layout", "wifi_p2p_dialog"), null);

        if (textEntryView == null) {
            loge("fail to get text entry view, do not show invitation dialog");
            return;
        }

        ViewGroup group = (ViewGroup) textEntryView.findViewById(getWifiResId("id", "info"));
        addRowToDialog(li, group, getWifiResId("string", "wifi_p2p_from_message"), getDeviceName(
                mSavedPeerConfig.deviceAddress));

        final EditText pin = (EditText) textEntryView.findViewById(getWifiResId("id", "wifi_p2p_wps_pin"));

        AlertDialog dialog = new AlertDialog.Builder(mContext)
            .setTitle(mWifiRes.getString(getWifiResId("string", "wifi_p2p_invitation_to_connect_title")))
            .setView(textEntryView)
            .setPositiveButton(mWifiRes.getString(getWifiResId("string", "accept")), new OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            if (wps.setup == WpsInfo.KEYPAD) {
                                mSavedPeerConfig.wps.pin = pin.getText().toString();
                            }
                            if (DBG) logd(getName() + " accept invitation " + mSavedPeerConfig);
                            sendMessage(PEER_CONNECTION_USER_ACCEPT);
                        }
                    })
            .setNegativeButton(mWifiRes.getString(getWifiResId("string", "decline")), new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            if (DBG) logd(getName() + " ignore connect");
                            sendMessage(PEER_CONNECTION_USER_REJECT);
                        }
                    })
            .setOnCancelListener(new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface arg0) {
                            if (DBG) logd(getName() + " ignore connect");
                            sendMessage(PEER_CONNECTION_USER_REJECT);
                        }
                    })
            .create();

        //make the enter pin area or the display pin area visible
        switch (wps.setup) {
            case WpsInfo.KEYPAD:
                if (DBG) logd("Enter pin section visible");
                textEntryView.findViewById(getWifiResId("id", "enter_pin_section")).setVisibility(View.VISIBLE);
                break;
            case WpsInfo.DISPLAY:
                if (DBG) logd("Shown pin section visible");
                addRowToDialog(li, group, getWifiResId("string", "wifi_p2p_show_pin_message"), wps.pin);
                break;
            default:
                break;
        }

        if ((mWifiRes.getConfiguration().uiMode & Configuration.UI_MODE_TYPE_APPLIANCE) ==
                Configuration.UI_MODE_TYPE_APPLIANCE) {
            // For appliance devices, add a key listener which accepts.
            dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {

                @Override
                public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                    // TODO: make the actual key come from a config value.
                    if (keyCode == KeyEvent.KEYCODE_VOLUME_MUTE) {
                        sendMessage(PEER_CONNECTION_USER_ACCEPT);
                        dialog.dismiss();
                        return true;
                    }
                    return false;
                }
            });
            // TODO: add timeout for this dialog.
            // TODO: update UI in appliance mode to tell user what to do.
        }

        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        WindowManager.LayoutParams attrs = dialog.getWindow().getAttributes();
        attrs.privateFlags = WindowManager.LayoutParams.SYSTEM_FLAG_SHOW_FOR_ALL_USERS;
        dialog.getWindow().setAttributes(attrs);
        dialog.show();
    }

    /**
     * Synchronize the persistent group list between
     * wpa_supplicant and mGroups.
     */
    private void updatePersistentNetworks(boolean reload) {
        String listStr = mWigigNative.listNetworks();
        if (listStr == null) return;

        boolean isSaveRequired = false;
        String[] lines = listStr.split("\n");
        if (lines == null) return;

        if (reload) mGroups.clear();

        // Skip the first line, which is a header
        for (int i = 1; i < lines.length; i++) {
            String[] result = lines[i].split("\t");
            if (result == null || result.length < 4) {
                continue;
            }
            // network-id | ssid | bssid | flags
            int netId = -1;
            String ssid = result[1];
            String bssid = result[2];
            String flags = result[3];
            try {
                netId = Integer.parseInt(result[0]);
            } catch(NumberFormatException e) {
                e.printStackTrace();
                continue;
            }

            if (flags.indexOf("[CURRENT]") != -1) {
                continue;
            }
            if (flags.indexOf("[P2P-PERSISTENT]") == -1) {
                /*
                 * The unused profile is sometimes remained when the p2p group formation is failed.
                 * So, we clean up the p2p group here.
                 */
                if (DBG) logd("clean up the unused persistent group. netId=" + netId);
                mWigigNative.removeNetwork(netId);
                isSaveRequired = true;
                continue;
            }

            if (mGroups.contains(netId)) {
                continue;
            }

            WifiP2pGroup group = new WifiP2pGroup();
            group.setNetworkId(netId);
            group.setNetworkName(ssid);
            String mode = mWigigNative.getNetworkVariable(netId, "mode");
            if (mode != null && mode.equals("3")) {
                group.setIsGroupOwner(true);
            }
            if (bssid.equalsIgnoreCase(mThisDevice.deviceAddress)) {
                group.setOwner(mThisDevice);
            } else {
                WifiP2pDevice device = new WifiP2pDevice();
                device.deviceAddress = bssid;
                group.setOwner(device);
            }
            mGroups.add(group);
            isSaveRequired = true;
        }

        if (reload || isSaveRequired) {
            mWigigNative.saveConfig();
            sendP2pPersistentGroupsChangedBroadcast();
        }
    }

    /**
     * A config is valid if it has a peer address that has already been
     * discovered
     * @return true if it is invalid, false otherwise
     */
    private boolean isConfigInvalid(WifiP2pConfig config) {
        if (config == null) return true;
        if (TextUtils.isEmpty(config.deviceAddress)) return true;
        if (mPeers.get(config.deviceAddress) == null) return true;
        return false;
    }

    /* TODO: The supplicant does not provide group capability changes as an event.
     * Having it pushed as an event would avoid polling for this information right
     * before a connection
     */
    private WifiP2pDevice fetchCurrentDeviceDetails(WifiP2pConfig config) {
        /* Fetch & update group capability from supplicant on the device */
        int gc = mWigigNative.getGroupCapability(config.deviceAddress);
        mPeers.updateGroupCapability(config.deviceAddress, gc);
        return mPeers.get(config.deviceAddress);
    }

    /**
     * Start a p2p group negotiation and display pin if necessary
     * @param config for the peer
     */
    private void p2pConnectWithPinDisplay(WifiP2pConfig config) {
        WifiP2pDevice dev = fetchCurrentDeviceDetails(config);

        String pin = mWigigNative.p2pConnect(config, dev.isGroupOwner());
        try {
            Integer.parseInt(pin);
            notifyInvitationSent(pin, config.deviceAddress);
        } catch (NumberFormatException ignore) {
            // do nothing if p2pConnect did not return a pin
        }
    }

    /**
     * Reinvoke a persistent group.
     *
     * @param config for the peer
     * @return true on success, false on failure
     */
    private boolean reinvokePersistentGroup(WifiP2pConfig config) {
        WifiP2pDevice dev = fetchCurrentDeviceDetails(config);

        boolean join = dev.isGroupOwner();
        String ssid = mWigigNative.p2pGetSsid(dev.deviceAddress);
        if (DBG) logd("target ssid is " + ssid + " join:" + join);

        if (join && dev.isGroupLimit()) {
            if (DBG) logd("target device reaches group limit.");

            // if the target group has reached the limit,
            // try group formation.
            join = false;
        } else if (join) {
            int netId = mGroups.getNetworkId(dev.deviceAddress, ssid);
            if (netId >= 0) {
                // Skip WPS and start 4way handshake immediately.
                if (!mWigigNative.p2pGroupAdd(netId)) {
                    return false;
                }
                return true;
            }
        }

        if (!join && dev.isDeviceLimit()) {
            loge("target device reaches the device limit.");
            return false;
        }

        if (!join && dev.isInvitationCapable()) {
            int netId = WifiP2pGroup.PERSISTENT_NET_ID;
            if (config.netId >= 0) {
                if (config.deviceAddress.equals(mGroups.getOwnerAddr(config.netId))) {
                    netId = config.netId;
                }
            } else {
                netId = mGroups.getNetworkId(dev.deviceAddress);
            }
            if (netId < 0) {
                netId = getNetworkIdFromClientList(dev.deviceAddress);
            }
            if (DBG) logd("netId related with " + dev.deviceAddress + " = " + netId);
            if (netId >= 0) {
                // Invoke the persistent group.
                if (mWigigNative.p2pReinvoke(netId, dev.deviceAddress)) {
                    // Save network id. It'll be used when an invitation result event is received.
                    config.netId = netId;
                    return true;
                } else {
                    loge("p2pReinvoke() failed, update networks");
                    updatePersistentNetworks(RELOAD);
                    return false;
                }
            }
        }

        return false;
    }

    /**
     * Return the network id of the group owner profile which has the p2p client with
     * the specified device address in it's client list.
     * If more than one persistent group of the same address is present in its client
     * lists, return the first one.
     *
     * @param deviceAddress p2p device address.
     * @return the network id. if not found, return -1.
     */
    private int getNetworkIdFromClientList(String deviceAddress) {
        if (deviceAddress == null) return -1;

        Collection<WifiP2pGroup> groups = mGroups.getGroupList();
        for (WifiP2pGroup group : groups) {
            int netId = group.getNetworkId();
            String[] p2pClientList = getClientList(netId);
            if (p2pClientList == null) continue;
            for (String client : p2pClientList) {
                if (deviceAddress.equalsIgnoreCase(client)) {
                    return netId;
                }
            }
        }
        return -1;
    }

    /**
     * Return p2p client list associated with the specified network id.
     * @param netId network id.
     * @return p2p client list. if not found, return null.
     */
    private String[] getClientList(int netId) {
        String p2pClients = mWigigNative.getNetworkVariable(netId, "p2p_client_list");
        if (p2pClients == null) {
            return null;
        }
        return p2pClients.split(" ");
    }

    /**
     * Remove the specified p2p client from the specified profile.
     * @param netId network id of the profile.
     * @param addr p2p client address to be removed.
     * @param isRemovable if true, remove the specified profile if its client list becomes empty.
     * @return whether removing the specified p2p client is successful or not.
     */
    private boolean removeClientFromList(int netId, String addr, boolean isRemovable) {
        StringBuilder modifiedClientList =  new StringBuilder();
        String[] currentClientList = getClientList(netId);
        boolean isClientRemoved = false;
        if (currentClientList != null) {
            for (String client : currentClientList) {
                if (!client.equalsIgnoreCase(addr)) {
                    modifiedClientList.append(" ");
                    modifiedClientList.append(client);
                } else {
                    isClientRemoved = true;
                }
            }
        }
        if (modifiedClientList.length() == 0 && isRemovable) {
            // the client list is empty. so remove it.
            if (DBG) logd("Remove unknown network");
            mGroups.remove(netId);
            return true;
        }

        if (!isClientRemoved) {
            // specified p2p client is not found. already removed.
            return false;
        }

        if (DBG) logd("Modified client list: " + modifiedClientList);
        if (modifiedClientList.length() == 0) {
            modifiedClientList.append("\"\"");
        }
        mWigigNative.setNetworkVariable(netId,
                "p2p_client_list", modifiedClientList.toString());
        mWigigNative.saveConfig();
        return true;
    }

    private void setWifiP2pInfoOnGroupFormation(InetAddress serverInetAddress) {
        mWifiP2pInfo.groupFormed = true;
        mWifiP2pInfo.isGroupOwner = mGroup.isGroupOwner();
        mWifiP2pInfo.groupOwnerAddress = serverInetAddress;
    }

    private void resetWifiP2pInfo() {
        mWifiP2pInfo.groupFormed = false;
        mWifiP2pInfo.isGroupOwner = false;
        mWifiP2pInfo.groupOwnerAddress = null;
    }

    private String getDeviceName(String deviceAddress) {
        WifiP2pDevice d = mPeers.get(deviceAddress);
        if (d != null) {
                return d.deviceName;
        }
        //Treat the address as name if there is no match
        return deviceAddress;
    }

    private String getPersistedDeviceName() {
        // TODO need to use WIGIG settings
        String deviceName = Settings.Global.getString(mContext.getContentResolver(),
                Settings.Global.WIFI_P2P_DEVICE_NAME);
        if (deviceName == null) {
            /* We use the 4 digits of the ANDROID_ID to have a friendly
             * default that has low likelihood of collision with a peer */
            String id = Settings.Secure.getString(mContext.getContentResolver(),
                    Settings.Secure.ANDROID_ID);
            return "Android_" + id.substring(0,4);
        }
        return deviceName;
    }

    private boolean setAndPersistDeviceName(String devName) {
        if (devName == null) return false;

        if (!mWigigNative.setDeviceName(devName)) {
            loge("Failed to set device name " + devName);
            return false;
        }

        mThisDevice.deviceName = devName;
        mWigigNative.setP2pSsidPostfix("-" + mThisDevice.deviceName);

        Settings.Global.putString(mContext.getContentResolver(),
                Settings.Global.WIFI_P2P_DEVICE_NAME, devName);
        sendThisDeviceChangedBroadcast();
        return true;
    }

    private boolean setWfdInfo(WifiP2pWfdInfo wfdInfo) {
        boolean success;

        if (!wfdInfo.isWfdEnabled()) {
            success = mWigigNative.setWfdEnable(false);
        } else {
            // wpa_supplicant expects the IE length to be included in the string
            // but WifiP2PWfdInfo.getDeviceInfoHex does not include it, so add here
            String hex = wfdInfo.getDeviceInfoHex();
            int len = hex.length() / 2;
            hex = String.format("%04X", len) + hex;
            success =
                mWigigNative.setWfdEnable(true)
                && mWigigNative.setWfdDeviceInfo(hex);
        }

        if (!success) {
            loge("Failed to set wfd properties");
            return false;
        }

        mThisDevice.wfdInfo = wfdInfo;
        sendThisDeviceChangedBroadcast();
        return true;
    }

    private void initializeP2pSettings() {
        mWigigNative.setPersistentReconnect(true);
        mThisDevice.deviceName = getPersistedDeviceName();
        mWigigNative.setDeviceName(mThisDevice.deviceName);
        // DIRECT-XY-DEVICENAME (XY is randomly generated)
        mWigigNative.setP2pSsidPostfix("-" + mThisDevice.deviceName);
        mWigigNative.setDeviceType(mThisDevice.primaryDeviceType);
        // Supplicant defaults to using virtual display with display
        // which refers to a remote display. Use physical_display
        mWigigNative.setConfigMethods("virtual_push_button physical_display keypad");
        // STA has higher priority over P2P
        mWigigNative.setConcurrencyPriority("sta");

        mThisDevice.deviceAddress = mWigigNative.p2pGetDeviceAddress();
        updateThisDevice(WifiP2pDevice.AVAILABLE);
        if (DBG) logd("DeviceAddress: " + mThisDevice.deviceAddress);

        mClientInfoList.clear();
        mWigigNative.p2pFlush();
        mWigigNative.p2pServiceFlush();
        mServiceTransactionId = 0;
        mServiceDiscReqId = null;

        updatePersistentNetworks(RELOAD);
    }

    private void updateThisDevice(int status) {
        mThisDevice.status = status;
        sendThisDeviceChangedBroadcast();
    }

    private void handleGroupCreationFailure() {
        resetWifiP2pInfo();
        mNetworkInfo.setDetailedState(NetworkInfo.DetailedState.FAILED, null, null);
        sendP2pConnectionChangedBroadcast();

        // Remove only the peer we failed to connect to so that other devices discovered
        // that have not timed out still remain in list for connection
        boolean peersChanged = mPeers.remove(mPeersLostDuringConnection);
        if (TextUtils.isEmpty(mSavedPeerConfig.deviceAddress) == false &&
                mPeers.remove(mSavedPeerConfig.deviceAddress) != null) {
            peersChanged = true;
        }
        if (peersChanged) {
            sendPeersChangedBroadcast();
        }

        mPeersLostDuringConnection.clear();
        mServiceDiscReqId = null;
        sendMessage(WigigP2pManager.DISCOVER_PEERS);
    }

    private void handleGroupRemoved() {
        if (mGroup.isGroupOwner()) {
            stopDhcpServer(mGroup.getInterface());
        } else {
            if (DBG) logd("stop IpClient");
            stopIpClient();
            try {
                mNwService.removeInterfaceFromLocalNetwork(mGroup.getInterface());
            } catch (RemoteException e) {
                loge("Failed to remove iface from local network " + e);
            }
        }

        try {
            mNwService.clearInterfaceAddresses(mGroup.getInterface());
        } catch (Exception e) {
            loge("Failed to clear addresses " + e);
        }

        // Clear any timeout that was set. This is essential for devices
        // that reuse the main p2p interface for a created group.
        mWigigNative.setP2pGroupIdle(mGroup.getInterface(), 0);

        boolean peersChanged = false;
        // Remove only peers part of the group, so that other devices discovered
        // that have not timed out still remain in list for connection
        for (WifiP2pDevice d : mGroup.getClientList()) {
            if (mPeers.remove(d)) peersChanged = true;
        }
        if (mPeers.remove(mGroup.getOwner())) peersChanged = true;
        if (mPeers.remove(mPeersLostDuringConnection)) peersChanged = true;
        if (peersChanged) {
            sendPeersChangedBroadcast();
        }

        mGroup = null;
        mPeersLostDuringConnection.clear();
        mServiceDiscReqId = null;

        if (mTemporarilyDisconnectedWigig) {
            mWigigChannel.sendMessage(WigigP2pServiceImpl.DISCONNECT_WIGIG_REQUEST, 0);
            mTemporarilyDisconnectedWigig = false;
        }
   }

    //State machine initiated requests can have replyTo set to null indicating
    //there are no recipients, we ignore those reply actions
    private void replyToMessage(Message msg, int what) {
        if (msg.replyTo == null) return;
        Message dstMsg = obtainMessage(msg);
        dstMsg.what = what;
        mReplyChannel.replyToMessage(msg, dstMsg);
    }

    private void replyToMessage(Message msg, int what, int arg1) {
        if (msg.replyTo == null) return;
        Message dstMsg = obtainMessage(msg);
        dstMsg.what = what;
        dstMsg.arg1 = arg1;
        mReplyChannel.replyToMessage(msg, dstMsg);
    }

    private void replyToMessage(Message msg, int what, Object obj) {
        if (msg.replyTo == null) return;
        Message dstMsg = obtainMessage(msg);
        dstMsg.what = what;
        dstMsg.obj = obj;
        mReplyChannel.replyToMessage(msg, dstMsg);
    }

    /* arg2 on the source message has a hash code that needs to be retained in replies
     * see WigigP2pManager for details */
    private Message obtainMessage(Message srcMsg) {
        Message msg = Message.obtain();
        msg.arg2 = srcMsg.arg2;
        return msg;
    }

    @Override
    protected void logd(String s) {
        WLog.v(TAG, s);
    }

    @Override
    protected void loge(String s) {
        Slog.e(TAG, s);
    }

    /**
     * Update service discovery request to wpa_supplicant.
     */
    private boolean updateSupplicantServiceRequest() {
        clearSupplicantServiceRequest();

        StringBuffer sb = new StringBuffer();
        for (ClientInfo c: mClientInfoList.values()) {
            int key;
            WifiP2pServiceRequest req;
            for (int i=0; i < c.mReqList.size(); i++) {
                req = c.mReqList.valueAt(i);
                if (req != null) {
                    sb.append(req.getSupplicantQuery());
                }
            }
        }

        if (sb.length() == 0) {
            return false;
        }

        mServiceDiscReqId = mWigigNative.p2pServDiscReq("00:00:00:00:00:00", sb.toString());
        if (mServiceDiscReqId == null) {
            return false;
        }
        return true;
    }

    /**
     * Clear service discovery request in wpa_supplicant
     */
    private void clearSupplicantServiceRequest() {
        if (mServiceDiscReqId == null) return;

        mWigigNative.p2pServDiscCancelReq(mServiceDiscReqId);
        mServiceDiscReqId = null;
    }

    /* TODO: We could track individual service adds separately and avoid
     * having to do update all service requests on every new request
     */
    private boolean addServiceRequest(Messenger m, WifiP2pServiceRequest req) {
        clearClientDeadChannels();
        ClientInfo clientInfo = getClientInfo(m, true);
        if (clientInfo == null) {
            return false;
        }

        ++mServiceTransactionId;
        //The WiGig p2p spec says transaction id should be non-zero
        if (mServiceTransactionId == 0) ++mServiceTransactionId;
        req.setTransactionId(mServiceTransactionId);
        clientInfo.mReqList.put(mServiceTransactionId, req);

        if (mServiceDiscReqId == null) {
            return true;
        }

        return updateSupplicantServiceRequest();
    }

    private void removeServiceRequest(Messenger m, WifiP2pServiceRequest req) {
        ClientInfo clientInfo = getClientInfo(m, false);
        if (clientInfo == null) {
            return;
        }

        //Application does not have transaction id information
        //go through stored requests to remove
        boolean removed = false;
        for (int i=0; i<clientInfo.mReqList.size(); i++) {
            if (req.equals(clientInfo.mReqList.valueAt(i))) {
                removed = true;
                clientInfo.mReqList.removeAt(i);
                break;
            }
        }

        if (!removed) return;

        if (clientInfo.mReqList.size() == 0 && clientInfo.mServList.size() == 0) {
            if (DBG) logd("remove client information from framework");
            mClientInfoList.remove(clientInfo.mMessenger);
        }

        if (mServiceDiscReqId == null) {
            return;
        }

        updateSupplicantServiceRequest();
    }

    private void clearServiceRequests(Messenger m) {

        ClientInfo clientInfo = getClientInfo(m, false);
        if (clientInfo == null) {
            return;
        }

        if (clientInfo.mReqList.size() == 0) {
            return;
        }

        clientInfo.mReqList.clear();

        if (clientInfo.mServList.size() == 0) {
            if (DBG) logd("remove channel information from framework");
            mClientInfoList.remove(clientInfo.mMessenger);
        }

        if (mServiceDiscReqId == null) {
            return;
        }

        updateSupplicantServiceRequest();
    }

    private boolean addLocalService(Messenger m, WifiP2pServiceInfo servInfo) {
        clearClientDeadChannels();
        ClientInfo clientInfo = getClientInfo(m, true);
        if (clientInfo == null) {
            return false;
        }

        if (!clientInfo.mServList.add(servInfo)) {
            return false;
        }

        if (!mWigigNative.p2pServiceAdd(servInfo)) {
            clientInfo.mServList.remove(servInfo);
            return false;
        }

        return true;
    }

    private void removeLocalService(Messenger m, WifiP2pServiceInfo servInfo) {
        ClientInfo clientInfo = getClientInfo(m, false);
        if (clientInfo == null) {
            return;
        }

        mWigigNative.p2pServiceDel(servInfo);

        clientInfo.mServList.remove(servInfo);
        if (clientInfo.mReqList.size() == 0 && clientInfo.mServList.size() == 0) {
            if (DBG) logd("remove client information from framework");
            mClientInfoList.remove(clientInfo.mMessenger);
        }
    }

    private void clearLocalServices(Messenger m) {
        ClientInfo clientInfo = getClientInfo(m, false);
        if (clientInfo == null) {
            return;
        }

        for (WifiP2pServiceInfo servInfo: clientInfo.mServList) {
            mWigigNative.p2pServiceDel(servInfo);
        }

        clientInfo.mServList.clear();
        if (clientInfo.mReqList.size() == 0) {
            if (DBG) logd("remove client information from framework");
            mClientInfoList.remove(clientInfo.mMessenger);
        }
    }

    private void clearClientInfo(Messenger m) {
        clearLocalServices(m);
        clearServiceRequests(m);
    }

    private void setServiceResponseListener(Messenger client, IBinder binder,
            IServiceResponseListener callback) {
        ClientInfo clientInfo = getClientInfo(client, true);
        if (clientInfo == null) {
            loge("null clientInfo, should never happen");
            return;
        }

        // only one callback is supported
        if (callback != null) {
            clientInfo.mServResponseListeners.add(binder, callback, 0);
        } else {
            clientInfo.mServResponseListeners.clear();
        }
    }

    private void setDnsSdServiceResponseListener(Messenger client, IBinder binder,
            IDnsSdServiceResponseListener callback) {
        ClientInfo clientInfo = getClientInfo(client, true);
        if (clientInfo == null) {
            loge("null clientInfo, should never happen");
            return;
        }

        // only one callback is supported
        if (callback != null) {
            clientInfo.mDnsSdServResponseListeners.add(binder, callback, 0);
        } else {
            clientInfo.mDnsSdServResponseListeners.clear();
        }
    }

    private void setDnsSdTxtRecordListener(Messenger client, IBinder binder,
            IDnsSdTxtRecordListener callback) {
        ClientInfo clientInfo = getClientInfo(client, true);
        if (clientInfo == null) {
            loge("null clientInfo, should never happen");
            return;
        }

        // only one callback is supported
        if (callback != null) {
            clientInfo.mDnsSdTxtRecordListeners.add(binder, callback, 0);
        } else {
            clientInfo.mDnsSdTxtRecordListeners.clear();
        }
    }

    private void setUpnpServiceResponseListener(Messenger client, IBinder binder,
            IUpnpServiceResponseListener callback) {
        ClientInfo clientInfo = getClientInfo(client, true);
        if (clientInfo == null) {
            loge("null clientInfo, should never happen");
            return;
        }

        // only one callback is supported
        if (callback != null) {
            clientInfo.mUpnpServResponseListeners.add(binder, callback, 0);
        } else {
            clientInfo.mUpnpServResponseListeners.clear();
        }
    }

    /**
     * Send the service response to the WigigP2pManager.Channel.
     *
     * @param resp
     */
    private void sendServiceResponse(WifiP2pServiceResponse resp) {
        if (resp instanceof WifiP2pDnsSdServiceResponse) {
            sendDnsSdServiceResponse((WifiP2pDnsSdServiceResponse)resp);
        } else if (resp instanceof WifiP2pUpnpServiceResponse) {
            sendUpnpServiceResponse((WifiP2pUpnpServiceResponse)resp);
        } else {
            for (ClientInfo c : mClientInfoList.values()) {
                WifiP2pServiceRequest req = c.mReqList.get(resp.getTransactionId());
                IServiceResponseListener listener = c.mServResponseListeners.get(0);
                if (req != null && listener != null) {
                    try {
                        listener.onServiceAvailable(resp.getServiceType(), resp.getRawData(),
                            resp.getSrcDevice());
                    } catch (RemoteException e) {
                        if (DBG) logd("detect dead channel");
                        clearClientInfo(c.mMessenger);
                        // continue with other clients
                    }
                }
            }
        }
    }

    private void sendDnsSdServiceResponse(WifiP2pDnsSdServiceResponse resp) {
        for (ClientInfo c : mClientInfoList.values()) {
            WifiP2pServiceRequest req = c.mReqList.get(resp.getTransactionId());
            if (req == null) {
                continue;
            }
            if (resp.getDnsType() == WifiP2pDnsSdServiceInfo.DNS_TYPE_PTR) {
                IDnsSdServiceResponseListener listener = c.mDnsSdServResponseListeners.get(0);
                if (listener != null) {
                    try {
                        listener.onDnsSdServiceAvailable(resp.getInstanceName(),
                            resp.getDnsQueryName(), resp.getSrcDevice());
                    } catch (RemoteException e) {
                        if (DBG) logd("detect dead channel");
                        clearClientInfo(c.mMessenger);
                        // continue with other clients
                    }
                }
            } else if (resp.getDnsType() == WifiP2pDnsSdServiceInfo.DNS_TYPE_TXT) {
                IDnsSdTxtRecordListener listener = c.mDnsSdTxtRecordListeners.get(0);
                if (listener != null) {
                    try {
                        listener.onDnsSdTxtRecordAvailable(resp.getDnsQueryName(),
                            resp.getTxtRecord(), resp.getSrcDevice());
                    } catch (RemoteException e) {
                        if (DBG) logd("detect dead channel");
                        clearClientInfo(c.mMessenger);
                        // continue with other clients
                    }
                }
            }
        }
    }

    private void sendUpnpServiceResponse(WifiP2pUpnpServiceResponse resp) {
        for (ClientInfo c : mClientInfoList.values()) {
            WifiP2pServiceRequest req = c.mReqList.get(resp.getTransactionId());
            IUpnpServiceResponseListener listener = c.mUpnpServResponseListeners.get(0);
            if (req != null && listener != null) {
                try {
                    listener.onUpnpServiceAvailable(resp.getUniqueServiceNames(),
                        resp.getSrcDevice());
                } catch (RemoteException e) {
                    if (DBG) logd("detect dead channel");
                    clearClientInfo(c.mMessenger);
                    // continue with other clients
                }
            }
        }
    }

    /**
     * We dont get notifications of clients that have gone away.
     * We detect this actively when services are added and throw
     * them away.
     *
     * TODO: This can be done better with full async channels.
     */
    private void clearClientDeadChannels() {
        ArrayList<Messenger> deadClients = new ArrayList<Messenger>();

        for (ClientInfo c : mClientInfoList.values()) {
            Message msg = Message.obtain();
            msg.what = WigigP2pManager.PING;
            msg.arg1 = 0;
            msg.arg2 = 0;
            msg.obj = null;
            try {
                c.mMessenger.send(msg);
            } catch (RemoteException e) {
                if (DBG) logd("detect dead channel");
                deadClients.add(c.mMessenger);
            }
        }

        for (Messenger m : deadClients) {
            clearClientInfo(m);
        }
    }

    /**
     * Return the specified ClientInfo.
     * @param m Messenger
     * @param createIfNotExist if true and the specified channel info does not exist,
     * create new client info.
     * @return the specified ClientInfo.
     */
    private ClientInfo getClientInfo(Messenger m, boolean createIfNotExist) {
        ClientInfo clientInfo = mClientInfoList.get(m);

        if (clientInfo == null && createIfNotExist) {
            if (DBG) logd("add a new client");
            clientInfo = new ClientInfo(m);
            mClientInfoList.put(m, clientInfo);
        }

        return clientInfo;
    }

    }

    /**
     * Information about a particular client and we track the service discovery requests
     * and the local services registered by the client.
     */
    private class ClientInfo {

        /*
         * A reference to WigigP2pManager.Channel handler.
         * The response of this request is notified to WigigP2pManager.Channel handler
         */
        private Messenger mMessenger;

        /*
         * A service discovery request list.
         */
        private SparseArray<WifiP2pServiceRequest> mReqList;

        /*
         * A local service information list.
         */
        private List<WifiP2pServiceInfo> mServList;

        /*
         * tracks IServiceResponseListener callback
         */
        private final ExternalCallbackTracker<IServiceResponseListener> mServResponseListeners;

        /*
         * tracks IDnsSdServiceResponseListener callback
         */
        private final ExternalCallbackTracker<IDnsSdServiceResponseListener>
            mDnsSdServResponseListeners;

        /*
         * tracks IDnsSdTxtRecordListener callback
         */
        private final ExternalCallbackTracker<IDnsSdTxtRecordListener> mDnsSdTxtRecordListeners;

        /*
         * tracks IUpnpServiceResponseListener callback
         */
        private final ExternalCallbackTracker<IUpnpServiceResponseListener>
            mUpnpServResponseListeners;

        private ClientInfo(Messenger m) {
            mMessenger = m;
            mReqList = new SparseArray();
            mServList = new ArrayList<WifiP2pServiceInfo>();
            mServResponseListeners = new ExternalCallbackTracker<>(mClientHandler);
            mDnsSdServResponseListeners = new ExternalCallbackTracker<>(mClientHandler);
            mDnsSdTxtRecordListeners = new ExternalCallbackTracker<>(mClientHandler);
            mUpnpServResponseListeners = new ExternalCallbackTracker<>(mClientHandler);
        }
    }
}
