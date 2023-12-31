/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.xdivert;

import android.app.AlertDialog;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Looper;
import android.preference.CheckBoxPreference;
import android.telephony.TelephonyManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.util.Log;
import android.util.AttributeSet;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
//import com.android.phone.TimeConsumingPreferenceActivity;
//import static com.android.phone.TimeConsumingPreferenceActivity.RESPONSE_ERROR;

// This class handles the actual processing required for XDivert feature.
// Handles the checkbox display.

public class XDivertCheckBoxPreference extends CheckBoxPreference {
    private static final String LOG_TAG = "XDivertCheckBoxPreference";
    private final boolean DBG = true; //(PhoneGlobals.DBG_LEVEL >= 2);

    int mNumPhones;
    int mAction; // Holds the CFNRc value.i.e.Registration/Disable
    int mReason; // Holds Call Forward reason.i.e.CF_REASON_NOT_REACHABLE
    Phone[] mPhoneObj; // Holds the phone objects for both the slots
    String[] mLine1Number; // Holds the line numbers for both the slots
    String[] mCFLine1Number;// Holds the CFNRc number for both the slots
    String[] mCFBLine1Number;// Holds the CFB number for both the slots

    // Holds the status of Call Waiting for both the slot
    boolean[] mSlotCallWaiting;

    // Holds the value of XDivert feature
    boolean mXdivertStatus;
    TimeConsumingPreferenceListener mTcpListener;
    private static Context mContext;
    private XDivertUtility mXDivertUtility;

    private static final int SLOT1 = 0;
    private static final int SLOT2 = 1;
    private static final int MESSAGE_GET_CFNRC = 2;
    private static final int MESSAGE_GET_CALL_WAITING = 3;
    private static final int MESSAGE_GET_CFB = 4;
    private static final int MESSAGE_SET_CFNRC = 5;
    private static final int MESSAGE_SET_CALL_WAITING = 6;
    private static final int MESSAGE_SET_CFB = 7;
    private static final int REVERT_SET_CFNRC = 8;
    private static final int REVERT_SET_CALL_WAITING = 9;
    private static final int START = 10;
    private static final int STOP = 11;
    private static final int INVALID_SLOT = 12;
    static final int RESPONSE_ERROR = 400; //Should be in sync with TimeConsumingPreferenceActivity

    public XDivertCheckBoxPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
    }

    public XDivertCheckBoxPreference(Context context, AttributeSet attrs) {
        this(context, attrs, com.android.internal.R.attr.checkBoxPreferenceStyle);
    }

    public XDivertCheckBoxPreference(Context context) {
        this(context, null);
    }

    void init(TimeConsumingPreferenceListener listener, boolean skipReading, String[] line1Number){
        mTcpListener = listener;

        mXDivertUtility = XDivertUtility.getInstance();
        TelephonyManager telephonyManager;
        telephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        mNumPhones = telephonyManager.getPhoneCount();
        // Store the numbers to shared preference
        for (int i = 0; i < mNumPhones; i++) {
            Log.d(LOG_TAG, "init slot" + i + " = " + line1Number[i]);
            mXDivertUtility.storeNumber(line1Number[i], i);
        }

        processStartDialog(START, true);
        if (!skipReading) {
            mPhoneObj = new Phone[mNumPhones];
            mLine1Number = new String[mNumPhones];
            mCFLine1Number = new String[mNumPhones];
            mSlotCallWaiting = new boolean[mNumPhones];
            mCFBLine1Number = new String[mNumPhones];
            for (int i = 0; i < mNumPhones; i++) {
                mPhoneObj[i] = PhoneFactory.getPhone(i);
                mLine1Number[i] = line1Number[i];
            }

            //Query for CFNRc for SLOT1.
            mPhoneObj[SLOT1].getCallForwardingOption(CommandsInterface.CF_REASON_NOT_REACHABLE,
                    mGetOptionComplete.obtainMessage(MESSAGE_GET_CFNRC, SLOT1, 0));
        }
    }

    @Override
    protected void onClick() {
        super.onClick();

        processStartDialog(START, false);
        Log.d(LOG_TAG,"onClick mXdivertStatus = " + mXdivertStatus);
        mSlotCallWaiting[SLOT1] = mXdivertStatus;
        mSlotCallWaiting[SLOT2] = mXdivertStatus;
        mAction = (mXdivertStatus ?
                CommandsInterface.CF_ACTION_DISABLE:
                CommandsInterface.CF_ACTION_REGISTRATION);
        mReason = CommandsInterface.CF_REASON_NOT_REACHABLE;
        int time = (mReason != CommandsInterface.CF_REASON_NO_REPLY) ? 0 : 20;

        //Check if CFNRc(SLOT1) & CW(SLOT1) is already set due to Partial Setting operation.
        //if set,then send the request for SLOT2, else for SLOT1.
        boolean requestForSlot1 = PhoneNumberUtils.compare(mCFLine1Number[SLOT1],
                mLine1Number[SLOT2]);
        if ((requestForSlot1) && (requestForSlot1 == mSlotCallWaiting[SLOT1])
                && (mAction == CommandsInterface.CF_ACTION_REGISTRATION)) {
            // Due to limitation in lower layers, back-to-back requests to SLOT1 & SLOT2
            // cannot be sent.For ex: CF request on SLOT1 followed by CF request on SLOT2.
            // Miminum of 3000ms delay is needed to send the 2nd request.
            // Hence added postDelayed to CF request for SLOT2.
            mGetOptionComplete.postDelayed(new Runnable() {
                public void run() {
                    //Set CFNRc for SLOT2.
                    mPhoneObj[SLOT2].setCallForwardingOption(mAction,
                            mReason,
                            mLine1Number[SLOT1],
                            time,
                            mSetOptionComplete.obtainMessage(MESSAGE_SET_CFNRC, SLOT2, 0));
                }
            }, 3000);
        } else {
            // Due to limitation in lower layers, back-to-back requests to SLOT1 & SLOT2
            // cannot be sent.For ex: CF request on SLOT2 followed by CF request on SLOT1.
            // Miminum of 3000ms delay is needed to send the 2nd request.
            // Hence added postDelayed to CF request for SLOT1.
            //Set CFNRc for SLOT1.
            mGetOptionComplete.postDelayed(new Runnable() {
                public void run() {
                    //Set CFNRc for SLOT2.
                    mPhoneObj[SLOT1].setCallForwardingOption(mAction,
                            mReason,
                            mLine1Number[SLOT2],
                            time,
                            mSetOptionComplete.obtainMessage(MESSAGE_SET_CFNRC, SLOT1, 0));
                }
            }, 3000);
        }
    }

    void queryCallWaiting(int arg) {
        //Get Call Waiting for "arg" slot
        mPhoneObj[arg].getCallWaiting(mGetOptionComplete.obtainMessage(MESSAGE_GET_CALL_WAITING,
                arg, MESSAGE_GET_CALL_WAITING));
    }

    void queryCallForwardingBusy(int arg) {
        //Get call Forwarding Busy status
        mPhoneObj[arg].getCallForwardingOption(CommandsInterface.CF_REASON_BUSY,
                mGetOptionComplete.obtainMessage(MESSAGE_GET_CFB, arg, 0));
    }

    private boolean validateXDivert() {
        // Compares if - SLot1 line number == CFNRc number of Slot2
        // Slot2 line number == CFNRc number of Slot1.
        boolean check1 = PhoneNumberUtils.compare(mCFLine1Number[SLOT1], mLine1Number[SLOT2]);
        boolean check2 = PhoneNumberUtils.compare(mCFLine1Number[SLOT2], mLine1Number[SLOT1]);
        boolean status = false;
        Log.d(LOG_TAG," CFNR SLOT1 = " + check1 + " CFNR SLOT2 = " + check2 + " mSlotCallWaiting = "
                + mSlotCallWaiting[SLOT1] + " mSlotCallWaiting = " + mSlotCallWaiting[SLOT2]);
        if ((mCFLine1Number[SLOT1] != null) && (mCFLine1Number[SLOT2] != null)) {
            if ((check1) && (check1 == check2)) {
                if (mSlotCallWaiting[SLOT1] && (mSlotCallWaiting[SLOT1] ==
                        mSlotCallWaiting[SLOT2])) {
                    status = true;
                }
            }
        }
        return status;
    }

    private boolean validateSmartCallForward(int slotId) {
        int nextSlotId = mXDivertUtility.getNextPhoneId(slotId);
        return PhoneNumberUtils.compare(mCFBLine1Number[slotId], mLine1Number[nextSlotId]);
    }

    public void displayAlertMessage(boolean status) {
        int slotStatus[] = {R.string.xdivert_not_active, R.string.xdivert_not_active};
        int resSlotId[] = {R.string.sub_1, R.string.sub_2};
        String dispMsg = this.getContext().getString(R.string.xdivert_status_active);
        if (!status) {
            dispMsg = this.getContext().getString(R.string.xdivert_status_not_active);
        }

        Log.d(LOG_TAG, "displayAlertMessage:  dispMsg = " + dispMsg);
        //check if activity is not in finishing state to diplay alert box
        if (!(((Activity)(this.getContext())).isFinishing())){
           new AlertDialog.Builder(this.getContext())
           .setTitle(R.string.xdivert_status)
           .setMessage(dispMsg)
           .setIcon(android.R.drawable.ic_dialog_alert)
           .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                      public void onClick(DialogInterface dialog, int whichButton) {
                           Log.d(LOG_TAG, "displayAlertMessage:  onClick");
                         }
                      })
           .show()
           .setOnDismissListener(new DialogInterface.OnDismissListener() {
                      public void onDismiss(DialogInterface dialog) {
                           Log.d(LOG_TAG, "displayAlertMessage:  onDismiss");
                        }
                     });
           }
    }

    private void processStopDialog(final int state, final boolean read) {
        if (mTcpListener != null) {
            Log.d(LOG_TAG,"stop");
            mTcpListener.onFinished(XDivertCheckBoxPreference.this, read);
        }
    }

    private void processStartDialog(final int state, final boolean read) {
        new Thread(new Runnable() {
            public void run() {
                Looper.prepare();
                int mode = state;
                if (mode == START) {
                    if (mTcpListener != null) {
                        Log.d(LOG_TAG,"start");
                        mTcpListener.onStarted(XDivertCheckBoxPreference.this, read);
                    }
                }
                Looper.loop();
            }
        }).start();
    }

    private final Handler mGetOptionComplete = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult result = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MESSAGE_GET_CFNRC:
                    handleGetCFNRCResponse(result, msg.arg1);
                    break;

                case MESSAGE_GET_CALL_WAITING:
                    handleGetCallWaitingResponse(result, msg.arg1, msg.arg2);
                    break;

                case MESSAGE_GET_CFB:
                    handleGetCFBResponse(result, msg.arg1);
                    break;
            }
        }
    };

    private final Handler mSetOptionComplete = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult result = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MESSAGE_SET_CFNRC:
                    handleSetCFNRCResponse(result, msg.arg1);
                    break;

                case MESSAGE_SET_CALL_WAITING:
                    handleSetCallWaitingResponse(result, msg.arg1);
                    break;

                case MESSAGE_SET_CFB:
                    handleSetCFBResponse(result, msg.arg1);
                    break;
            }
        }
    };

    /*Revert operations would be handled as follows:
    **case 1: CFNRc(SLOT1)->failure
    **        No Revert operation.
    **case 2: CFNRc(SLOT1)->success, CW(SLOT1)->failure
    **        Revert CFNRc(SLOT1).
    **case 3: CFNRc(SLOT1)->success, CW(SLOT1)->success,
    **        CFNRc(SLOT2)->failure
    **        No Revert operation. Display toast msg stating XDivert set only for Slot0.
    **case 4: CFNRc(SLOT1)->success, CW(SLOT1)->success,
    **        CFNRc(SLOT2)->success, CW(SLOT2)->failure
    **        Revert CFNRc(SLOT2) and display toast msg stating XDivert set only for Slot0.
    */
    private final Handler mRevertOptionComplete = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult result = (AsyncResult) msg.obj;
            switch (msg.what) {
                case REVERT_SET_CFNRC:
                    handleRevertSetCFNRC(result, msg.arg2);
                    break;
            }
        }
    };

    private void handleGetCFNRCResponse(AsyncResult ar, int arg) {
        if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: done arg = " + arg);
        if (ar.exception != null) {
            if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: ar.exception = " + ar.exception);
            if (mTcpListener != null) {
                if (ar.exception instanceof CommandException) {
                    mTcpListener.onException(XDivertCheckBoxPreference.this,
                            (CommandException) ar.exception);
                } else {
                    mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                }
            }
            processStopDialog(STOP, true);
        } else if (ar.userObj instanceof Throwable) {
                mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                processStopDialog(STOP, true);
        } else {
            final CallForwardInfo cfInfoArray[] = (CallForwardInfo[]) ar.result;
            if (cfInfoArray == null) {
                if (DBG) Log.d(LOG_TAG, "handleGetCFResponse: cfInfoArray.length==0");
                mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
            } else {
                for (int i = 0, length = cfInfoArray.length; i < length; i++) {
                    if (DBG) Log.d(LOG_TAG, "handleGetCFResponse, cfInfoArray[" + i + "]="
                            + cfInfoArray[i]);
                    if ((CommandsInterface.SERVICE_CLASS_VOICE &
                            cfInfoArray[i].serviceClass) != 0 && arg == SLOT1) {
                        CallForwardInfo info = cfInfoArray[i];
                        if (info.status == 1) {
                            mCFLine1Number[SLOT1] = info.number;
                        }

                        //Query Call Waiting for SLOT1
                        queryCallWaiting(SLOT1);
                    } else if ((CommandsInterface.SERVICE_CLASS_VOICE &
                             cfInfoArray[i].serviceClass) != 0 && arg == SLOT2) {
                        CallForwardInfo info = cfInfoArray[i];
                        if (info.status == 1) {
                            mCFLine1Number[SLOT2] = info.number;
                        }

                        //Query Call Waiting for SLOT2
                        queryCallWaiting(SLOT2);
                    }
                }
            }
        }
    }

    private void handleGetCFBResponse(AsyncResult ar, int arg) {
        if (DBG) Log.d(LOG_TAG, "handleGetCFBResponse: done arg = " + arg);
        if (ar.exception != null) {
            if (DBG) Log.d(LOG_TAG, "handleGetCFBResponse: ar.exception = " + ar.exception);
            if (mTcpListener != null) {
                if (ar.exception instanceof CommandException) {
                    mTcpListener.onException(XDivertCheckBoxPreference.this,
                            (CommandException) ar.exception);
                } else {
                    mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                }
            }
            processStopDialog(STOP, true);
        } else if (ar.userObj instanceof Throwable) {
                mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                processStopDialog(STOP, true);
        } else {
            final CallForwardInfo cfInfoArray[] = (CallForwardInfo[]) ar.result;
            if (cfInfoArray == null) {
                if (DBG) Log.d(LOG_TAG, "handleGetCFBResponse: cfInfoArray.length==0");
                mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
            } else {
                for (int i = 0, length = cfInfoArray.length; i < length; i++) {
                    if (DBG) Log.d(LOG_TAG, "handleGetCFBResponse, cfInfoArray[" + i + "]="
                            + cfInfoArray[i]);
                    if ((CommandsInterface.SERVICE_CLASS_VOICE &
                            cfInfoArray[i].serviceClass) != 0) {
                        CallForwardInfo info = cfInfoArray[i];
                        if (info.status == 1) {
                            mCFBLine1Number[arg] = info.number;
                        }
                    }
                }
                processStopDialog(STOP, true);

                //Check if CF numbers match the slot's phone numbers and
                //Call Waiting is enabled and
                //Call forwarding Busy is enabled then set the checkbox accordingly.
                mXdivertStatus = validateXDivert();
                mXdivertStatus &= validateSmartCallForward(arg);
                displayAlertMessage(mXdivertStatus);
                setChecked(mXdivertStatus);
                mXDivertUtility.onXDivertChanged(mXdivertStatus);
                mXDivertUtility.setXDivertStatus(mXdivertStatus);
            }
        }
    }

    private void handleSetCFBResponse(AsyncResult ar, int arg) {
        if (DBG) Log.d(LOG_TAG, "handleSetCFBResponse: done on Slot = " + arg);

        if (ar.exception != null) {
            if (DBG) Log.d(LOG_TAG, "handleSetCFBResponse: ar.exception = " + ar.exception);
            if (mTcpListener != null) {
                if (ar.exception instanceof CommandException) {
                    mTcpListener.onException(XDivertCheckBoxPreference.this,
                            (CommandException) ar.exception);
                } else {
                    mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                }
                mTcpListener.onFinished(XDivertCheckBoxPreference.this, false);
            }
        } else if (ar.userObj instanceof Throwable) {
            if (mTcpListener != null) {
                mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                mTcpListener.onFinished(XDivertCheckBoxPreference.this, false);
            }
        } else {
            int nextSlotId = mXDivertUtility.getNextPhoneId(arg);
            mCFBLine1Number[arg] = mLine1Number[nextSlotId];

            if (mTcpListener != null) {
                mTcpListener.onFinished(XDivertCheckBoxPreference.this, false);
            }

            //Check if CF numbers match the slot's phone numbers and
            //Call Waiting is enabled and
            //Call forwarding Busy is enabled then set the checkbox accordingly.
            mXdivertStatus = validateXDivert();
            displayAlertMessage(mXdivertStatus);
            setChecked(mXdivertStatus);
            mXDivertUtility.onXDivertChanged(mXdivertStatus);
            mXDivertUtility.setXDivertStatus(mXdivertStatus);
        }
    }

    private void handleSetCFNRCResponse(AsyncResult ar, int arg) {
        if (DBG) Log.d(LOG_TAG, "handleSetCFResponse: done on Slot = " + arg);

        if (ar.exception != null) {
            if (DBG) Log.d(LOG_TAG, "handleSetCFResponse: ar.exception = " + ar.exception);
            if (mTcpListener != null) {
                if (ar.exception instanceof CommandException) {
                    mTcpListener.onException(XDivertCheckBoxPreference.this,
                            (CommandException) ar.exception);
                } else {
                    mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                }
            }
            handleRevertOperation(arg, REVERT_SET_CFNRC);
        } else if (ar.userObj instanceof Throwable) {
            if (mTcpListener != null) mTcpListener.onError(XDivertCheckBoxPreference.this,
                    RESPONSE_ERROR);
            handleRevertOperation(arg, REVERT_SET_CFNRC);
        } else {
            if (arg == SLOT1) {
                mCFLine1Number[arg] = mLine1Number[SLOT2];
            } else {
                mCFLine1Number[arg] = mLine1Number[SLOT1];
            }

            //Set Call Waiting for the "arg" slot
            mPhoneObj[arg].setCallWaiting(true,
                   mSetOptionComplete.obtainMessage(MESSAGE_SET_CALL_WAITING, arg, 0));
        }
    }

    private void handleGetCallWaitingResponse(AsyncResult ar, int arg1, int arg2) {
        if (ar.exception != null) {
            Log.d(LOG_TAG, "handleGetCallWaitingResponse: ar.exception = " + ar.exception);
            if (mTcpListener != null) {
                if (ar.exception instanceof CommandException) {
                    mTcpListener.onException(XDivertCheckBoxPreference.this,
                            (CommandException) ar.exception);
                } else {
                    mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                }
            }
            processStopDialog(STOP, true);
        } else if (ar.userObj instanceof Throwable) {
            if (mTcpListener != null) mTcpListener.onError(XDivertCheckBoxPreference.this,
                    RESPONSE_ERROR);
            processStopDialog(STOP, true);
        } else {
            if (DBG) Log.d(LOG_TAG, "handleGetCallWaitingResponse: CW state successfully queried.");
            //If cwArray[0] is = 1, then cwArray[1] must follow,
            //with the TS 27.007 service class bit vector of services
            //for which call waiting is enabled.
            int[] cwArray = (int[])ar.result;
            if (arg1 == SLOT1) {
                mSlotCallWaiting[SLOT1] = ((cwArray[0] == 1) && ((cwArray[1] & 0x01) == 0x01));
                Log.d(LOG_TAG,"CW for Slot0 = " + mSlotCallWaiting[SLOT1]);
                // Due to limitation in lower layers, back-to-back requests to Slot1 & Slot2
                // cannot be sent.For ex: CF request on Slot1 followed by CF request on Slot2.
                // Miminum of 3000ms delay is needed to send the 2nd request.
                // Hence added postDelayed to CF request for SLOT2.
                mGetOptionComplete.postDelayed(new Runnable() {
                    public void run() {
                        //Query Call Forward for SLOT2
                        mPhoneObj[SLOT2].getCallForwardingOption(CommandsInterface
                                .CF_REASON_NOT_REACHABLE,mGetOptionComplete
                                .obtainMessage(MESSAGE_GET_CFNRC, SLOT2, 0));
                    }
                }, 3000);
            } else if (arg1 == SLOT2) {
                mSlotCallWaiting[SLOT2] = ((cwArray[0] == 1) && ((cwArray[1] & 0x01) == 0x01));
                Log.d(LOG_TAG,"CW for Slot1 = " + mSlotCallWaiting[SLOT2]);

                //validate smart call forward
                int imsSlotId = getImsSlotId();
                if (imsSlotId != INVALID_SLOT) {
                    queryCallForwardingBusy(imsSlotId);
                    return;
                }
                processStopDialog(STOP, true);

                //Check if CF numbers match the slot's phone numbers and
                //Call Waiting is enabled, then set the checkbox accordingly.
                mXdivertStatus = validateXDivert();
                displayAlertMessage(mXdivertStatus);
                setChecked(mXdivertStatus);
                mXDivertUtility.onXDivertChanged(mXdivertStatus);
                mXDivertUtility.setXDivertStatus(mXdivertStatus);
            }
        }
    }

    private int getImsSlotId() {
        if (!(mXDivertUtility.isCarrierOneSupported())) {
            return INVALID_SLOT;
        }

        int imsSlotId = INVALID_SLOT;
        Phone imsPhone;
        for (int i = 0; i < mNumPhones; i++) {
             imsPhone = mPhoneObj[i].getImsPhone();
             if ((imsPhone != null) &&
                     ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE) ||
                     imsPhone.isUtEnabled())) {
                 imsSlotId = i;
                 break;
             }
        }
        return imsSlotId;
    }

    private void handleSetCallWaitingResponse(AsyncResult ar, int arg) {
        if (ar.exception != null) {
            if (DBG) Log.d(LOG_TAG, "handleSetCallWaitingResponse: ar.exception = " + ar.exception);
            handleRevertOperation(arg, REVERT_SET_CALL_WAITING);
        } else {
            Log.d(LOG_TAG, "handleSetCallWaitingResponse success arg = " + arg);
            int time = (mReason != CommandsInterface.CF_REASON_NO_REPLY) ? 0 : 20;
            if (arg == SLOT1) {
                // Due to limitation in lower layers, back-to-back requests to Slot1 & Slot2
                // cannot be sent.For ex: CF request on Slot1 followed by CF request on Slot2.
                // Miminum of 3000ms delay is needed to send the 2nd request.
                // Hence added postDelayed to CF request for SLOT2.
                mSetOptionComplete.postDelayed(new Runnable() {
                    public void run() {
                        mSlotCallWaiting[SLOT1] = (!mSlotCallWaiting[SLOT1]);
                        //Set Call Forward for SLOT2
                        mPhoneObj[SLOT2].setCallForwardingOption(mAction,
                                mReason,
                                mLine1Number[SLOT1],
                                time,
                                mSetOptionComplete.obtainMessage(MESSAGE_SET_CFNRC, SLOT2, 0));
                    }
                }, 3000);
            } else if (arg == SLOT2) {
                mSlotCallWaiting[SLOT2] = !(mSlotCallWaiting[SLOT2]);

                //validate smart call forward
                int imsSlotId = getImsSlotId();
                if (imsSlotId != INVALID_SLOT) {
                    int nextSlotId = mXDivertUtility.getNextPhoneId(imsSlotId);
                    mPhoneObj[imsSlotId].setCallForwardingOption(mAction,
                            CommandsInterface.CF_REASON_BUSY, mLine1Number[nextSlotId], 0,
                            mSetOptionComplete.obtainMessage(MESSAGE_SET_CFB, imsSlotId, 0));
                    return;
                }

                if (mTcpListener != null) {
                    mTcpListener.onFinished(XDivertCheckBoxPreference.this, false);
                }

                //After successful operation of setting CFNRc & CW,
                //set the checkbox accordingly.
                mXdivertStatus = validateXDivert();
                displayAlertMessage(mXdivertStatus);
                setChecked(mXdivertStatus);
                mXDivertUtility.onXDivertChanged(mXdivertStatus);
                mXDivertUtility.setXDivertStatus(mXdivertStatus);
           }
       }
   }

    private void handleRevertOperation(int slot, int event) {
        Log.d(LOG_TAG,"handleRevertOperation slot = " + slot + "Event = " + event);
        setChecked(mXdivertStatus);
        if (slot == SLOT1) {
            switch (event) {
                case REVERT_SET_CFNRC:
                    mCFLine1Number[slot] = null;
                    if (mTcpListener != null) {
                        mTcpListener.onFinished(XDivertCheckBoxPreference.this, false);
                    }
                break;

                case REVERT_SET_CALL_WAITING:
                    revertCFNRC(SLOT1);
                break;
            }
        } else if (slot == SLOT2) {
            switch (event) {
                case REVERT_SET_CFNRC:
                    mCFLine1Number[slot] = null;
                    if (mTcpListener != null) {
                        mTcpListener.onFinished(XDivertCheckBoxPreference.this, false);
                    }

                    Toast toast = Toast.makeText(this.getContext(),
                            R.string.xdivert_partial_set,
                            Toast.LENGTH_LONG);
                            toast.show();
                break;

                case REVERT_SET_CALL_WAITING:
                    revertCFNRC(SLOT2);
                break;
            }
        }
    }

    private void revertCFNRC(int arg) {
        int action = (mXdivertStatus ?
                CommandsInterface.CF_ACTION_REGISTRATION:
                CommandsInterface.CF_ACTION_DISABLE);
        int reason = CommandsInterface.CF_REASON_NOT_REACHABLE;
        int time = (reason != CommandsInterface.CF_REASON_NO_REPLY) ? 0 : 20;

        Log.d(LOG_TAG,"revertCFNRc arg = " + arg);
        if (arg == SLOT1) {
            mPhoneObj[SLOT1].setCallForwardingOption(action,
                    reason,
                    mLine1Number[SLOT2],
                    time,
                    mRevertOptionComplete.obtainMessage(REVERT_SET_CFNRC,
                            action, SLOT1));
        } else if (arg == SLOT2) {
            mPhoneObj[SLOT2].setCallForwardingOption(action,
                    reason,
                    mLine1Number[SLOT1],
                    time,
                    mRevertOptionComplete.obtainMessage(REVERT_SET_CFNRC,
                            action, SLOT2));
        }
    }

    private void handleRevertSetCFNRC(AsyncResult ar, int arg) {
        if (DBG) Log.d(LOG_TAG, "handleRevertSetCFNRC: done arg = " + arg+ "res = " + ar);
        processStopDialog(STOP, false);

        if (ar.exception != null) {
            if (DBG) Log.d(LOG_TAG, "handleRevertSetCFNRC: ar.exception = " + ar.exception);
            if (mTcpListener != null) {
                if (ar.exception instanceof CommandException) {
                    mTcpListener.onException(XDivertCheckBoxPreference.this,
                            (CommandException) ar.exception);
                } else {
                    mTcpListener.onError(XDivertCheckBoxPreference.this, RESPONSE_ERROR);
                }
            }
        } else if (ar.userObj instanceof Throwable) {
            if (mTcpListener != null) mTcpListener.onError(XDivertCheckBoxPreference.this,
                    RESPONSE_ERROR);
        }

        Toast toast = Toast.makeText(this.getContext(),
                R.string.xdivert_partial_set,
                Toast.LENGTH_LONG);
                toast.show();
    }

}
