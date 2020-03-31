package com.example.sgnssid;

import android.content.Context;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.text.format.Formatter;
import android.util.Log;

import org.json.JSONObject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;


public class RxThread implements Runnable {
    private static RxThread rxThread = null;
    List<OnDataListener> listeners;

    DatagramSocket socket;
    int UDP_Tx_PORT;
    int UDP_Rx_PORT;
    int MAX_UDP_DATAGRAM_LEN;

    private RxThread() {
        UDP_Tx_PORT = 4951;
        UDP_Rx_PORT = 4953;
        MAX_UDP_DATAGRAM_LEN = 25*1024;
        listeners = new ArrayList<>();
    }

    public static RxThread getInstance() {
        if(null == rxThread) {
            rxThread = new RxThread();
            new Thread(rxThread).start();
        }
        return rxThread;
    }

    synchronized public void subscribe(OnDataListener listener) {
        for(Iterator<OnDataListener> it = listeners.iterator(); it.hasNext();) {
            if(it.next().getName().equals(listener.getName())) {
                it.remove();
                break;
            }
        }
        listeners.add(listener);
    }

    public void sendPktTo(final String strIP, final byte[] byData) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                InetAddress IPAddress;
                try {
                    DatagramSocket client_socket    = new DatagramSocket();
                    IPAddress = InetAddress.getByName(strIP);

                    int total = byData.length;
                    DatagramPacket pkt;
                    if(0 < total) {
                        pkt = new DatagramPacket(byData, 0, total, IPAddress, UDP_Tx_PORT);
                    	Log.i("SGN", "Sending " + byData + " to " + IPAddress + ":" + UDP_Tx_PORT);
                        client_socket.send(pkt);
                    }
                    client_socket.close();
                } catch(SocketException se) {
                    se.printStackTrace();
                    Log.i("SGN", "SocketException " + se);
                } catch (UnknownHostException uhe) {
                    uhe.printStackTrace();
                    Log.i("SGN", "UnknownHostException");
                } catch (IOException ioe) {
                    ioe.printStackTrace();
                    Log.i("SGN", "IOException");
                }
            }
        }).start();
    }

    String getReqPkt() {
        JSONObject jsonObject   = new JSONObject();
        try {
            jsonObject.put("command", "where_are_you");
        } catch (org.json.JSONException je) { je.printStackTrace();}

        return jsonObject.toString();
    }

    public String getBCastIP(boolean useIPv4) {
        try {
            List<NetworkInterface> interfaces = Collections.list(NetworkInterface.getNetworkInterfaces());
            for (NetworkInterface intf : interfaces) {
                List<InetAddress> addrs = Collections.list(intf.getInetAddresses());
                for (InetAddress addr : addrs) {
                    if (!addr.isLoopbackAddress()) {
                        String sAddr = addr.getHostAddress();
                        byte bcaddrs[] = addr.getAddress();
                        boolean isIPv4 = sAddr.indexOf(':')<0;

                        if (useIPv4) {
                            if (isIPv4) {
                                bcaddrs[3] = -1;
                                InetAddress ia = InetAddress.getByAddress(bcaddrs);
                                sAddr   = ia.getHostAddress();
                                return sAddr;
                            }
                        } else {
                            if (!isIPv4) {
                                int delim = sAddr.indexOf('%'); // drop ip6 zone suffix
                                return delim<0 ? sAddr.toUpperCase() : sAddr.substring(0, delim).toUpperCase();
                            }
                        }
                    }
                }
            }
        } catch (Exception ex) { } // for now eat exceptions
        return "";
    }

    public String getIPAddress(boolean useIPv4) {
        try {
            List<NetworkInterface> interfaces = Collections.list(NetworkInterface.getNetworkInterfaces());
            for (NetworkInterface intf : interfaces) {
                List<InetAddress> addrs = Collections.list(intf.getInetAddresses());
                for (InetAddress addr : addrs) {
                    if (!addr.isLoopbackAddress()) {
                        String sAddr = addr.getHostAddress();
                        //boolean isIPv4 = InetAddressUtils.isIPv4Address(sAddr);
                        boolean isIPv4 = sAddr.indexOf(':')<0;

                        if (useIPv4) {
                            if (isIPv4)
                                return sAddr;
                        } else {
                            if (!isIPv4) {
                                int delim = sAddr.indexOf('%'); // drop ip6 zone suffix
                                return delim<0 ? sAddr.toUpperCase() : sAddr.substring(0, delim).toUpperCase();
                            }
                        }
                    }
                }
            }
        } catch (Exception ex) { } // for now eat exceptions
        return "";
    }

    @Override
    public void run() {
        byte[] lmessage         = new byte[MAX_UDP_DATAGRAM_LEN];
        DatagramPacket packet   = new DatagramPacket(lmessage, lmessage.length);

        while (true) {
            try {
                socket = new DatagramSocket(UDP_Rx_PORT);
                socket.receive(packet);
                String sockPkt  = new String(packet.getData(), 0, packet.getLength());
                Log.i("SGN", "Received packet - " + sockPkt);
                for (OnDataListener listener : listeners) {
                    listener.onUDPPacket(sockPkt);
                }
                socket.close();
            } catch (SocketException e) {
                e.printStackTrace();
            } catch (IOException io) {
                io.printStackTrace();
            }
        }
    }
}
