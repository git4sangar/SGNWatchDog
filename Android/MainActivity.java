package com.example.sgnssid;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import static java.lang.Thread.sleep;

public class MainActivity extends AppCompatActivity {
    private TextView txtVwRPiIp;
    private EditText edtSSID, edtPSK;
    private Button btnPing, btnSet;
    private String strRPiIP = "", strBCastIP = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final RxThread rxThread = RxThread.getInstance();

        txtVwRPiIp      = findViewById(R.id.txtVwRPiIP);
        btnPing         = findViewById(R.id.btnBroadCast);
        btnSet          = findViewById(R.id.sgnBtnSet);
        edtSSID         = findViewById(R.id.edtTxtSSID);
        edtPSK          = findViewById(R.id.edtTxtPSK);

        strBCastIP      = rxThread.getBCastIP(true);

        rxThread.subscribe(new OnDataListener() {
            @Override
            public void onUDPPacket(String respPkt) {
                String strCmd;
                try {
                    JSONObject jObject = new JSONObject(respPkt );

                    strCmd  = jObject.getString("command");
                    if(strCmd.equals("where_are_you")) {
                        String strIP = jObject.getString("ip");
                        txtVwRPiIp.setText(strIP);
                        strRPiIP    = strIP;
                    }
                    if(strCmd.equals("wifi_details")) {
                        final boolean isSuccess = jObject.getBoolean("success");
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if(isSuccess) {
                                    renderToast("WiFi credentials set successfully", false);
                                } else {
                                    renderToast("Could not set WiFi credentials", false);
                                }
                            }
                        });
                    }
                } catch (org.json.JSONException je) {
                    Log.i("SGN", "Exception parsing response");
                }
            }
        });

        btnPing.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String strPkt  = rxThread.getReqPkt();
                Log.i("SGN", "Sending " + strPkt);
                rxThread.sendPktTo(strBCastIP, strPkt.getBytes());
            }
        });

        btnSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String strPkt = "";
                String strSSID  = edtSSID.getText().toString();
                if(strSSID.isEmpty()) {
                    renderToast("Please enter your WiFi Name", false);
                    return;
                }
                String strPSK   = edtPSK.getText().toString();
                if(strPSK.isEmpty()) {
                    renderToast("Please enter your WiFi Password", false);
                    return;
                }
                JSONObject jsonObject = new JSONObject();
                try {
                    jsonObject.put("command", "wifi_details");
                    jsonObject.put("ssid", strSSID);
                    jsonObject.put("psk", strPSK);
                    strPkt  = jsonObject.toString();
                } catch (org.json.JSONException je) { je.printStackTrace();}
                rxThread.sendPktTo(strRPiIP, strPkt.getBytes());
            }
        });
    }

    private void renderToast(String strMsg, boolean isLong) {
        Toast toast;
        toast = Toast.makeText(getApplicationContext(),strMsg, isLong ? Toast.LENGTH_LONG : Toast.LENGTH_SHORT);
        //toast.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL, 0, 0);
        toast.show();
    }
}
